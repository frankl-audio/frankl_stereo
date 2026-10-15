/*
readlat.c                Copyright frankl 2026

Measures the duration of each read of one or more files with O_DIRECT
(no page cache, no readahead), e.g., to compare bit-identical copies of
a music file on the same device. The files are read in several passes,
in alternating order, and for each file and pass the distribution of
the read times is printed.

Compile:  gcc -O2 -Wall -o readlat readlat.c

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sched.h>
#include <sys/stat.h>

static void usage(void)
{
  fprintf(stderr,
"Usage:  readlat [options] file1 [file2 ...]\n"
"\n"
"  Reads the files with O_DIRECT in blocks and measures the duration of\n"
"  each read. Prints for each file and pass the read times in us\n"
"  (median, 90%%, 99%%, 99.9%%, maximum, mean) and at the end the same\n"
"  for all passes together.\n"
"\n"
"Options:\n"
"  -b n   block size in bytes, multiple of 4096 (default 4096)\n"
"  -n n   number of passes (default 5); odd passes read the files in\n"
"         reverse order\n"
"  -r n   reads per second (default 0: as fast as possible); e.g. with\n"
"         -b 4096 -r 12 about the rate of reading a 16/44.1 flac file\n"
"  -m n   read at most n MB of each file per pass (default: all)\n"
"  -c n   run on CPU n\n"
"  -o f   write all read times (ns) to file f, one line per read:\n"
"         pass file block ns\n");
}

static long now(void)
{
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec * 1000000000L + t.tv_nsec;
}

static int cmpl(const void *a, const void *b)
{
  long x = *(const long *)a, y = *(const long *)b;
  return x < y ? -1 : x > y;
}

/* print distribution of n values (ns) in us */
static void report(const char *label, long *v, long n)
{
  long *s, i;
  double sum = 0.0;

  if (n == 0) {
    printf("%-40s no reads\n", label);
    return;
  }
  s = malloc(n * sizeof(long));
  memcpy(s, v, n * sizeof(long));
  qsort(s, n, sizeof(long), cmpl);
  for (i = 0; i < n; i++)
    sum += s[i];
  printf("%-40s %8ld %8.1f %8.1f %8.1f %8.1f %9.1f %8.2f\n", label, n,
         s[n/2] / 1000.0, s[(long)(n * 0.9)] / 1000.0,
         s[(long)(n * 0.99)] / 1000.0, s[(long)(n * 0.999)] / 1000.0,
         s[n-1] / 1000.0, sum / n / 1000.0);
  free(s);
}

int main(int argc, char *argv[])
{
  int optc, cpu = -1, passes = 5, nf, p, j, f, fd;
  long bs = 4096, rate = 0, maxmb = 0, maxblocks, nb, t0, t1, next, period;
  long **all, *nall, *cur;
  char *buf, *raw = NULL, label[256];
  struct stat sb;
  off_t size0 = 0;
  struct timespec ts;
  FILE *rf = NULL;
  ssize_t r;

  while ((optc = getopt(argc, argv, "b:n:r:m:c:o:h")) != -1) {
    switch (optc) {
    case 'b': bs = atol(optarg); break;
    case 'n': passes = atoi(optarg); break;
    case 'r': rate = atol(optarg); break;
    case 'm': maxmb = atol(optarg); break;
    case 'c': cpu = atoi(optarg); break;
    case 'o': raw = optarg; break;
    default: usage(); exit(1);
    }
  }
  nf = argc - optind;
  if (nf < 1 || bs < 4096 || bs % 4096 != 0 || passes < 1) {
    usage();
    exit(1);
  }
  if (cpu >= 0) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    if (sched_setaffinity(0, sizeof(set), &set) != 0)
      perror("readlat: sched_setaffinity");
  }
  if (posix_memalign((void **)&buf, 4096, bs)) {
    fprintf(stderr, "readlat: out of memory\n");
    exit(2);
  }
  if (raw && !(rf = fopen(raw, "w"))) {
    fprintf(stderr, "readlat: cannot open %s\n", raw);
    exit(2);
  }
  /* maximal number of blocks per file and pass */
  maxblocks = 0;
  for (j = 0; j < nf; j++) {
    if (stat(argv[optind + j], &sb) != 0) {
      fprintf(stderr, "readlat: cannot stat %s: %s\n", argv[optind + j],
              strerror(errno));
      exit(2);
    }
    if (j == 0)
      size0 = sb.st_size;
    else if (sb.st_size != size0)
      fprintf(stderr, "readlat: warning: %s has a different size\n",
              argv[optind + j]);
    nb = (sb.st_size + bs - 1) / bs;
    if (maxmb > 0 && nb > maxmb * 1048576 / bs)
      nb = maxmb * 1048576 / bs;
    if (nb > maxblocks)
      maxblocks = nb;
  }
  all = calloc(nf, sizeof(long *));
  nall = calloc(nf, sizeof(long));
  cur = malloc(maxblocks * sizeof(long));
  for (j = 0; j < nf; j++)
    all[j] = malloc(passes * maxblocks * sizeof(long));
  if (!all || !nall || !cur) {
    fprintf(stderr, "readlat: out of memory\n");
    exit(2);
  }
  period = rate > 0 ? 1000000000L / rate : 0;

  printf("block size %ld bytes, %d passes, %s\n", bs, passes,
         rate > 0 ? "paced" : "as fast as possible");
  if (rate > 0)
    printf("%ld reads per second\n", rate);
  printf("read times in us:\n%-40s %8s %8s %8s %8s %8s %9s %8s\n",
         "file (pass)", "reads", "median", "90%", "99%", "99.9%", "max", "mean");
  for (p = 0; p < passes; p++) {
    for (j = 0; j < nf; j++) {
      f = (p % 2 == 0) ? j : nf - 1 - j;
      fd = open(argv[optind + f], O_RDONLY | O_DIRECT | O_NOATIME);
      if (fd < 0)
        fd = open(argv[optind + f], O_RDONLY | O_DIRECT);
      if (fd < 0) {
        fprintf(stderr, "readlat: cannot open %s with O_DIRECT: %s\n",
                argv[optind + f], strerror(errno));
        exit(3);
      }
      nb = 0;
      next = now();
      while (nb < maxblocks) {
        if (period) {
          next += period;
          ts.tv_sec = next / 1000000000L;
          ts.tv_nsec = next % 1000000000L;
          clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
        }
        t0 = now();
        r = read(fd, buf, bs);
        t1 = now();
        if (r < 0) {
          fprintf(stderr, "readlat: read error in %s: %s\n",
                  argv[optind + f], strerror(errno));
          exit(4);
        }
        if (r == 0)
          break;
        cur[nb] = t1 - t0;
        if (rf)
          fprintf(rf, "%d %d %ld %ld\n", p, f, nb, t1 - t0);
        nb++;
        if (r < bs)
          break;
      }
      close(fd);
      memcpy(all[f] + nall[f], cur, nb * sizeof(long));
      nall[f] += nb;
      snprintf(label, sizeof(label), "%.34s (%d)", argv[optind + f], p + 1);
      report(label, cur, nb);
    }
  }
  printf("all passes:\n");
  for (j = 0; j < nf; j++)
    report(argv[optind + j], all[j], nall[j]);
  if (rf)
    fclose(rf);
  return 0;
}
