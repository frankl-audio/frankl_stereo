/*
alsaplay.c                Copyright frankl 2025-2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Player thread as in myplayhrt_c4 (only its code path for shared memory
input with --mmap and --stripped), see alsaplay.h. Like myplayhrt_c4 this
is compiled without optimization.

Timing: the thread sleeps with clock_nanosleep until shift ns before the
commit and then waits actively. By default this active wait is a
calibrated loop (nsloop); with -DC4TIMER (only on aarch64, intended for
the Odroid C4) it is a loop on the 24 MHz system counter as in
myplayhrt_c4.
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
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <alsa/asoundlib.h>
#include "cprefresh.h"
#include "alsaplay.h"

#define BPF 8      /* bytes per frame: 2 channels S32_LE */

struct alsaplay {
  snd_pcm_t *pcm;
  long hwbufsize, olen, shift, nrcp;
  int warmup, cpu, prio, verbose;
  double nsec, ntick;          /* loop length in ns and in counter ticks */
  /* the ring, rpos and wpos count all bytes read and written so far */
  char *ring;
  long size;
  atomic_long rpos, wpos;
  atomic_int eof;
  atomic_int full;             /* producer had to wait for space */
  atomic_long minfill;         /* smallest fill before a write, see
                                  alsaplay_fill */
  long bps;                    /* bytes per second */
  void *tbufs[1024];
  long underruns;
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
static void fromring(alsaplay *p, char *dest, long pos, long n)
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

static void *playloop(void *arg)
{
  alsaplay *p = arg;
  snd_pcm_t *pcm = p->pcm;
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset, frames;
  long count, startcount, tcount, mtime, mtick, etick, ilen, rp, avail;
  char *iptr, *tbuf;
  int k, last;

  if (p->cpu >= 0) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(p->cpu, &set);
    if (pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0)
      fprintf(stderr, "alsaplay: cannot set affinity to CPU %d.\n", p->cpu);
  }
  if (p->prio > 0) {
    struct sched_param sp = { .sched_priority = p->prio };
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp) != 0)
      fprintf(stderr, "alsaplay: cannot set SCHED_FIFO priority %d.\n", p->prio);
  }
  /* avoid waiting 50000 ns collecting more sleep requests */
  prctl(PR_SET_TIMERSLACK, 1L);
  timer_init();

  /* wait until the ring is full or the input complete */
  while (!atomic_load_explicit(&p->full, memory_order_acquire) &&
         !atomic_load_explicit(&p->eof, memory_order_acquire))
    sleepns(1000000);

  tbuf = p->tbufs[1];
  startcount = p->hwbufsize/(2*p->olen);
  count = 1;
  last = 0;
  rp = 0;
  /* set start time (- nsec) */
  mtime = gettime();
  /* 24 ticks per microsecond */
  mtick = getticks() + (long)(round(p->shift*TICKSPERNS));
  tcount = 0;
  while (!last) {
    /* start playing when half of hwbuffer is filled */
    if (count == startcount)
      snd_pcm_start(pcm);
    frames = p->olen;
    snd_pcm_avail(pcm);
    snd_pcm_mmap_begin(pcm, &areas, &offset, &frames);
    ilen = frames * BPF;
    iptr = (char*)areas[0].addr + offset * BPF;
    avail = atomic_load_explicit(&p->wpos, memory_order_acquire) - rp;
    if (avail < ilen) {
      if (atomic_load_explicit(&p->eof, memory_order_acquire)) {
        /* end of input: complete the last chunk with silence */
        avail = atomic_load_explicit(&p->wpos, memory_order_acquire) - rp;
        fromring(p, iptr, rp, avail);
        memset(iptr + avail, 0, ilen - avail);
        last = 1;
      } else {
        /* the convolver was too slow, we lose the timing here */
        p->underruns++;
        while (atomic_load_explicit(&p->wpos, memory_order_acquire) - rp < ilen
               && !atomic_load_explicit(&p->eof, memory_order_acquire))
          sleepns(100000);
        /* continue with timing relative to now */
        mtime = gettime() - (long)(round(tcount*p->nsec));
        mtick = getticks() + (long)(round(p->shift*TICKSPERNS))
                - (long)(round(tcount*p->ntick));
        continue;
      }
    } else {
      fromring(p, iptr, rp, ilen);
    }
    rp += ilen;
    atomic_store_explicit(&p->rpos, rp, memory_order_release);
    for (k = p->nrcp; k; k--) {
      memclean(tbuf, ilen);
      cprefresh(tbuf, iptr, ilen);
      memclean(iptr, ilen);
      cprefresh(iptr, tbuf, ilen);
    }
    tcount++;
    sleepto(mtime, tcount*p->nsec);
    /* a first system call after sleep is slower, take it now */
    if (p->warmup)
      syscall(SYS_getppid);
    /* etick is tick to wait for before commit  */
    etick = mtick + (long)(round(tcount*p->ntick));
    spinto(etick);
    snd_pcm_mmap_commit(pcm, offset, frames);
    count++;
  }
  /* blocking mode, such that drain waits until all is played */
  snd_pcm_nonblock(pcm, 0);
  snd_pcm_drain(pcm);
  snd_pcm_close(pcm);
  return NULL;
}

alsaplay *alsaplay_new(const char *device, int rate, long loopspersec,
                       long hwbufsize, double extrabps, int nonblock,
                       long shift, int warmup, int nrcp, long ringsize,
                       int cpu, int prio, int verbose)
{
  alsaplay *p;
  snd_pcm_t *pcm;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  snd_pcm_uframes_t hwb;
  double extraerr;
  int i;

  if (rate % loopspersec != 0) {
    fprintf(stderr, "alsaplay: sample rate %d is not a multiple of loops "
            "per second %ld.\n", rate, loopspersec);
    exit(30);
  }
  p = calloc(1, sizeof(alsaplay));
  if (!p) {
    fprintf(stderr, "alsaplay: out of memory.\n");
    exit(31);
  }
  p->olen = rate/loopspersec;
  p->shift = shift;
  p->warmup = warmup;
  p->nrcp = nrcp;
  p->cpu = cpu;
  p->prio = prio;
  p->verbose = verbose;
  p->bps = (long)rate * BPF;
  /* compute nanoseconds per loop (wrt local clock) */
  extraerr = 1.0*BPF*rate;
  extraerr = extraerr/(extraerr+extrabps);
  p->nsec = 1000000000*extraerr/loopspersec;
  p->ntick = p->nsec*TICKSPERNS;
  if (verbose)
    fprintf(stderr, "alsaplay: %ld frames per loop, step size is %lf nsec.\n",
            p->olen, p->nsec);
  /* ring, at least a few loops */
  p->size = ringsize - ringsize % BPF;
  if (p->size < 4*p->olen*BPF)
    p->size = 4*p->olen*BPF;
  if (posix_memalign((void**)&p->ring, 4096, p->size)) {
    fprintf(stderr, "alsaplay: cannot allocate ring of %ld bytes.\n", p->size);
    exit(31);
  }
  memset(p->ring, 0, p->size);
  atomic_init(&p->rpos, 0);
  atomic_init(&p->wpos, 0);
  atomic_init(&p->eof, 0);
  atomic_init(&p->full, 0);
  atomic_init(&p->minfill, -1);
  /* temporary buffers */
  for (i = 1; i < (nrcp > 1 ? nrcp : 2); i++) {
    if (posix_memalign(p->tbufs+i, 4096, 2*p->olen*BPF)) {
      fprintf(stderr, "alsaplay: cannot allocate buffer for cleaning.\n");
      exit(31);
    }
  }
  /* for mmap the hwbuffer must be a multiple of output per loop */
  hwbufsize = hwbufsize - (hwbufsize % p->olen);

  /* setup sound device */
  snd_pcm_hw_params_malloc(&hwparams);
  if (snd_pcm_open(&pcm, device, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
    fprintf(stderr, "alsaplay: error opening PCM device %s\n", device);
    exit(32);
  }
  if (nonblock && snd_pcm_nonblock(pcm, 1) < 0) {
    fprintf(stderr, "alsaplay: cannot set non-block mode.\n");
    exit(33);
  }
  if (snd_pcm_hw_params_any(pcm, hwparams) < 0) {
    fprintf(stderr, "alsaplay: cannot configure this PCM device.\n");
    exit(34);
  }
  if (snd_pcm_hw_params_set_access(pcm, hwparams,
                                   SND_PCM_ACCESS_MMAP_INTERLEAVED) < 0) {
    fprintf(stderr, "alsaplay: error setting mmap access.\n");
    exit(35);
  }
  if (snd_pcm_hw_params_set_format(pcm, hwparams, SND_PCM_FORMAT_S32_LE) < 0) {
    fprintf(stderr, "alsaplay: error setting format S32_LE.\n");
    exit(36);
  }
  if (snd_pcm_hw_params_set_rate(pcm, hwparams, rate, 0) < 0) {
    fprintf(stderr, "alsaplay: error setting rate %d.\n", rate);
    exit(37);
  }
  if (snd_pcm_hw_params_set_channels(pcm, hwparams, 2) < 0) {
    fprintf(stderr, "alsaplay: error setting channels to 2.\n");
    exit(38);
  }
  if (verbose) {
    snd_pcm_uframes_t min=1, max=100000000;
    snd_pcm_hw_params_set_buffer_size_minmax(pcm, hwparams, &min, &max);
    fprintf(stderr, "alsaplay: min and max buffer size of device %ld .. %ld - ",
            min, max);
  }
  if (snd_pcm_hw_params_set_buffer_size(pcm, hwparams, hwbufsize) < 0) {
    fprintf(stderr, "\nalsaplay: error setting buffersize to %ld.\n", hwbufsize);
    exit(39);
  }
  snd_pcm_hw_params_get_buffer_size(hwparams, &hwb);
  if (verbose)
    fprintf(stderr, " using %ld.\n", (long)hwb);
  if (snd_pcm_hw_params(pcm, hwparams) < 0) {
    fprintf(stderr, "alsaplay: error setting HW params.\n");
    exit(40);
  }
  snd_pcm_hw_params_free(hwparams);
  if (snd_pcm_sw_params_malloc(&swparams) < 0 ||
      snd_pcm_sw_params_current(pcm, swparams) < 0 ||
      snd_pcm_sw_params_set_start_threshold(pcm, swparams, hwb/2) < 0 ||
      snd_pcm_sw_params(pcm, swparams) < 0) {
    fprintf(stderr, "alsaplay: cannot set SW params.\n");
    exit(41);
  }
  snd_pcm_sw_params_free(swparams);
  p->pcm = pcm;
  p->hwbufsize = hwb;
  return p;
}

void alsaplay_start(alsaplay *p)
{
  if (pthread_create(&p->thread, NULL, playloop, p) != 0) {
    fprintf(stderr, "alsaplay: cannot create player thread.\n");
    exit(42);
  }
}

void alsaplay_write(alsaplay *p, const char *data, long len)
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

void alsaplay_fill(alsaplay *p, double *ms, double *minms, long *waits)
{
  long mf;

  *ms = (atomic_load_explicit(&p->wpos, memory_order_relaxed) -
         atomic_load_explicit(&p->rpos, memory_order_relaxed))
        * 1000.0 / p->bps;
  mf = atomic_exchange(&p->minfill, -1);
  *minms = mf < 0 ? -1.0 : mf * 1000.0 / p->bps;
  *waits = p->underruns;
}

void alsaplay_finish(alsaplay *p)
{
  atomic_store_explicit(&p->eof, 1, memory_order_release);
  pthread_join(p->thread, NULL);
  if (p->verbose || p->underruns)
    fprintf(stderr, "alsaplay: %ld bytes played, %ld times waited for input.\n",
            (long)atomic_load(&p->rpos), p->underruns);
}
