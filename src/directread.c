/*
directread.c                Copyright frankl 2026

This file is part of frankl's stereo utilities.
See the file License.txt of the distribution and
http://www.gnu.org/licenses/gpl.txt for license details.

See directread.h.
*/
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "directread.h"

#define DRPS 4096

struct directread {
    int fd;
    char *ring;
    long chunk;      /* bytes per read */
    long rsize;      /* ring size in bytes, multiple of chunk */
    long filelen;
    /* the ring contains the file data in [lo, fend); the byte at file
       offset f is at ring index (f - origin) % rsize; origin is page
       aligned and fend - origin is a multiple of chunk (except after a
       short read at the end of file) */
    long origin, lo, fend;
    long pos;        /* read position of libsndfile */
    /* statistics */
    long nreads, nfull, nunder, nrestart, minfill;
};

/* read one chunk at fend, the caller ensures that there is space */
static void readchunk(directread *d)
{
    long off = (d->fend - d->origin) % d->rsize;
    ssize_t got;

    do {
        got = pread(d->fd, d->ring + off, d->chunk, d->fend);
    } while (got < 0 && errno == EINTR);
    if (got < 0) {
        fprintf(stderr, "directread: read error at offset %ld (%s).\n",
                d->fend, strerror(errno));
        exit(2);
    }
    if (got < d->chunk && d->fend + got < d->filelen) {
        fprintf(stderr, "directread: short read at offset %ld.\n", d->fend);
        exit(2);
    }
    d->nreads++;
    if (d->fend + d->chunk - d->rsize > d->lo)
        d->lo = d->fend + d->chunk - d->rsize;
    d->fend += got;
}

/* restart the ring at the page containing pos */
static void restart(directread *d)
{
    d->origin = d->lo = d->fend = d->pos & ~(long)(DRPS-1);
    d->nrestart++;
}

directread *directread_open(char *fname, long chunkpages, long ringpages)
{
    directread *d;
    struct stat sb;

    if (chunkpages < 1) {
        fprintf(stderr, "directread: invalid number of pages per read.\n");
        exit(1);
    }
    if (ringpages < chunkpages)
        ringpages = chunkpages;
    ringpages = ((ringpages + chunkpages - 1) / chunkpages) * chunkpages;
    d = calloc(1, sizeof(directread));
    if (!d) {
        fprintf(stderr, "directread: out of memory.\n");
        exit(1);
    }
    d->chunk = chunkpages * DRPS;
    d->rsize = ringpages * DRPS;
    if (posix_memalign((void**)&d->ring, DRPS, d->rsize)) {
        fprintf(stderr, "directread: cannot allocate ring buffer.\n");
        exit(1);
    }
    memset(d->ring, 0, d->rsize);
    d->fd = open(fname, O_RDONLY | O_DIRECT | O_NOATIME);
    if (d->fd < 0 && errno == EPERM)
        d->fd = open(fname, O_RDONLY | O_DIRECT);
    if (d->fd < 0) {
        fprintf(stderr, "directread: cannot open %s with O_DIRECT (%s).\n",
                fname, strerror(errno));
        exit(2);
    }
    if (fstat(d->fd, &sb) < 0) {
        fprintf(stderr, "directread: cannot stat %s.\n", fname);
        exit(2);
    }
    d->filelen = sb.st_size;
    d->minfill = -1;
    return d;
}

void directread_fill(directread *d)
{
    if (d->pos < d->lo || d->pos > d->fend)
        restart(d);
    while (d->fend < d->filelen && d->fend + d->chunk - d->rsize <= d->pos)
        readchunk(d);
}

void directread_periodic(directread *d)
{
    long fill;

    if (d->pos < d->lo || d->pos > d->fend)
        restart(d);
    if (d->fend >= d->filelen)
        return;
    fill = d->fend - d->pos;
    if (fill < 0)
        fill = 0;
    if (d->minfill < 0 || fill < d->minfill)
        d->minfill = fill;
    if (d->fend + d->chunk - d->rsize <= d->pos)
        readchunk(d);
    else
        d->nfull++;
}

long directread_filelen(directread *d)
{
    return d->filelen;
}

long directread_pos(directread *d)
{
    return d->pos;
}

void directread_stats(directread *d, FILE *f)
{
    fprintf(f, "directread: %ld reads of %ld bytes, %ld loops with full ring, "
            "%ld underruns, %ld restarts, min fill %ld bytes (ring %ld)\n",
            d->nreads, d->chunk, d->nfull, d->nunder, d->nrestart,
            d->minfill, d->rsize);
    d->minfill = -1;
}

void directread_close(directread *d)
{
    close(d->fd);
    free(d->ring);
    free(d);
}

/* the callbacks for libsndfile */

static sf_count_t vio_filelen(void *user)
{
    return ((directread*)user)->filelen;
}

static sf_count_t vio_seek(sf_count_t offset, int whence, void *user)
{
    directread *d = user;
    long np;

    switch (whence) {
    case SEEK_SET: np = offset; break;
    case SEEK_CUR: np = d->pos + offset; break;
    case SEEK_END: np = d->filelen + offset; break;
    default: return -1;
    }
    if (np < 0)
        return -1;
    d->pos = np;
    return np;
}

static sf_count_t vio_read(void *ptr, sf_count_t count, void *user)
{
    directread *d = user;
    char *p = ptr;
    long n, off, k;
    sf_count_t total = 0;

    while (count > 0 && d->pos < d->filelen) {
        if (d->pos < d->lo || d->pos > d->fend)
            restart(d);
        if (d->pos >= d->fend) {
            /* ring is empty: read on demand; this is an underrun if the
               ring is larger than one chunk and was not just restarted */
            if (d->rsize > d->chunk && d->fend > d->origin)
                d->nunder++;
            readchunk(d);
            if (d->pos >= d->fend)
                break;
        }
        n = d->fend - d->pos;
        if (n > count)
            n = count;
        /* copy, maybe in two parts at the end of the ring */
        off = (d->pos - d->origin) % d->rsize;
        k = d->rsize - off;
        if (k > n)
            k = n;
        memcpy(p, d->ring + off, k);
        if (k < n)
            memcpy(p + k, d->ring, n - k);
        p += n;
        d->pos += n;
        count -= n;
        total += n;
    }
    return total;
}

static sf_count_t vio_write(const void *ptr, sf_count_t count, void *user)
{
    (void)ptr; (void)count; (void)user;
    return 0;
}

static sf_count_t vio_tell(void *user)
{
    return ((directread*)user)->pos;
}

SF_VIRTUAL_IO directread_vio = {
    vio_filelen, vio_seek, vio_read, vio_write, vio_tell
};
