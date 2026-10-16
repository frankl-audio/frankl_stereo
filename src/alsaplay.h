/*
alsaplay.h                Copyright frankl 2025-2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Player thread as in myplayhrt_c4 (mmap access, timing with sleep and an
active wait before each commit), fed from a ring buffer in memory. It
works on any Linux system; compiled with -DC4TIMER it uses the 24 MHz
system counter of the Odroid C4 for the active wait.

The ring has a single producer (alsaplay_write) and a single consumer (the
player thread), it is lock free: the player thread never waits for a
lock and makes no system calls besides the ones for sleeping and for
ALSA. The producer sleeps while there is not enough space in the ring.

Sample format is S32_LE with 2 channels.
*/

typedef struct alsaplay alsaplay;

/* open and configure the sound device, allocate a ring of ringsize bytes
   (rounded to full frames); exits with a message in case of a problem
   rate:       sample rate, must be a multiple of loopspersec
   hwbufsize:  frames in the hardware buffer
   extrabps:   as --extra-bytes-per-second of myplayhrt_c4
   shift:      ns before the commit at which the sleep ends
   warmup:     system call directly after the sleep
   nrcp:       copies through cleaned buffers before the commit
   cpu, prio:  CPU and SCHED_FIFO priority of the thread (-1, 0: no change)
*/
alsaplay *alsaplay_new(const char *device, int rate, long loopspersec,
                       long hwbufsize, double extrabps, int nonblock,
                       long shift, int warmup, int nrcp, long ringsize,
                       int cpu, int prio, int verbose);

/* start the player thread, it starts playing when the ring is full or
   alsaplay_finish was called */
void alsaplay_start(alsaplay *p);

/* producer: copy len bytes into the ring (waits for enough space) */
void alsaplay_write(alsaplay *p, const char *data, long len);

/* for statistics (from any thread): current fill of the ring in ms, the
   smallest fill (in ms) before a write of the producer since the last call
   (-1 if no write since then), and how often the player waited for input */
void alsaplay_fill(alsaplay *p, double *ms, double *minms, long *waits);

/* producer: mark end of input, wait until all is played, close device */
void alsaplay_finish(alsaplay *p);
