/*
fftconv.c                Copyright frankl 2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/

/* Uniformly partitioned overlap-save convolution, see fftconv.h.

   The filter h of length hlen is split into np = ceil(hlen/n) partitions
   h_0, h_1, ... of length n. Each is zero padded to length 2n and
   transformed: H_k. For each new input block x_i of length n we transform
   the 2n samples (x_{i-1}, x_i) to X_i and store it in a ring buffer
   (frequency domain delay line). Then
        Y = sum_k  H_k * X_{i-k}
   and the second half of the inverse transform of Y is the output
   block y_i (the first half contains the circular aliasing).

   The 1/(2n) normalization of the unnormalized FFTW transforms is
   applied to the H_k.                                                   */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fftw3.h>
#include "fftconv.h"

/* set when plans were computed which were not found in wisdom */
static int newwisdom = 0;

struct fftconv {
  int n;             /* block length */
  int nb;            /* number of complex bins, n+1 */
  int np;            /* number of partitions */
  int stride;        /* doubles per spectrum in hspec/fdl, 2*nb padded
                        to multiple of 8 such that all spectra have the
                        same alignment as needed by FFTW               */
  int cur;           /* position of newest spectrum in fdl */
  double *xbuf;      /* last 2n input samples */
  double *ybuf;      /* 2n output samples of inverse transform */
  double *hspec;     /* np spectra of filter partitions, (re,im) pairs */
  double *fdl;       /* np spectra of input blocks */
  double *acc;       /* accumulated output spectrum */
  fftw_plan fwd, bwd;
};

double *fftconv_readcoeffs(const char *fname, long *len)
{
  FILE *f;
  double *h;
  long sz;

  f = fopen(fname, "rb");
  if (!f) {
    fprintf(stderr, "fftconv: cannot open %s.\n", fname);
    exit(2);
  }
  fseek(f, 0, SEEK_END);
  sz = ftell(f) / sizeof(double);
  fseek(f, 0, SEEK_SET);
  if (sz <= 0) {
    fprintf(stderr, "fftconv: no coefficients in %s.\n", fname);
    exit(2);
  }
  h = malloc(sz * sizeof(double));
  if (!h || fread(h, sizeof(double), sz, f) != (size_t)sz) {
    fprintf(stderr, "fftconv: cannot read %s.\n", fname);
    exit(2);
  }
  fclose(f);
  *len = sz;
  return h;
}

int fftconv_loadwisdom(const char *fname)
{
  return fftw_import_wisdom_from_filename(fname);
}

void fftconv_savewisdom(const char *fname)
{
  if (newwisdom && !fftw_export_wisdom_to_filename(fname))
    fprintf(stderr, "fftconv: cannot write wisdom file %s.\n", fname);
  newwisdom = 0;
}

fftconv *fftconv_new(const double *h, long hlen, int blen)
{
  fftconv *c;
  int k, nb, n = blen;
  long i, m;
  double scale, *part;

  c = calloc(1, sizeof(fftconv));
  if (!c) {
    fprintf(stderr, "fftconv: out of memory.\n");
    exit(3);
  }
  nb = n + 1;
  c->n = n;
  c->nb = nb;
  c->np = (hlen + n - 1) / n;
  c->stride = (2 * nb + 7) & ~7;
  c->xbuf = fftw_malloc(2 * n * sizeof(double));
  c->ybuf = fftw_malloc(2 * n * sizeof(double));
  c->hspec = fftw_malloc((size_t)c->np * c->stride * sizeof(double));
  c->fdl = fftw_malloc((size_t)c->np * c->stride * sizeof(double));
  c->acc = fftw_malloc(nb * 2 * sizeof(double));
  if (!c->xbuf || !c->ybuf || !c->hspec || !c->fdl || !c->acc) {
    fprintf(stderr, "fftconv: out of memory.\n");
    exit(3);
  }
  /* all forward transforms go from xbuf into some spectrum in hspec or
     fdl (same alignment), we use the new-array execute functions       */
  c->fwd = fftw_plan_dft_r2c_1d(2 * n, c->xbuf, (fftw_complex *)c->fdl,
                                FFTW_MEASURE | FFTW_WISDOM_ONLY);
  c->bwd = fftw_plan_dft_c2r_1d(2 * n, (fftw_complex *)c->acc, c->ybuf,
                                FFTW_MEASURE | FFTW_WISDOM_ONLY);
  if (!c->fwd || !c->bwd) {
    /* not in wisdom, compute plans */
    if (c->fwd)
      fftw_destroy_plan(c->fwd);
    if (c->bwd)
      fftw_destroy_plan(c->bwd);
    c->fwd = fftw_plan_dft_r2c_1d(2 * n, c->xbuf, (fftw_complex *)c->fdl,
                                  FFTW_MEASURE);
    c->bwd = fftw_plan_dft_c2r_1d(2 * n, (fftw_complex *)c->acc, c->ybuf,
                                  FFTW_MEASURE);
    newwisdom = 1;
  }
  if (!c->fwd || !c->bwd) {
    fprintf(stderr, "fftconv: cannot create FFTW plans.\n");
    exit(3);
  }

  /* transform filter partitions */
  scale = 1.0 / (2.0 * n);
  for (k = 0; k < c->np; k++) {
    part = c->hspec + (size_t)k * c->stride;
    memset(c->xbuf, 0, 2 * n * sizeof(double));
    m = hlen - (long)k * n;
    if (m > n)
      m = n;
    for (i = 0; i < m; i++)
      c->xbuf[i] = h[(long)k * n + i] * scale;
    fftw_execute_dft_r2c(c->fwd, c->xbuf, (fftw_complex *)part);
  }
  fftconv_reset(c);
  return c;
}

void fftconv_reset(fftconv *c)
{
  memset(c->xbuf, 0, 2 * c->n * sizeof(double));
  memset(c->fdl, 0, (size_t)c->np * c->stride * sizeof(double));
  c->cur = 0;
}

void fftconv_process(fftconv *c, const double *in, double *out)
{
  int n = c->n, nb2 = 2 * c->nb, np = c->np, st = c->stride, k, j, idx;
  double *x, *h, *acc = c->acc;

  /* shift input history and append new block */
  memmove(c->xbuf, c->xbuf + n, n * sizeof(double));
  memcpy(c->xbuf + n, in, n * sizeof(double));
  c->cur = (c->cur + 1) % np;
  fftw_execute_dft_r2c(c->fwd, c->xbuf,
                       (fftw_complex *)(c->fdl + (size_t)c->cur * st));

  /* acc = sum_k H_k * X_{cur-k} */
  memset(acc, 0, nb2 * sizeof(double));
  for (k = 0; k < np; k++) {
    idx = c->cur - k;
    if (idx < 0)
      idx += np;
    h = c->hspec + (size_t)k * st;
    x = c->fdl + (size_t)idx * st;
    for (j = 0; j < nb2; j += 2) {
      acc[j]   += h[j] * x[j]   - h[j+1] * x[j+1];
      acc[j+1] += h[j] * x[j+1] + h[j+1] * x[j];
    }
  }
  fftw_execute(c->bwd);
  memcpy(out, c->ybuf + n, n * sizeof(double));
}

int fftconv_partitions(fftconv *c)
{
  return c->np;
}

void fftconv_free(fftconv *c)
{
  fftw_destroy_plan(c->fwd);
  fftw_destroy_plan(c->bwd);
  fftw_free(c->xbuf);
  fftw_free(c->ybuf);
  fftw_free(c->hspec);
  fftw_free(c->fdl);
  fftw_free(c->acc);
  free(c);
}
