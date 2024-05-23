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
   int32_t out[256], tout[256], *op;
   int i, mlen, nrcp;
   struct timespec mtime;

   nrcp = 0;
   if (argc == 2) {
       nrcp = atoi(argv[1]);
       if (nrcp < 0 || nrcp > 5000)
           nrcp = 0;
   }
   if (clock_gettime(CLOCK_MONOTONIC, &mtime) < 0) {
        exit(1);
   }

   while (1) {
      memclean((char*)inp, 8192);  
      mlen = fread((void*)inp, sizeof(double), 1024, stdin);
      if (mlen == 0) break;
      memclean((char*)out, 1024);
      for (i=mlen/8, ip=inp, op=out; i; ip+=8, op+=2, i--) {
        *op = (int32_t) (*ip * 2147483647);
        *(op+1) = (int32_t) (*(ip+1) * 2147483647);
      }
      clock_gettime(CLOCK_MONOTONIC, &mtime);
      for(i=nrcp; i; i--) {
          mtime.tv_nsec += 150000;
          if (mtime.tv_nsec > 999999999) {
            mtime.tv_nsec -= 1000000000;
            mtime.tv_sec++;
          }
          memclean((char*)tout, mlen);
          cprefresh((char*)tout, (char*)out, mlen);
          clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &mtime, NULL);
          memclean((char*)out, mlen);
          cprefresh((char*)out, (char*)tout, mlen);
      }
      fwrite((void*)out, sizeof(int32_t), mlen/4, stdout);
   }
   return 0;
}
