/*
usbplay.c                Copyright frankl 2025-2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Player thread for USB Audio Class 2 devices with libusb, see usbplay.h.
The timing code is the same as in alsaplay.c (sleep, then active wait;
with -DC4TIMER on the 24 MHz system counter of the Odroid C4), and like
alsaplay.c this is compiled without optimization.

Timing: the USB bus takes one packet per (micro)frame, so a transfer of
ppt packets is played in exactly ppt/pktps seconds of the bus clock; the
number of frames per packet is given by the feedback of the device. The
player keeps about half of its transfers in flight and submits the next
one in the middle between the completion of a transfer and the time when
the bus needs the new one. If the loop is too early (or after an
underrun) the timing is resynchronized to the completion of a transfer.
If it is late (too few transfers in flight, e.g., because completions
were reported late) transfers are submitted without waiting until enough
are in flight again, and then the timing is resynchronized.

The completions are reported by the interrupt thread of the USB host
controller (e.g., irq/19-xhci-hcd), it must not be starved by threads
with higher priority on the same CPU.
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <libusb-1.0/libusb.h>
#include "cprefresh.h"
#include "usbplay.h"

#define BPF 8      /* bytes per frame: 2 channels S32_LE */
#define MAXTR 64   /* maximal number of data transfers */
#define NFB 4      /* number of feedback transfers */
#define NOSHIFT 100

struct slot {
  struct usbplay *p;
  int i;
};

struct usbplay {
  libusb_context *ctx;
  libusb_device_handle *h;
  int fd;
  int acif, asif, alt, clockid, selid;
  unsigned char epout, epfb;
  int maxpkt, fbpkt;           /* max bytes of data/feedback packet */
  int pktps, ppt, nt;          /* packets per second, packets per transfer,
                                  number of transfers */
  int rate;
  struct libusb_transfer *tr[MAXTR], *fbtr[NFB];
  struct slot slots[MAXTR];
  int mapped[MAXTR];           /* buffer from libusb_dev_mem_alloc */
  int done[MAXTR];
  long nsubmitted, ncompleted; /* data transfers, in order */
  int fbactive, fbshift, stopping, nodev;
  double fb, acc;              /* frames per packet from feedback */
  atomic_long fbrate;          /* frames per second * 1000 */
  long xerrs, pkterrs;
  long shift, nrcp;
  int warmup, cpu, prio, verbose;
  double nsec, ntick;          /* loop length in ns and in counter ticks */
  double off;                  /* submit off loops before the bus needs
                                  the transfer */
  long mtime, mtick;           /* time bases of the loop */
  char *tbuf;
  /* the ring, rpos and wpos count all bytes read and written so far */
  char *ring;
  long size, rp;
  atomic_long rpos, wpos;
  atomic_int eof;
  atomic_int full;             /* producer had to wait for space */
  atomic_long minfill;         /* smallest fill before a write, see
                                  usbplay_fill */
  long bps;                    /* bytes per second */
  long underruns;
  atomic_long resyncs;
  /* statistics since the last usbplay_stats call (counter ticks) */
  atomic_long inmin, inmax;    /* transfers in flight at the check */
  atomic_long latemax;         /* wake up after the submit time */
  atomic_long submax;          /* duration of submit */
  atomic_long handlemax;       /* duration of handle(p, 0) */
  atomic_long xerrsa, pkterrsa;
  pthread_t thread;
};

/* time in ns */
static long gettime()
{
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return time.tv_sec*1000000000+time.tv_nsec;
}

/* sleep to base + (long)(round(dns)) ns */
static void sleepto(long base, double dns)
{
  long t;
  struct timespec time;
  t = base + (long)(round(dns));
  time.tv_sec = t/1000000000;
  time.tv_nsec = t - time.tv_sec*1000000000;
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
}

/* sleep for t ns (relative) */
static void sleepns(long t)
{
  struct timespec time;
  time.tv_sec = t/1000000000;
  time.tv_nsec = t - time.tv_sec*1000000000;
  nanosleep(&time, NULL);
}

#ifdef C4TIMER
#ifdef __aarch64__
/* timing on the Odroid C4: ticks of the 24 MHz system counter, the
   active wait after the sleep is a loop on this counter */
#define TICKSPERNS 0.024

static void timer_init()
{
}

/* read the 24 MHz system counter */
static inline long getticks()
{
  long t;
  __asm__ __volatile__("isb\nmrs %0, cntvct_el0" : "=r"(t));
  return t;
}

/* active loop until the 24 MHz counter reaches etick, see the comments
   in myplayhrt_c4.c */
static inline void spinto(long etick)
{
  __asm__ __volatile__ (
         "mov x8, #0 \n\t"
  "1: \n\t"
         "isb  \n\t"
         "mrs x7, cntvct_el0 \n\t"
         "cmp x7, %[et] \n\t"
         "blt 1b \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "isb  \n\t"
         "mrs x8, cntvct_el0 \n\t"
         "cmp x7, x8 \n\t"
         "blt 2f \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
         "add x8, x8, 1 \n\t"
  "2: \n\t"
  :
  : [et] "r" (etick)
  : "x7", "x8" );
}
#else
#error "C4TIMER needs an aarch64 system counter (Odroid C4)"
#endif
#else
/* generic timing: ticks are nanoseconds of CLOCK_MONOTONIC, the active
   wait after the sleep is a calibrated loop */
#define TICKSPERNS 1.0

/* after calibration nsloop takes about cnt ns */
static double nsloopfactor = 1.0;

static long nsloop(long cnt)
{
  long i, j;
  for (j = 1, i = (long)(cnt*nsloopfactor); i > 0; i--) j = j+i;
  return j;
}

/* calibrate nsloop, call it on the CPU where the loop is used */
static void timer_init()
{
  long t0, t1;
  nsloop(1000000);
  t0 = gettime();
  nsloop(10000000);
  t1 = gettime();
  nsloopfactor = 1.0*10000000/(t1-t0-50);
}

static inline long getticks()
{
  return gettime();
}

/* active loop until CLOCK_MONOTONIC reaches etick ns */
static inline void spinto(long etick)
{
  long t = gettime();
  if (etick > t)
    nsloop(etick - t);
}
#endif

/* copy n bytes from ring position pos to dest (with cleaning) */
static void fromring(usbplay *p, char *dest, long pos, long n)
{
  long off = pos % p->size, k;

  k = p->size - off;
  if (k > n)
    k = n;
  memclean(dest, n);
  cprefresh(dest, p->ring + off, k);
  if (k < n)
    cprefresh(dest + k, p->ring, n - k);
}

/* USB callbacks, they are called within libusb_handle_events* in the
   player thread */
/* data transfer completed: count errors and mark its slot as free */
static void datacb(struct libusb_transfer *t)
{
  struct slot *s = t->user_data;
  usbplay *p = s->p;
  int k;

  if (t->status == LIBUSB_TRANSFER_NO_DEVICE)
    p->nodev = 1;
  else if (t->status != LIBUSB_TRANSFER_COMPLETED)
    p->xerrs++;
  else
    for (k = 0; k < t->num_iso_packets; k++)
      if (t->iso_packet_desc[k].status != LIBUSB_TRANSFER_COMPLETED)
        p->pkterrs++;
  p->done[s->i] = 1;
  p->ncompleted++;
}

/* the device sends the number of frames per packet as fixed point number,
   for high speed in 16.16 format; as snd-usb-audio we accept other shifts
   (some devices send 10.14 or frames per ms) if the value is within
   12.5% of the nominal value */
static void newfeedback(usbplay *p, unsigned long v)
{
  double f, nom;
  int s;

  nom = (double)p->rate/p->pktps;
  f = v/65536.0;
  if (p->fbshift == NOSHIFT) {
    for (s = -4; s <= 4; s++)
      if (fabs(ldexp(f, s)/nom - 1.0) < 0.125) {
        p->fbshift = s;
        if (p->verbose)
          fprintf(stderr, "usbplay: feedback 0x%lx, using shift %d.\n", v, s);
        break;
      }
  }
  if (p->fbshift == NOSHIFT)
    return;
  f = ldexp(f, p->fbshift);
  if (fabs(f/nom - 1.0) < 0.125) {
    p->fb = f;
    atomic_store_explicit(&p->fbrate, (long)(f*p->pktps*1000.0),
                          memory_order_relaxed);
  }
}

/* feedback transfer completed: take the new value and resubmit the
   transfer (unless we are stopping) */
static void fbcb(struct libusb_transfer *t)
{
  usbplay *p = t->user_data;
  unsigned char *b = t->buffer;
  int len = t->iso_packet_desc[0].actual_length;

  if (t->status == LIBUSB_TRANSFER_NO_DEVICE)
    p->nodev = 1;
  if (t->status == LIBUSB_TRANSFER_COMPLETED &&
      t->iso_packet_desc[0].status == LIBUSB_TRANSFER_COMPLETED && len >= 3)
    newfeedback(p, b[0] | (b[1] << 8) | (b[2] << 16) |
                   (len >= 4 ? (unsigned long)b[3] << 24 : 0));
  if (!p->stopping && !p->nodev && libusb_submit_transfer(t) == 0)
    return;
  p->fbactive--;
}

/* handle pending USB events (completions, callbacks), wait at most usec
   microseconds; exits if the device has gone */
static void handle(usbplay *p, long usec)
{
  struct timeval tv;

  tv.tv_sec = usec/1000000;
  tv.tv_usec = usec - tv.tv_sec*1000000;
  libusb_handle_events_timeout_completed(p->ctx, &tv, NULL);
  if (p->nodev) {
    fprintf(stderr, "usbplay: USB device disappeared.\n");
    exit(50);
  }
}

/* for statistics: update maximum (setmax) or minimum (setmin), only the
   player thread writes, usbplay_stats resets */
static void setmax(atomic_long *a, long v)
{
  if (v > atomic_load_explicit(a, memory_order_relaxed))
    atomic_store_explicit(a, v, memory_order_relaxed);
}

static void setmin(atomic_long *a, long v)
{
  if (v < atomic_load_explicit(a, memory_order_relaxed))
    atomic_store_explicit(a, v, memory_order_relaxed);
}

/* submit transfer number t (in slot t % nt), exits on error */
static void submit(usbplay *p, long t)
{
  int r, s = t % p->nt;

  p->done[s] = 0;
  r = libusb_submit_transfer(p->tr[s]);
  if (r != 0) {
    fprintf(stderr, "usbplay: cannot submit transfer: %s.\n",
            libusb_error_name(r));
    exit(51);
  }
  p->nsubmitted++;
}

/* set the packet lengths of transfer from the feedback, returns the
   number of bytes */
static long setpackets(usbplay *p, struct libusb_transfer *tr)
{
  long k, n, len, maxf = p->maxpkt/BPF;

  for (len = 0, k = 0; k < p->ppt; k++) {
    p->acc += p->fb;
    n = (long)p->acc;
    if (n > maxf) {
      n = maxf;
      p->acc = 0.0;
    } else
      p->acc -= n;
    tr->iso_packet_desc[k].length = n*BPF;
    len += n*BPF;
  }
  return len;
}

/* fill transfer number t from the ring, returns 1 at end of input; sets
   *waited if the player had to wait for input */
static int filltransfer(usbplay *p, long t, int *waited)
{
  struct libusb_transfer *tr = p->tr[t % p->nt];
  char *buf = (char*)tr->buffer;
  long ilen, avail;
  int last = 0;

  ilen = setpackets(p, tr);
  while (1) {
    avail = atomic_load_explicit(&p->wpos, memory_order_acquire) - p->rp;
    if (avail >= ilen) {
      fromring(p, buf, p->rp, ilen);
      break;
    }
    if (atomic_load_explicit(&p->eof, memory_order_acquire)) {
      /* end of input: complete the last transfer with silence */
      avail = atomic_load_explicit(&p->wpos, memory_order_acquire) - p->rp;
      fromring(p, buf, p->rp, avail);
      memset(buf + avail, 0, ilen - avail);
      last = 1;
      break;
    }
    /* the convolver was too slow, we lose the timing here */
    p->underruns++;
    *waited = 1;
    while (atomic_load_explicit(&p->wpos, memory_order_acquire) - p->rp < ilen
           && !atomic_load_explicit(&p->eof, memory_order_acquire))
      sleepns(100000);
  }
  p->rp += ilen;
  atomic_store_explicit(&p->rpos, p->rp, memory_order_release);
  return last;
}

/* set the time bases such that transfer t is submitted off loops before
   the bus needs it: wait for the completion of the oldest transfer in
   flight and take its completion time as reference */
static void resync(usbplay *p, long t)
{
  long j = p->ncompleted, now, nowtick;

  if (p->nsubmitted > j) {
    while (p->ncompleted <= j)
      handle(p, 100000);
    now = gettime();
    nowtick = getticks();
    /* transfer j+1 starts on the bus now */
    p->mtime = now - (long)(round((j+1+p->off)*p->nsec)) - p->shift;
    p->mtick = nowtick - (long)(round((j+1+p->off)*p->ntick));
  } else {
    /* nothing in flight, transfer t will start on the bus now */
    now = gettime();
    nowtick = getticks();
    p->mtime = now - (long)(round((t+p->off)*p->nsec)) - p->shift;
    p->mtick = nowtick - (long)(round((t+p->off)*p->ntick));
  }
  atomic_fetch_add(&p->resyncs, 1);
}

/* the player thread: prefill and submit all transfers, then in each loop
   refill the next free transfer from the ring and submit it at the
   precise time; at the end wait until all is played and stop the
   feedback transfers */
static void *playloop(void *arg)
{
  usbplay *p = arg;
  long t, k, ilen, expfl, infl, t0, etick;
  int i, last, waited, synced, catchup;
  char *buf;

  if (p->cpu >= 0) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(p->cpu, &set);
    if (pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0)
      fprintf(stderr, "usbplay: cannot set affinity to CPU %d.\n", p->cpu);
  }
  if (p->prio > 0) {
    struct sched_param sp = { .sched_priority = p->prio };
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp) != 0)
      fprintf(stderr, "usbplay: cannot set SCHED_FIFO priority %d.\n", p->prio);
  }
  /* avoid waiting 50000 ns collecting more sleep requests */
  prctl(PR_SET_TIMERSLACK, 1L);
  timer_init();

  /* wait until the ring is full or the input complete */
  while (!atomic_load_explicit(&p->full, memory_order_acquire) &&
         !atomic_load_explicit(&p->eof, memory_order_acquire))
    sleepns(1000000);

  /* start reading the feedback */
  for (i = 0; i < NFB; i++) {
    if (libusb_submit_transfer(p->fbtr[i]) != 0) {
      fprintf(stderr, "usbplay: cannot submit feedback transfer.\n");
      exit(52);
    }
    p->fbactive++;
  }
  /* all transfers are submitted at once, the bus starts with them */
  last = 0;
  waited = 0;
  for (t = 0; t < p->nt && !last; t++) {
    last = filltransfer(p, t, &waited);
    submit(p, t);
  }
  /* expected number of transfers in flight directly after a submit */
  expfl = (p->nt+1)/2;
  synced = 0;
  catchup = 0;
  if (!last) {
    resync(p, t);
    synced = 1;
  }
  while (!last) {
    t0 = getticks();
    handle(p, 0);
    setmax(&p->handlemax, getticks() - t0);
    infl = p->nsubmitted - p->ncompleted;
    setmin(&p->inmin, infl);
    setmax(&p->inmax, infl);
    if (!synced) {
      if (!p->done[t % p->nt] || infl > expfl + 1) {
        /* we are too early */
        resync(p, t);
        catchup = 0;
      } else if (infl < expfl - 1) {
        /* we are late (e.g., completions were reported late): submit
           without waiting until enough transfers are in flight */
        if (!catchup)
          atomic_fetch_add(&p->resyncs, 1);
        catchup = 1;
      } else if (catchup) {
        /* caught up, now take the timing from the bus again */
        resync(p, t);
        catchup = 0;
      }
    }
    synced = 0;
    waited = 0;
    last = filltransfer(p, t, &waited);
    if (waited) {
      resync(p, t);
      synced = 1;
      catchup = 0;
    }
    buf = (char*)p->tr[t % p->nt]->buffer;
    for (ilen = 0, i = 0; i < p->ppt; i++)
      ilen += p->tr[t % p->nt]->iso_packet_desc[i].length;
    for (k = p->nrcp; k; k--) {
      memclean(p->tbuf, ilen);
      cprefresh(p->tbuf, buf, ilen);
      memclean(buf, ilen);
      cprefresh(buf, p->tbuf, ilen);
    }
    if (catchup) {
      submit(p, t);
    } else {
      sleepto(p->mtime, t*p->nsec);
      /* a first system call after sleep is slower, take it now */
      if (p->warmup)
        syscall(SYS_getppid);
      etick = p->mtick + (long)(round(t*p->ntick));
      setmax(&p->latemax, getticks() - etick);
      spinto(etick);
      submit(p, t);
      setmax(&p->submax, getticks() - etick);
    }
    atomic_store_explicit(&p->xerrsa, p->xerrs, memory_order_relaxed);
    atomic_store_explicit(&p->pkterrsa, p->pkterrs, memory_order_relaxed);
    t++;
  }
  /* wait until all is played */
  while (p->nsubmitted > p->ncompleted)
    handle(p, 100000);
  p->stopping = 1;
  for (i = 0; i < NFB; i++)
    libusb_cancel_transfer(p->fbtr[i]);
  while (p->fbactive > 0)
    handle(p, 100000);
  return NULL;
}

/* read a number from file name in directory dir of sysfs */
static long sysval(const char *dir, const char *name, int base)
{
  char fn[512], s[64];
  FILE *f;
  long v = -1;

  snprintf(fn, 512, "/sys/bus/usb/devices/%s/%s", dir, name);
  f = fopen(fn, "r");
  if (f) {
    if (fgets(s, 64, f))
      v = strtol(s, NULL, base);
    fclose(f);
  }
  return v;
}

/* find the device in sysfs and open its usbfs file, returns the file
   descriptor and the speed in Mbit/s */
static int opendevice(long vid, long pid, long *speed)
{
  DIR *d;
  struct dirent *e;
  char fn[64];
  int fd = -1;

  d = opendir("/sys/bus/usb/devices");
  if (!d)
    return -1;
  while ((e = readdir(d))) {
    if (e->d_name[0] == '.' || strchr(e->d_name, ':'))
      continue;
    if (sysval(e->d_name, "idVendor", 16) == vid &&
        sysval(e->d_name, "idProduct", 16) == pid) {
      snprintf(fn, 64, "/dev/bus/usb/%03ld/%03ld",
               sysval(e->d_name, "busnum", 10),
               sysval(e->d_name, "devnum", 10));
      *speed = sysval(e->d_name, "speed", 10);
      fd = open(fn, O_RDWR);
      if (fd < 0)
        fprintf(stderr, "usbplay: cannot open %s (permissions?).\n", fn);
      break;
    }
  }
  closedir(d);
  return fd;
}

/* find the audio control interface with its clock source and selector,
   and the streaming interface and alternate setting with 2 channels of
   32-bit PCM and explicit feedback */
static void parsedescriptors(usbplay *p, int hs)
{
  struct libusb_config_descriptor *cfg;
  const struct libusb_interface_descriptor *d;
  const struct libusb_endpoint_descriptor *ep;
  const unsigned char *x;
  int i, a, e, ok, w, outint = 1;
  unsigned long fmts;

  if (libusb_get_active_config_descriptor(libusb_get_device(p->h), &cfg)) {
    fprintf(stderr, "usbplay: cannot read configuration descriptor.\n");
    exit(53);
  }
  p->acif = p->asif = p->alt = p->clockid = p->selid = -1;
  for (i = 0; i < cfg->bNumInterfaces; i++)
    for (a = 0; a < cfg->interface[i].num_altsetting; a++) {
      d = cfg->interface[i].altsetting + a;
      if (d->bInterfaceClass != LIBUSB_CLASS_AUDIO)
        continue;
      if (d->bInterfaceSubClass == 1 && p->acif < 0) {
        p->acif = d->bInterfaceNumber;
        for (x = d->extra; x + 3 < d->extra + d->extra_length && x[0] > 0;
             x += x[0])
          if (x[1] == 0x24 && x[2] == 0x0a && p->clockid < 0)
            p->clockid = x[3];
          else if (x[1] == 0x24 && x[2] == 0x0b && p->selid < 0)
            p->selid = x[3];
      }
      if (d->bInterfaceSubClass == 2 && d->bNumEndpoints > 0 && p->alt < 0) {
        ok = 0;
        for (x = d->extra; x + 5 < d->extra + d->extra_length && x[0] > 0;
             x += x[0]) {
          if (x[1] == 0x24 && x[2] == 0x01 && x[0] >= 11) {
            fmts = x[6] | (x[7] << 8) | (x[8] << 16) |
                   ((unsigned long)x[9] << 24);
            if ((fmts & 1) && x[10] == 2)
              ok++;
          }
          if (x[1] == 0x24 && x[2] == 0x02 && x[3] == 1 && x[4] == 4 &&
              x[5] == 32)
            ok++;
        }
        if (ok != 2)
          continue;
        p->epout = p->epfb = 0;
        for (e = 0; e < d->bNumEndpoints; e++) {
          ep = d->endpoint + e;
          if ((ep->bmAttributes & 3) != LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
            continue;
          w = ep->wMaxPacketSize;
          if (!(ep->bEndpointAddress & 0x80) &&
              ((ep->bmAttributes >> 4) & 3) == 0) {
            p->epout = ep->bEndpointAddress;
            p->maxpkt = (w & 0x7ff) * (((w >> 11) & 3) + 1);
            outint = ep->bInterval;
          } else if ((ep->bEndpointAddress & 0x80) &&
                     ((ep->bmAttributes >> 4) & 3) == 1) {
            p->epfb = ep->bEndpointAddress;
            p->fbpkt = w & 0x7ff;
          }
        }
        if (p->epout && p->epfb) {
          p->asif = d->bInterfaceNumber;
          p->alt = d->bAlternateSetting;
        }
      }
    }
  libusb_free_config_descriptor(cfg);
  if (p->acif < 0 || p->clockid < 0 || p->alt < 0) {
    fprintf(stderr, "usbplay: no USB Audio Class 2 interface with "
            "32-bit stereo PCM and feedback endpoint found.\n");
    exit(54);
  }
  p->pktps = hs ? 8000 >> (outint > 0 ? outint-1 : 0) : 1000;
}

/* class specific request to the audio control interface */
static int acrequest(usbplay *p, int in, int cs, int id, unsigned char *b,
                     int len)
{
  return libusb_control_transfer(p->h,
           (in ? LIBUSB_ENDPOINT_IN : LIBUSB_ENDPOINT_OUT) |
           LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
           0x01 /* CUR */, cs << 8, (id << 8) | p->acif, b, len, 1000);
}

/* the following functions are described in usbplay.h */

/* open and configure the device, allocate transfers and ring */
usbplay *usbplay_new(const char *device, int rate, long loopspersec,
                     int ntransfers, long shift, int warmup, int nrcp,
                     int zerocopy, long ringsize, int cpu, int prio,
                     int verbose)
{
  usbplay *p;
  long vid, pid, speed = -1, size, r;
  unsigned char b[4];
  int i, nmapped;
  char *buf;

  p = calloc(1, sizeof(usbplay));
  if (!p) {
    fprintf(stderr, "usbplay: out of memory.\n");
    exit(31);
  }
  if (sscanf(device, "%lx:%lx", &vid, &pid) != 2) {
    fprintf(stderr, "usbplay: give device as vid:pid, e.g. 152a:85dd.\n");
    exit(30);
  }
  p->rate = rate;
  p->shift = shift;
  p->warmup = warmup;
  p->nrcp = nrcp;
  p->cpu = cpu;
  p->prio = prio;
  p->verbose = verbose;
  p->bps = (long)rate * BPF;
  p->fbshift = NOSHIFT;

  /* we open the device ourselves, then libusb needs no device discovery
     and starts no thread of its own */
  p->fd = opendevice(vid, pid, &speed);
  if (p->fd < 0) {
    fprintf(stderr, "usbplay: cannot open USB device %s.\n", device);
    exit(32);
  }
  libusb_set_option(NULL, LIBUSB_OPTION_NO_DEVICE_DISCOVERY);
  if (libusb_init(&p->ctx) != 0 ||
      libusb_wrap_sys_device(p->ctx, (intptr_t)p->fd, &p->h) != 0) {
    fprintf(stderr, "usbplay: cannot initialize libusb for %s.\n", device);
    exit(33);
  }
  parsedescriptors(p, speed >= 480);
  if (p->pktps % loopspersec != 0) {
    fprintf(stderr, "usbplay: number of transfers per second must divide "
            "%d.\n", p->pktps);
    exit(34);
  }
  p->ppt = p->pktps / loopspersec;
  if (ntransfers < 3 || ntransfers > MAXTR) {
    fprintf(stderr, "usbplay: number of transfers must be in 3..%d.\n",
            MAXTR);
    exit(34);
  }
  p->nt = ntransfers;
  p->off = (p->nt % 2) ? (p->nt-2)/2.0 : (p->nt-3)/2.0;
  p->fb = (double)rate/p->pktps;
  if (p->fb * BPF > p->maxpkt) {
    fprintf(stderr, "usbplay: rate %d too high for this device.\n", rate);
    exit(34);
  }
  atomic_init(&p->fbrate, (long)rate*1000);
  p->nsec = 1000000000.0/loopspersec;
  p->ntick = p->nsec*TICKSPERNS;

  /* detach snd-usb-audio, it is reattached when we release the
     interfaces */
  libusb_set_auto_detach_kernel_driver(p->h, 1);
  if (libusb_claim_interface(p->h, p->acif) != 0 ||
      libusb_claim_interface(p->h, p->asif) != 0) {
    fprintf(stderr, "usbplay: cannot claim audio interfaces (in use?).\n");
    exit(35);
  }
  libusb_set_interface_alt_setting(p->h, p->asif, 0);
  /* select the (first) clock source and set the sample rate */
  if (p->selid >= 0) {
    b[0] = 1;
    acrequest(p, 0, 1, p->selid, b, 1);
  }
  b[0] = rate & 0xff;
  b[1] = (rate >> 8) & 0xff;
  b[2] = (rate >> 16) & 0xff;
  b[3] = (rate >> 24) & 0xff;
  if (acrequest(p, 0, 1, p->clockid, b, 4) < 0) {
    fprintf(stderr, "usbplay: cannot set sample rate %d.\n", rate);
    exit(36);
  }
  memset(b, 0, 4);
  if (acrequest(p, 1, 1, p->clockid, b, 4) == 4) {
    r = b[0] | (b[1] << 8) | (b[2] << 16) | ((long)b[3] << 24);
    if (r != rate) {
      fprintf(stderr, "usbplay: device reports rate %ld instead of %d.\n",
              r, rate);
      exit(36);
    }
  }
  if (libusb_set_interface_alt_setting(p->h, p->asif, p->alt) != 0) {
    fprintf(stderr, "usbplay: cannot set alternate setting %d.\n", p->alt);
    exit(37);
  }

  /* transfers */
  size = p->ppt * p->maxpkt;
  nmapped = 0;
  for (i = 0; i < p->nt; i++) {
    p->tr[i] = libusb_alloc_transfer(p->ppt);
    buf = zerocopy ? (char*)libusb_dev_mem_alloc(p->h, size) : NULL;
    if (buf) {
      p->mapped[i] = 1;
      nmapped++;
    } else if (posix_memalign((void**)&buf, 4096, size)) {
      fprintf(stderr, "usbplay: cannot allocate transfer buffers.\n");
      exit(31);
    }
    memset(buf, 0, size);
    p->slots[i].p = p;
    p->slots[i].i = i;
    libusb_fill_iso_transfer(p->tr[i], p->h, p->epout, (unsigned char*)buf,
                             size, p->ppt, datacb, p->slots + i, 1000);
    p->done[i] = 1;
  }
  if (posix_memalign((void**)&p->tbuf, 4096, size)) {
    fprintf(stderr, "usbplay: cannot allocate buffer.\n");
    exit(31);
  }
  for (i = 0; i < NFB; i++) {
    p->fbtr[i] = libusb_alloc_transfer(1);
    buf = calloc(1, p->fbpkt > 4 ? p->fbpkt : 4);
    libusb_fill_iso_transfer(p->fbtr[i], p->h, p->epfb, (unsigned char*)buf,
                             p->fbpkt, 1, fbcb, p, 1000);
    libusb_set_iso_packet_lengths(p->fbtr[i], p->fbpkt);
  }
  if (verbose)
    fprintf(stderr, "usbplay: %04lx:%04lx, %ld Mbit/s, interfaces %d/%d, "
            "alt %d, clock %d, ep 0x%02x/0x%02x, max packet %d bytes, %d "
            "packets per s, %d per transfer, %d transfers (%d zero copy), "
            "submit %.1f loops ahead.\n", vid, pid, speed, p->acif, p->asif,
            p->alt, p->clockid, p->epout, p->epfb, p->maxpkt, p->pktps,
            p->ppt, p->nt, nmapped, p->off);

  /* ring, at least for all transfers twice */
  p->size = ringsize - ringsize % BPF;
  if (p->size < 2 * p->nt * size)
    p->size = 2 * p->nt * size;
  if (posix_memalign((void**)&p->ring, 4096, p->size)) {
    fprintf(stderr, "usbplay: cannot allocate ring of %ld bytes.\n", p->size);
    exit(31);
  }
  memset(p->ring, 0, p->size);
  atomic_init(&p->rpos, 0);
  atomic_init(&p->wpos, 0);
  atomic_init(&p->eof, 0);
  atomic_init(&p->full, 0);
  atomic_init(&p->minfill, -1);
  atomic_init(&p->resyncs, 0);
  atomic_init(&p->inmin, 1000);
  atomic_init(&p->inmax, -1);
  atomic_init(&p->latemax, -1000000000);
  atomic_init(&p->submax, 0);
  atomic_init(&p->handlemax, 0);
  atomic_init(&p->xerrsa, 0);
  atomic_init(&p->pkterrsa, 0);
  return p;
}

/* start the player thread */
void usbplay_start(usbplay *p)
{
  if (pthread_create(&p->thread, NULL, playloop, p) != 0) {
    fprintf(stderr, "usbplay: cannot create player thread.\n");
    exit(42);
  }
}

/* producer: copy data into the ring, sleep while it is full */
void usbplay_write(usbplay *p, const char *data, long len)
{
  long n, free, wp, off, k, fill, mf;

  wp = atomic_load_explicit(&p->wpos, memory_order_relaxed);
  /* fill level before the write is the low point of the ring */
  fill = wp - atomic_load_explicit(&p->rpos, memory_order_acquire);
  mf = atomic_load_explicit(&p->minfill, memory_order_relaxed);
  if (mf < 0 || fill < mf)
    atomic_store_explicit(&p->minfill, fill, memory_order_relaxed);
  while (len > 0) {
    n = len < p->size/2 ? len : p->size/2;
    /* sleep until n bytes are free */
    while ((free = p->size - (wp -
              atomic_load_explicit(&p->rpos, memory_order_acquire))) < n) {
      atomic_store_explicit(&p->full, 1, memory_order_release);
      sleepns((n - free)*1000000000/p->bps + 100000);
    }
    off = wp % p->size;
    k = p->size - off;
    if (k > n)
      k = n;
    memclean(p->ring + off, k);
    cprefresh(p->ring + off, (char*)data, k);
    if (k < n) {
      memclean(p->ring, n - k);
      cprefresh(p->ring, (char*)data + k, n - k);
    }
    wp += n;
    atomic_store_explicit(&p->wpos, wp, memory_order_release);
    data += n;
    len -= n;
  }
}

/* statistics for the verbose output of the main program */
void usbplay_fill(usbplay *p, double *ms, double *minms, long *waits,
                  long *resyncs, double *fbrate)
{
  long mf;

  *ms = (atomic_load_explicit(&p->wpos, memory_order_relaxed) -
         atomic_load_explicit(&p->rpos, memory_order_relaxed))
        * 1000.0 / p->bps;
  mf = atomic_exchange(&p->minfill, -1);
  *minms = mf < 0 ? -1.0 : mf * 1000.0 / p->bps;
  *waits = p->underruns;
  *resyncs = atomic_load(&p->resyncs);
  *fbrate = atomic_load_explicit(&p->fbrate, memory_order_relaxed) / 1000.0;
}

/* mark end of input, wait for the player thread, release the device
   (snd-usb-audio is reattached) and free all resources */
void usbplay_finish(usbplay *p)
{
  int i;

  atomic_store_explicit(&p->eof, 1, memory_order_release);
  pthread_join(p->thread, NULL);
  libusb_set_interface_alt_setting(p->h, p->asif, 0);
  libusb_release_interface(p->h, p->asif);
  libusb_release_interface(p->h, p->acif);
  for (i = 0; i < p->nt; i++) {
    if (p->mapped[i])
      libusb_dev_mem_free(p->h, p->tr[i]->buffer, p->ppt * p->maxpkt);
    else
      free(p->tr[i]->buffer);
    libusb_free_transfer(p->tr[i]);
  }
  for (i = 0; i < NFB; i++) {
    free(p->fbtr[i]->buffer);
    libusb_free_transfer(p->fbtr[i]);
  }
  libusb_close(p->h);
  libusb_exit(p->ctx);
  close(p->fd);
  if (p->verbose || p->underruns || p->xerrs || p->pkterrs)
    fprintf(stderr, "usbplay: %ld bytes played, %ld times waited for input, "
            "%ld resyncs, %ld transfer errors, %ld packet errors.\n",
            (long)atomic_load(&p->rpos), p->underruns,
            (long)atomic_load(&p->resyncs), p->xerrs, p->pkterrs);
}

/* format the diagnostics since the last call and reset them */
void usbplay_stats(usbplay *p, char *buf, int len)
{
  /* ticks to microseconds */
  snprintf(buf, len, "in flight %ld..%ld (expected %d), max wake late %.1f us, "
           "max submit %.1f us, max event handling %.1f us, "
           "errors %ld transfers %ld packets",
           atomic_exchange(&p->inmin, 1000), atomic_exchange(&p->inmax, -1),
           (p->nt+1)/2,
           atomic_exchange(&p->latemax, -1000000000)/(TICKSPERNS*1000.0),
           atomic_exchange(&p->submax, 0)/(TICKSPERNS*1000.0),
           atomic_exchange(&p->handlemax, 0)/(TICKSPERNS*1000.0),
           atomic_load(&p->xerrsa), atomic_load(&p->pkterrsa));
}
