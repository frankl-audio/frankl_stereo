/* reads 2-channel audio in 64 bit float samples and output every
   fourth frame as int32 samples */
/* we use this to reduce sample rate 192000 to 48000 with a signal
   that has no content above 24kHz */
#include "version.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include "cprefresh.h"

int main(int argc, char *argv[])
{
    double inp[1024], *ip;
    int32_t out[256], *op, pm = 1;
    int i, mlen, nrcp, nz, post;
    void *tbufs[1024];

    nrcp = 0;
    if (argc == 2) {
        nrcp = atoi(argv[1]);
        if (nrcp < 0 || nrcp > 5000)
            nrcp = 0;
    }
    if (nrcp) {
        /* temporary buffers */
        for (i=1; i < nrcp; i++) {
            if (posix_memalign(tbufs+i, 4096, 4096)) {
                fprintf(stderr, "cptoshm: Cannot allocate buffer for cleaning.\n");
                exit(2);
            }
        }
    }
    tbufs[0] = out;
    tbufs[nrcp] = out;

    /* pre- and append 5 sec of silence */
    nz = 1875;
    mlen = 1024;
    post = 0;
    while (1) {
       memclean((char*)inp, 8192);  
       if (nz) {
          for (i = 0; i< 1024; i++) ((double *)inp)[i] = 0.0;
          nz--;
       } else if (post) break;
       else {
          mlen = fread((void*)inp, sizeof(double), 1024, stdin);
       }
       if (mlen == 0) {
          post = 1;
          for (i = 0; i< 1024; i++) ((double *)inp)[i] = 0.0;
          nz = 1874;
          mlen = 1024;
       }
       memclean((char*)out, 1024);
       for (i=mlen/8, ip=inp, op=out; i; ip+=8, op+=2, i--) {
         *op = (int32_t) (*ip * 2147483647);
         *(op+1) = (int32_t) (*(ip+1) * 2147483647);
         /* avoid zero samples on left channel */
         if (*op == 0) {
             *op = pm;
             pm = -pm;
         }
       }
       if (nrcp) {
           for (i=1; i <= nrcp; i++) {
               memclean((char*)(tbufs[i]), mlen);
               cprefresh((char*)(tbufs[i]), (char*)(tbufs[i-1]), mlen);
               memclean((char*)(tbufs[i-1]), mlen);
           }
       }
       fwrite((void*)out, sizeof(int32_t), mlen/4, stdout);
    }
    return 0;
}
