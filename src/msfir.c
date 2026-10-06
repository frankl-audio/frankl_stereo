/*
msfir.c                Copyright frankl 2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/

#include "version.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "fftconv.h"

/* help page */
/* vim hint to remove resp. add quotes:
      s/^"\(.*\)\\n"$/\1/
      s/.*$/"\0\\n"/
*/
void usage( ) {
  fprintf(stderr,
          "msfir (version %s of frankl's stereo utilities)\nUSAGE:\n",
          VERSION);
  fprintf(stderr,
"\n"
"  msfir [options] sfilter lfilter rfilter\n"
"\n"
"  This command works as a filter for stereo audio streams in raw\n"
"  64-bit floating point format (FLOAT64_LE), it reads from stdin and\n"
"  writes to stdout.\n"
"\n"
"  With input channels L and R it computes\n"
"      M = 0.4 (L + R),   S = 0.8 (L - R),\n"
"      S' = S convolved with the coefficients in 'sfilter',\n"
"      L' = M + S',  R' = M - S',\n"
"  and outputs L' convolved with 'lfilter' as left channel and R'\n"
"  convolved with 'rfilter' as right channel.\n"
"\n"
"  The filter files contain raw 64-bit floating point coefficients\n"
"  (FLOAT64_LE), their lengths can be arbitrary.\n"
"\n"
"  The convolution is done with partitioned overlap-save in the\n"
"  frequency domain as in 'brutefir'. There is no delay between input\n"
"  and output (apart from delays inherent in the filters). By default\n"
"  the output has the same length as the input, as with 'brutefir'\n"
"  using file input and output.\n"
"\n"
"  OPTIONS\n"
"\n"
"  --block-length=intval, -b intval\n"
"      the length of the blocks (filter partitions) used for the\n"
"      convolution. Larger values need less CPU, the FFT length is\n"
"      twice this value. Default is 16384.\n"
"\n"
"  --m-factor=floatval, -m floatval\n"
"      the factor in the definition of M. Default is 0.4.\n"
"\n"
"  --s-factor=floatval, -s floatval\n"
"      the factor in the definition of S. Default is 0.8.\n"
"\n"
"  --wisdom-file=fname, -w fname\n"
"      file to store FFTW wisdom (optimized plans for the FFTs). It is\n"
"      read at startup and written if new plans were computed. This\n"
"      avoids the planning time (almost a second with the default block\n"
"      length) in later runs. Default is no wisdom file.\n"
"\n"
"  --by4int32, -4\n"
"      output only every fourth frame (frames 0, 4, 8, ...) with samples\n"
"      as 32-bit signed integers (S32_LE), e.g., 48000 Hz output from\n"
"      192000 Hz input. This trivial downsampling is only valid if the\n"
"      filters remove all content above a quarter of the input sample\n"
"      rate. Samples outside [-1,1] are clipped.\n"
"\n"
"  --tail, -t\n"
"      at end of input continue with silence until the complete\n"
"      response of the filters is written.\n"
"\n"
"  --help, -h\n"
"      show this help.\n"
"\n"
"  --verbose, -p\n"
"      shows some information during startup.\n"
"\n"
"  --version, -V\n"
"      show the version of this program and exit.\n"
"\n"
"  EXAMPLE\n"
"\n"
"  Replacement for 'brutefir fromcache64.conf -quiet' with a brutefir\n"
"  configuration implementing the computation described above:\n"
"      msfir -w ~/.msfir_wisdom FHR-192.dbl LeftFiltSt.pcm RightFiltSt.pcm\n"
"\n"
);
}

/* read up to n frames, returns number of frames read */
static long readframes(double *buf, long n)
{
  return fread(buf, 2 * sizeof(double), n, stdin);
}

static void writeframes(double *buf, long n)
{
  if (fwrite(buf, 2 * sizeof(double), n, stdout) != (size_t)n) {
    fprintf(stderr, "msfir: write error.\n");
    exit(4);
  }
}

/* write every fourth frame as 32-bit integers, pos is the number of
   frames written before (to keep the frame selection across blocks) */
static void writeby4int32(double *buf, long n, long pos, int32_t *obuf)
{
  long i, k;
  double v;

  for (i = (4 - pos % 4) % 4, k = 0; i < n; i += 4) {
    v = buf[2*i];
    obuf[k++] = v >= 1.0 ? INT32_MAX : v <= -1.0 ? -INT32_MAX :
                (int32_t)(v * 2147483647);
    v = buf[2*i+1];
    obuf[k++] = v >= 1.0 ? INT32_MAX : v <= -1.0 ? -INT32_MAX :
                (int32_t)(v * 2147483647);
  }
  if (fwrite(obuf, sizeof(int32_t), k, stdout) != (size_t)k) {
    fprintf(stderr, "msfir: write error.\n");
    exit(4);
  }
}

int main(int argc, char *argv[])
{
  int optc, optidx, blen, verbose, tail, by4, i;
  long hslen, hllen, hrlen, nin, nout, rest, total, written;
  char *wisdom;
  double mfac, sfac, *hs, *hl, *hr, *buf, *m, *s, *l, *r;
  int32_t *obuf;
  fftconv *cs, *cl, *cr;

  if (argc == 1) {
    usage();
    exit(1);
  }
  /* defaults */
  blen = 16384;
  mfac = 0.4;
  sfac = 0.8;
  tail = 0;
  by4 = 0;
  verbose = 0;
  wisdom = NULL;
  /* read command line options */
  static struct option longoptions[] = {
    {"block-length", required_argument, 0, 'b' },
    {"m-factor", required_argument, 0, 'm' },
    {"s-factor", required_argument, 0, 's' },
    {"wisdom-file", required_argument, 0, 'w' },
    {"by4int32", no_argument, 0, '4' },
    {"tail", no_argument, 0, 't' },
    {"verbose", no_argument, 0, 'p' },
    {"version", no_argument, 0, 'V' },
    {"help", no_argument, 0, 'h' },
    {0,         0,                 0,  0 }
  };
  while ((optc = getopt_long(argc, argv, "b:m:s:w:4tpVh",
          longoptions, &optidx)) != -1) {
    switch (optc) {
    case 'b':
      blen = atoi(optarg);
      break;
    case 'm':
      mfac = atof(optarg);
      break;
    case 's':
      sfac = atof(optarg);
      break;
    case 'w':
      wisdom = optarg;
      break;
    case '4':
      by4 = 1;
      break;
    case 't':
      tail = 1;
      break;
    case 'p':
      verbose = 1;
      break;
    case 'V':
      fprintf(stderr, "msfir (version %s of frankl's stereo utilities)\n",
              VERSION);
      exit(0);
    default:
      usage();
      exit(1);
    }
  }
  if (argc - optind != 3) {
    fprintf(stderr, "msfir: need three filter files as arguments.\n");
    exit(1);
  }
  if (blen < 16) {
    fprintf(stderr, "msfir: block length too small.\n");
    exit(1);
  }

  hs = fftconv_readcoeffs(argv[optind], &hslen);
  hl = fftconv_readcoeffs(argv[optind+1], &hllen);
  hr = fftconv_readcoeffs(argv[optind+2], &hrlen);
  if (wisdom && !fftconv_loadwisdom(wisdom) && verbose)
    fprintf(stderr, "msfir: no wisdom read from %s\n", wisdom);
  cs = fftconv_new(hs, hslen, blen);
  cl = fftconv_new(hl, hllen, blen);
  cr = fftconv_new(hr, hrlen, blen);
  if (wisdom)
    fftconv_savewisdom(wisdom);
  free(hs); free(hl); free(hr);
  if (verbose) {
    fprintf(stderr, "msfir: block length %d, M factor %g, S factor %g\n",
            blen, mfac, sfac);
    fprintf(stderr, "msfir: S filter %ld taps (%d partitions)\n",
            hslen, fftconv_partitions(cs));
    fprintf(stderr, "msfir: L filter %ld taps (%d partitions)\n",
            hllen, fftconv_partitions(cl));
    fprintf(stderr, "msfir: R filter %ld taps (%d partitions)\n",
            hrlen, fftconv_partitions(cr));
  }

  buf = malloc(2 * blen * sizeof(double));
  m = malloc(blen * sizeof(double));
  s = malloc(blen * sizeof(double));
  l = malloc(blen * sizeof(double));
  r = malloc(blen * sizeof(double));
  obuf = malloc(2 * (blen / 4 + 1) * sizeof(int32_t));
  if (!buf || !m || !s || !l || !r || !obuf) {
    fprintf(stderr, "msfir: out of memory.\n");
    exit(3);
  }

  /* main loop: one block per iteration; after end of input 'rest' is
     the number of frames still to write                                */
  total = 0;
  written = 0;
  rest = -1;
  while (rest != 0) {
    if (rest < 0) {
      nin = readframes(buf, blen);
      total += nin;
      if (nin < blen) {
        memset(buf + 2 * nin, 0, 2 * (blen - nin) * sizeof(double));
        rest = nin;
        if (tail)
          rest += (hslen - 1) + (hllen > hrlen ? hllen : hrlen) - 1;
      }
    } else
      memset(buf, 0, 2 * blen * sizeof(double));

    for (i = 0; i < blen; i++) {
      m[i] = mfac * (buf[2*i] + buf[2*i+1]);
      s[i] = sfac * (buf[2*i] - buf[2*i+1]);
    }
    fftconv_process(cs, s, s);
    for (i = 0; i < blen; i++) {
      l[i] = m[i] + s[i];
      r[i] = m[i] - s[i];
    }
    fftconv_process(cl, l, l);
    fftconv_process(cr, r, r);
    for (i = 0; i < blen; i++) {
      buf[2*i] = l[i];
      buf[2*i+1] = r[i];
    }

    nout = blen;
    if (rest >= 0) {
      if (rest < nout)
        nout = rest;
      rest -= nout;
    }
    if (by4)
      writeby4int32(buf, nout, written, obuf);
    else
      writeframes(buf, nout);
    written += nout;
  }
  fflush(stdout);
  if (verbose)
    fprintf(stderr, "msfir: %ld input frames\n", total);

  fftconv_free(cs);
  fftconv_free(cl);
  fftconv_free(cr);
  free(buf); free(m); free(s); free(l); free(r); free(obuf);
  return 0;
}
