/*
cleancp4.c                Copyright frankl 2024

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

int main(int argc, char *argv[]) {
    int             fdin, fdout;
    int             syncfreq, sy, npages, nrrefreshs;
    long            psize, bsize, fsize, nloops, nsec, n;
    long            c;
    char            *buf, *ptmp, *nptr;
    struct stat     sb;
    struct timespec mtime;

    if ((argc < 3) || (argc > 7)) {
        fprintf(stderr, "%s infile outfile [pagespersecond syncfrequency npages nrrefresh]\n", argv[0]);
        fprintf(stderr, "   'loopspersecond': 1..5000 (default 300),\n");
        fprintf(stderr, "   'syncfrequency': 0..20 (default 3) output sync'ed every 2^syncfreq loops,\n");
        fprintf(stderr, "   'npages': 1..512 (default 2) number of pages per loop,\n");
        fprintf(stderr, "   'nrrefresh': 0..100000 (default 1) number of refreshs before write.\n");
        exit(EXIT_FAILURE);
    }

    /* optional arguments */
    if (argc > 3) {
        n = atoi(argv[3]);
        if (n < 1 || n > 5000) n = 300;
        nsec = 1000000000/n;
    } else {
        nsec = 1000000000/300;
    }
    if (argc > 4) {
        n = atoi(argv[4]);
        if (n < 0 || n > 20) n = 3;
        syncfreq = 1 << n;
    } else {
        syncfreq = 8;
    }
    if (argc > 5) {
        npages = atoi(argv[5]);
        if (npages < 1 || npages > 512) npages = 2;
    } else {
        npages = 2;
    }
    if (argc > 6) {
        nrrefreshs = atoi(argv[6]);
        if (nrrefreshs < 0 || nrrefreshs > 100000) nrrefreshs = 1;
    } else {
        nrrefreshs = 1;
    }


    /* determine all parameters 
         psize: size of pages
         bsize: npages * psize
         buf:   buffer aligned to page size
         fsize: size of input and output file 
         nloops:number of loops
    */
    /* if first argument is integer, this amount is read from stdin */
    fsize = strtol(argv[1], &nptr, 10);
    if (*nptr == 0 && fsize > 0) {
        fdin = 0;
    } else {
        fdin = open(argv[1], O_RDONLY|O_NOATIME);
        if (fdin == -1) {
            fprintf(stderr, "cleancp: cannot open input file: %s\n", argv[1]);
            exit(2);
        }
        /* get file size */
        if (fstat(fdin, &sb) == -1) {
            fprintf(stderr, "cleancp: cannot get file size: %s\n", argv[1]);
            exit(3);
        }
        fsize = sb.st_size;
    }
    psize = sysconf(_SC_PAGE_SIZE);
    bsize = npages * psize;
    nloops = fsize / bsize;

    /* input buffer */
    ptmp = (char *)malloc((nloops+2)*bsize);
    for (buf = ptmp; (long)buf % psize != 0; buf++);

    memclean(buf, fsize);

    /* read input */
    for(n=nloops*npages, ptmp=buf; n; ptmp+=psize, n--) {
         c = read(fdin, (void*)ptmp, psize);
         if (c < psize) {
             fprintf(stderr, "cleancp: Problem reading input.\n");
             exit(4);
         }
    }
    n = nloops*bsize;
    if (n < fsize) {
        c = read(fdin, (void*)ptmp, fsize - n);
    }
    close(fdin);

    /* remove output file if it exists */
    if (access(argv[2], F_OK)) remove(argv[2]);
    fdout = open(argv[2], O_RDWR|O_CREAT|O_NOATIME, 00644);
    if (fdout == -1) {
        fprintf(stderr, "cleancp: cannot open output file: %s\n", argv[2]);
        exit(5);
    }

    /* the main loop */
    clock_gettime(CLOCK_MONOTONIC, &mtime);
    for(n=nloops, sy=syncfreq, ptmp=buf; n; ptmp+=bsize, sy--, n--) {
         /* sync output every syncfreq pages, reset timer */
         if (!sy) {
             fdatasync(fdout);
             sy = syncfreq;
             clock_gettime(CLOCK_MONOTONIC, &mtime);
         }
         /* increase time to wake up */
         mtime.tv_nsec += nsec;
         if (mtime.tv_nsec > 999999999) {
           mtime.tv_nsec -= 1000000000;
           mtime.tv_sec++;
         }
         /* refresh data to write next, npages pages should fit into L1 cache */
         refreshmems(buf, bsize, nrrefreshs);
         while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &mtime, NULL)
                     != 0);
         /* now copy the refreshed page */
         c = write(fdout, (void*)ptmp, bsize);
    }
         
    /* handle partial page at the end */
    n = nloops*bsize;
    if (n < fsize) {
        c = write(fdout, (void*)ptmp, fsize - n);
    }

    fdatasync(fdout);
    close(fdout);

    exit(EXIT_SUCCESS);
}

