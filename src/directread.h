/*
directread.h                Copyright frankl 2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

Input of a sound file with O_DIRECT for libsndfile: the file is read in
chunks of full pages at page aligned file offsets into a page aligned
ring buffer, bypassing the page cache. libsndfile reads (arbitrary sizes
and offsets) via sf_open_virtual and is served from the ring buffer.

Two modes:
  - on demand: the ring holds one chunk, a new chunk is read when
    libsndfile needs data behind it
  - periodic: the ring holds several chunks, the caller reads one chunk
    per loop with directread_periodic (if there is space), reads on
    demand only happen when the ring runs empty (counted as underruns)
A seek of libsndfile outside the data in the ring restarts the ring at
the new position.
*/

#include <sndfile.h>

typedef struct directread directread;

/* the virtual I/O functions, use with
   sf_open_virtual(&directread_vio, SFM_READ, &sfinfo, d) */
extern SF_VIRTUAL_IO directread_vio;

/* open fname with O_DIRECT; chunks of chunkpages pages, ring of ringpages
   pages (rounded up to a multiple of chunkpages); exits with a message
   in case of a problem */
directread *directread_open(char *fname, long chunkpages, long ringpages);

/* read chunks until the ring is full or the end of file is reached */
void directread_fill(directread *d);

/* read one chunk if there is space in the ring and the end of file is
   not reached; call once per loop in periodic mode */
void directread_periodic(directread *d);

/* file length and current read position of libsndfile */
long directread_filelen(directread *d);
long directread_pos(directread *d);

/* print statistics (chunks read, loops without read because the ring was
   full, underruns, restarts, minimal fill) and reset the minimal fill */
void directread_stats(directread *d, FILE *f);

/* close the file and free the ring */
void directread_close(directread *d);
