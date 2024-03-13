/*
music2nf.c                Copyright frankl 2024

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

    gcc -o music2nf -O2 music2nf.c cprefresh_ass.o  cprefresh.o -llibsnd -lrt

*/

#include "version.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <sndfile.h>
#include "cprefresh.h"
#include "nf_io.h"

#define TBUF 536870912
#define MAXBUFSIZE 2147483648

void usage( ) {
  fprintf(stderr,
          "music2nf (version %s of frankl's stereo utilities)\n\nUSAGE:\n",
          VERSION);
  fprintf(stderr,
"\n"
"  music2nf ... \n"
"\n"
"  This program copies a music file in raw format into an nf info file.\n"
"  A new nf file is created and the nf info file must be specified.\n"
"  When the input file contains 16 bit sample, the raw file will also contain\n"
"  16 bit integer samples and otherwise 32 bit integer samples.\n"
"  This can be overwritten with the --float option when the samples will be \n"
"  stored as 32 bit floats (marked as 28 bit in the nf info file.\n"
"  In general the nf info file will use its information slots to remember\n"
"  bit depth, sampling rate and the path to the input file.\n"
"  \n"
"  Alternatively, with option --stdin the raw samples can be read from \n"
"  standard input. In that case the options --bits, --rate ans --text must\n"
"  also be used, to specify the information for the nf info file.\n"
"  \n"
"  On success this program prints the nf id of the new file.\n"
"  \n"
"  The options are:\n"
"  \n"
"  --file=fnam, -F fnam\n"
"      the name of the input music file (readable by libsnd library).\n"
"  \n"
"  --nfinfo=nfnam, -M nfnam\n"
"      the name of the nf info file.\n"
"  \n"
"  --float\n"
"      store samples in float format (32 bit), we use 28 in bits entry of \n"
"      nf info.\n"
"  \n"
"  --loops-per-second=intnum, -n intnum\n"
"      loops per second when writing data to nf file. In each loop a number of\n"
"      pages of 4096 bytes, given by --pages-per-loop, are written to the \n"
"      nf device.\n"
"      The default is 1024.\n"
"  \n"
"  --pages-per-loop=intnum, -p intnum\n"
"      see previous option. Default is 2 pages of 4096 bytes.\n"
"  \n"
"  --dsync-number=intnum, -D intnum\n"
"      every given number of loops data are synchronized with the block\n"
"      device (disk, CF Card,...). The default is 32.\n"
"  \n"
"  --number-copies=intnum, -R intnum\n"
"      before writing the data to the block device they are copied the\n"
"      given number of times to a cleaned temporary buffer in RAM and back\n"
"      to the original buffer. The default is 0. Using this option may\n"
"      improve the sound quality of the stored nf file.\n"
"  \n"
"  --stdin, -S\n"
"      instead of --file option. Raw input samples are read from stdin \n"
"      (max 2GB).\n"
"      In this case the follwing three options must also be used.\n"
"  \n"
"  --rate nr, -r nr\n"
"      sampling rate of raw sample from stdin\n"
"  \n"
"  --bits k, -b k\n"
"      bit depth of raw samples from stdin.\n"
"  \n"
"  --text txt, -T txt\n"
"      the text in the nf info file.\n"
"\n"
);
}

int main(int argc, char *argv[])
{
    int lps, sf, sy, npages, nrcp, optc, sfloat, rate, bits, bpf, verbose, stdinp;
    long bsize, fsize, nloops, cnt, c, id, s, e;
    long i, j, nsec, frames;
    char *file, *nfinfo, *txt;
    char *buf, *ptmp, *tbuf;
    struct nfrec *check;
    SF_INFO sfinfo;
    SNDFILE *sndfile;
    struct timespec mtime;

    /* read command line options */
    static struct option longoptions[] = {
        {"file", required_argument, 0, 'F' },
        {"nfinfo", required_argument, 0, 'M' },
        {"loops-per-second", required_argument, 0,  'n' },
        {"dsync-number", required_argument, 0,  'D' },
        {"pages-per-loop", required_argument, 0,  'P' },
        {"number-copies", required_argument, 0, 'R' },
        {"stdin", no_argument, 0, 'S' },
        {"float", no_argument, 0, 'f' },
        {"rate", required_argument, 0, 'r' },
        {"bits", required_argument, 0, 'b' },
        {"text", required_argument, 0, 'T' },
        {"verbose", no_argument, 0, 'v' },
        {"version", no_argument, 0, 'V' },
        {"help", no_argument, 0, 'h' },
        {0,         0,                 0,  0 }
    };

    if (argc == 1) {
       usage();
       exit(0);
    }

    /* defaults */
    lps = 1024;
    sf = 32;
    npages = 2;
    nrcp = 0;
    rate = 48000;
    bits = 32;
    stdinp = 0;
    file = NULL;
    nfinfo = NULL;
    buf = NULL;
    txt = "";
    sfloat = 0;
    verbose = 0;
    frames = 0;
    bsize = 0;
    fsize = 0;
    nloops = 0;


    while ((optc = getopt_long(argc, argv, "F:M:n:D:P:R:Sr:b:T:vVh",
            longoptions, &optind)) != -1) {
        switch (optc) {
        case 'F':
          file = optarg;
          break;
        case 'M':
          nfinfo = optarg;
          break;
        case 'n':
          lps = atoi(optarg);
          if (lps < 1 || lps > 5000) lps = 1024;
          break;
        case 'D':
          sf = atoi(optarg);
          if (sf < 1 || sf > 5000) sf = 32;
          break;
        case 'P':
          npages = atoi(optarg);
          if (npages < 1 || npages > 1000) npages = 2;
          break;
        case 'R':
          nrcp = atoi(optarg);
          if (nrcp < 0 || nrcp > 1000) nrcp = 0;
          break;
        case 'S':
          stdinp = 1;
          break;
        case 'r':
          rate = atoi(optarg);
          break;
        case 'b':
          bits = atoi(optarg);
          break;
        case 'T':
          txt = optarg;
          break;
        case 'f':
          sfloat = 1;
          break;
        case 'v':
          verbose = 1;
          break;
        case 'V':
          usage();
          exit(0);
        default:
          usage();
          exit(3);
        }
    }

    nsec = 1000000000/lps;

    if (file) {
        /* get parameters of sound file */
        sfinfo.format = 0;
        sndfile = sf_open(file, SFM_READ, &sfinfo);
        if (sndfile == NULL) {
            fprintf(stderr, "music2nf: cannot open file %s.\n", file);
            exit(2);
        }
        rate = sfinfo.samplerate;
        bits = sfinfo.format & 0xFFFF;
        if (bits == 2) {
            bits = 16;
            bpf = 4;
        } else if (bits == 3 || bits == 4) {
            bits = 32;
            bpf = 8;
        } else {
            fprintf(stderr, "music2nf: cannot get bit depth.\n");
            exit(3);
        }
        if (sfloat) {
            bits = 28;
            bpf = 8;
        }
        frames = sfinfo.frames;
        /* seek to start 
        if (start != 0 && sfinfo.seekable) {
            sf_seek(sndfile, start, SEEK_SET);
        }*/
        /* determine all parameters 
             PS: size of pages from nf_io
             bsize: npages * psize
             buf:   buffer aligned to page size
             fsize: size of output file 
             nloops:number of loops
        */
        bsize = npages * PS;
        fsize = frames*bpf;
        nloops = fsize / bsize;
        txt = file;

        /* input buffer, align to page size */
        ptmp = (char *)malloc((nloops+2)*bsize);
        for (buf = ptmp; (long)buf % PS != 0; buf++);

        /* read all samples in 16bit/32bit unsigned ints or floats */
        memclean(buf, fsize);
        if (bits == 16)
            cnt = sf_readf_short(sndfile, (short *)buf, frames);
        else if (bits == 32)
            cnt = sf_readf_int(sndfile, (int *)buf, frames);
        else if (bits == 28)
            cnt = sf_readf_float(sndfile, (float *)buf, frames);
        if (cnt < frames) {
            fprintf(stderr, "music2nf: could not read all frames (%ld/%ld).\n",
                    cnt, frames);
            exit(4);
        }
        sf_close(sndfile);
    }
    if (stdinp) {
        /* page size aligned temporary buffer */
        ptmp = (char *)malloc(MAXBUFSIZE+PS);
        for (buf = ptmp; (long)buf % PS != 0; buf++);
        c = 1;
        cnt = 0;
        for (ptmp=buf; c; ) {
            memclean((char*)ptmp, PS);
            c = read(0, ptmp, PS);
            cnt += c;
            if (cnt+PS >= MAXBUFSIZE) {
                fprintf(stderr, "music2nf: input must be <= 2GB \n");
                exit(4);
            }
            ptmp += c;
        }
        bsize = npages * PS;
        fsize = cnt;
        nloops = fsize / bsize;
    }

    /* maybe make refreshed copies */
    if (nrcp > 0) {
        /* page size aligned temporary buffer */
        ptmp = (char *)malloc(TBUF+PS);
        for (tbuf = ptmp; (long)tbuf % PS != 0; tbuf++);
        for (i=nrcp;  i; i--) {
             for (s = 0, e = s+TBUF; s < fsize; s += TBUF, e += TBUF) {
                 if (e > fsize) e = fsize;
                 memclean(tbuf, e-s);
                 cprefresh(tbuf, buf+s, e-s);
                 memclean(buf+s, e-s);
                 cprefresh(buf+s, tbuf, e-s);
             }
        }
        free(ptmp);
    }

    /* allocate space on nf disk */
    id = nfnew(nfinfo, fsize, rate, bits, txt); 
    if (verbose)
        fprintf(stderr, "musik2nf: new nf file has id %ld.\n", id);

    /* open nf file to write */
    check = nfopen(nfinfo, id, O_RDWR, 0);
    if (check == NULL) {
        fprintf(stderr, "music2nf: cannot open nf file %s, id=%ld.\n",
                nfinfo, id);
        exit(5);
    }

    /* write out */
    ptmp = buf;
    clock_gettime(CLOCK_MONOTONIC, &mtime);
    for (sy = 0, i = nloops; i; i--) {
        mtime.tv_nsec += nsec;
        if (mtime.tv_nsec > 999999999) {
          mtime.tv_nsec -= 1000000000;
          mtime.tv_sec++;
        }
        refreshmem((char*)ptmp, PS*npages);
        while (clock_nanosleep(CLOCK_MONOTONIC, 
                               TIMER_ABSTIME, &mtime, NULL) != 0) ;
        /* write npages pages */
        for (j=npages; j; j--) {
            nfwritepage(ptmp);
            ptmp += PS;
        }
        sy++;
        if (sy == sf) {
             fdatasync(nffd);
             sy = 0;
             /* duration of sync depends on hardware, reset timer */
             clock_gettime(CLOCK_MONOTONIC, &mtime);
        }
    }
    /* write trailing pages */
    for(; ptmp < buf+fsize;) {
        refreshmem((char*)ptmp, PS);
        nfwritepage(ptmp);
        ptmp += PS;
    }
    fdatasync(nffd);
    nfclose();
    printf("%ld", id);
    return 0;
}

