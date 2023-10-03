/*
gcc -c -O0 cprefresh_x8664.s
gcc -o clreg86 -O0 -z execstack clreg86.c cprefresh_x8664.o
*/

#include <time.h>

extern void cleanregs_x8664();

int main() {
  int i;
  struct timespec mtime;
  mtime.tv_sec = 0;
  mtime.tv_nsec = 10000000;
  for (;;) {
    for(i=0;i<1000;i++)
        cleanregs_x8664();
    clock_nanosleep(CLOCK_MONOTONIC, 0, &mtime, NULL);
  }
}
