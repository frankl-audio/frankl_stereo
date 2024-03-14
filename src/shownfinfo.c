/*
shownfinfo.c                Copyright frankl 2024

    shownfinfo finfo
   
This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/
#include "version.h"
#include <stdio.h>
#include "nf_io.h"

void usage( ) {
  fprintf(stderr,
          "shownfinfo (version %s of frankl's stereo utilities)\n\nUSAGE:\n",
          VERSION);
  fprintf(stderr,
"\n"
"  shownfinfo path/to/nfinfofile\n"
"\n"
"  nf files and nf info files\n"
"  ==========================\n"
"  nf files are data files which are stored in (part of) a fixed size\n"
"  container file. A container is typically  a block device (SSD, SD or CF\n"
"  flash card, hard disk), but can be any seekable fixed size file.\n"
"\n"
"  An nf file is stored is a consecutive part of the container. The 'nf'\n"
"  means 'no filesystem'. We need some way to store meta data for the nf file\n"
"  like position in the container, length, name and so on.\n"
"  This meta information is stored in an nf info file which we keep outside\n"
"  the container. We organize the container files in blocks of 4096 bytes\n"
"  (the typical 'page size' of the operating system, a multiple of the \n"
"  block size of typical storage media, often 512 bytes, sometime 1024\n"
"  or 4096). All nf files start at a position divisible by 4096 and we\n"
"  read and write always blocks of length 4096 bytes.\n"
"  \n"
"  The format of an nf info file is very simple, it contains white space \n"
"  separated entries: The first two are the name and length in bytes of\n"
"  the container file. This is followed by a sequence of either six entries\n"
"  starting with a positive integer or a single negative integer.\n"
"  In the first case, the positive integer is the \n"
"       - 64 bit ID of an nf file, followed by\n"
"       - the number of the block where the file starts, \n"
"       - the length in bytes,\n"
"       - a 64 bit integer i1,\n"
"       - a 64 bit integer i2,\n"
"       - a string without whitespace with at most 255 bytes.\n"
"  In the second case the negative number -n means that an entry as \n"
"  before with ID n is discarded.\n"
"  \n"
"  An nf info file can be changed by only appending to the existing file, \n"
"  either append the six entries for a new nf file or append the negative\n"
"  of the ID of a previous entry to discard that file.\n"
"  \n"
"  The utility 'shownfinfo' prints out the nf info file but with all\n"
"  items belonging to discarded files removed; the result is again a \n"
"  valid nf info file that only shows the currently stored nf files.\n"
"  \n"
"  EXAMPLE nf info file:\n"
"  \n"
"  /dev/mmcblk0\n"
"  34359738368\n"
"  1 0 45900248 48000 32 /music/my.flac\n"
"  2 11207 124245104 44100 14 /music/other.wav\n"
"  -1\n"
"  \n"
"  The command 'shownfinfo' applied to this file would not show the last\n"
"  and third last line because the nf file with ID 1 is discarded.\n"
"  \n"
);
}

int main(int argc, char *argv[]) {
    struct nfrec *start, *p;

    if (argc == 1) {
        usage();
        exit(0);
    }
    start = nfreadinfo(argv[1]);
    for(p=start; p; p=p->next) {
         if (p->id == -1) {
             printf("%s\n%ld\n", p->txt, p->length);
         } else {
             printf("%ld %ld %ld %ld %ld %s\n",
                    p->id, p->startpage, p->length, p->info1,
                    p->info2, p-> txt);
         }
    }
    return 0;
}
