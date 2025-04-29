/*
forimprove_c4.c                Copyright frankl 2025

Specific variant for the Odroid C4 using assembler code to access the
24Mhz system counter for the timing. Instead of intervals between writes in 
d nanoseconds we use intervalls of d*3/125 ticks of the counter.
For precision we use 1000*d*3/125 for the increments and divide by
1000 in the timing loop.

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/


#define _GNU_SOURCE
#include "version.h"
#include "net.h"
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include "cprefresh.h"

#define TBUF 134217728 /* 2^27 */
#define ilen 8192
#define shift 20000

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

int main(int argc, char *argv[])
{
    long lps, bps, nsync, outcopies, olen, lcount, dcount, tcount, i, k, wnext, s, fsize;
    long outtime, mtime, mtime1, mtick, etick;
    double nsec, ntick;
    void *tbuf, *cbuf, *obufs[1024], *iptr, *optr;
    int ifd, ofd, moreinput;

    if (argc < 7) {
       fprintf(stderr, "Usage:  forimprove_c4 origfile copiedfile lps bps nsync ncp\n");
       exit(0);
    }
    if ((ifd = open(argv[1], O_RDONLY | O_NOATIME)) == -1) {
        fprintf(stderr, "forimprove_c4:: Cannot open input file %s: %s.\n", 
                        argv[1], strerror(errno));
        exit(2);
    }
    ofd = open(argv[2], O_WRONLY | O_CREAT | O_NOATIME, 00644);
    if (ofd == -1) {
        fprintf(stderr, "forimprove_c4: Cannot open output file %s.\n   %s\n",
                         argv[2], strerror(errno));
        exit(3);
    }
    lps = atol(argv[3]);
    bps = atol(argv[4]);
    nsync = atol(argv[5]);
    outcopies = atol(argv[6]);
    olen = bps/lps;
    nsec = 1000000000.0/lps;
    ntick = nsec*3/125;
    outtime = (long)(nsec/outcopies/3);

    /* avoid waiting 50000 ns collecting more sleep requests */
    prctl(PR_SET_TIMERSLACK, 1L);

    /* calibrate sleep loop */
    mtime = gettime();
    nsloop(10000000);
    mtime1 = gettime();
    nsloopfactor = 1.0*10000000/(mtime1-mtime-50);

    if (posix_memalign(&tbuf, 4096, TBUF)) {
        fprintf(stderr, "forimprove_c4: Cannot allocate input buffer.\n");
        exit(2);
    }
    if (posix_memalign(&cbuf, 4096, TBUF)) {
        fprintf(stderr, "forimprove_c4: Cannot allocate cleaning buffer.\n");
        exit(2);
    }
    for (i=1; i < outcopies; i++) {
        if (posix_memalign(obufs+i, 4096, 2*olen)) {
            fprintf(stderr, "forimprove_c4: Cannot allocate buffer for output cleaning.\n");
            exit(20);
        }
    }


    /* counter for fdatasync */
    dcount = 0;
    for (moreinput=1; moreinput; ) {
       /* fill buffer */
       memclean(tbuf, TBUF);
       for (iptr = tbuf; iptr < tbuf + TBUF - ilen; ) {
           s = read(ifd, iptr, ilen);
           if (s < 0) {
               fprintf(stderr, "forimprove_c4: Read error.\n");
               exit(18);
           }
           if (s == 0) {
               moreinput = 0;
               break;
           }
           iptr += s;
       }
       fsize = iptr - tbuf;

       /* make refreshed copies */
       memclean(cbuf, fsize);
       cprefresh(cbuf, tbuf, fsize);
       memclean(tbuf, fsize);
       cprefresh(tbuf, cbuf, fsize);

       /* write out */
       optr = tbuf;
       wnext = (iptr - optr <= olen) ? (iptr - optr) : olen;
       mtime = gettime();
       __asm__ __volatile__("isb\nmrs %0, cntvct_el0" : "=r"(mtick));
       tcount = 0;
       for (lcount=0; optr < iptr; lcount++) {
           /* once cache is filled and other side is reading we reset time */
           if (lcount == 50) {
              mtime = gettime();
              __asm__ __volatile__("isb\nmrs %0, cntvct_el0" : "=r"(mtick));
              tcount = 0;
           }
           tcount++;
           mtime1 = gettime();
           if (mtime1 > mtime+tcount*nsec) {
              /* reset to current time + 0.1 sec */
              mtime = gettime();
              __asm__ __volatile__("isb\nmrs %0, cntvct_el0" : "=r"(mtick));
              tcount = (long)(100000000/nsec);
           }
           /* refresh output chunk before sleep */
           obufs[0] = optr;
           obufs[outcopies] = optr;
           for (k=1; k <= outcopies; k++) {
               sleepfor(outtime);
               memclean((char*)(obufs[k]), wnext);
               cprefresh((char*)(obufs[k]), (char*)(obufs[k-1]), wnext);
               memclean((char*)(obufs[k-1]), wnext);
           }
           
           sleepto(mtime, tcount*nsec);
           etick = mtick + shift + (long)(round(tcount*ntick));
           __asm__ __volatile__ (
           ".LOOP: \n\t"
                  "isb  \n\t"
                  "mrs x7, cntvct_el0 \n\t"
                  "cmp x7, %[et] \n\t"
                  "blt .LOOP \n\t"
           :
           : [et] "r" (etick)
           : "x7" );
           /* write a chunk, this comes first after waking from sleep */
           s = write(ofd, optr, wnext);
           if (s < 0) {
               fprintf(stderr, "bufhrt: Write error.\n");
               exit(15);
           }
           dcount++;
           if (dcount >= nsync) {
              fdatasync(ofd);
              dcount = 0;
              /* duration of sync depends on hardware, reset timer */
              mtime1 = gettime();
              if (mtime1+0.5*nsec > mtime+(tcount+1)*nsec) {
                  mtime = gettime();
                  __asm__ __volatile__("isb\nmrs %0, cntvct_el0" : "=r"(mtick));
                  tcount = 0;
              }
           }
           optr += s;
           wnext = olen + wnext - s;
           if (wnext >= 2*olen) {
              fprintf(stderr, "bufhrt: Underrun by %ld.\n",
                        wnext - 2*olen);
              wnext = 2*olen-1;
           }
           s = iptr - optr;
           if (s <= wnext) {
               wnext = s;
           }
       }
       fdatasync(ofd);
    }

    posix_fadvise(ofd, 0, 0, POSIX_FADV_DONTNEED);
    close(ofd);
    posix_fadvise(ifd, 0, 0, POSIX_FADV_DONTNEED);
    close(ifd);
    exit(0);
}


