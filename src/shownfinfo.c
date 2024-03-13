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
"  to be extended by documentation of nf info and nf files ...\n");
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
