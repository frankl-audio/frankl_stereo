/*
cprefresh.h                Copyright frankl 2013-2024

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Here are some utility functions to clean or refresh RAM.
Optionally, they use assembler functions on ARM and X86_64.
*/


#ifdef ARMREFRESH

inline void memcp_regX(void*, void*, int);
inline void memclean_regX(void*, int);

#endif

#ifdef VFPREFRESH

inline void memcp_vfpX(void*, void*, int);
inline void memclean_vfpX(void*, int);

#endif

#ifdef REFRESHAA64
#include <stdint.h>

/* n is number of 8 byte quad words */
inline void refreshmem_aa64(void* addr, int n);
/* same with k iterations */
inline void refreshmems_aa64(void* addr, int n, int k);
inline void memclean_aa64(void* addr, int n);
inline void cprefresh_aa64(void* addr, int n, void* dest);

#endif

inline void refreshmem(char* ptr, int n);
inline void refreshmems(char* ptr, int n, int k);
inline void memclean(char* ptr, int n);
inline void cprefresh(char* dest, char* ptr, long n);


