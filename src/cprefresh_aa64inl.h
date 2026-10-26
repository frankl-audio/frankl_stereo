/*
cprefresh_aa64inl.h                Copyright frankl 2013-2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Inline version of the AA64 functions in cprefresh.c, used with
'make REFRESH=AA64INL' (included by cprefresh.h, cprefresh.c is then
empty). The assembler kernels contain exactly the instructions of the
corresponding functions in cprefresh_aa64.s (generated from it), as
inline assembler. They are always inlined, also in code compiled with
-O0. 'volatile' and the "memory" clobber make sure that the compiler
never removes them, also with -O2. The bytes at unaligned ends are
written via volatile pointers for the same reason.
*/

#include <string.h>

/* refresh n 8-byte words in place (8 times each, via d11), inline version of refreshmem_aa64 in cprefresh_aa64.s */
static inline __attribute__((always_inline))
void refreshmem_aa64inl(void *addr, long n)
{
  register void *rx0 __asm__("x0") = addr;
  register long rx1 __asm__("x1") = n;
  __asm__ __volatile__ (
    "mov x4, x0\n\t"
    "mov x5, #1\n\t"
    ".LOOP%=:\n\t"
    "cmp x1, x5\n\t"
    "blt .RET%=\n\t"
    "orr v11.8b, v11.8b, v11.8b\n\t"
    "ldr d11, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str d11, [x4]\n\t"
    "orr v11.8b, v11.8b, v11.8b\n\t"
    "ldr d11, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str d11, [x4]\n\t"
    "orr v11.8b, v11.8b, v11.8b\n\t"
    "ldr d11, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str d11, [x4]\n\t"
    "orr v11.8b, v11.8b, v11.8b\n\t"
    "ldr d11, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str d11, [x4]\n\t"
    "orr v11.8b, v11.8b, v11.8b\n\t"
    "ldr d11, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str d11, [x4]\n\t"
    "orr v11.8b, v11.8b, v11.8b\n\t"
    "ldr d11, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str d11, [x4]\n\t"
    "orr v11.8b, v11.8b, v11.8b\n\t"
    "ldr d11, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str d11, [x4]\n\t"
    "orr v11.8b, v11.8b, v11.8b\n\t"
    "ldr d11, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str d11, [x4]\n\t"
    "add x4, x4, 8\n\t"
    "add x5, x5, 1\n\t"
    "b .LOOP%=\n\t"
    ".RET%=:\n\t"
    :
    : "r"(rx0), "r"(rx1)
    : "x4", "x5", "v11", "cc", "memory");
}

/* refresh n 8-byte words in place via 24 registers, inline version of refresh64bit_aa64 in cprefresh_aa64.s */
static inline __attribute__((always_inline))
void refresh64bit_aa64inl(void *addr, long n)
{
  register void *rx0 __asm__("x0") = addr;
  register long rx1 __asm__("x1") = n;
  __asm__ __volatile__ (
    "mov x4, x0\n\t"
    "mov x5, #1\n\t"
    ".SLOOP%=:\n\t"
    "cmp x1, x5\n\t"
    "blt .SRET%=\n\t"
    "eor v0.8b, v0.8b, v0.8b\n\t"
    "eor v1.8b, v1.8b, v1.8b\n\t"
    "eor v2.8b, v2.8b, v2.8b\n\t"
    "eor v3.8b, v3.8b, v3.8b\n\t"
    "eor v4.8b, v4.8b, v4.8b\n\t"
    "eor v5.8b, v5.8b, v5.8b\n\t"
    "eor v6.8b, v6.8b, v6.8b\n\t"
    "eor v7.8b, v7.8b, v7.8b\n\t"
    "eor v16.8b, v16.8b, v16.8b\n\t"
    "eor v17.8b, v17.8b, v17.8b\n\t"
    "eor v18.8b, v18.8b, v18.8b\n\t"
    "eor v19.8b, v19.8b, v19.8b\n\t"
    "eor v20.8b, v20.8b, v20.8b\n\t"
    "eor v21.8b, v21.8b, v21.8b\n\t"
    "eor v22.8b, v22.8b, v22.8b\n\t"
    "eor v23.8b, v23.8b, v23.8b\n\t"
    "eor v24.8b, v24.8b, v24.8b\n\t"
    "eor v25.8b, v25.8b, v25.8b\n\t"
    "eor v26.8b, v26.8b, v26.8b\n\t"
    "eor v27.8b, v27.8b, v27.8b\n\t"
    "eor v28.8b, v28.8b, v28.8b\n\t"
    "eor v29.8b, v29.8b, v29.8b\n\t"
    "eor v30.8b, v30.8b, v30.8b\n\t"
    "eor v31.8b, v31.8b, v31.8b\n\t"
    "ldr d0, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "str xzr, [x4]\n\t"
    "orr v0.8b, v0.8b, v0.8b\n\t"
    "orr v1.8b, v0.8b, v0.8b\n\t"
    "orr v2.8b, v0.8b, v0.8b\n\t"
    "orr v3.8b, v0.8b, v0.8b\n\t"
    "orr v4.8b, v0.8b, v0.8b\n\t"
    "orr v5.8b, v0.8b, v0.8b\n\t"
    "orr v6.8b, v0.8b, v0.8b\n\t"
    "orr v7.8b, v0.8b, v0.8b\n\t"
    "orr v16.8b, v0.8b, v0.8b\n\t"
    "orr v17.8b, v0.8b, v0.8b\n\t"
    "orr v18.8b, v0.8b, v0.8b\n\t"
    "orr v19.8b, v0.8b, v0.8b\n\t"
    "orr v20.8b, v0.8b, v0.8b\n\t"
    "orr v21.8b, v0.8b, v0.8b\n\t"
    "orr v22.8b, v0.8b, v0.8b\n\t"
    "orr v23.8b, v0.8b, v0.8b\n\t"
    "orr v24.8b, v0.8b, v0.8b\n\t"
    "orr v25.8b, v0.8b, v0.8b\n\t"
    "orr v26.8b, v0.8b, v0.8b\n\t"
    "orr v27.8b, v0.8b, v0.8b\n\t"
    "orr v28.8b, v0.8b, v0.8b\n\t"
    "orr v29.8b, v0.8b, v0.8b\n\t"
    "orr v30.8b, v0.8b, v0.8b\n\t"
    "orr v31.8b, v0.8b, v0.8b\n\t"
    "orr v0.8b, v0.8b, v31.8b\n\t"
    "orr v1.8b, v1.8b, v0.8b\n\t"
    "orr v2.8b, v2.8b, v1.8b\n\t"
    "orr v3.8b, v3.8b, v2.8b\n\t"
    "orr v4.8b, v4.8b, v3.8b\n\t"
    "orr v5.8b, v5.8b, v4.8b\n\t"
    "orr v6.8b, v6.8b, v5.8b\n\t"
    "orr v7.8b, v7.8b, v6.8b\n\t"
    "orr v16.8b, v16.8b, v7.8b\n\t"
    "orr v17.8b, v17.8b, v16.8b\n\t"
    "orr v18.8b, v18.8b, v17.8b\n\t"
    "orr v19.8b, v19.8b, v18.8b\n\t"
    "orr v20.8b, v20.8b, v19.8b\n\t"
    "orr v21.8b, v21.8b, v20.8b\n\t"
    "orr v22.8b, v22.8b, v21.8b\n\t"
    "orr v23.8b, v23.8b, v22.8b\n\t"
    "orr v24.8b, v24.8b, v23.8b\n\t"
    "orr v25.8b, v25.8b, v24.8b\n\t"
    "orr v26.8b, v26.8b, v25.8b\n\t"
    "orr v27.8b, v27.8b, v26.8b\n\t"
    "orr v28.8b, v28.8b, v27.8b\n\t"
    "orr v29.8b, v29.8b, v28.8b\n\t"
    "orr v30.8b, v30.8b, v29.8b\n\t"
    "orr v31.8b, v31.8b, v30.8b\n\t"
    "orr v0.8b, v0.8b, v16.8b\n\t"
    "orr v1.8b, v1.8b, v17.8b\n\t"
    "orr v2.8b, v2.8b, v18.8b\n\t"
    "orr v3.8b, v3.8b, v19.8b\n\t"
    "orr v4.8b, v4.8b, v20.8b\n\t"
    "orr v5.8b, v5.8b, v21.8b\n\t"
    "orr v6.8b, v6.8b, v22.8b\n\t"
    "orr v7.8b, v7.8b, v23.8b\n\t"
    "orr v16.8b, v16.8b, v24.8b\n\t"
    "orr v17.8b, v17.8b, v25.8b\n\t"
    "orr v18.8b, v18.8b, v26.8b\n\t"
    "orr v19.8b, v19.8b, v27.8b\n\t"
    "orr v20.8b, v20.8b, v28.8b\n\t"
    "orr v21.8b, v21.8b, v29.8b\n\t"
    "orr v22.8b, v22.8b, v30.8b\n\t"
    "orr v23.8b, v23.8b, v31.8b\n\t"
    "orr v24.8b, v24.8b, v0.8b\n\t"
    "orr v25.8b, v25.8b, v1.8b\n\t"
    "orr v26.8b, v26.8b, v2.8b\n\t"
    "orr v27.8b, v27.8b, v3.8b\n\t"
    "orr v28.8b, v28.8b, v4.8b\n\t"
    "orr v29.8b, v29.8b, v5.8b\n\t"
    "orr v30.8b, v30.8b, v6.8b\n\t"
    "orr v31.8b, v31.8b, v7.8b\n\t"
    "str xzr, [x4]\n\t"
    "str d0, [x4]\n\t"
    "str d1, [x4]\n\t"
    "str d2, [x4]\n\t"
    "str d3, [x4]\n\t"
    "str d4, [x4]\n\t"
    "str d5, [x4]\n\t"
    "str d6, [x4]\n\t"
    "str d7, [x4]\n\t"
    "str d16, [x4]\n\t"
    "str d17, [x4]\n\t"
    "str d18, [x4]\n\t"
    "str d19, [x4]\n\t"
    "str d20, [x4]\n\t"
    "str d21, [x4]\n\t"
    "str d22, [x4]\n\t"
    "str d23, [x4]\n\t"
    "str d24, [x4]\n\t"
    "str d25, [x4]\n\t"
    "str d26, [x4]\n\t"
    "str d27, [x4]\n\t"
    "str d28, [x4]\n\t"
    "str d29, [x4]\n\t"
    "str d30, [x4]\n\t"
    "str d31, [x4]\n\t"
    "str d0, [x4]\n\t"
    "str d1, [x4]\n\t"
    "str d2, [x4]\n\t"
    "str d3, [x4]\n\t"
    "str d4, [x4]\n\t"
    "str d5, [x4]\n\t"
    "str d6, [x4]\n\t"
    "str d7, [x4]\n\t"
    "str d16, [x4]\n\t"
    "str d17, [x4]\n\t"
    "str d18, [x4]\n\t"
    "str d19, [x4]\n\t"
    "str d20, [x4]\n\t"
    "str d21, [x4]\n\t"
    "str d22, [x4]\n\t"
    "str d23, [x4]\n\t"
    "str d24, [x4]\n\t"
    "str d25, [x4]\n\t"
    "str d26, [x4]\n\t"
    "str d27, [x4]\n\t"
    "str d28, [x4]\n\t"
    "str d29, [x4]\n\t"
    "str d30, [x4]\n\t"
    "str d31, [x4]\n\t"
    "add x4, x4, 8\n\t"
    "add x5, x5, 1\n\t"
    "b .SLOOP%=\n\t"
    ".SRET%=:\n\t"
    :
    : "r"(rx0), "r"(rx1)
    : "x4", "x5", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v16",
      "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26",
      "v27", "v28", "v29", "v30", "v31", "cc", "memory");
}

/* copy n 8-byte words from addr to dest via 24 registers, inline version of cp64bit_aa64 in cprefresh_aa64.s */
static inline __attribute__((always_inline))
void cp64bit_aa64inl(void *addr, long n, void *dest)
{
  register void *rx0 __asm__("x0") = addr;
  register long rx1 __asm__("x1") = n;
  register void *rx2 __asm__("x2") = dest;
  __asm__ __volatile__ (
    "mov x4, x0\n\t"
    "mov x6, x2\n\t"
    "mov x5, #1\n\t"
    ".TLOOP%=:\n\t"
    "cmp x1, x5\n\t"
    "blt .TRET%=\n\t"
    "eor v0.8b, v0.8b, v0.8b\n\t"
    "eor v1.8b, v1.8b, v1.8b\n\t"
    "eor v2.8b, v2.8b, v2.8b\n\t"
    "eor v3.8b, v3.8b, v3.8b\n\t"
    "eor v4.8b, v4.8b, v4.8b\n\t"
    "eor v5.8b, v5.8b, v5.8b\n\t"
    "eor v6.8b, v6.8b, v6.8b\n\t"
    "eor v7.8b, v7.8b, v7.8b\n\t"
    "eor v16.8b, v16.8b, v16.8b\n\t"
    "eor v17.8b, v17.8b, v17.8b\n\t"
    "eor v18.8b, v18.8b, v18.8b\n\t"
    "eor v19.8b, v19.8b, v19.8b\n\t"
    "eor v20.8b, v20.8b, v20.8b\n\t"
    "eor v21.8b, v21.8b, v21.8b\n\t"
    "eor v22.8b, v22.8b, v22.8b\n\t"
    "eor v23.8b, v23.8b, v23.8b\n\t"
    "eor v24.8b, v24.8b, v24.8b\n\t"
    "eor v25.8b, v25.8b, v25.8b\n\t"
    "eor v26.8b, v26.8b, v26.8b\n\t"
    "eor v27.8b, v27.8b, v27.8b\n\t"
    "eor v28.8b, v28.8b, v28.8b\n\t"
    "eor v29.8b, v29.8b, v29.8b\n\t"
    "eor v30.8b, v30.8b, v30.8b\n\t"
    "eor v31.8b, v31.8b, v31.8b\n\t"
    "ldr d0, [x4]\n\t"
    "str xzr, [x6]\n\t"
    "str xzr, [x6]\n\t"
    "orr v0.8b, v0.8b, v0.8b\n\t"
    "orr v1.8b, v0.8b, v0.8b\n\t"
    "orr v2.8b, v0.8b, v0.8b\n\t"
    "orr v3.8b, v0.8b, v0.8b\n\t"
    "orr v4.8b, v0.8b, v0.8b\n\t"
    "orr v5.8b, v0.8b, v0.8b\n\t"
    "orr v6.8b, v0.8b, v0.8b\n\t"
    "orr v7.8b, v0.8b, v0.8b\n\t"
    "orr v16.8b, v0.8b, v0.8b\n\t"
    "orr v17.8b, v0.8b, v0.8b\n\t"
    "orr v18.8b, v0.8b, v0.8b\n\t"
    "orr v19.8b, v0.8b, v0.8b\n\t"
    "orr v20.8b, v0.8b, v0.8b\n\t"
    "orr v21.8b, v0.8b, v0.8b\n\t"
    "orr v22.8b, v0.8b, v0.8b\n\t"
    "orr v23.8b, v0.8b, v0.8b\n\t"
    "orr v24.8b, v0.8b, v0.8b\n\t"
    "orr v25.8b, v0.8b, v0.8b\n\t"
    "orr v26.8b, v0.8b, v0.8b\n\t"
    "orr v27.8b, v0.8b, v0.8b\n\t"
    "orr v28.8b, v0.8b, v0.8b\n\t"
    "orr v29.8b, v0.8b, v0.8b\n\t"
    "orr v30.8b, v0.8b, v0.8b\n\t"
    "orr v31.8b, v0.8b, v0.8b\n\t"
    "orr v0.8b, v0.8b, v31.8b\n\t"
    "orr v1.8b, v1.8b, v0.8b\n\t"
    "orr v2.8b, v2.8b, v1.8b\n\t"
    "orr v3.8b, v3.8b, v2.8b\n\t"
    "orr v4.8b, v4.8b, v3.8b\n\t"
    "orr v5.8b, v5.8b, v4.8b\n\t"
    "orr v6.8b, v6.8b, v5.8b\n\t"
    "orr v7.8b, v7.8b, v6.8b\n\t"
    "orr v16.8b, v16.8b, v7.8b\n\t"
    "orr v17.8b, v17.8b, v16.8b\n\t"
    "orr v18.8b, v18.8b, v17.8b\n\t"
    "orr v19.8b, v19.8b, v18.8b\n\t"
    "orr v20.8b, v20.8b, v19.8b\n\t"
    "orr v21.8b, v21.8b, v20.8b\n\t"
    "orr v22.8b, v22.8b, v21.8b\n\t"
    "orr v23.8b, v23.8b, v22.8b\n\t"
    "orr v24.8b, v24.8b, v23.8b\n\t"
    "orr v25.8b, v25.8b, v24.8b\n\t"
    "orr v26.8b, v26.8b, v25.8b\n\t"
    "orr v27.8b, v27.8b, v26.8b\n\t"
    "orr v28.8b, v28.8b, v27.8b\n\t"
    "orr v29.8b, v29.8b, v28.8b\n\t"
    "orr v30.8b, v30.8b, v29.8b\n\t"
    "orr v31.8b, v31.8b, v30.8b\n\t"
    "orr v0.8b, v0.8b, v16.8b\n\t"
    "orr v1.8b, v1.8b, v17.8b\n\t"
    "orr v2.8b, v2.8b, v18.8b\n\t"
    "orr v3.8b, v3.8b, v19.8b\n\t"
    "orr v4.8b, v4.8b, v20.8b\n\t"
    "orr v5.8b, v5.8b, v21.8b\n\t"
    "orr v6.8b, v6.8b, v22.8b\n\t"
    "orr v7.8b, v7.8b, v23.8b\n\t"
    "orr v16.8b, v16.8b, v24.8b\n\t"
    "orr v17.8b, v17.8b, v25.8b\n\t"
    "orr v18.8b, v18.8b, v26.8b\n\t"
    "orr v19.8b, v19.8b, v27.8b\n\t"
    "orr v20.8b, v20.8b, v28.8b\n\t"
    "orr v21.8b, v21.8b, v29.8b\n\t"
    "orr v22.8b, v22.8b, v30.8b\n\t"
    "orr v23.8b, v23.8b, v31.8b\n\t"
    "orr v24.8b, v24.8b, v0.8b\n\t"
    "orr v25.8b, v25.8b, v1.8b\n\t"
    "orr v26.8b, v26.8b, v2.8b\n\t"
    "orr v27.8b, v27.8b, v3.8b\n\t"
    "orr v28.8b, v28.8b, v4.8b\n\t"
    "orr v29.8b, v29.8b, v5.8b\n\t"
    "orr v30.8b, v30.8b, v6.8b\n\t"
    "orr v31.8b, v31.8b, v7.8b\n\t"
    "str xzr, [x6]\n\t"
    "str d0, [x6]\n\t"
    "str d1, [x6]\n\t"
    "str d2, [x6]\n\t"
    "str d3, [x6]\n\t"
    "str d4, [x6]\n\t"
    "str d5, [x6]\n\t"
    "str d6, [x6]\n\t"
    "str d7, [x6]\n\t"
    "str d16, [x6]\n\t"
    "str d17, [x6]\n\t"
    "str d18, [x6]\n\t"
    "str d19, [x6]\n\t"
    "str d20, [x6]\n\t"
    "str d21, [x6]\n\t"
    "str d22, [x6]\n\t"
    "str d23, [x6]\n\t"
    "str d24, [x6]\n\t"
    "str d25, [x6]\n\t"
    "str d26, [x6]\n\t"
    "str d27, [x6]\n\t"
    "str d28, [x6]\n\t"
    "str d29, [x6]\n\t"
    "str d30, [x6]\n\t"
    "str d31, [x6]\n\t"
    "str d0, [x6]\n\t"
    "str d1, [x6]\n\t"
    "str d2, [x6]\n\t"
    "str d3, [x6]\n\t"
    "str d4, [x6]\n\t"
    "str d5, [x6]\n\t"
    "str d6, [x6]\n\t"
    "str d7, [x6]\n\t"
    "str d16, [x6]\n\t"
    "str d17, [x6]\n\t"
    "str d18, [x6]\n\t"
    "str d19, [x6]\n\t"
    "str d20, [x6]\n\t"
    "str d21, [x6]\n\t"
    "str d22, [x6]\n\t"
    "str d23, [x6]\n\t"
    "str d24, [x6]\n\t"
    "str d25, [x6]\n\t"
    "str d26, [x6]\n\t"
    "str d27, [x6]\n\t"
    "str d28, [x6]\n\t"
    "str d29, [x6]\n\t"
    "str d30, [x6]\n\t"
    "str d31, [x6]\n\t"
    "add x4, x4, 8\n\t"
    "add x6, x6, 8\n\t"
    "add x5, x5, 1\n\t"
    "b .TLOOP%=\n\t"
    ".TRET%=:\n\t"
    :
    : "r"(rx0), "r"(rx1), "r"(rx2)
    : "x4", "x5", "x6", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
      "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25",
      "v26", "v27", "v28", "v29", "v30", "v31", "cc", "memory");
}

/* zero n 8-byte words, inline version of clean64bit_aa64 in cprefresh_aa64.s */
static inline __attribute__((always_inline))
void clean64bit_aa64inl(void *addr, long n)
{
  register void *rx0 __asm__("x0") = addr;
  register long rx1 __asm__("x1") = n;
  __asm__ __volatile__ (
    "mov x4, x0\n\t"
    "mov x5, #1\n\t"
    "eor v0.8b, v0.8b, v0.8b\n\t"
    "eor v1.8b, v1.8b, v1.8b\n\t"
    "eor v2.8b, v2.8b, v2.8b\n\t"
    "eor v3.8b, v3.8b, v3.8b\n\t"
    "eor v4.8b, v4.8b, v4.8b\n\t"
    "eor v5.8b, v5.8b, v5.8b\n\t"
    "eor v6.8b, v6.8b, v6.8b\n\t"
    "eor v7.8b, v7.8b, v7.8b\n\t"
    "eor v16.8b, v16.8b, v16.8b\n\t"
    "eor v17.8b, v17.8b, v17.8b\n\t"
    "eor v18.8b, v18.8b, v18.8b\n\t"
    "eor v19.8b, v19.8b, v19.8b\n\t"
    "eor v20.8b, v20.8b, v20.8b\n\t"
    "eor v21.8b, v21.8b, v21.8b\n\t"
    "eor v22.8b, v22.8b, v22.8b\n\t"
    "eor v23.8b, v23.8b, v23.8b\n\t"
    "eor v24.8b, v24.8b, v24.8b\n\t"
    "eor v25.8b, v25.8b, v25.8b\n\t"
    "eor v26.8b, v26.8b, v26.8b\n\t"
    "eor v27.8b, v27.8b, v27.8b\n\t"
    "eor v28.8b, v28.8b, v28.8b\n\t"
    "eor v29.8b, v29.8b, v29.8b\n\t"
    "eor v30.8b, v30.8b, v30.8b\n\t"
    "eor v31.8b, v31.8b, v31.8b\n\t"
    "orr v0.8b, v0.8b, v31.8b\n\t"
    "orr v1.8b, v1.8b, v0.8b\n\t"
    "orr v2.8b, v2.8b, v1.8b\n\t"
    "orr v3.8b, v3.8b, v2.8b\n\t"
    "orr v4.8b, v4.8b, v3.8b\n\t"
    "orr v5.8b, v5.8b, v4.8b\n\t"
    "orr v6.8b, v6.8b, v5.8b\n\t"
    "orr v7.8b, v7.8b, v6.8b\n\t"
    "orr v16.8b, v16.8b, v7.8b\n\t"
    "orr v17.8b, v17.8b, v16.8b\n\t"
    "orr v18.8b, v18.8b, v17.8b\n\t"
    "orr v19.8b, v19.8b, v18.8b\n\t"
    "orr v20.8b, v20.8b, v19.8b\n\t"
    "orr v21.8b, v21.8b, v20.8b\n\t"
    "orr v22.8b, v22.8b, v21.8b\n\t"
    "orr v23.8b, v23.8b, v22.8b\n\t"
    "orr v24.8b, v24.8b, v23.8b\n\t"
    "orr v25.8b, v25.8b, v24.8b\n\t"
    "orr v26.8b, v26.8b, v25.8b\n\t"
    "orr v27.8b, v27.8b, v26.8b\n\t"
    "orr v28.8b, v28.8b, v27.8b\n\t"
    "orr v29.8b, v29.8b, v28.8b\n\t"
    "orr v30.8b, v30.8b, v29.8b\n\t"
    "orr v31.8b, v31.8b, v30.8b\n\t"
    "orr v0.8b, v0.8b, v16.8b\n\t"
    "orr v1.8b, v1.8b, v17.8b\n\t"
    "orr v2.8b, v2.8b, v18.8b\n\t"
    "orr v3.8b, v3.8b, v19.8b\n\t"
    "orr v4.8b, v4.8b, v20.8b\n\t"
    "orr v5.8b, v5.8b, v21.8b\n\t"
    "orr v6.8b, v6.8b, v22.8b\n\t"
    "orr v7.8b, v7.8b, v23.8b\n\t"
    "orr v16.8b, v16.8b, v24.8b\n\t"
    "orr v17.8b, v17.8b, v25.8b\n\t"
    "orr v18.8b, v18.8b, v26.8b\n\t"
    "orr v19.8b, v19.8b, v27.8b\n\t"
    "orr v20.8b, v20.8b, v28.8b\n\t"
    "orr v21.8b, v21.8b, v29.8b\n\t"
    "orr v22.8b, v22.8b, v30.8b\n\t"
    "orr v23.8b, v23.8b, v31.8b\n\t"
    "orr v24.8b, v24.8b, v0.8b\n\t"
    "orr v25.8b, v25.8b, v1.8b\n\t"
    "orr v26.8b, v26.8b, v2.8b\n\t"
    "orr v27.8b, v27.8b, v3.8b\n\t"
    "orr v28.8b, v28.8b, v4.8b\n\t"
    "orr v29.8b, v29.8b, v5.8b\n\t"
    "orr v30.8b, v30.8b, v6.8b\n\t"
    "orr v31.8b, v31.8b, v7.8b\n\t"
    ".ZLOOP%=:\n\t"
    "cmp x1, x5\n\t"
    "blt .ZRET%=\n\t"
    "str xzr, [x4]\n\t"
    "str d0, [x4]\n\t"
    "str d1, [x4]\n\t"
    "str d2, [x4]\n\t"
    "str d3, [x4]\n\t"
    "str d4, [x4]\n\t"
    "str d5, [x4]\n\t"
    "str d6, [x4]\n\t"
    "str d7, [x4]\n\t"
    "str d16, [x4]\n\t"
    "str d17, [x4]\n\t"
    "str d18, [x4]\n\t"
    "str d19, [x4]\n\t"
    "str d20, [x4]\n\t"
    "str d21, [x4]\n\t"
    "str d22, [x4]\n\t"
    "str d23, [x4]\n\t"
    "str d24, [x4]\n\t"
    "str d25, [x4]\n\t"
    "str d26, [x4]\n\t"
    "str d27, [x4]\n\t"
    "str d28, [x4]\n\t"
    "str d29, [x4]\n\t"
    "str d30, [x4]\n\t"
    "str d31, [x4]\n\t"
    "str d0, [x4]\n\t"
    "str d1, [x4]\n\t"
    "str d2, [x4]\n\t"
    "str d3, [x4]\n\t"
    "str d4, [x4]\n\t"
    "str d5, [x4]\n\t"
    "str d6, [x4]\n\t"
    "str d7, [x4]\n\t"
    "str d16, [x4]\n\t"
    "str d17, [x4]\n\t"
    "str d18, [x4]\n\t"
    "str d19, [x4]\n\t"
    "str d20, [x4]\n\t"
    "str d21, [x4]\n\t"
    "str d22, [x4]\n\t"
    "str d23, [x4]\n\t"
    "str d24, [x4]\n\t"
    "str d25, [x4]\n\t"
    "str d26, [x4]\n\t"
    "str d27, [x4]\n\t"
    "str d28, [x4]\n\t"
    "str d29, [x4]\n\t"
    "str d30, [x4]\n\t"
    "str d31, [x4]\n\t"
    "add x4, x4, 8\n\t"
    "add x5, x5, 1\n\t"
    "b .ZLOOP%=\n\t"
    ".ZRET%=:\n\t"
    :
    : "r"(rx0), "r"(rx1)
    : "x4", "x5", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v16",
      "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26",
      "v27", "v28", "v29", "v30", "v31", "cc", "memory");
}

/* the functions declared in cprefresh.h, as in cprefresh.c for AA64 */

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
  refresh64bit_aa64inl(vp, n);
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
      refreshmem_aa64inl(vp, n);
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
  clean64bit_aa64inl((void*)(ptr+off), n0);
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
      cp64bit_aa64inl((void*)ptr, n0, (void*)dest);
      n0 *= 8;
      for (; n0 < n; n0++) vd[n0] = ptr[n0];
  }
}

