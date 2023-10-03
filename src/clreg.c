/*
gcc -c -O0 cprefresh_aa64.s
gcc -o clreg -O0 clreg.c cprefresh_aa64.o
*/

#include <time.h>

extern void cleanregs_aa64();

int main() {
  int i;
  struct timespec mtime;
  mtime.tv_sec = 4;
  mtime.tv_nsec = 0;
  for (;;) {
    for(i=0;i<1000;i++)
        cleanregs_aa64();
    clock_nanosleep(CLOCK_MONOTONIC, 0, &mtime, NULL);
  }
}
