/*
fftconv.h                Copyright frankl 2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/

/* FIR convolution of a single channel with a long filter, using
   uniformly partitioned overlap-save in the frequency domain (as in
   brutefir). Data are processed in blocks of fixed length; output block
   i is the exact (truncated) linear convolution of the input up to
   block i with the filter, there is no added delay.                    */

#ifndef FFTCONV_H
#define FFTCONV_H

typedef struct fftconv fftconv;

/* read raw 64-bit floating point (little endian) coefficients from file,
   returns malloc'ed array and sets *len, exits on error */
double *fftconv_readcoeffs(const char *fname, long *len);

/* FFTW wisdom: load before creating convolvers, save afterwards to
   avoid the planning time (almost a second for FFT length 32768) in
   later runs; load returns 1 on success, save writes the file only if
   new plans were computed                                             */
int fftconv_loadwisdom(const char *fname);
void fftconv_savewisdom(const char *fname);

/* h: filter coefficients (copied), hlen: their number,
   blen: block length (partition length), FFT length is 2*blen */
fftconv *fftconv_new(const double *h, long hlen, int blen);

/* convolve next block of blen samples, in == out is allowed */
void fftconv_process(fftconv *c, const double *in, double *out);

/* forget input history (start with silence) */
void fftconv_reset(fftconv *c);

void fftconv_free(fftconv *c);

/* number of partitions used for filter */
int fftconv_partitions(fftconv *c);

#endif
