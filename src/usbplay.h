/*
usbplay.h                Copyright frankl 2025-2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Player thread which sends the audio data directly to a USB Audio Class 2
device with asynchronous feedback (e.g., a DDC), without ALSA and
snd-usb-audio. It uses libusb; the kernel driver is detached from the
audio interfaces while playing and reattached afterwards.

As in c4play the player is fed from a lock free ring buffer (single
producer, single consumer). The player thread fills isochronous transfers
from the ring and submits them at precise times (sleep and active loop on
the 24 MHz system counter as in myplayhrt_c4), all USB event handling is
done in this thread as well.

Sample format is S32_LE with 2 channels (32-bit alternate setting).
*/

typedef struct usbplay usbplay;

/* open and configure the USB device, allocate a ring of ringsize bytes
   (rounded to full frames); exits with a message in case of a problem
   device:     "vid:pid" in hex, e.g. "152a:85dd"
   rate:       sample rate
   loopspersec: transfers per second, must divide the number of USB
               packets per second (8000 for high speed)
   ntransfers: number of transfers, about half of them are in flight
   shift:      ns before the submit at which the sleep ends
   warmup:     system call directly after the sleep
   nrcp:       copies through cleaned buffers before the submit
   zerocopy:   use transfer buffers mapped from the kernel (no copy in
               the kernel at submit), falls back to normal memory
   cpu, prio:  CPU and SCHED_FIFO priority of the thread (-1, 0: no change)
*/
usbplay *usbplay_new(const char *device, int rate, long loopspersec,
                     int ntransfers, long shift, int warmup, int nrcp,
                     int zerocopy, long ringsize, int cpu, int prio,
                     int verbose);

/* start the player thread, it starts playing when the ring is full or
   usbplay_finish was called */
void usbplay_start(usbplay *p);

/* producer: copy len bytes into the ring (waits for enough space) */
void usbplay_write(usbplay *p, const char *data, long len);

/* for statistics (from any thread): current fill of the ring in ms, the
   smallest fill (in ms) before a write of the producer since the last call
   (-1 if no write since then), how often the player waited for input,
   how often the timing was resynchronized to the USB bus and the
   current feedback value (frames per second) */
void usbplay_fill(usbplay *p, double *ms, double *minms, long *waits,
                  long *resyncs, double *fbrate);

/* for diagnostics: write into buf a line with the range of transfers in
   flight, the maximal delays of the player loop and the USB errors since
   the last call */
void usbplay_stats(usbplay *p, char *buf, int len);

/* producer: mark end of input, wait until all is played, release device */
void usbplay_finish(usbplay *p);
