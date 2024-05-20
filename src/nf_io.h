
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/file.h>

/* block and page size */
#define BS 512
#define PS 4096

/* file name and descriptor of block device */
static char nffn[256];
static int nffd;

/* offset and size in bytes of currently accessed nf_file */
static long nfoff=-1;
static long nfsize=-1;
/* current position */
static long nfpos=-1;

/* open device file, lock it, seek to startpage, and set status variables */
int nfopen_init(char* fn, int flags, long startpage, long len)
{
    off_t off;
    if ((nffd = open(fn, flags)) < 0) {
        fprintf(stderr, "nfopen: Cannot open device file %s.\n", fn);
        exit(1);
    }
    if (flock(nffd, LOCK_EX | LOCK_NB) == -1) {
        fprintf(stderr, "nfopen: Cannot lock device file %s.\n", fn);
        close(nffd);
        exit(2);
    }
    off = startpage * PS;
    if (off != lseek(nffd, off, SEEK_SET)) {
        fprintf(stderr, "nfopen: Cannot seek to %ld.\n", off);
        exit(3);
    }
    strncpy(nffn, fn, 255);
    nfoff = off;
    nfpos = off;
    nfsize = len;
    return nffd; 
}

/* close current open device file */
int nfclose()
{
    fdatasync(nffd);
    flock(nffd, LOCK_UN);
    close(nffd);
    nffd = -1;
    nfoff = -1;
    nfsize = -1;
    nfpos = -1;
    return 0;
}

/* read a page */
int nfreadpage(void* dest)
{   
    int count;
    if (nfpos >= nfoff+nfsize) return 0;
    count = (nfpos + PS <= nfoff + nfsize) ? PS : nfoff+nfsize-nfpos;
    if (read(nffd, dest, count) != count){
        fprintf(stderr, "nfread: Could not read page.\n");
        nfclose();
        exit(4);
    }
    nfpos += PS;
    return count;
}

/* write a page */
int nfwritepage(void* buf)
{
    int count;
    if (nfpos >= nfoff+nfsize) return 0;
    count = (nfpos + PS <= nfoff + nfsize) ? PS : nfoff+nfsize-nfpos;
    if (write(nffd, buf, count) != count){
        fprintf(stderr, "nfwrite: Could not write page.\n");
        nfclose();
        exit(5);
    }
    nfpos += PS;
    return count;
}

/* write (not necessarily full page) */
int nfwrite(void* buf, int len)
{
    int count;
    if (nfpos >= nfoff+nfsize) return 0;
    count = (nfpos + len <= nfoff + nfsize) ? len : nfoff+nfsize-nfpos;
    if (write(nffd, buf, count) != count){
        fprintf(stderr, "nfwrite: Could not write %d bytes.\n", len);
        nfclose();
        exit(5);
    }
    nfpos += count;
    return count;
}

/* we use an external nfinfo file for information about the data in 
   a block device file, we store info in a linked list */
/* struct for single records */
struct nfrec {
    long id; /* running number */
    long startpage; /* number of page where the data start */
    long length;    /* total length in bytes */
    long info1;     /* this and next two for private use */
    long info2;     
    char txt[256];  
    struct nfrec* next;
};

/* an nfinfo starts with the path to the block device file and its total
   length, followed by
   an arbitrary number of either 6 entries as in struct nfrec or one
   negative number meaning to discard the entry with this id;
   the first record had id -1 and the name of the nfinfo file in txt */
struct nfrec*  nfreadinfo(char* fname) {
    FILE *info;
    struct nfrec *start, *p, *del;
    int n; 
    long d;
    if ((info = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "nfreadinfo: Cannot open file %s.\n", fname);
        exit(7);
    }
    start = (struct nfrec*)calloc(1,sizeof(struct nfrec));
    //printf("malloc: %lu \n", (unsigned long)start);
    start->id = -1;
    start->startpage = -1;
    start->next = NULL;
    n = fscanf(info, "%255s", start->txt);
    if (n != 1) {
        fprintf(stderr, "nfreadinfo: file does not start with string.\n");
        exit(8);
    }
    n = fscanf(info, "%ld", &(start->length));
    if (n != 1) {
        fprintf(stderr, "nfreadinfo: total length must be second entry.\n");
        exit(9);
    }
    for(;;) {
        n = fscanf(info, "%ld", &d);
        //printf("n=%d d=%ld\n",n,d);
        if (n != 1) break;
        if (d < 0) {
            for(p=start; p->next; ) {
                 if ((p->next)->id == -d) {
                      del = p->next;
                      p->next = (p->next)->next;
                      free((void*)del);
                      //printf("free: %lu \n", (unsigned long)del);
                 } else {
                      p = p-> next;
                 }
            }
        } else {
            for(p=start; p->next && p->id != d; ) 
                p = p->next;
            if (p->id != d) {
                p->next = (struct nfrec*)calloc(1,sizeof(struct nfrec));
                //printf("malloc: %lu \n", (unsigned long)(p->next));
                p = p->next;
                p->id = d;
            }
            n = fscanf(info,"%ld", &(p->startpage));
            if (n != 1) break;
            n = fscanf(info,"%ld", &(p->length));
            if (n != 1) break;
            n = fscanf(info,"%ld", &(p->info1));
            if (n != 1) break;
            n = fscanf(info,"%ld", &(p->info2));
            if (n != 1) break;
            n = fscanf(info,"%255s", p->txt);
            if (n != 1) break;
        }     
                     
    }
    fclose(info);
    return start;
}

/* allocate new nf file: fname is nfinfo file, len length of new file,
   inf1, inf2, txt is the corresponding info

   the new file is put after the last (with highest id), starting from
   beginning if not enough space

   existing nf files in that range are discarded

   return value is id of new nf file, the corresponding data are appended 
   to the nf info file      */


long nfnew(char* fname, long len, long inf1, long inf2, char* txt) {
    struct nfrec *start, *p, *max;
    long newid, nsp, nspb, neb, b, e, i;
    char ttxt[256];
    FILE* finfo;

    /* safe copy of txt */
    for (i=0; txt[i] && i < 255; i++) {
         if (isspace(txt[i]))
             ttxt[i] = '_';
         else
             ttxt[i] = txt[i];
    }
    ttxt[i] = '\0';

    start = nfreadinfo(fname);
    max = start;
    for(p=start; p; p=p->next) {
        if (max->id < p->id) max = p;
    }
    /* compute start page */
    if (max->id == -1) {
        nsp = 0;
        newid = 1;
    } else {
        nsp = max->startpage + (max->length + PS)/PS;
        newid = max->id + 1;
    }
    nspb = nsp*PS;
    neb = nspb + len;
    if (neb >= start->length) {
        /* start from beginning */
        nsp = 0;
        nspb = 0;
        neb = len + PS - (len % PS) - 1;
        if (neb >= start->length) {
            fprintf(stderr, 
               "nfnew: requested file size %ld larger than device (%ld bytes\n",
               len, start->length);
            exit(10);
        }
    }
    /* discard nf files in range of new file */
    finfo = fopen(fname, "a");
    for(p=start; p; p=p->next) {
        if (p->id != -1) {
            b = (p->startpage)*PS;
            e = b + p->length;
            e = e + PS - (e % PS) -1;
            if (!(e < nspb || b > neb)) {
                 fprintf(finfo, "\n%ld ", -p->id);
            }
        }
    }
    fprintf(finfo,"\n%ld %ld %ld %ld %ld %s ",
            newid, nsp, len, inf1, inf2, ttxt);       
    fclose(finfo);

    return newid;
}

/* open nf file to write or read, fname is nfinfo file to look for id,
   skip in byte is rounded to next page */
struct nfrec *nfopen(char* fname, long id, int flags, long skip) {
    struct nfrec *start, *p;
    long sp;

    start = nfreadinfo(fname);
    for(p=start; p; p=p->next) {
        if (p->id == id) break;
    }
    if (p->id != id) return NULL;
    nfopen_init(start->txt, flags, p->startpage, p->length);
    if (skip > 0) {
        sp = (skip/PS)*PS;
        if (2*(skip-sp) > PS) sp -= PS;
        nfpos += sp;
        lseek(nffd, nfpos, SEEK_SET);
    }
    return p;
}


