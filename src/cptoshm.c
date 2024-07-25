/*  cptoshm.c     copy file into shared memory
    gcc -Wall -O0 -o cptoshm cptoshm.c -lrt
*/
#include "version.h"
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "cprefresh.h"

/* help page */
/* vim hint to remove resp. add quotes:
      s/^"\(.*\)\\n"$/\1/
      s/.*$/"\0\\n"/
*/
void usage( ) {
  fprintf(stderr,
          "cptoshm (version %s of frankl's stereo utilities)\nUSAGE:\n",
          VERSION);
  fprintf(stderr,
"  cptoshm --shmname=/<name> [options]\n"
"\n"
"  This program copies the content of a file (or of stdin) into a shared \n"
"  memory area. \n"
"\n"
"  USAGE HINTS\n"
"\n"
"  The converse is done by 'shmcat'. This can be used for audio playback \n"
"  by copying music files from hard disc to memory before playing them.\n"
"  This approach causes less operating system overhead compared to copying \n"
"  music files to a ramdisk.\n"
"\n"
"  OPTIONS\n"
"\n"
"  --file=name, -i name\n"
"      the name of the input file. If this is not given input is taken\n"
"      from stdin (and in this case you have to specify --max-input).\n"
"\n"
"  --shmname=string, -o string\n"
"      the name of the shared memory area to which the input is written.\n"
"      By convention this name should start with a slash (/).\n"
"\n"
"  --buffer-size=intval, -b intval\n"
"      the internal buffer size in bytes. The default is about 4 MB.\n"
"\n"
"  --max-input=intval, -m intval\n"
"      this is the maximal number of bytes read from the input file.\n"
"      If input is a file the default is the length of that file.\n"
"      If input is stdin then this  option must be given but may be\n"
"      larger than the actual input.\n"
"\n"
"  --number-copies=intnum, -R intnum\n"
"      before writing data they are copied the specfied number of\n"
"      times through  cleaned temporary buffers in RAM.\n"
"      The default is 0. \n"
"\n"
"  --verbose, -v\n"
"      print some information during startup and operation.\n"
"\n"
"  --version, -V\n"
"      print information about the version of the program and abort.\n"
"\n"
"  --help, -h\n"
"      print this help page and abort.\n"
"\n"
"  EXAMPLES\n"
"\n"
"  Copy a flac file to memory before playing and copy it into a playback\n"
"  pipe.\n"
"\n"
"  cptoshm --file=mymusic.flac --shmname=/play.flac\n"
"\n"
"  shmcat --shmname=/play.flac | ...\n"
"\n"
  );
}

int main(int argc, char *argv[])
{
    char *infile, *memname;
    int ifd, fd, bufsize, optc, i, nrcp, verbose;
    size_t length, done, rlen;
    struct stat sb;
    char *mem, *ptr;
    void *buf, *tbufs[1024];

    /* read command line options */
    static struct option longoptions[] = {
        {"file", required_argument, 0, 'i' },
        {"shmname", required_argument, 0, 'o' },
        {"buffer-size", required_argument, 0,  'b' },
        {"max-input", required_argument, 0, 'm' },
        {"number-copies", required_argument, 0, 'R' },
        {"overwrite", required_argument, 0, 'O' }, /* ignored */
        {"verbose", no_argument, 0, 'v' },
        {"version", no_argument, 0, 'V' },
        {"help", no_argument, 0, 'h' },
        {0,         0,                 0,  0 }
    };

    if (argc == 1) {
       usage();
       exit(0);
    }
    /* defaults */
    bufsize = 4194304;
    ifd = 0; /* stdin */
    fd = -1;
    length = 0;
    nrcp = 0;
    verbose = 0;
    infile = NULL;
    while ((optc = getopt_long(argc, argv, "i:o:b:m:O:R:vVh",
            longoptions, &optind)) != -1) {
        switch (optc) {
        case 'i':
          infile = optarg;
          if (strcmp(infile, "-")==0)
            break;
          if ((ifd = open(infile, O_RDONLY)) == -1) {
              fprintf(stderr, "cptoshm: Cannot open input file %s.\n", infile);
              exit(2);
          }
          if (fstat(ifd, &sb) == -1) {
              fprintf(stderr, "cptoshm: Cannot stat input file %s.\n", infile);
              exit(4);
          }
          length = sb.st_size;
          break;
        case 'o':
          memname = optarg;
          if ((fd = shm_open(memname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR))
                                                                      == -1){
              fprintf(stderr, "cptoshm: Cannot open memory %s.\n", memname);
              exit(3);
          }
          break;
        case 'b':
          bufsize = atoi(optarg);
          break;
        case 'm':
          length = atol(optarg);
          break;
        case 'R':
          nrcp = atoi(optarg);
          if (nrcp < 0 || nrcp > 1000) nrcp = 0;
          break;
        case 'O':
          break;
        case 'v':
          verbose = 1;
          break;
        case 'V':
          fprintf(stderr,
                  "cptoshm (version %s of frankl's stereo utilities)\n",
                  VERSION);
          exit(0);
        default:
          usage();
          exit(0);
        }
    }
    if (ifd == 0) {
        infile = "stdin";
    }
    if (verbose) {
        fprintf(stderr,
                "cptoshm: input %s, output %s, buffer %ld, max %ld\n",
                infile, memname, (long) bufsize, (long)length);
        if (nrcp)
            fprintf(stderr, "         refreshs %ld\n", (long)nrcp);
    }
    buf = NULL;
    if (posix_memalign(&buf, 4096, bufsize)) {
        fprintf(stderr, "cptoshm: Cannot allocate buffer of length %ld.\n",
                        (long)bufsize);
        exit(1);
    }
    if (ftruncate(fd, length) == -1) {
        fprintf(stderr, "cptoshm: Cannot truncate shared memory to %ld.", (long)length);
        exit(5);
    }
    mem = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        fprintf(stderr, "cptoshm: Cannot map shared memory.\n");
        exit(6);
    }
    if (nrcp) {
        /* temporary buffers */
        for (i=1; i < nrcp; i++) {
            if (posix_memalign(tbufs+i, 4096, bufsize)) {
                fprintf(stderr, "cptoshm: Cannot allocate buffer for cleaning.\n");
                exit(2);
            }
        }
    }
        
    /* copy data */
    ptr = mem;
    done = 0;
    while (done < length) {
        memclean(buf, bufsize);
        rlen = read(ifd, buf, bufsize);
        rlen = (done+rlen > length)? length-done:rlen;
        if (rlen == 0) break;

        if (nrcp) {
            tbufs[0] = buf;
            tbufs[nrcp] = buf;
            for (i=1; i <= nrcp; i++) {
                memclean((char*)(tbufs[i]), rlen);
                cprefresh((char*)(tbufs[i]), (char*)(tbufs[i-1]), rlen);
                memclean((char*)(tbufs[i-1]), rlen);
            }
        }
        memclean(ptr, rlen);
        cprefresh(ptr, buf, rlen);
        done += rlen;
        ptr += rlen;
    }
    if (done < length) {
      if (ftruncate(fd, done) == -1) {
          fprintf(stderr, "cptoshm: Cannot truncate shared memory to true length %ld.",
                          (long)done);
          exit(7);
      }
    }
    if (verbose)
        fprintf(stderr, "cptoshm: Copied %ld bytes.\n", (long)done);
    exit(0);
}
