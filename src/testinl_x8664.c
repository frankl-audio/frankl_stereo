/*
testinl_x8664.c               Copyright frankl 2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Test of cprefresh_x8664inl.h (REFRESH=X8664INL) on x86_64: compares the
inline functions with the functions in cprefresh_x8664.s and the
wrappers from cprefresh.c, for random data, many lengths and all
alignments, with guard bytes around the buffers. Built by
'make testinl' with -O2 and -O0.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cprefresh.h"

/* the functions in cprefresh_x8664.s */
void refresh64_x8664(void* addr, int n);
void cp64_x8664(void* addr, int n, void* dest);
void memclean64_x8664(void* addr, int n);

/* the wrappers from cprefresh.c for X8664, with the functions above */
static void ref_refreshmem(char* ptr, long nb) {
  long n, off;
  void *vp;
  off = (long)ptr % 8;
  if (off != 0){
     off = 8-off;
  }
  vp = (void*)ptr + off;
  n = (nb-off)/8;
  refresh64_x8664(vp, n);
}

static void ref_refreshmems(char* ptr, long nb, long k) {
  long n, off, j;
  void *vp;
  off = (long)ptr % 8;
  if (off != 0){
     off = 8-off;
  }
  vp = (void*)ptr + off;
  n = (nb-off)/8;
  for(j=k; j; j--)
      refresh64_x8664(vp, n);
}

static void ref_memclean(char* ptr, long n)
{
  int i, off, n0;
  off = (unsigned long)ptr % 8;
  if (off > 0) {
    off = 8-off;
    for (i=0; i < off; i++) ptr[i] = 0;
  }
  n0 = (n-off)/8;
  memclean64_x8664((void*)(ptr+off), n0);
  n0 *= 8;
  for (; n0+off < n; n0++) ptr[n0+off] = 0;
}

static void ref_cprefresh(char* dest, char* ptr, long n)
{
  int n0;
  if (((unsigned long)ptr % 8) != 0 || ((unsigned long)dest % 8) != 0) {
      ref_memclean(dest, n);
      memcpy(dest, ptr, n);
      ref_refreshmem(dest, n);
  } else {
      n0 = n/8;
      cp64_x8664((void*)ptr, n0, (void*)dest);
      n0 *= 8;
      for (; n0 < n; n0++) dest[n0] = ptr[n0];
  }
}

#define G 64        /* guard bytes */
#define MAXN 300
#define BS (MAXN + 2*G + 16)

static char A[BS] __attribute__((aligned(16)));
static char B[BS] __attribute__((aligned(16)));
static char S[BS] __attribute__((aligned(16)));
static char D[BS] __attribute__((aligned(16)));
static long nerr = 0, ntests = 0;

static void fill(char *p) { int i; for (i = 0; i < BS; i++) p[i] = rand(); }

static void check(const char *what, long n, int a1, int a2)
{
  ntests++;
  if (memcmp(A, B, BS) != 0) {
    if (nerr < 20)
      fprintf(stderr, "ERROR: %s n=%ld align=%d,%d\n", what, n, a1, a2);
    nerr++;
  }
}

int main()
{
  long n, k;
  int a, b;

  srand(12345);
  for (n = 0; n <= MAXN; n++) {
    for (a = 0; a < 16; a++) {
      /* memclean */
      fill(A); memcpy(B, A, BS);
      memclean(A+G+a, n); ref_memclean(B+G+a, n);
      check("memclean", n, a, 0);
      /* refreshmem */
      fill(A); memcpy(B, A, BS);
      refreshmem(A+G+a, n); ref_refreshmem(B+G+a, n);
      check("refreshmem", n, a, 0);
      /* refreshmems */
      for (k = 0; k < 3; k++) {
        fill(A); memcpy(B, A, BS);
        refreshmems(A+G+a, n, k); ref_refreshmems(B+G+a, n, k);
        check("refreshmems", n, a, (int)k);
      }
      /* cprefresh */
      for (b = 0; b < 16; b++) {
        fill(S); fill(D); memcpy(A, D, BS); memcpy(B, D, BS);
        cprefresh(A+G+b, S+G+a, n); ref_cprefresh(B+G+b, S+G+a, n);
        check("cprefresh", n, a, b);
        if (n > 0 && memcmp(A+G+b, S+G+a, n) != 0) {
          if (nerr < 20)
            fprintf(stderr, "ERROR: cprefresh copy n=%ld align=%d,%d\n", n, a, b);
          nerr++;
        }
      }
    }
  }
  /* the kernels directly, on aligned data */
  for (n = 0; n <= MAXN/8; n++) {
    fill(A); memcpy(B, A, BS);
    memclean64_x8664inl(A+G, n); memclean64_x8664(B+G, n);
    check("memclean64", n, 0, 0);
    fill(A); memcpy(B, A, BS);
    refresh64_x8664inl(A+G, n); refresh64_x8664(B+G, n);
    check("refresh64", n, 0, 0);
    fill(S); fill(A); memcpy(B, A, BS);
    cp64_x8664inl(S+G, n, A+G); cp64_x8664(S+G, n, B+G);
    check("cp64", n, 0, 0);
  }
  printf("%ld tests, %ld errors\n", ntests, nerr);
  return nerr != 0;
}
