/*
cprefresh_x8664inl.h               Copyright frankl 2013-2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Inline version of the X8664 functions in cprefresh.c, used with
'make REFRESH=X8664INL' (included by cprefresh.h, cprefresh.c is then
empty). The assembler kernels contain exactly the instructions of the
corresponding functions in cprefresh_x8664.s (generated from it), as
inline assembler. They are always inlined, also in code compiled with
-O0. 'volatile' and the "memory" clobber make sure that the compiler
never removes them, also with -O2. The bytes at unaligned ends are
written via volatile pointers for the same reason.
*/

#include <string.h>

/* refresh n 8-byte words in place via 16 registers, inline version of
   refresh64_x8664 in cprefresh_x8664.s */
static inline __attribute__((always_inline))
void refresh64_x8664inl(void *addr, long n)
{
  __asm__ __volatile__ (
    "xorq %%rcx, %%rcx\n\t"
    "jmp .fin2%=\n\t"
    ".L2%=:\n\t"
    "movq %%rcx, %%xmm0\n\t"
    "movq %%xmm0, %%xmm1\n\t"
    "movq %%xmm0, %%xmm2\n\t"
    "movq %%xmm0, %%xmm3\n\t"
    "movq %%xmm0, %%xmm4\n\t"
    "movq %%xmm0, %%xmm5\n\t"
    "movq %%xmm0, %%xmm6\n\t"
    "movq %%xmm0, %%xmm7\n\t"
    "movq %%xmm0, %%xmm8\n\t"
    "movq %%xmm0, %%xmm9\n\t"
    "movq %%xmm0, %%xmm10\n\t"
    "movq %%xmm0, %%xmm11\n\t"
    "movq %%xmm0, %%xmm12\n\t"
    "movq %%xmm0, %%xmm13\n\t"
    "movq %%xmm0, %%xmm14\n\t"
    "movq %%xmm0, %%xmm15\n\t"
    "movsd (%%rdi), %%xmm0\n\t"
    "movsd %%xmm1, (%%rdi)\n\t"
    "movq %%xmm0, %%xmm1\n\t"
    "movq %%xmm0, %%xmm2\n\t"
    "movq %%xmm0, %%xmm3\n\t"
    "movq %%xmm0, %%xmm4\n\t"
    "movq %%xmm0, %%xmm5\n\t"
    "movq %%xmm0, %%xmm6\n\t"
    "movq %%xmm0, %%xmm7\n\t"
    "movq %%xmm0, %%xmm8\n\t"
    "movq %%xmm0, %%xmm9\n\t"
    "movq %%xmm0, %%xmm10\n\t"
    "movq %%xmm0, %%xmm11\n\t"
    "movq %%xmm0, %%xmm12\n\t"
    "movq %%xmm0, %%xmm13\n\t"
    "movq %%xmm0, %%xmm14\n\t"
    "movq %%xmm0, %%xmm15\n\t"
    "movq %%xmm7, %%xmm0\n\t"
    "movq %%xmm8, %%xmm1\n\t"
    "movq %%xmm9, %%xmm2\n\t"
    "movq %%xmm10, %%xmm3\n\t"
    "movq %%xmm11, %%xmm4\n\t"
    "movq %%xmm12, %%xmm5\n\t"
    "movq %%xmm13, %%xmm6\n\t"
    "movq %%xmm14, %%xmm7\n\t"
    "movq %%xmm15, %%xmm8\n\t"
    "movq %%xmm0, %%xmm9\n\t"
    "movq %%xmm1, %%xmm10\n\t"
    "movq %%xmm2, %%xmm11\n\t"
    "movq %%xmm3, %%xmm12\n\t"
    "movq %%xmm4, %%xmm13\n\t"
    "movq %%xmm5, %%xmm14\n\t"
    "movq %%xmm6, %%xmm15\n\t"
    "movsd %%xmm0, (%%rdi)\n\t"
    "movsd %%xmm1, (%%rdi)\n\t"
    "movsd %%xmm2, (%%rdi)\n\t"
    "movsd %%xmm3, (%%rdi)\n\t"
    "movsd %%xmm4, (%%rdi)\n\t"
    "movsd %%xmm5, (%%rdi)\n\t"
    "movsd %%xmm6, (%%rdi)\n\t"
    "movsd %%xmm7, (%%rdi)\n\t"
    "movsd %%xmm8, (%%rdi)\n\t"
    "movsd %%xmm9, (%%rdi)\n\t"
    "movsd %%xmm10, (%%rdi)\n\t"
    "movsd %%xmm11, (%%rdi)\n\t"
    "movsd %%xmm12, (%%rdi)\n\t"
    "movsd %%xmm13, (%%rdi)\n\t"
    "movsd %%xmm14, (%%rdi)\n\t"
    "movsd %%xmm15, (%%rdi)\n\t"
    "decq %%rsi\n\t"
    "addq $8, %%rdi\n\t"
    ".fin2%=:\n\t"
    "cmpq %%rsi, %%rcx\n\t"
    "jl .L2%=\n\t"
    : "+D"(addr), "+S"(n)
    :
    : "rcx", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
      "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
      "cc", "memory");
}

/* copy n 8-byte words from addr to dest via 16 registers, inline version of
   cp64_x8664 in cprefresh_x8664.s */
static inline __attribute__((always_inline))
void cp64_x8664inl(void *addr, long n, void *dest)
{
  __asm__ __volatile__ (
    "xorq %%rcx, %%rcx\n\t"
    "jmp .fin3%=\n\t"
    ".L3%=:\n\t"
    "movq %%rcx, %%xmm0\n\t"
    "movq %%xmm0, %%xmm1\n\t"
    "movq %%xmm0, %%xmm2\n\t"
    "movq %%xmm0, %%xmm3\n\t"
    "movq %%xmm0, %%xmm4\n\t"
    "movq %%xmm0, %%xmm5\n\t"
    "movq %%xmm0, %%xmm6\n\t"
    "movq %%xmm0, %%xmm7\n\t"
    "movq %%xmm0, %%xmm8\n\t"
    "movq %%xmm0, %%xmm9\n\t"
    "movq %%xmm0, %%xmm10\n\t"
    "movq %%xmm0, %%xmm11\n\t"
    "movq %%xmm0, %%xmm12\n\t"
    "movq %%xmm0, %%xmm13\n\t"
    "movq %%xmm0, %%xmm14\n\t"
    "movq %%xmm0, %%xmm15\n\t"
    "movsd (%%rdi), %%xmm0\n\t"
    "movsd %%xmm1, (%%rdx)\n\t"
    "movq %%xmm0, %%xmm1\n\t"
    "movq %%xmm0, %%xmm2\n\t"
    "movq %%xmm0, %%xmm3\n\t"
    "movq %%xmm0, %%xmm4\n\t"
    "movq %%xmm0, %%xmm5\n\t"
    "movq %%xmm0, %%xmm6\n\t"
    "movq %%xmm0, %%xmm7\n\t"
    "movq %%xmm0, %%xmm8\n\t"
    "movq %%xmm0, %%xmm9\n\t"
    "movq %%xmm0, %%xmm10\n\t"
    "movq %%xmm0, %%xmm11\n\t"
    "movq %%xmm0, %%xmm12\n\t"
    "movq %%xmm0, %%xmm13\n\t"
    "movq %%xmm0, %%xmm14\n\t"
    "movq %%xmm0, %%xmm15\n\t"
    "movq %%xmm7, %%xmm0\n\t"
    "movq %%xmm8, %%xmm1\n\t"
    "movq %%xmm9, %%xmm2\n\t"
    "movq %%xmm10, %%xmm3\n\t"
    "movq %%xmm11, %%xmm4\n\t"
    "movq %%xmm12, %%xmm5\n\t"
    "movq %%xmm13, %%xmm6\n\t"
    "movq %%xmm14, %%xmm7\n\t"
    "movq %%xmm15, %%xmm8\n\t"
    "movq %%xmm0, %%xmm9\n\t"
    "movq %%xmm1, %%xmm10\n\t"
    "movq %%xmm2, %%xmm11\n\t"
    "movq %%xmm3, %%xmm12\n\t"
    "movq %%xmm4, %%xmm13\n\t"
    "movq %%xmm5, %%xmm14\n\t"
    "movq %%xmm6, %%xmm15\n\t"
    "movsd %%xmm0, (%%rdx)\n\t"
    "movsd %%xmm1, (%%rdx)\n\t"
    "movsd %%xmm2, (%%rdx)\n\t"
    "movsd %%xmm3, (%%rdx)\n\t"
    "movsd %%xmm4, (%%rdx)\n\t"
    "movsd %%xmm5, (%%rdx)\n\t"
    "movsd %%xmm6, (%%rdx)\n\t"
    "movsd %%xmm7, (%%rdx)\n\t"
    "movsd %%xmm8, (%%rdx)\n\t"
    "movsd %%xmm9, (%%rdx)\n\t"
    "movsd %%xmm10, (%%rdx)\n\t"
    "movsd %%xmm11, (%%rdx)\n\t"
    "movsd %%xmm12, (%%rdx)\n\t"
    "movsd %%xmm13, (%%rdx)\n\t"
    "movsd %%xmm14, (%%rdx)\n\t"
    "movsd %%xmm15, (%%rdx)\n\t"
    "decq %%rsi\n\t"
    "addq $8, %%rdi\n\t"
    "addq $8, %%rdx\n\t"
    ".fin3%=:\n\t"
    "cmpq %%rsi, %%rcx\n\t"
    "jl .L3%=\n\t"
    : "+D"(addr), "+S"(n), "+d"(dest)
    :
    : "rcx", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
      "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
      "cc", "memory");
}

/* zero n 8-byte words, inline version of
   memclean64_x8664 in cprefresh_x8664.s */
static inline __attribute__((always_inline))
void memclean64_x8664inl(void *addr, long n)
{
  __asm__ __volatile__ (
    "xorq %%rcx, %%rcx\n\t"
    "movq %%rcx, %%xmm0\n\t"
    "movq %%xmm0, %%xmm1\n\t"
    "movq %%xmm0, %%xmm2\n\t"
    "movq %%xmm0, %%xmm3\n\t"
    "movq %%xmm0, %%xmm4\n\t"
    "movq %%xmm0, %%xmm5\n\t"
    "movq %%xmm0, %%xmm6\n\t"
    "movq %%xmm0, %%xmm7\n\t"
    "movq %%xmm0, %%xmm8\n\t"
    "movq %%xmm0, %%xmm9\n\t"
    "movq %%xmm0, %%xmm10\n\t"
    "movq %%xmm0, %%xmm11\n\t"
    "movq %%xmm0, %%xmm12\n\t"
    "movq %%xmm0, %%xmm13\n\t"
    "movq %%xmm0, %%xmm14\n\t"
    "movq %%xmm0, %%xmm15\n\t"
    "movq %%xmm7, %%xmm0\n\t"
    "movq %%xmm8, %%xmm1\n\t"
    "movq %%xmm9, %%xmm2\n\t"
    "movq %%xmm10, %%xmm3\n\t"
    "movq %%xmm11, %%xmm4\n\t"
    "movq %%xmm12, %%xmm5\n\t"
    "movq %%xmm13, %%xmm6\n\t"
    "movq %%xmm14, %%xmm7\n\t"
    "movq %%xmm15, %%xmm8\n\t"
    "movq %%xmm0, %%xmm9\n\t"
    "movq %%xmm1, %%xmm10\n\t"
    "movq %%xmm2, %%xmm11\n\t"
    "movq %%xmm3, %%xmm12\n\t"
    "movq %%xmm4, %%xmm13\n\t"
    "movq %%xmm5, %%xmm14\n\t"
    "movq %%xmm6, %%xmm15\n\t"
    "jmp .fin4%=\n\t"
    ".L4%=:\n\t"
    "movsd %%xmm0, (%%rdi)\n\t"
    "movsd %%xmm1, (%%rdi)\n\t"
    "movsd %%xmm2, (%%rdi)\n\t"
    "movsd %%xmm3, (%%rdi)\n\t"
    "movsd %%xmm4, (%%rdi)\n\t"
    "movsd %%xmm5, (%%rdi)\n\t"
    "movsd %%xmm6, (%%rdi)\n\t"
    "movsd %%xmm7, (%%rdi)\n\t"
    "movsd %%xmm8, (%%rdi)\n\t"
    "movsd %%xmm9, (%%rdi)\n\t"
    "movsd %%xmm10, (%%rdi)\n\t"
    "movsd %%xmm11, (%%rdi)\n\t"
    "movsd %%xmm12, (%%rdi)\n\t"
    "movsd %%xmm13, (%%rdi)\n\t"
    "movsd %%xmm14, (%%rdi)\n\t"
    "movsd %%xmm15, (%%rdi)\n\t"
    "decq %%rsi\n\t"
    "addq $8, %%rdi\n\t"
    ".fin4%=:\n\t"
    "cmpq %%rsi, %%rcx\n\t"
    "jl .L4%=\n\t"
    : "+D"(addr), "+S"(n)
    :
    : "rcx", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
      "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
      "cc", "memory");
}

/* the functions declared in cprefresh.h, as in cprefresh.c for X8664 */

/* nb is number of bytes */
static inline __attribute__((always_inline))
void refreshmem(char* ptr, long nb)
{
  long n, off;
  void *vp;
  /* ignore first and last bytes if not 8-byte aligned */
  off = (long)ptr % 8;
  if (off != 0){
     off = 8-off;
  }
  vp = (void*)ptr + off;
  n = (nb-off)/8;
  refresh64_x8664inl(vp, n);
}

static inline __attribute__((always_inline))
void refreshmems(char* ptr, long nb, long k)
{
  long n, off, j;
  void *vp;
  /* ignore first and last bytes if not 8-byte aligned */
  off = (long)ptr % 8;
  if (off != 0){
     off = 8-off;
  }
  vp = (void*)ptr + off;
  n = (nb-off)/8;
  for(j=k; j; j--)
      refresh64_x8664inl(vp, n);
}

static inline __attribute__((always_inline))
void memclean(char* ptr, long n)
{
  volatile char *vp = ptr;
  long i, off, n0;
  /* handle first and last bytes separately, if not 8-byte aligned */
  off = (unsigned long)ptr % 8;
  if (off > 0) {
    off = 8-off;
    for (i=0; i < off; i++) vp[i] = 0;
  }
  n0 = (n-off)/8;
  memclean64_x8664inl((void*)(ptr+off), n0);
  n0 *= 8;
  for (; n0+off < n; n0++) vp[n0+off] = 0;
}

static inline __attribute__((always_inline))
void cprefresh(char* dest, char* ptr, long n)
{
  volatile char *vd = dest;
  long n0;
  if (((unsigned long)ptr % 8) != 0 || ((unsigned long)dest % 8) != 0) {
      memclean(dest, n);
      memcpy(dest, ptr, n);
      refreshmem(dest, n);
  } else {
      /* both pointers must be 8-byte aligned */
      n0 = n/8;
      cp64_x8664inl((void*)ptr, n0, (void*)dest);
      n0 *= 8;
      for (; n0 < n; n0++) vd[n0] = ptr[n0];
  }
}

