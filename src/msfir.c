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
#include "msconv.h"

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
"  msfir [options]\n"
"\n"
"  This command works as a filter for stereo audio streams in raw\n"
"  64-bit floating point format (FLOAT64_LE), it reads from stdin and\n"
"  writes to stdout.\n"
"\n"
"  With input channels L and R and option --Sfilter it computes\n"
"      M = 0.4 (L + R),   S = 0.8 (L - R),\n"
"      S' = S convolved with the S filter,\n"
"      L' = M + S',  R' = M - S'.\n"
"  Without --Sfilter we just have L' = L and R' = R.\n"
"  With option --LRfilters the output is L' convolved with the L filter\n"
"  as left channel and R' convolved with the R filter as right channel,\n"
"  otherwise the output is L', R'.\n"
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
"  --Sfilter=sfilt, -S sfilt\n"
"      file with the filter for the S channel, as described above.\n"
"\n"
"  --LRfilters=lfilt,rfilt, -L lfilt,rfilt\n"
"      files with the filters for the left and right channel, as\n"
"      described above (the file names must not contain commas).\n"
"\n"
"  --block-length=intval, -b intval\n"
"      the length of the blocks (filter partitions) used for the\n"
"      convolution. Larger values need less CPU, the FFT length is\n"
"      twice this value. Default is 16384.\n"
"\n"
"  --m-factor=floatval, -m floatval\n"
"      the factor in the definition of M (only used with --Sfilter).\n"
"      Default is 0.4.\n"
"\n"
"  --s-factor=floatval, -s floatval\n"
"      the factor in the definition of S (only used with --Sfilter).\n"
"      Default is 0.8.\n"
"\n"
"  --wisdom-file=fname, -w fname\n"
"      file to store FFTW wisdom (optimized plans for the FFTs). It is\n"
"      read at startup and written if new plans were computed. This\n"
"      avoids the planning time (almost a second with the default block\n"
"      length) in later runs. Default is no wisdom file.\n"
"\n"
"  --toint32, -I\n"
"      output 32-bit signed integer samples (S32_LE) instead of 64-bit\n"
"      floating point samples. Samples outside [-1,1] are clipped.\n"
"\n"
"  --by4int32, -4\n"
"      output only every fourth frame (frames 0, 4, 8, ...) with samples\n"
"      as 32-bit signed integers (S32_LE), e.g., 48000 Hz output from\n"
"      192000 Hz input. This trivial downsampling is only valid if the\n"
"      filters remove all content above half of the output sample\n"
"      rate (= 1/8th of the input sample rate).\n"
"      Samples outside [-1,1] are clipped.\n"
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
"  Replacement for 'brutefir SLRfilter.conf -quiet' with a brutefir\n"
"  configuration implementing the computation described above:\n"
"      msfir -w ~/.msfir_wisdom --Sfilter=Sfilt.dbl \\\n"
"            --LRfilters=Leftfilt.dbl,Rightfilt.dbl\n"
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

int main(int argc, char *argv[])
{
  int optc, optidx, blen, verbose, tail, by4, out32;
  long nin, nout, rest, total, written, k;
  char *wisdom, *sfilt, *lfilt, *rfilt;
  double mfac, sfac, *buf;
  int32_t *obuf;
  msconv *ms;

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
  out32 = 0;
  verbose = 0;
  wisdom = NULL;
  sfilt = NULL;
  lfilt = NULL;
  rfilt = NULL;
  /* read command line options */
  static struct option longoptions[] = {
    {"Sfilter", required_argument, 0, 'S' },
    {"LRfilters", required_argument, 0, 'L' },
    {"block-length", required_argument, 0, 'b' },
    {"m-factor", required_argument, 0, 'm' },
    {"s-factor", required_argument, 0, 's' },
    {"wisdom-file", required_argument, 0, 'w' },
    {"by4int32", no_argument, 0, '4' },
    {"toint32", no_argument, 0, 'I' },
    {"tail", no_argument, 0, 't' },
    {"verbose", no_argument, 0, 'p' },
    {"version", no_argument, 0, 'V' },
    {"help", no_argument, 0, 'h' },
    {0,         0,                 0,  0 }
  };
  while ((optc = getopt_long(argc, argv, "S:L:b:m:s:w:4ItpVh",
          longoptions, &optidx)) != -1) {
    switch (optc) {
    case 'S':
      sfilt = optarg;
      break;
    case 'L':
      lfilt = strdup(optarg);
      rfilt = strchr(lfilt, ',');
      if (!rfilt || rfilt[1] == '\0' || rfilt == lfilt) {
        fprintf(stderr, "msfir: need two file names in --LRfilters.\n");
        exit(1);
      }
      *rfilt++ = '\0';
      break;
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
    case 'I':
      out32 = 1;
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
  if (optind < argc) {
    fprintf(stderr, "msfir: unexpected argument %s (filters must be given "
            "with --Sfilter and --LRfilters).\n", argv[optind]);
    exit(1);
  }
  if (blen < 16) {
    fprintf(stderr, "msfir: block length too small.\n");
    exit(1);
  }

  if (wisdom && !fftconv_loadwisdom(wisdom) && verbose)
    fprintf(stderr, "msfir: no wisdom read from %s\n", wisdom);
  ms = msconv_new(sfilt, lfilt, rfilt, blen, mfac, sfac, verbose);
  if (wisdom)
    fftconv_savewisdom(wisdom);

  buf = malloc(2 * blen * sizeof(double));
  obuf = malloc(2 * blen * sizeof(int32_t));
  if (!buf || !obuf) {
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
          rest += msconv_taillength(ms);
      }
    } else
      memset(buf, 0, 2 * blen * sizeof(double));

    msconv_process(ms, buf);

    nout = blen;
    if (rest >= 0) {
      if (rest < nout)
        nout = rest;
      rest -= nout;
    }
    if (by4) {
      k = msconv_by4int32(buf, nout, written, obuf);
      if (fwrite(obuf, 2 * sizeof(int32_t), k, stdout) != (size_t)k) {
        fprintf(stderr, "msfir: write error.\n");
        exit(4);
      }
    } else if (out32) {
      msconv_toint32(buf, nout, obuf);
      if (fwrite(obuf, 2 * sizeof(int32_t), nout, stdout) != (size_t)nout) {
        fprintf(stderr, "msfir: write error.\n");
        exit(4);
      }
    } else
      writeframes(buf, nout);
    written += nout;
  }
  fflush(stdout);
  if (verbose)
    fprintf(stderr, "msfir: %ld input frames\n", total);

  msconv_free(ms);
  free(buf); free(obuf);
  return 0;
}
