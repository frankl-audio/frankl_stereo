/*
cprefresh.c                Copyright frankl 2013-2024

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Here are some utility functions to clean or refresh RAM.
Optionally, they use assembler functions on ARM.
*/

#include <string.h>

#ifdef REFRESHVFP
/* on ARM with some version of the CPU feature 'vfp' */
inline void memcp_vfpX(void*, void*, int);
inline void memclean_vfpX(void*, int);

inline void refreshmem(char* ptr, long n)
{
  /* we make shure below that we can access ptr-off */
  int off;
  off = (unsigned int)ptr % 8;
  memcp_vfpX((void*)(ptr-off), (void*)(ptr-off), ((n+off)/8)*8);
}
inline void refreshmems(char* ptr, long n, int k)
{
  /* we make shure below that we can access ptr-off */
  int off, j;
  off = (unsigned int)ptr % 8;
  for(j=k; j; j--)
      memcp_vfpX((void*)(ptr-off), (void*)(ptr-off), ((n+off)/8)*8);
}
inline void memclean(char* ptr, long n)
{
  int i, off, n0;
  off = (unsigned int)ptr % 8;
  if (off > 0) {
    off = 8-off;
    for (i=0; i < off; i++) ptr[i] = 0;
  }
  n0 = ((n-off)/8)*8;
  memclean_vfpX((void*)(ptr+off), n0);
  for (; n0+off < n; n0++) ptr[n0+off] = 0;
}

inline void cprefresh(char* dest, char* ptr, long n)
{
  memclean(dest, n);
  memcpy(dest, ptr, n);
  refreshmem(dest, n);
}


#else
#ifdef REFRESHARM
/* on ARM with no CPU feature 'neon' or 'vfp', currently disabled, we use C-version instead */
inline void memcp_regX(void*, void*, int);
inline void memclean_reg(void*, int);

inline void refreshmem(char* ptr, long n)
{
  /* we make shure below that we can access ptr-off */
  int off;
  off = (unsigned int)ptr % 4;
  memcp_regX((void*)(ptr-off), (void*)(ptr-off), ((n+off)/4)*4);
}
inline void refreshmems(char* ptr, long n, int k)
{
  /* we make shure below that we can access ptr-off */
  int off, j;
  off = (unsigned int)ptr % 4;
  for(j=k; j; j--)
      memcp_regX((void*)(ptr-off), (void*)(ptr-off), ((n+off)/4)*4);
}
inline void memclean(char* ptr, long n)
{
  int i, off, n0;
  off = (unsigned int)ptr % 4;
  if (off > 0) {
    off = 4-off;
    for (i=0; i < off; i++) ptr[i] = 0;
  }
  n0 = ((n-off)/4)*4;
  memclean_reg((void*)(ptr+off), n0);
  for (; n0+off < n; n0++) ptr[n0+off] = 0;
}

inline void cprefresh(char* dest, char* ptr, long n)
{
  memclean(dest, n);
  memcpy(dest, ptr, n);
  refreshmem(dest, n);
}
#else
#ifdef REFRESHAA64
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* n is number of 8 byte quad words */
inline void refreshmem_aa64(void* addr, int n);
inline void memclean_aa64(void* addr, int n);
inline void cprefresh_aa64(void* addr, int n, void* dest);
inline void refresh64bit_aa64(void* addr, int n);
inline void cp64bit_aa64(void* addr, int n, void* dest);
inline void clean64bit_aa64(void* addr, int n);

/* nb is number of bytes */
inline void refreshmem(char* ptr, long nb) {
  long n, off;
  void *vp;
  off = (long)ptr % 8;
  if (off != 0){
     vp = (void*)ptr + 8 - off;
  } else {
     vp = (void*)ptr;
  }
  n = (nb-off)/8;
  //refreshmem_aa64(vp, n);
  refresh64bit_aa64(vp, n);
}

inline void refreshmems(char* ptr, long nb, long k) {
  long n, off, j;
  void *vp;
  off = (long)ptr % 8;
  if (off != 0){
     vp = (void*)ptr + 8 - off;
  } else {
     vp = (void*)ptr;
  }
  n = (nb-off)/8;
  for(j=k; j; j--)
      refreshmem_aa64(vp, n);
}

inline void memclean(char* ptr, long n)
{
  int i, off, n0;
  off = (unsigned long)ptr % 8;
  if (off > 0) {
    off = 8-off;
    for (i=0; i < off; i++) ptr[i] = 0;
  }
  n0 = (n-off)/8;
  clean64bit_aa64((void*)(ptr+off), n0);
  n0 *= 8;
  for (; n0+off < n; n0++) ptr[n0+off] = 0;
}

inline void cprefresh(char* dest, char* ptr, long n)
{
  int n0;
  if (((unsigned long)ptr % 8) != 0 || ((unsigned long)dest % 8) != 0) {
      memclean(dest, n);
      memcpy(dest, ptr, n);
      refreshmem(dest, n);
  } else {
      /* both pointers must get 8-byte aligned */
      n0 = n/8;
      //cprefresh_aa64((void*)ptr, n0, (void*)dest);
      cp64bit_aa64((void*)ptr, n0, (void*)dest);
      n0 *= 8;
      for (; n0 < n; n0++) dest[n0] = ptr[n0];
  }
}

#else
#ifdef REFRESHX8664
#include <stdint.h>

/* n is number of 8 byte quad words */
inline void refreshmem_x8664(void* addr, int n);
inline void refresh64_x8664(void* addr, int n);
inline void cp64_x8664(void* addr, int n, void* dest);
inline void memclean64_x8664(void* addr, int n);

/* nb is number of bytes */
inline void refreshmem(char* ptr, long nb) {
  long n, off;
  void *vp;
  off = (long)ptr % 8;
  if (off != 0){
     vp = (void*)ptr + 8 - off;
  } else {
     vp = (void*)ptr;
  }
  n = (nb-off)/8;
  //refreshmem_x8664(vp, n);
  refresh64_x8664(vp, n);
}
inline void refreshmems(char* ptr, long nb, long k) {
  long n, off, j;
  void *vp;
  off = (long)ptr % 8;
  if (off != 0){
     vp = (void*)ptr + 8 - off;
  } else {
     vp = (void*)ptr;
  }
  n = (nb-off)/8;
  for(j=k; j; j--)
      //refreshmem_x8664(vp, n);
      refresh64_x8664(vp, n);
}

inline void memclean(char* ptr, long nb) {
  long n, off;
  void *vp;
  char *tp;
  off = (long)ptr % 8;
  if (off != 0){
     vp = (void*)ptr + 8 - off;
     for (tp=ptr; tp<(char*)vp; tp++)
        *tp = 0;
  } else {
     vp = (void*)ptr;
  }
  n = (nb-off)/8;
  //refreshmem_x8664(vp, n);
  memclean64_x8664(vp, n);
  for (tp=ptr+(off+8*n); tp<ptr+nb; tp++)
     *tp = 0;
}

inline void cprefresh(char* dest, char* ptr, long n)
{
  if ((long)ptr % 8 !=0 || (long)dest % 8 != 0 || n % 8 != 0) {
      memclean(dest, n);
      memcpy(dest, ptr, n);
      refreshmem(dest, n);
  } else {
      cp64_x8664((void*)ptr, n/8, (void*)dest);
  }
}
#else
#include <stdint.h>
/* default version in C, compile with -O0, such that this is not
   optimized away */
inline void refreshmem(char* ptr, long n)
{
  /* we make shure below that we can access ptr-off */
  long i, sz, off;
  unsigned int x, y, d, *up;
  sz = sizeof(unsigned int);
  off = (uintptr_t)ptr % sz;
  for(i=0, up=(unsigned int*)(ptr-off); i < (n+off)/sz; i++) {
      x = *up;
      d = 0xFFFFFFFF;
      y = x ^ d;
      *up = 0x0;
      *up = y ^ d;
      up++;
  }
}
inline void refreshmems(char* ptr, long n, long k)
{
  long j;
  for(j=k; j; j--)
      refreshmem(ptr, n);
}

inline void memclean(char* ptr, long n)
{
  long i, off, n0, sz;
  unsigned int *up;
  sz = sizeof(int);
  off = (uintptr_t)ptr % sz;
  if (off > 0) {
    off = sz-off;
    for (i=0; i < off; i++) ptr[i] = 0;
  }
  n0 = (n-off)/sz;
  for(up = (unsigned int*)(ptr+off), i=0; i < n0; i++) {
      *up = 0xFFFFFFFF;
      *up = 0;
      *up = 0;
      up++;
  }
  n0 *= sz;
  for (; n0+off < n; n0++) ptr[n0+off] = 0;
}

inline void cprefresh(char* dest, char* ptr, long n)
{
  memclean(dest, n);
  memcpy(dest, ptr, n);
  refreshmem(dest, n);
}

#endif
#endif
#endif
#endif




