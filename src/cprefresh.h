/*
cprefresh.h                Copyright frankl 2013-2024

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Here are some utility functions to clean or refresh RAM.
Optionally, they use assembler functions on ARM and X86_64
(chosen with REFRESH=... in the Makefile, see cprefresh.c).
*/

void refreshmem(char* ptr, long n);
void refreshmems(char* ptr, long n, long k);
void memclean(char* ptr, long n);
void cprefresh(char* dest, char* ptr, long n);

