/*
msconv.h                Copyright frankl 2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/

/* Convolution of a stereo stream (interleaved 64-bit float frames) in
   blocks of fixed length:
   - optional M/S processing with input L, R:
        M = mfac (L + R),  S = sfac (L - R),
        S' = S convolved with S filter,
        L' = M + S',  R' = M - S'
     (without S filter: L' = L, R' = R)
   - optional L'' = L' convolved with L filter, R'' = R' convolved with
     R filter (without L/R filters: L'' = L', R'' = R')
   There is no added delay, see fftconv.h.                            */

#ifndef MSCONV_H
#define MSCONV_H

#include <stdint.h>

typedef struct msconv msconv;

/* sfilt: file with S filter or NULL, lfilt, rfilt: files with L and R
   filters or both NULL; files contain raw FLOAT64_LE coefficients.
   Call fftconv_loadwisdom before and fftconv_savewisdom after this,
   if wanted. With verbose some information is shown on stderr.     */
msconv *msconv_new(const char *sfilt, const char *lfilt, const char *rfilt,
                   int blen, double mfac, double sfac, int verbose);

/* process next block of blen frames in place */
void msconv_process(msconv *ms, double *buf);

/* number of frames of the filter response after the end of input */
long msconv_taillength(msconv *ms);

/* convert every fourth frame of the n frames in buf to 32-bit integers
   in obuf (clipped to [-1,1]), pos is the number of frames converted
   before (to keep the frame selection across blocks); returns the
   number of frames in obuf                                          */
long msconv_by4int32(const double *buf, long n, long pos, int32_t *obuf);

/* convert the n frames in buf to 32-bit integers in obuf (clipped to
   [-1,1])                                                           */
void msconv_toint32(const double *buf, long n, int32_t *obuf);

void msconv_free(msconv *ms);

#endif
