/*
cleancp.c                Copyright frankl 2024

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/

#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "version.h"
#include "cprefresh.h"

/* shortcuts */
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define cpblk(ptrin, ptrout) \
    for(i=0, ip=(ptrin), op=(ptrout); i < lsize; *op=*ip, ip++, op++, i++);

int main(int argc, char *argv[]) {
    int             fdin, fdout;
    int             syncfreq;
    long            psize, fsize, lsize, npages, nsec, i, n;
    long            *ptrin, *ptrout, *ip, *op;
    char            *addrin, *addrout, *buf, area[16384];
    struct stat     sb;
    struct timespec mtime, checktime;

    psize = sysconf(_SC_PAGE_SIZE);
    lsize = psize / sizeof(long);
    for (buf = area; (long)buf % psize != 0; buf++);

    if ((argc < 3) || (argc > 5)) {
        fprintf(stderr, "%s infile outfile [pagespersecond, syncfrequency]\n", argv[0]);
        fprintf(stderr, "   'pagespersecond': 1..5000 (default 2000),\n");
        fprintf(stderr, "   'syncfrequency': 0..20 (default 5) output sync'ed every 2^syncfreq pages.\n");
        exit(EXIT_FAILURE);
    }

    /* optional arguments */
    if (argc > 3) {
        n = atoi(argv[3]);
        if (n < 1 || n > 5000) n = 2000;
        nsec = 1000000000/n;
    } else {
        nsec = 500000;
    }
    if (argc > 4) {
        n = atoi(argv[4]);
        if (n < 0 || n > 20) n = 5;
        syncfreq = 1 << n;
    } else {
        syncfreq = 32;
    }

    fdin = open(argv[1], O_RDONLY);
    if (fdin == -1)
        handle_error("open input file");

    if (fstat(fdin, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");
    fsize = sb.st_size;
    npages = fsize / psize;

    /* remove output file if it exists */
    if (access(argv[2], F_OK)) remove(argv[2]);
    fdout = open(argv[2], O_RDWR|O_CREAT, 00644);
    if (fdout == -1)
        handle_error("open output file");

    /* pre-allocate space for output file */
    if (posix_fallocate(fdout, 0, fsize))
        handle_error("fallocate");

    /* mmap input and output */
    addrin = mmap(NULL, fsize, PROT_READ,
                  MAP_PRIVATE | MAP_POPULATE, fdin, 0);
    if (addrin == MAP_FAILED)
        handle_error("mmap input file");

    addrout = mmap(NULL, fsize, PROT_WRITE,
                   MAP_SHARED, fdout, 0);
    if (addrout == MAP_FAILED)
        handle_error("mmap output file");

    /* the main loop */
    clock_gettime(CLOCK_MONOTONIC, &mtime);
    clock_gettime(CLOCK_MONOTONIC, &checktime);
    for(n=0, ptrin = (long*)addrin, ptrout = (long*)addrout; 
             n<npages; 
             ptrin+=lsize, ptrout+=lsize, n++) {
         /* sync output every syncfreq pages, reset timer */
         if (n > 0 && n % syncfreq == 0) {
             msync(addrout+(n-syncfreq)*psize, syncfreq*psize, MS_SYNC);
             clock_gettime(CLOCK_MONOTONIC, &mtime);
         }
         /* increase time to wake up */
         mtime.tv_nsec += nsec;
         if (mtime.tv_nsec > 999999999) {
           mtime.tv_nsec -= 1000000000;
           mtime.tv_sec++;
         }
         /* refresh data to write next, page should fit into L1 cache */
         memclean(buf, psize);         
         cpblk(ptrin, (long*)buf);
         refreshmem(buf, psize);
         refreshmem(buf, psize);
         refreshmem(buf, psize);
         /* temporary (only seem messages with more than 4000 pages/sec
         clock_gettime(CLOCK_MONOTONIC, &checktime);
         if ((checktime.tv_sec > mtime.tv_sec)
             || (checktime.tv_sec == mtime.tv_sec
                 && checktime.tv_nsec > mtime.tv_nsec)) {
             fprintf(stderr, "cleancp: delayed block %ld\n", n);
            clock_gettime(CLOCK_MONOTONIC, &mtime);
            mtime.tv_nsec += (100000000);
            if (mtime.tv_nsec > 999999999) {
              mtime.tv_nsec -= 1000000000;
              mtime.tv_sec++;
            }
         }
          end temporary ??? */
         while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &mtime, NULL)
                     != 0);
         /* now copy the refreshed page */
         cpblk((long*)buf, ptrout);
    }
         
    /* handle partial page at the end */
    n = npages*psize;
    if (n < fsize) {
        memcpy(buf, addrin+n, fsize - n);
        refreshmem(buf, fsize - n);
        memcpy(addrout+n, buf, fsize - n);
    }

     msync(addrout, fsize, MS_SYNC);

     munmap(addrin, fsize);
     munmap(addrout, fsize);
     close(fdin);
     close(fdout);

     exit(EXIT_SUCCESS);
}

