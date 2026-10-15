/*
improvetest.c                Copyright frankl 2025-2026

Extended variant of forimprovedirect_c4.c for experiments (Odroid C4, on
other machines only for testing the program logic). Without options it
does the same as forimprovedirect_c4, see the usage message for the
additional options.

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/


#define _GNU_SOURCE
#include "version.h"
#include <getopt.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include "cprefresh.h"

#define shift 20000

void usage()
{
  fprintf(stderr,
"Usage:  improvetest [options] origfile copiedfile lps bps nsync ncp\n"
"\n"
"  Copies origfile to copiedfile in lps loops per second with bps bytes\n"
"  per second, calls fdatasync every nsync loops and copies each chunk\n"
"  through ncp cleaned buffers before writing, as forimprovedirect_c4.\n"
"\n"
"Options:\n"
"  -o     write with O_DIRECT: each write goes to the device at once,\n"
"         not into the page cache\n"
"  -y     write with O_DSYNC: each write returns when the data are on the\n"
"         device (with -o: no write cache of the device in between)\n"
"  -i     read with O_DIRECT (no page cache)\n"
"  -a n   chunk size per loop (bps/lps) is rounded down to a multiple of\n"
"         n bytes, default 4096 with -o or -i, otherwise 1 (with -o and\n"
"         -i it must be a multiple of the logical block size, usually\n"
"         512 or 4096)\n"
"  -w     warm-up: a cheap system call directly after the sleep\n"
"  -p     preallocate the output file with fallocate\n"
"  -v     print the used parameters and statistics at the end\n"
"\n"
"  The input is always dropped from the page cache before reading.\n"
"  With -o the last partial chunk is written padded to a full chunk and\n"
"  the file is then truncated to the correct length.\n");
}

/* after calibration nsloop will take cnt ns */
double nsloopfactor = 1.0;

static inline long  nsloop(long cnt) {
  long i, j;
  for (j = 1, i = (long)(cnt*nsloopfactor); i > 0; i--) j = j+i;
  return j;
}

/* time in ns */
long gettime()
{
     struct timespec time;
     clock_gettime(CLOCK_MONOTONIC, &time);
     return time.tv_sec*1000000000+time.tv_nsec;
}
/* sleep to base + (long)(round(dns)) ns */
void sleepto(long base, double dns)
{
     long t;
     struct timespec time;
     t = base + (long)(round(dns));
     time.tv_sec = t/1000000000;
     time.tv_nsec = t - time.tv_sec*1000000000;
     clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
}
/* sleep for t ns */
void sleepfor(long t)
{
     struct timespec time;
     time.tv_sec = t/1000000000;
     time.tv_nsec = t - time.tv_sec*1000000000;
     nanosleep(&time, NULL);
}

#ifdef __aarch64__
static inline long getticks()
{
  long t;
  __asm__ __volatile__("isb\nmrs %0, cntvct_el0" : "=r"(t));
  return t;
}
static inline void spinto(long etick)
{
  __asm__ __volatile__ (
  "1: \n\t"
         "isb  \n\t"
         "mrs x7, cntvct_el0 \n\t"
         "cmp x7, %[et] \n\t"
         "blt 1b \n\t"
  :
  : [et] "r" (etick)
  : "x7" );
}
#else
/* only for testing the program logic on other machines */
static inline long getticks()
{
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return (t.tv_sec*1000000000+t.tv_nsec)*3/125;
}
static inline void spinto(long etick)
{
  while (getticks() < etick)
    ;
}
#endif

int main(int argc, char *argv[])
{
    long lps, bps, nsync, outcopies, olen, dcount, tcount, i, k, s, w;
    long outtime, mtime, mtime1, mtick, etick, align, total, nresets, nsyncs;
    double nsec, ntick;
    void *tbuf, *obufs[1024];
    int ifd, ofd, moreinput, optc, odirect, dsync, idirect, warmup, prealloc,
        verbose, oflags, iflags;
    struct stat sb;

    odirect = dsync = idirect = warmup = prealloc = verbose = 0;
    align = 0;
    while ((optc = getopt(argc, argv, "oyia:wpvh")) != -1) {
        switch (optc) {
        case 'o': odirect = 1; break;
        case 'y': dsync = 1; break;
        case 'i': idirect = 1; break;
        case 'a': align = atol(optarg); break;
        case 'w': warmup = 1; break;
        case 'p': prealloc = 1; break;
        case 'v': verbose = 1; break;
        default: usage(); exit(1);
        }
    }
    if (argc - optind < 6) {
       usage();
       exit(0);
    }
    if (align <= 0)
        align = (odirect || idirect) ? 4096 : 1;
    iflags = O_RDONLY | O_NOATIME;
    if (idirect)
        iflags |= O_DIRECT;
    if ((ifd = open(argv[optind], iflags)) == -1) {
        fprintf(stderr, "improvetest: Cannot open input file %s: %s.\n",
                        argv[optind], strerror(errno));
        exit(2);
    }
    /* make sure that we read from the device */
    posix_fadvise(ifd, 0, 0, POSIX_FADV_DONTNEED);
    oflags = O_WRONLY | O_CREAT | O_NOATIME;
    if (odirect)
        oflags |= O_DIRECT;
    if (dsync)
        oflags |= O_DSYNC;
    ofd = open(argv[optind+1], oflags, 00644);
    if (ofd == -1) {
        fprintf(stderr, "improvetest: Cannot open output file %s.\n   %s\n",
                         argv[optind+1], strerror(errno));
        exit(3);
    }
    if (prealloc && fstat(ifd, &sb) == 0 && sb.st_size > 0 &&
        fallocate(ofd, 0, 0, sb.st_size) != 0)
        fprintf(stderr, "improvetest: fallocate failed: %s.\n", strerror(errno));
    lps = atol(argv[optind+2]);
    bps = atol(argv[optind+3]);
    nsync = atol(argv[optind+4]);
    outcopies = atol(argv[optind+5]);
    if (lps < 1 || bps < 1 || nsync < 1 || outcopies < 1 || outcopies > 1000) {
        fprintf(stderr, "improvetest: invalid arguments.\n");
        exit(4);
    }
    olen = bps/lps;
    olen -= olen % align;
    if (olen <= 0) {
        fprintf(stderr, "improvetest: chunk size %ld/%ld is smaller than %ld.\n",
                bps, lps, align);
        exit(4);
    }
    nsec = 1000000000.0/lps;
    ntick = nsec*3/125;
    outtime = (long)(nsec/outcopies/3);
    if (verbose)
        fprintf(stderr, "improvetest: %ld bytes per loop (%ld bytes/s), "
                "output%s%s, input%s, warm-up %d, preallocate %d\n",
                olen, olen*lps, odirect ? " O_DIRECT" : "",
                dsync ? " O_DSYNC" : "", idirect ? " O_DIRECT" : "",
                warmup, prealloc);

    /* avoid waiting 50000 ns collecting more sleep requests */
    prctl(PR_SET_TIMERSLACK, 1L);

    /* calibrate sleep loop */
    mtime = gettime();
    nsloop(10000000);
    mtime1 = gettime();
    nsloopfactor = 1.0*10000000/(mtime1-mtime-50);

    if (posix_memalign(&tbuf, 4096, 2*olen)) {
        fprintf(stderr, "improvetest: Cannot allocate input buffer.\n");
        exit(2);
    }
    for (i=1; i < outcopies; i++) {
        if (posix_memalign(obufs+i, 4096, 2*olen)) {
            fprintf(stderr, "improvetest: Cannot allocate buffer for output cleaning.\n");
            exit(20);
        }
    }

    /* counter for fdatasync */
    dcount = 0;
    total = 0;
    nresets = 0;
    nsyncs = 0;
    /* read and write loop */
    obufs[0] = tbuf;
    obufs[outcopies] = tbuf;
    mtime = gettime();
    mtick = getticks();
    tcount = 0;
    for (moreinput=1; moreinput;) {
        s = read(ifd, tbuf, olen);
        if (s < 0) {
            fprintf(stderr, "improvetest: Read error: %s.\n", strerror(errno));
            exit(18);
        }
        if (s == 0) {
            moreinput = 0;
            break;
        }
        /* with O_DIRECT output only full chunks can be written: pad the
           last chunk with zeros, the file is truncated at the end */
        if (odirect && s < olen) {
            memset((char*)tbuf + s, 0, olen - s);
            w = olen;
        } else
            w = s;
        tcount++;
        mtime1 = gettime();
        if (mtime1 > mtime+tcount*nsec) {
           /* reset to current time + 0.1 sec */
           mtime = gettime();
           mtick = getticks();
           tcount = (long)(100000000/nsec);
           nresets++;
        }
        /* refresh output chunk before sleep */
        for (k=1; k <= outcopies; k++) {
            sleepfor(outtime);
            memclean((char*)(obufs[k]), w);
            cprefresh((char*)(obufs[k]), (char*)(obufs[k-1]), w);
        }

        sleepto(mtime, tcount*nsec);
        /* a first system call after sleep is slower, take it now */
        if (warmup)
            syscall(SYS_getppid);
        etick = mtick + shift + (long)(round(tcount*ntick));
        spinto(etick);
        /* write a chunk, this comes first after waking from sleep */
        if (write(ofd, tbuf, w) != w) {
            fprintf(stderr, "improvetest: Write error: %s.\n", strerror(errno));
            exit(15);
        }
        total += s;
        dcount++;
        if (dcount >= nsync) {
           fdatasync(ofd);
           nsyncs++;
           dcount = 0;
           /* duration of sync depends on hardware, reset timer */
           mtime1 = gettime();
           if (mtime1+0.5*nsec > mtime+(tcount+1)*nsec) {
               mtime = gettime();
               mtick = getticks();
               tcount = 0;
               nresets++;
           }
        }
    }
    fdatasync(ofd);
    /* remove the padding of the last chunk */
    if (odirect && ftruncate(ofd, total) != 0) {
        fprintf(stderr, "improvetest: Cannot truncate output: %s.\n", strerror(errno));
        exit(16);
    }
    fdatasync(ofd);
    if (verbose)
        fprintf(stderr, "improvetest: %ld bytes written, %ld fdatasync, "
                "timer reset %ld times\n", total, nsyncs, nresets);

    posix_fadvise(ofd, 0, 0, POSIX_FADV_DONTNEED);
    close(ofd);
    posix_fadvise(ifd, 0, 0, POSIX_FADV_DONTNEED);
    close(ifd);
    exit(0);
}
