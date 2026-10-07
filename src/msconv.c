/*
msconv.c                Copyright frankl 2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/

/* see msconv.h */

#include <stdlib.h>
#include <stdio.h>
#include "fftconv.h"
#include "msconv.h"

struct msconv {
  int blen;
  double mfac, sfac;
  fftconv *cs, *cl, *cr;   /* NULL if not used */
  long tail;
  double *m, *s, *l, *r;   /* single channel buffers of length blen */
};

static fftconv *newconv(const char *fname, int blen, long *len,
                        const char *name, int verbose)
{
  double *h;
  fftconv *c;

  h = fftconv_readcoeffs(fname, len);
  c = fftconv_new(h, *len, blen);
  free(h);
  if (verbose)
    fprintf(stderr, "msconv: %s filter %s: %ld taps (%d partitions)\n",
            name, fname, *len, fftconv_partitions(c));
  return c;
}

msconv *msconv_new(const char *sfilt, const char *lfilt, const char *rfilt,
                   int blen, double mfac, double sfac, int verbose)
{
  msconv *ms;
  long slen, llen, rlen;

  ms = calloc(1, sizeof(msconv));
  if (!ms) {
    fprintf(stderr, "msconv: out of memory.\n");
    exit(3);
  }
  ms->blen = blen;
  ms->mfac = mfac;
  ms->sfac = sfac;
  if (verbose)
    fprintf(stderr, "msconv: block length %d\n", blen);
  if (sfilt) {
    ms->cs = newconv(sfilt, blen, &slen, "S", verbose);
    ms->tail += slen - 1;
    if (verbose)
      fprintf(stderr, "msconv: M factor %g, S factor %g\n", mfac, sfac);
  }
  if (lfilt && rfilt) {
    ms->cl = newconv(lfilt, blen, &llen, "L", verbose);
    ms->cr = newconv(rfilt, blen, &rlen, "R", verbose);
    ms->tail += (llen > rlen ? llen : rlen) - 1;
  }
  ms->m = malloc(blen * sizeof(double));
  ms->s = malloc(blen * sizeof(double));
  ms->l = malloc(blen * sizeof(double));
  ms->r = malloc(blen * sizeof(double));
  if (!ms->m || !ms->s || !ms->l || !ms->r) {
    fprintf(stderr, "msconv: out of memory.\n");
    exit(3);
  }
  return ms;
}

void msconv_process(msconv *ms, double *buf)
{
  int i, blen = ms->blen;
  double *m = ms->m, *s = ms->s, *l = ms->l, *r = ms->r;

  if (ms->cs) {
    for (i = 0; i < blen; i++) {
      m[i] = ms->mfac * (buf[2*i] + buf[2*i+1]);
      s[i] = ms->sfac * (buf[2*i] - buf[2*i+1]);
    }
    fftconv_process(ms->cs, s, s);
    for (i = 0; i < blen; i++) {
      l[i] = m[i] + s[i];
      r[i] = m[i] - s[i];
    }
  } else {
    for (i = 0; i < blen; i++) {
      l[i] = buf[2*i];
      r[i] = buf[2*i+1];
    }
  }
  if (ms->cl) {
    fftconv_process(ms->cl, l, l);
    fftconv_process(ms->cr, r, r);
  }
  for (i = 0; i < blen; i++) {
    buf[2*i] = l[i];
    buf[2*i+1] = r[i];
  }
}

long msconv_taillength(msconv *ms)
{
  return ms->tail;
}

static inline int32_t toint32(double v)
{
  return v >= 1.0 ? INT32_MAX : v <= -1.0 ? -INT32_MAX :
         (int32_t)(v * 2147483647);
}

long msconv_by4int32(const double *buf, long n, long pos, int32_t *obuf)
{
  long i, k;

  for (i = (4 - pos % 4) % 4, k = 0; i < n; i += 4) {
    obuf[k++] = toint32(buf[2*i]);
    obuf[k++] = toint32(buf[2*i+1]);
  }
  return k / 2;
}

void msconv_toint32(const double *buf, long n, int32_t *obuf)
{
  long i;

  for (i = 0; i < 2 * n; i++)
    obuf[i] = toint32(buf[i]);
}

void msconv_free(msconv *ms)
{
  if (ms->cs)
    fftconv_free(ms->cs);
  if (ms->cl) {
    fftconv_free(ms->cl);
    fftconv_free(ms->cr);
  }
  free(ms->m); free(ms->s); free(ms->l); free(ms->r);
  free(ms);
}
