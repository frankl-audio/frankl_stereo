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
    int             syncfreq, npages, nrrefreshs;
    long            psize, bsize, fsize, lsize, nloops, nsec, i, n;
    long            *ptrin, *ptrout, *ip, *op;
    char            *addrin, *addrout, *buf, area[65536], *ptmp;
    struct stat     sb;
    struct timespec mtime, checktime;

    if ((argc < 3) || (argc > 7)) {
        fprintf(stderr, "%s infile outfile [pagespersecond  syncfrequency  npages]\n", argv[0]);
        fprintf(stderr, "   'loopspersecond': 1..5000 (default 1600),\n");
        fprintf(stderr, "   'syncfrequency': 0..20 (default 5) output sync'ed every 2^syncfreq loops,\n");
        fprintf(stderr, "   'npages': 1..512 (default 2) number of pages per loop,\n");
        fprintf(stderr, "   'nrrefresh': 0..100000 (default 100) number of refreshs before write.\n");
        exit(EXIT_FAILURE);
    }

    /* optional arguments */
    if (argc > 3) {
        n = atoi(argv[3]);
        if (n < 1 || n > 5000) n = 1600;
        nsec = 1000000000/n;
    } else {
        nsec = 625000;
    }
    if (argc > 4) {
        n = atoi(argv[4]);
        if (n < 0 || n > 20) n = 5;
        syncfreq = 1 << n;
    } else {
        syncfreq = 32;
    }
    if (argc > 5) {
        npages = atoi(argv[5]);
        if (npages < 1 || npages > 512) npages = 2;
    } else {
        npages = 2;
    }
    if (argc > 6) {
        nrrefreshs = atoi(argv[6]);
        if (nrrefreshs < 0 || nrrefreshs > 100000) nrrefreshs = 100;
    } else {
        nrrefreshs = 100;
    }


    fdin = open(argv[1], O_RDONLY|O_NOATIME);
    if (fdin == -1)
        handle_error("open input file");

    if (fstat(fdin, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");
    
    /* determine all parameters 
         psize: size of pages
         bsize: npages * psize
         lsize: long ints in blocks of length npages*psize 
         buf:   buffer aligned to page size
         fsize: size of input and output file 
         nloops:number of loops
    */
    psize = sysconf(_SC_PAGE_SIZE);
    bsize = npages * psize;
    lsize = bsize / sizeof(long);
    if (npages < 16)
        for (buf = area; (long)buf % psize != 0; buf++);
    else {
        ptmp = (char *)malloc(bsize+psize);
        for (buf = ptmp; (long)buf % psize != 0; buf++);
    }

    fsize = sb.st_size;
    nloops = fsize / bsize;

    /* remove output file if it exists */
    if (access(argv[2], F_OK)) remove(argv[2]);
    fdout = open(argv[2], O_RDWR|O_CREAT|O_NOATIME, 00644);
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
             n<nloops; 
             ptrin+=lsize, ptrout+=lsize, n++) {
         /* sync output every syncfreq pages, reset timer */
         if (n > 0 && n % syncfreq == 0) {
             msync(addrout+(n-syncfreq)*bsize, syncfreq*bsize, MS_SYNC);
             clock_gettime(CLOCK_MONOTONIC, &mtime);
         }
         /* increase time to wake up */
         mtime.tv_nsec += nsec;
         if (mtime.tv_nsec > 999999999) {
           mtime.tv_nsec -= 1000000000;
           mtime.tv_sec++;
         }
         /* refresh data to write next, npages pages should fit into L1 cache */
         memclean(buf, bsize);         
         cpblk(ptrin, (long*)buf);
         refreshmems(buf, bsize, nrrefreshs);
         while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &mtime, NULL)
                     != 0);
         /* now copy the refreshed pages */
         cpblk((long*)buf, ptrout);
    }
         
    /* handle partial page at the end */
    n = nloops*bsize;
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

