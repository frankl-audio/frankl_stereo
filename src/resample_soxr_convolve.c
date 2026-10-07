/*
resample_soxr_convolve.c                Copyright frankl 2018-2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
For compilation see the Makefile (needs also msconv.c, fftconv.c,
cprefresh.c and the libraries soxr, sndfile, fftw3, pthread).
*/
#define _GNU_SOURCE
#include "version.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sndfile.h>
#include <soxr.h>
#include <pthread.h>
#include <sched.h>
#include "cprefresh.h"
#include "nf_io.h"
#include "fftconv.h"
#include "msconv.h"

/* help page */
/* vim hint to remove resp. add quotes:
      s/^"\(.*\)\\n"$/\1/
      s/.*$/"\0\\n"/
*/
void usage( ) {
  fprintf(stderr,
          "resample_soxr_convolve (version %s of frankl's stereo utilities)\n\nUSAGE:\n",
          VERSION);
  fprintf(stderr,
"\n"
"  resample_soxr_convolve [options] \n"
"\n"
"  By default this program works as a resampling filter for stereo audio \n"
"  streams in raw double format (in some programs denoted FLOAT64_LE).\n"
"\n"
"  Here 'filter' means that input comes from stdin and output is\n"
"  written to stdout. Use pipes and 'sox' or other programs to deal with \n"
"  other stream formats. \n"
"\n"
"  Alternatively, this program can use as input any sound file that can be \n"
"  read with 'libsndfile' (flac, wav, ogg, aiff, ...). This  can be \n"
"  a file in the file system or in shared memory (see '--file' and\n"
"  '--shmname' options). In this case it is also possible to read only a \n"
"  part of the file.\n"
"\n"
"  The main options are the input sample rate and the output sample rate.\n"
"\n"
"  The volume of the output can be changed and a RACE filter can be applied\n"
"  to the output; see options '--volume', '--race-delay' and '--race-volume'.\n"
"  If instead of these options their values are given in a file via \n"
"  '--param-file', then these parameters can be changed while the program\n"
"  is running. See 'volrace --help' for more information.\n"
"\n"
"  This program uses the 'soxr' standalone resampling library (see\n"
"  https://sourceforge.net/projects/soxr/) with the highest quality \n"
"  settings, all computations are done with 64-bit floating point \n"
"  numbers.\n"
"\n"
"  The computation is similar to using 'sox' with effect 'rate -v -I'.\n"
"  But 'sox' applies all effects internally to 32-bit signed integer\n"
"  samples (that is, the 64-bit input precision is lost).\n"
"\n"
"  OPTIONS\n"
"  \n"
"  --inrate=floatval, -i floatval\n"
"      the input sample rate as floating point number (must not be an \n"
"      integer). Default is 44100. In case of file input this value is\n"
"      overwritten by the sampling rate specified in the file (so, this\n"
"      option is not needed).\n"
"\n"
"  --outrate=floatval, -o floatval\n"
"      the output sample rate as floating point number (must not be an \n"
"      integer). Default is 192000.\n"
"\n"
"  --channels=intval, -c intval\n"
"      number of interleaved channels in the input. Default is 2 (stereo).\n"
"      In case of input from a file this number is overwritten by the \n"
"      the number of channels in the file.\n"
"\n"
"  --buffer-length=intval, -b intval\n"
"      the size of the input buffer in number of frames. The default\n"
"      (and minimal value) is 8192 and should usually be fine.\n"
"\n"
"  --phase=floatval, -P floatval\n"
"      the phase response of the filter used during resampling; see the \n"
"      documentation of the 'rate' effect in 'sox' for more details. This\n"
"      is a number from 0 (minimum phase) to 100 (maximal phase), with \n"
"      50 (linear phase) and 25 (intermediate phase). The default is 25,\n"
"      and should usually be fine.\n"
"\n"
"  --band-width=floatval, -B floatval\n"
"      the band-width of the filter used during resampling; see the \n"
"      documentation of the rate effect in 'sox' for more details. The value\n"
"      is given as percentage (of the Nyquist frequency of the smaller \n"
"      sampling rate). The allowed range is 74.0..99.7, the default is 91.09\n"
"      (that is the filter is flat up to about 20kHz).\n"
"\n"
"  --precision=floatval, -e floatval\n"
"      the bit precision for resampling; higher values cause higher CPU usage.\n"
"      The valid range is 16.0..33.0, the default is 33.0 and should usually\n"
"      be fine (except lower CPU usage is essential).\n"
"\n"
"  --file=fname, -f fname\n"
"      name of an input audio file (flac, wav, aiff, ...). The default\n"
"      is input from stdin.\n"
"\n"
"  --shmname=sname, -i sname\n"
"      name of an audio file in shared memory. The default\n"
"      is input from stdin.\n"
"\n"
"  --nfinfo=nfnam, -M nfnam\n"
"      with this and the following option raw input can be read from an nf \n"
"      file, this option specifies the nf info file. The sampling rate and \n"
"      bit depth of the input is read from the nf information.\n"
"\n"
"  --nfid=id, -N id\n"
"      this is the nf id of the file to read, the id must be found in the \n"
"      nf info file specified by the previous option.\n"
"\n"
"  --toint32, -I\n"
"      output 32-bit integer samples instead of double floats. With\n"
"      convolution (see below) this applies to the output of the\n"
"      convolver and samples outside [-1,1] are clipped.\n"
"\n"
"The follwing three option allow to read only part of the input, but this is\n"
"only possible for input from file or shared memory.\n"
"\n"
"  --start=intval, -s intval\n"
"      number of the frame to start from. Default is 0.\n"
"  \n"
"  --until=intval, -u intval\n"
"      number of frame to stop. Must be larger than start frame.\n"
"      Default is the end of the audio file.\n"
"\n"
"  --number-frames=intval, -n intval\n"
"      number of frames (from start frame) to write.\n"
"      Default is all frames until end of the audio file.\n"
"\n"
"And here are options for volume and RACE filtering of output.\n"
"\n"
"  --volume=floatval, -v floatval\n"
"      the volume as floating point factor (e.g., '0.5' for an attenuation\n"
"      of 6dB). A small attenuation (say, 0.9) can be useful to avoid \n"
"      intersample clipping. Default is 1.0, that is no volume change.\n"
"      The default is '1.0', that is no volume change.\n"
"\n"
"  --replay-gain=floatval, -r floatval\n"
"      the value of --volume is multiplied by this factor.\n"
"      The default is '1.0'. This can be useful if the volume is changed\n"
"      during runtime via a --param-file.\n"
"\n"
"  --race-delay=val, -d val\n"
"      the delay for RACE as (integer) number of samples (per channel).\n"
"      Default is 12.\n"
"\n"
"  --race-volume=floatval, -a floatval\n"
"      the volume of the RACE signal copied to the other channel.\n"
"      Default is '0.0', that is no RACE filter.\n"
"\n"
"  --param-file=fname, -F fname\n"
"      the name of a file which can be given instead of the previous three\n"
"      options. That file must contain the values for --volume, \n"
"      --race-delay and --race-volume, separated by whitespace.\n"
"      This file is reread by the program when it was changed, and the\n"
"      parameters are faded to the new values. This way this program can\n"
"      be used as volume (and RACE parameter) control during audio playback.\n"
"      The file may only contain the value of --volume, in this case RACE\n"
"      will be disabled.\n"
"\n"
"  --fading-length=intval, -l intval\n"
"      number of frames used for fading to new parameters (when the\n"
"      file given in --param-file was changed). Default is 44100, for high \n"
"      sample rates a larger value may be desirable.\n"
"\n"
"  --number-copies=intnum, -R intnum\n"
"      before writing data they are copied the specfied number of\n"
"      times through  cleaned temporary buffers in RAM. With convolution\n"
"      (see below) this is done with the output of the convolver.\n"
"      The default is 0. \n"
"\n"
"  --number-copies-input=intnum, -C intnum\n"
"      same as --number-copies but applied to the just read input data.\n"
"      This may be in particular interesting with the --realtime option.\n"
"      The default is 0. \n"
"\n"
"  --realtime=floatval, -T floatval\n"
"      starts a timer for reading the input chunks. The given value is the \n"
"      number of seconds for reading the sample rate many frames, hence \n"
"      typically very close to 1.0. \n"
"      The default is 0.0 which does not enable this timer. \n"
"\n"
"  --cpus=rcpu,ccpu or --cpus=cpunr\n"
"      with convolution (see below) the resampling is done in the main\n"
"      thread on CPU number rcpu, and the convolution and the output are\n"
"      done in a second thread on CPU number ccpu. If only one number\n"
"      is given, all is done on that CPU. Without convolution only one\n"
"      number is used. By default the affinity is not changed (it can\n"
"      also be set externally with 'taskset').\n"
"\n"
"And here are options for convolution of the (resampled, volume and RACE\n"
"filtered) stereo stream as in 'msfir'. If one of '--Sfilter' or\n"
"'--LRfilters' is given the convolution and the output are done in a\n"
"separate thread. See 'msfir --help' for more details.\n"
"\n"
"  --Sfilter=sfilt, -S sfilt\n"
"      file with filter for the S channel, M/S encoding and decoding is\n"
"      only done if this option is given.\n"
"\n"
"  --LRfilters=lfilt,rfilt, -L lfilt,rfilt\n"
"      files with the filters for the left and right channel.\n"
"\n"
"  --m-factor=floatval, --s-factor=floatval\n"
"      the factors in the definitions of M and S. Defaults are 0.4 and\n"
"      0.8.\n"
"\n"
"  --conv-block-length=intval\n"
"      the block length for the convolution. Default is 16384.\n"
"\n"
"  --wisdom-file=fname, -w fname\n"
"      file to store FFTW wisdom, see 'msfir --help'.\n"
"\n"
"  --by4int32, -4\n"
"      output only every fourth frame of the convolver output as 32-bit\n"
"      integer samples (clipped), e.g., 48000 Hz output from 192000 Hz.\n"
"      This is only valid if the filters remove all content above a\n"
"      half of the output sample rate.\n"
"\n"
"  --tail, -t\n"
"      at end of input continue with silence until the complete\n"
"      response of the filters is written.\n"
"\n"
"  --help, -h\n"
"      show this help.\n"
"\n"
"  --verbose, -p\n"
"      shows some information during startup and operation.\n"
"\n"
"  --version, -V\n"
"      show the version of this program and exit.\n"
"\n"
"   EXAMPLES\n"
"\n"
"   Convert a file 'musicfile' that can be read by 'sox' to a 96/32\n"
"   wav-file using a pipe:\n"
"      ORIGRATE=`sox --i musicfile | grep \"Sample Rate\" | \\\n"
"                cut -d: -f2 | sed -e \"s/ //g\"`\n"
"      sox musicfile -t raw -e float -b 64 - | \\\n"
"          resample_soxr_convolve --inrate=$ORIGRATE --outrate=96000 --volume=0.9 | \\\n"
"          sox -t raw -e float -b 64 -c 2 -r 96000 - -e signed -b 32 out.wav\n"
"\n"
"   If 'resample_soxr_convolve' can read 'musicfile' this can also be achieved by:\n"
"      resample_soxr_convolve --file=musicfile --outrate=96000 --volume=0.9 | \\\n"
"          sox -t raw -e float -b 64 -c 2 -r 96000 - -e signed -b 32 out.wav\n"
"\n"
"   During room measurements I notice that the clocks in my DAC and my \n"
"   recording soundcard are slightly different. Before computing an \n"
"   impulse response I correct this with a command like:\n"
"      sox recfile.wav -t raw -e float -b 64 - | \\\n"
"          resample_soxr_convolve -i 96000 -o 95999.13487320 | \\\n"
"          sox -t raw -e float -b 64 -c 2 -r 96000 - -e signed -b 32 \\\n"
"          reccorrected.wav\n"
"\n"
"   Read a flac file, resample to 192k, apply M/S and L/R filters in a\n"
"   second thread and write every fourth frame as 32-bit integers:\n"
"      resample_soxr_convolve --file=data.flac --outrate=192000 --cpus=2,3 \\\n"
"           --Sfilter=FHR-192.dbl --LRfilters=Left.pcm,Right.pcm \\\n"
"           --wisdom-file=/home/me/.msfir_wisdom --by4int32 | ...\n"
"\n"
"   Read input from file in shared memory, resample to 192k, apply race \n"
"   filter and pipe into 'brutefir' convolver:\n"
"\n"
"      cptoshm --file=data.flac --shmname=/pl.flac\n"
"      resample_soxr_convolve --shmname=/pl.flac --param-file=/tmp/volraceparams \\\n"
"           --outrate=192000  --fading-length=100000 | \\\n"
"        brutefir /tmp/bfconf | ...\n"
);
}

/* utility to get modification time of a file in nsec precision */
double mtimens (char* pnam) {
  struct stat sb;
  if (stat(pnam, &sb) == -1)
     return 0.0;
  else
    return (double)sb.st_mtim.tv_sec + (double) sb.st_mtim.tv_nsec*0.000000001;
}

/* read parameters for vol, delay, att from file
   use init==1 during initialization, program will terminate in case of
   problem; later use init==0, if a problem occurs, the parameters are
   just left as they are                                                */
int getraceparams(char* pnam, double* vp, int* delay, double* att, int init) {
  FILE* params;
  int ok;
  params = fopen(pnam, "r");
  if (!params) {
     if (init) {
       fprintf(stderr, "resample_soxr_convolve: Cannot open %s.\n", pnam);
       fflush(stderr);
       exit(2);
     } else
       return 0;
  }
  ok = fscanf(params, "%lf %d %lf", vp, delay, att);
  if (ok == EOF || ok == 0) {
     fclose(params);
     if (init) {
       fprintf(stderr, "resample_soxr_convolve: Cannot read parameters from  %s.\n", pnam);
       fflush(stderr);
       exit(3);
     }  else
       return 0;
  }
  if (ok < 3) {
     /* allow only volume in file, disable RACE */
     *delay = 12;
     *att = 0.0;
  }
  fclose(params);
  return 1;
}

/* correct too bad values */
void sanitizeraceparams(int* delay, double* att, long nch){
    if (*delay < 1 || *delay > 512) {
        fprintf(stderr, "resample_soxr_convolve: Invalid race delay, disabling.\n"); fflush(stderr);
        *delay = 12;
        *att = 0.0;
    }
    if (*att < -0.95 || *att > 0.95) {
        fprintf(stderr, "resample_soxr_convolve: Invalid race att, using 0.0 (disabled).\n"); fflush(stderr);
        *att = 0.0;
    }
    if (nch != 2 && *att != 0.0) {
        fprintf(stderr, "resample_soxr_convolve: race only possible for stereo, disabling.\n");
        *att = 0.0;
    }
}


/* The convolver thread: the main thread fills blocks of the convolver
   block length with frames and hands them over in a ring of NCBLOCKS
   blocks; the convolver thread processes them and writes the output.
   The last block contains less than blen frames (maybe 0).           */
#define NCBLOCKS 4
struct convthread {
  msconv *ms;
  int blen;
  double *blk[NCBLOCKS];
  long len[NCBLOCKS];      /* number of frames in block */
  int head, tail, count;   /* next block to fill/process, number of
                              blocks ready for processing             */
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int cpu, out32, by4;
  long nrcp;
  void *tbufs[512];
  int32_t *iout;
};

static void setcpu(int cpu)
{
  cpu_set_t set;

  CPU_ZERO(&set);
  CPU_SET(cpu, &set);
  if (pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0) {
    fprintf(stderr, "resample_soxr_convolve: cannot set affinity to CPU %d.\n", cpu);
    exit(20);
  }
}

/* main thread: wait for a free block and return it */
static double *ct_getblock(struct convthread *ct)
{
  pthread_mutex_lock(&ct->mutex);
  while (ct->count == NCBLOCKS)
    pthread_cond_wait(&ct->cond, &ct->mutex);
  pthread_mutex_unlock(&ct->mutex);
  return ct->blk[ct->head];
}

/* main thread: hand over block with len frames */
static void ct_putblock(struct convthread *ct, long len)
{
  pthread_mutex_lock(&ct->mutex);
  ct->len[ct->head] = len;
  ct->head = (ct->head + 1) % NCBLOCKS;
  ct->count++;
  pthread_cond_signal(&ct->cond);
  pthread_mutex_unlock(&ct->mutex);
}

static void *convloop(void *arg)
{
  struct convthread *ct = arg;
  double *buf;
  char *data;
  long len, pos, n, rlen, i;

  if (ct->cpu >= 0)
    setcpu(ct->cpu);
  pos = 0;
  do {
    pthread_mutex_lock(&ct->mutex);
    while (ct->count == 0)
      pthread_cond_wait(&ct->cond, &ct->mutex);
    pthread_mutex_unlock(&ct->mutex);
    buf = ct->blk[ct->tail];
    len = ct->len[ct->tail];
    if (len < ct->blen)
      memset(buf + 2 * len, 0, 2 * (ct->blen - len) * sizeof(double));
    msconv_process(ct->ms, buf);
    if (ct->by4 || ct->out32) {
      memclean((char*)ct->iout, 2 * ct->blen * sizeof(int32_t));
      if (ct->by4)
        n = msconv_by4int32(buf, len, pos, ct->iout);
      else {
        msconv_toint32(buf, len, ct->iout);
        n = len;
      }
      data = (char*)ct->iout;
      rlen = 2 * sizeof(int32_t) * n;
    } else {
      data = (char*)buf;
      rlen = 2 * sizeof(double) * len;
    }
    /* write output, optionally after cleaning */
    if (ct->nrcp) {
      ct->tbufs[0] = data;
      ct->tbufs[ct->nrcp] = data;
      for (i = 1; i <= ct->nrcp; i++) {
        memclean((char*)(ct->tbufs[i]), rlen);
        cprefresh((char*)(ct->tbufs[i]), (char*)(ct->tbufs[i-1]), rlen);
      }
    } else {
      refreshmem(data, rlen);
    }
    if (fwrite(data, 1, rlen, stdout) != (size_t)rlen) {
      fprintf(stderr, "resample_soxr_convolve: Error in write.\n");
      fflush(stderr);
      exit(2);
    }
    fflush(stdout);
    pos += len;
    pthread_mutex_lock(&ct->mutex);
    ct->tail = (ct->tail + 1) % NCBLOCKS;
    ct->count--;
    pthread_cond_signal(&ct->cond);
    pthread_mutex_unlock(&ct->mutex);
  } while (len == ct->blen);
  return NULL;
}

int main(int argc, char *argv[])
{
  /* variables for the resampler */
  double inrate, outrate, phase, bwidth, prec, rt, OLEN;
  double *inp, *out;
  char ibuf[PS];
  short *sptr;
  int *iptr;
  float *fptr;
  double *dptr;
  int32_t *iout = NULL;
  int verbose, optc, fd, out32, bpf, bits;
  long intotal = 0, outtotal = 0, blen, mlen, check, i, nch, nfid, nsec;
  long dtime;
  soxr_t soxr;
  soxr_error_t error;
  size_t indone, outdone;
  /* variables for optional input from sound file/shared memory */ 
  char *fnam, *memname, *nfinfo;
  struct stat sb;
  sf_count_t start, until, total;
  SNDFILE *sndfile=NULL;
  SF_INFO sfinfo;
  /* variables for optional volume and race parameters */
  double gain, vol, vg, nvol, att, natt, vdiff=0.0, carry[1024];
  double ptime=0.0, ntime;
  int delay, ndelay, change;
  long fadinglength, fadecount=-1;
  char *pnam;
  struct nfrec *nfr;
  /* buffer for optional output refresh */
  long nrcp, nricp, rlen;
  void *tbufs[512];
  void *tibufs[512];
  struct timespec mtime;
  struct timespec currtime;
  long cnt;
  /* variables for optional convolution in second thread */
  char *sfilt, *lfilt, *rfilt, *wisdom, *cpus;
  double mfac, sfac, *cblk;
  int cblen, by4, tail, rcpu, ccpu;
  long cfill, k, rest;
  struct convthread *ct;
  pthread_t cthread;

  for(i=0; i<1024; carry[i] = 0.0, i++);

  if (argc == 1) {
      usage();
      exit(1);
  }
  /* read command line options */
  static struct option longoptions[] = {
      {"inrate", required_argument, 0, 'i' },
      {"outrate", required_argument, 0, 'o' },
      {"phase", required_argument, 0, 'P' },
      {"band-width", required_argument, 0, 'B' },
      {"precision", required_argument, 0, 'e' },
      {"replay-gain", required_argument, 0, 'r' },
      {"volume", required_argument, 0, 'v' },
      {"race-delay", required_argument, 0, 'd' },
      {"race-volume", required_argument, 0, 'a' },
      {"param-file",  required_argument, 0, 'F'},
      {"fading-length", required_argument, 0, 'l' },
      {"channels", required_argument, 0, 'c' },
      {"file", required_argument, 0, 'f' },
      {"shmname", required_argument, 0, 'm' },
      {"nfinfo", required_argument, 0, 'M' },
      {"nfid", required_argument, 0, 'N' },
      {"start", required_argument, 0, 's' },
      {"until", required_argument, 0, 'u' },
      {"realtime", required_argument, 0, 'T' },
      {"number-frames", required_argument, 0, 'n' },
      {"buffer-length", required_argument, 0, 'b' },
      {"number-copies", required_argument, 0, 'R' },
      {"number-copies-input", required_argument, 0, 'C' },
      {"toint32", no_argument, 0, 'I' },
      {"Sfilter", required_argument, 0, 'S' },
      {"LRfilters", required_argument, 0, 'L' },
      {"m-factor", required_argument, 0, 1001 },
      {"s-factor", required_argument, 0, 1002 },
      {"conv-block-length", required_argument, 0, 1003 },
      {"wisdom-file", required_argument, 0, 'w' },
      {"by4int32", no_argument, 0, '4' },
      {"tail", no_argument, 0, 't' },
      {"cpus", required_argument, 0, 1004 },
      {"verbose", no_argument, 0, 'p' },
      {"version", no_argument, 0, 'V' },
      {"help", no_argument, 0, 'h' },
      {0,         0,        0,   0 }
  };
  /* defaults */
  inrate = 44100.0;
  outrate = 192000.0;
  out32 = 0;
  phase = 25.0; 
  bwidth = 0.0;
  prec = 33.0;
  rt = 0.0;
  nch = 2;
  blen = 8192;
  fnam = NULL;
  memname = NULL;
  nfinfo = NULL;
  nfr = NULL;
  bpf = 0;
  bits = 0;
  nfid = -2;
  start = 0;
  until = 0;
  total = 0;
  gain = 1.0;
  vol = 1.0;
  att = 0.0;
  delay = 12;
  fadinglength = 44100;
  pnam = NULL;
  verbose = 0;
  nrcp = 0;
  nricp = 0;
  sfilt = NULL;
  lfilt = NULL;
  rfilt = NULL;
  wisdom = NULL;
  cpus = NULL;
  mfac = 0.4;
  sfac = 0.8;
  cblen = 16384;
  by4 = 0;
  tail = 0;
  rcpu = -1;
  ccpu = -1;
  ct = NULL;
  while ((optc = getopt_long(argc, argv, 
          "M:N:i:o:P:B:e:T:r:v:d:a:F:l:c:f:m:s:u:n:C:R:b:IS:L:w:4tpVh",
          longoptions, &optind)) != -1) {
      switch (optc) {
      case 'v':
        vol = atof(optarg);
        break;
      case 'r':
        gain = atof(optarg);
        break;
      case 'i':
        inrate = atof(optarg);
        break;
      case 'o':
        outrate = atof(optarg);
        break;
      case 'P':
        phase = atof(optarg);
        if (phase < 0.0 || phase > 100.0)
           phase = 25.0;
        break;
      case 'B':
        bwidth = atof(optarg);
        if (bwidth < 74.0 || bwidth > 99.7)
           bwidth = 0.0;
        break;
      case 'e':
        prec = atof(optarg);
        if (prec < 16.0 || prec > 33.0)
           prec = 33.0;
        break;
      case 'T':
        rt = atof(optarg);
        break;
      case 'c':
        nch = atoi(optarg);
        break;
      case 's':
        start = atoi(optarg);
        break;
      case 'u':
        until = atoi(optarg);
        break;
      case 'n':
        total = atoi(optarg);
        break;
      case 'R':
        nrcp = atoi(optarg);
        if (nrcp < 0 || nrcp > 510) nrcp = 0;
        break;
      case 'C':
        nricp = atoi(optarg);
        if (nricp < 0 || nricp > 510) nricp = 0;
        break;
      case 'b':
        blen = atoi(optarg);
        if (blen < 256)
            blen = 8192;
        break;
      case 'f':
        fnam = strdup(optarg);
        break;
      case 'm':
        memname = strdup(optarg);
        break;
      case 'M':
        nfinfo = strdup(optarg);
        break;
      case 'N':
        nfid = atoi(optarg);
        break;
      case 'd':
        delay = atoi(optarg);
        break;
      case 'a':
        att = atof(optarg);
        break;
      case 'l':
        fadinglength = atoi(optarg);
        if (fadinglength < 1 || fadinglength > 400000) {
           fadinglength = 44100;
        }
        break;
      case 'F':
        pnam = strdup(optarg);
        getraceparams(pnam, &vol, &delay, &att, 1);
        break;
      case 'I':
        out32 = 1;
        break;
      case 'S':
        sfilt = strdup(optarg);
        break;
      case 'L':
        lfilt = strdup(optarg);
        rfilt = strchr(lfilt, ',');
        if (!rfilt || rfilt[1] == '\0' || rfilt == lfilt) {
          fprintf(stderr, "resample_soxr_convolve: need two file names in --LRfilters.\n");
          exit(1);
        }
        *rfilt++ = '\0';
        break;
      case 1001:
        mfac = atof(optarg);
        break;
      case 1002:
        sfac = atof(optarg);
        break;
      case 1003:
        cblen = atoi(optarg);
        if (cblen < 16) {
          fprintf(stderr, "resample_soxr_convolve: convolver block length too small.\n");
          exit(1);
        }
        break;
      case 'w':
        wisdom = strdup(optarg);
        break;
      case 't':
        tail = 1;
        break;
      case '4':
        by4 = 1;
        break;
      case 1004:
        cpus = strdup(optarg);
        break;
      case 'p':
        verbose = 1;
        break;
      case 'V':
        fprintf(stderr, "resample_soxr_convolve (version %s of frankl's stereo utilities)\n",
                VERSION);
        exit(0);
      default:
        usage();
        exit(1);
      }
  }
  if (until != 0) 
      total = until-start;
  if (total < 0) {
      fprintf(stderr, "resample_soxr_convolve: nothing to read.\n");
      exit(1);
  }
  if (cpus) {
      if (sscanf(cpus, "%d,%d", &rcpu, &ccpu) < 1 || rcpu < 0) {
          fprintf(stderr, "resample_soxr_convolve: invalid argument of --cpus.\n");
          exit(1);
      }
      if (ccpu < 0)
          ccpu = rcpu;
      setcpu(rcpu);
  }
  sanitizeraceparams(&delay, &att, nch);
  /* remember modification time of race parameter file */
  if (pnam)
      ptime = mtimens(pnam);

  /* open sound file if fnam or memname given */
  sfinfo.format = 0;
  if (fnam != NULL) {
      if (verbose) {
          fprintf(stderr, "resample_soxr_convolve: opening file %s.\n", fnam);
          fflush(stderr);
      }
      fd = open(fnam, O_RDONLY | O_NOATIME);
      sndfile = sf_open_fd(fd, SFM_READ, &sfinfo, 1);
      if (sndfile == NULL) {
          fprintf(stderr, "resample_soxr_convolve: cannot open file %s.\n", fnam);
          exit(2);
      }
  } else if (memname != NULL) {
      if (verbose) {
          fprintf(stderr, "resample_soxr_convolve: opening shared memory as soundfile.\n");
          fflush(stderr);
      }
      if ((fd = shm_open(memname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1){
	  fprintf(stderr, "resample_soxr_convolve: Cannot open memory %s.\n", memname);
	  exit(3);
      }
      if (fstat(fd, &sb) == -1) {
	  fprintf(stderr, "resample_soxr_convolve: Cannot stat shared memory %s.\n", memname);
	  exit(4);
      }
      sndfile = sf_open_fd(fd, SFM_READ, &sfinfo, 1);
      if (sndfile == NULL) {
          fprintf(stderr, "resample_soxr_convolve: cannot open stdin as sound file.\n"
                  "(%s)\n", sf_strerror(NULL));
          exit(7);
      }
  } else if (nfinfo != NULL) {
      nfr = nfopen(nfinfo, nfid, O_RDONLY, 0);
      if (nfr == NULL) {
          fprintf(stderr, "resample_soxr_convolve: cannot open nf file: %s id=%ld.",
                 nfinfo, nfid);
          exit(15);
      }
      inrate = nfr->info1;
      bits = nfr->info2;
      if (bits == 16) bpf = 4;
      else bpf = 8;
      if (start != 0) {
          nfclose();
          nfr = nfopen(nfinfo, nfid, O_RDONLY, start*bpf);
      }
      if (total == 0) total = (nfr->length)/bpf - start;
      if ((start + total)*bpf > nfr->length) total = (nfr->length)/bpf - start;
      blen = PS/bpf;
  }

  if (sndfile) { 
      /* seek to start */
      if (start != 0 && sfinfo.seekable) {
          if (verbose) {
              fprintf(stderr, "resample_soxr_convolve: seeking to frame %ld.\n", (long)start);
              fflush(stderr);
          }
          sf_seek(sndfile, start, SEEK_SET);
      }
      if (verbose && total != 0) {
          fprintf(stderr, "resample_soxr_convolve: writing (up to) %ld frames.\n", (long)total);
          fflush(stderr);
      }
      nch = sfinfo.channels;
      inrate = (double)(sfinfo.samplerate);
  } else {
      /* no seeking in stdin, ignore those arguments */
      if (!nfinfo) total = 0;
  }

  if (verbose) {
     fprintf(stderr, "resample_soxr_convolve: ");
     fprintf(stderr, "vol %.3f, input rate %.3f output rate %.3f.\n", 
                     (double)(vol*gain), (double)inrate, (double)outrate);
  }

  /* allocate buffer */
  inp = (double*) malloc(nch*blen*sizeof(double));
  if (nricp) {
      /* temporary buffers */
      for (i=1; i < nricp; i++) {
          if (posix_memalign(tibufs+i, 4096, nch*blen*sizeof(double))) {
              fprintf(stderr, "resample_soxr_convolve: Cannot allocate buffer for cleaning.\n");
              exit(8);
          }
      }
  }
  OLEN = (long)(blen*(outrate/inrate+1.0));
  out = (double*) malloc(nch*OLEN*sizeof(double));
  if (out32) 
      iout = (int32_t*) malloc(nch*OLEN*sizeof(int32_t));
  if (nrcp && !(sfilt || lfilt)) {
      /* temporary buffers (with convolution they are allocated below) */
      for (i=1; i < nrcp; i++) {
          if (posix_memalign(tbufs+i, 4096, nch*OLEN*sizeof(double))) {
              fprintf(stderr, "resample_soxr_convolve: Cannot allocate buffer for cleaning.\n");
              exit(2);
          }
      }
  }

  /* create resampler for 64 bit floats and high quality */
  /* the paramters are documented in the soxr.h file, see
          https://sourceforge.net/p/soxr/code/ci/master/tree/src/soxr.h
     the 0x17 sets the parameters as SOXR_32_BITQ + SOXR_INTERMEDIATE_PHASE
     and we can change specific components afterwards.  */
  soxr_quality_spec_t  q_spec = soxr_quality_spec(0x17, 0);
  q_spec.phase_response = phase;
  q_spec.precision = prec;
  if (bwidth != 0.0)
     q_spec.passband_end = bwidth/100.0;
  if (verbose) {
      fprintf(stderr, "resample_soxr_convolve: resampling with precision %.3f, "
              "phase %.3f, bandwidth %.3f\n", q_spec.precision,
              q_spec.phase_response, q_spec.passband_end*100.0);
      fflush(stderr);
  }
  soxr_io_spec_t const io_spec = soxr_io_spec(SOXR_FLOAT64_I,SOXR_FLOAT64_I);
  soxr_runtime_spec_t const runtime_spec = soxr_runtime_spec(1);

  /* now we can create the resampler */
  soxr = soxr_create(inrate, outrate, nch, &error, 
                     &io_spec, &q_spec, &runtime_spec);
  if (error) {
    fprintf(stderr, "resample_soxr_convolve: Cannot initialize resampler.\n");
    fflush(stderr);
    exit(1);
  }

  /* optional convolver in second thread */
  if (sfilt || lfilt) {
      if (nch != 2) {
          fprintf(stderr, "resample_soxr_convolve: convolution only for stereo input.\n");
          exit(1);
      }
      ct = calloc(1, sizeof(struct convthread));
      if (!ct) {
          fprintf(stderr, "resample_soxr_convolve: out of memory.\n");
          exit(2);
      }
      if (wisdom && !fftconv_loadwisdom(wisdom) && verbose)
          fprintf(stderr, "resample_soxr_convolve: no wisdom read from %s\n", wisdom);
      ct->ms = msconv_new(sfilt, lfilt, rfilt, cblen, mfac, sfac, verbose);
      if (wisdom)
          fftconv_savewisdom(wisdom);
      ct->blen = cblen;
      for (i = 0; i < NCBLOCKS; i++) {
          ct->blk[i] = (double*) malloc(2 * cblen * sizeof(double));
          if (!ct->blk[i]) {
              fprintf(stderr, "resample_soxr_convolve: out of memory.\n");
              exit(2);
          }
      }
      ct->iout = (int32_t*) malloc(2 * cblen * sizeof(int32_t));
      /* the temporary buffers for --number-copies are used in the
         convolver thread */
      ct->nrcp = nrcp;
      nrcp = 0;
      for (i = 1; i < ct->nrcp; i++) {
          if (posix_memalign(ct->tbufs+i, 4096, 2*cblen*sizeof(double))) {
              fprintf(stderr, "resample_soxr_convolve: Cannot allocate buffer for cleaning.\n");
              exit(2);
          }
      }
      ct->out32 = out32;
      ct->by4 = by4;
      ct->cpu = cpus ? ccpu : -1;
      pthread_mutex_init(&ct->mutex, NULL);
      pthread_cond_init(&ct->cond, NULL);
      if (pthread_create(&cthread, NULL, convloop, ct) != 0) {
          fprintf(stderr, "resample_soxr_convolve: cannot create convolver thread.\n");
          exit(2);
      }
  } else if (by4 || tail || ccpu != rcpu) {
      fprintf(stderr, "resample_soxr_convolve: --by4int32, --tail and two CPUs "
              "in --cpus only with convolution.\n");
      exit(1);
  }
  cblk = NULL;
  cfill = 0;

  /* we use a timer for reading the input in realtime mode */
  if (rt != 0.0) {
    nsec = (rt*blen/inrate)*1000000000;
    clock_gettime(CLOCK_MONOTONIC, &mtime);
    mtime.tv_nsec += nsec;
    if (mtime.tv_nsec > 999999999) {
      mtime.tv_nsec -= 1000000000;
      mtime.tv_sec++;
    }
  }
  /* we read from stdin or file/shared mem/nf file until eof or total (if >0) and write to stdout */
  cnt = 0;
  while (1) {
    cnt++;
    mlen = blen;
    if (total != 0) {
        if (intotal >= total)
            mlen = 0;
        if (intotal < total && total - intotal < blen) 
            mlen = total - intotal;
    }
    /* read input block */
    memclean((char*)inp, nch*sizeof(double)*mlen);
       
    if (rt != 0.0) {
      /* we check every 1000 loops if we are behind time and in that case
         reset (for example after startup and filling buffers) */
      if (cnt >= 1000) {
        cnt = 0;
        clock_gettime(CLOCK_MONOTONIC, &currtime);
        dtime = ((mtime.tv_sec - currtime.tv_sec)*1000000000 + mtime.tv_nsec -currtime.tv_nsec)/1000;
        if (verbose) {
          fprintf(stderr, "\nmusec to wait in realtime mode: %ld \n", dtime);
        }
        if (dtime < -1000) {
          clock_gettime(CLOCK_MONOTONIC, &mtime);
          mtime.tv_nsec += nsec;
          if (mtime.tv_nsec > 999999999) {
            mtime.tv_nsec -= 1000000000;
            mtime.tv_sec++;
          }
        }
      }
      clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &mtime, NULL);
    }
    if (sndfile) {
        mlen = sf_readf_double(sndfile, inp, mlen);
    }
    else if (nfinfo) {
        if (mlen == blen) {
             memclean((char*)ibuf, PS);
             mlen = nfreadpage((void*)ibuf)/bpf;
        }
        if (bpf==4) {
             for (sptr = (short*)ibuf, dptr = inp, i=0; i<2*mlen; 
                  i++, sptr++, dptr++)
                *dptr = (double)(*sptr)/32768.0;
        } else if (bits == 28) {
             for (fptr = (float*)ibuf, dptr = inp, i=0; i<2*mlen; 
                  i++, fptr++, dptr++)
                *dptr = (double)(*fptr);
        } else {
             for (iptr = (int*)ibuf, dptr = inp, i=0; i<2*mlen; 
                  i++, iptr++, dptr++)
                *dptr = (double)(*iptr)/2147483648.0;
        }
    }       
    else
        mlen = fread((void*)inp, nch*sizeof(double), mlen, stdin);
    if (mlen == 0) { 
      /* done */
      free(inp);
      inp = NULL;
    }
    /* call resampler, optionally clean input in memory */
    if (nricp) {
        rlen = nch*sizeof(double)*mlen;
        tibufs[0] = (void*)inp;
        tibufs[nricp] = (void*)inp;
        for (i=1; i <= nricp; i++) {
            memclean((char*)(tibufs[i]), rlen);
            cprefresh((char*)(tibufs[i]), (char*)(tibufs[i-1]), rlen);
            //memclean((char*)(tibufs[i-1]), rlen);
        }
    } else {
        refreshmem((char*)inp, nch*sizeof(double)*mlen);
    }
    error = soxr_process(soxr, inp, mlen, &indone,
                               out, OLEN, &outdone);
    if (mlen > indone) {
      fprintf(stderr, "resample_soxr_convolve: only %ld/%ld processed.\n",(long)indone,(long)mlen);
      fflush(stderr);
    }
    if (error) {
      fprintf(stderr, "resample_soxr_convolve: error (%s).\n", soxr_strerror(error));
      fflush(stderr);
      exit(3);
    }
    /* apply volume change */
    if (vol != 1.0 || gain != 1.0 || fadecount > 0) {
        for(i=0, vg=vol*gain; i<2*outdone; i++) {
            out[i] *= vg;
            /* change vol slightly while fading to new one */
            if (fadecount > 0) {
              vol += vdiff;
              vg = vol*gain;
              fadecount--;
              if (fadecount == 0) {
                  /* now set vol to precisely the new value and disable
                     fading */
                  vol = nvol;
                  vg = vol*gain;
                  fadecount = -1;
              }
            }
        }
    }
    /* apply race filter if output is longer than delay */
    if (att != 0.0 && outdone > delay) {
        for(i=0; i<delay; i++) {
            out[2*i+1] -= att*carry[2*i];
            out[2*i] -= att*carry[2*i+1];
        }
        for(i=delay; i<outdone; i++) {
            out[2*i+1] -= att*out[2*(i-delay)];
            out[2*i] -= att*out[2*(i-delay)+1];
        }
        for(i=0; i<delay; i++) {
            carry[2*i] = out[2*(outdone-delay+i)];
            carry[2*i+1] = out[2*(outdone-delay+i)+1];
        }
    }
 
    if (ct) {
        /* hand over output to convolver thread */
        for (i = 0; i < outdone; i += k) {
            if (!cblk) {
                cblk = ct_getblock(ct);
                cfill = 0;
            }
            k = cblen - cfill;
            if (k > (long)outdone - i)
                k = outdone - i;
            memcpy(cblk + 2*cfill, out + 2*i, 2*k*sizeof(double));
            cfill += k;
            if (cfill == cblen) {
                ct_putblock(ct, cfill);
                cblk = NULL;
            }
        }
        check = outdone;
    } else {
    /* write output, optionally after cleaning  */
    if (nrcp) {
        rlen = nch*sizeof(double)*outdone;
        tbufs[0] = (void*)out;
        tbufs[nrcp] = (void*)out;
        for (i=1; i <= nrcp; i++) {
            memclean((char*)(tbufs[i]), rlen);
            cprefresh((char*)(tbufs[i]), (char*)(tbufs[i-1]), rlen);
            //memclean((char*)(tbufs[i-1]), rlen);
        }
    } else {
        refreshmem((char*)out, nch*sizeof(double)*outdone);
    }
    if (out32) {
        memclean((char*)iout, nch*sizeof(int32_t)*outdone);
        for (i=0; i<nch*outdone; i++)
            iout[i] = (int32_t) (out[i] * 2147483647);
        refreshmem((char*)iout, nch*sizeof(int32_t)*outdone);
        check = fwrite((void*)iout, nch*sizeof(int32_t), outdone, stdout);
    } else {
        check = fwrite((void*)out, nch*sizeof(double), outdone, stdout);
    }
    fflush(stdout);
    }
    memclean((char*)out, nch*sizeof(double)*outdone);
    /* this should not happen, the whole block should be written */
    if (check < outdone) {
        fprintf(stderr, "resample_soxr_convolve: Error in write.\n");
        fflush(stderr);
        exit(2);
    }
    intotal += mlen;
    outtotal += outdone;
    if (mlen == 0)
      break;

    /* if we are not fading to new volume/race parameters we check after each 
       block if the modification time of the parameter file has changed; if yes
       we try to read new values; in case of a problem we stay with the old
       parameters */
    if (pnam != NULL && fadecount < 0) {
        ntime = mtimens(pnam);
        if (ntime > ptime+0.00001) {
           change = getraceparams(pnam, &nvol, &ndelay, &natt, 0);
           if (change) {
             sanitizeraceparams(&ndelay, &natt, nch);
             /* we fade to new vol within a number of frames */
             fadecount = 2*fadinglength;
             vdiff = (nvol-vol)/fadecount;
             att = natt;
             ptime = ntime;
             delay = ndelay;
             if (verbose) {
                fprintf(stderr, "resample_soxr_convolve: Reread new race parameters: (%f) ", ntime);
                fprintf(stderr, "vol %.3f, race att %.3f delay %ld.\n", 
                               (double)(nvol*gain), (double)natt, (long)ndelay);
             }
           }
        }
    }
    if (rt != 0.0) {
      mtime.tv_nsec += nsec;
      if (mtime.tv_nsec > 999999999) {
        mtime.tv_nsec -= 1000000000;
        mtime.tv_sec++;
      }
    }
  }
  if (ct) {
      /* optionally append silence for the filter tail */
      for (rest = tail ? msconv_taillength(ct->ms) : 0; rest > 0; rest -= k) {
          if (!cblk) {
              cblk = ct_getblock(ct);
              cfill = 0;
          }
          k = cblen - cfill;
          if (k > rest)
              k = rest;
          memset(cblk + 2*cfill, 0, 2*k*sizeof(double));
          cfill += k;
          if (cfill == cblen) {
              ct_putblock(ct, cfill);
              cblk = NULL;
          }
      }
      /* hand over last (partial, maybe empty) block and wait for the
         convolver thread */
      if (!cblk) {
          cblk = ct_getblock(ct);
          cfill = 0;
      }
      ct_putblock(ct, cfill);
      pthread_join(cthread, NULL);
  }
  soxr_delete(soxr);
  free(out);
  sf_close(sndfile);
  if (verbose) {
    fprintf(stderr, "resample_soxr_convolve: %ld input and %ld output samples\n", 
            (long)intotal, (long)outtotal);
  }
  return(0);
}

