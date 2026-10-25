
VERSION=0.9.5.dev

REFRESH=""

# for resample_soxr_convolve_play/USB on the Odroid C4: make C4TIMER=-DC4TIMER
# (timing with the 24 MHz system counter; rebuild after "make clean")
C4TIMER=

ARCH=$(shell uname -m)

# normal CFLAGS
CFLAGS=-O2 -Wall -z execstack -D_FILE_OFFSET_BITS=64 -fgnu89-inline -DREFRESH$(REFRESH)

# CFLAGS without optimization
CFLAGSNO=-O0 -Wall -z execstack -D_FILE_OFFSET_BITS=64 -fgnu89-inline -DREFRESH$(REFRESH)

# targets
ALL: bin tmp bin/volrace bin/bufhrt bin/highrestest \
     bin/writeloop bin/catloop bin/playhrt bin/cptoshm bin/shmcat \
     bin/resample_soxr bin/cat64 bin/shownfinfo bin/music2nf bin/improvedirect \
     bin/resample_soxr_convolve bin/resample_soxr_convolve_play \
     bin/resample_soxr_convolve_USB

bin:
	mkdir -p bin

tmp:
	mkdir -p tmp

src/version.h: Makefile
	echo "#define VERSION \""$(VERSION)"\""  > src/version.h

bin/volrace: src/version.h src/volrace.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGS) -o bin/volrace src/volrace.c tmp/cprefresh.o tmp/cprefresh_ass.o 

tmp/net.o: src/net.h src/net.c |tmp 
	$(CC) $(CFLAGS) -c -o tmp/net.o src/net.c

tmp/cprefresh_ass.o: src/cprefresh_default.s src/cprefresh_vfp.s src/cprefresh_aa64.s src/cprefresh_x8664.s src/cprefresh_arm.s |tmp 
	if [ $(REFRESH) = "" ]; then \
	  $(CC) -c $(CFLAGSNO) -o tmp/cprefresh_ass.o src/cprefresh_default.s; \
	elif [ $(REFRESH) = "ARM" ]; then \
	  $(CC) -c $(CFLAGSNO) -marm -o tmp/cprefresh_ass.o src/cprefresh_arm.s; \
	elif [ $(REFRESH) = "VFP" ]; then \
	  $(CC) -c $(CFLAGSNO) -marm -mfpu=neon-vfpv4 -o tmp/cprefresh_ass.o src/cprefresh_vfp.s; \
	elif [ $(REFRESH) = "AA64" ]; then \
	  $(CC) -c $(CFLAGSNO) -march=native -o tmp/cprefresh_ass.o src/cprefresh_aa64.s; \
	elif [ $(REFRESH) = "X8664" ]; then \
	  $(CC) -c $(CFLAGSNO) -march=native -o tmp/cprefresh_ass.o src/cprefresh_x8664.s; \
	fi

tmp/cprefresh.o: src/cprefresh.h src/cprefresh.c |tmp 
	$(CC) -c $(CFLAGSNO) -o tmp/cprefresh.o src/cprefresh.c

bin/playhrt: src/version.h tmp/net.o src/playhrt.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -o bin/playhrt src/playhrt.c tmp/net.o tmp/cprefresh.o tmp/cprefresh_ass.o -lasound -lrt

bin/playhrt_ALSANC: src/version.h tmp/net.o src/playhrt.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -DALSANC -I$(ALSANC)/include -L$(ALSANC)/lib -o bin/playhrt_ALSANC src/playhrt.c tmp/net.o tmp/cprefresh.o tmp/cprefresh_ass.o -lasound -lrt 

bin/playhrt_static: src/version.h tmp/net.o src/playhrt.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -DALSANC -I$(ALSANC)/include -L$(ALSANC)/lib -o bin/playhrt_static src/playhrt.c tmp/net.o tmp/cprefresh.o tmp/cprefresh_ass.o -lasound -lrt -lpthread -lm -ldl -static

bin/bufhrt: src/version.h tmp/net.o src/bufhrt.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -o bin/bufhrt tmp/net.o tmp/cprefresh.o tmp/cprefresh_ass.o src/bufhrt.c -lpthread -lrt

bin/highrestest: src/highrestest.c |bin
	$(CC) $(CFLAGSNO) -o bin/highrestest src/highrestest.c -lrt

bin/writeloop: src/version.h src/nf_io.h src/writeloop.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGS) -o bin/writeloop tmp/cprefresh.o tmp/cprefresh_ass.o src/writeloop.c -lpthread -lrt

bin/catloop: src/version.h src/catloop.c |bin
	$(CC) $(CFLAGS) -o bin/catloop src/catloop.c -lpthread -lrt

bin/cptoshm: src/version.h src/cptoshm.c tmp/cprefresh_ass.o tmp/cprefresh.o |bin
	$(CC) $(CFLAGS) -o bin/cptoshm src/cptoshm.c tmp/cprefresh_ass.o tmp/cprefresh.o -lrt

bin/shmcat: src/version.h src/shmcat.c tmp/cprefresh_ass.o tmp/cprefresh.o |bin
	$(CC) $(CFLAGS) -o bin/shmcat tmp/cprefresh_ass.o tmp/cprefresh.o src/shmcat.c -lrt

bin/resample_soxr: src/version.h src/nf_io.h src/resample_soxr.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGS) -o bin/resample_soxr src/resample_soxr.c tmp/cprefresh.o tmp/cprefresh_ass.o -lsoxr -lsndfile -lrt

resampler: bin/resample_soxr

tmp/fftconv.o: src/fftconv.h src/fftconv.c |tmp
	$(CC) $(CFLAGS) -O3 -c -o tmp/fftconv.o src/fftconv.c

tmp/msconv.o: src/msconv.h src/fftconv.h src/msconv.c |tmp
	$(CC) $(CFLAGS) -c -o tmp/msconv.o src/msconv.c

bin/msfir: src/version.h src/msfir.c src/msconv.h tmp/msconv.o tmp/fftconv.o |bin
	$(CC) $(CFLAGS) -o bin/msfir src/msfir.c tmp/msconv.o tmp/fftconv.o -lfftw3 -lm

msfir: bin/msfir

bin/resample_soxr_convolve: src/version.h src/nf_io.h src/resample_soxr_convolve.c src/msconv.h tmp/msconv.o tmp/fftconv.o tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGS) -o bin/resample_soxr_convolve src/resample_soxr_convolve.c tmp/msconv.o tmp/fftconv.o tmp/cprefresh.o tmp/cprefresh_ass.o -lsoxr -lsndfile -lfftw3 -lpthread -lm -lrt

resample_soxr_convolve: bin/resample_soxr_convolve

# player thread as in myplayhrt_c4, compiled without optimization like it
tmp/alsaplay.o: src/alsaplay.h src/alsaplay.c src/cprefresh.h |tmp
	$(CC) $(CFLAGSNO) $(C4TIMER) -c -o tmp/alsaplay.o src/alsaplay.c

bin/resample_soxr_convolve_play: src/version.h src/nf_io.h src/resample_soxr_convolve_play.c src/msconv.h src/alsaplay.h tmp/msconv.o tmp/fftconv.o tmp/alsaplay.o tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGS) -o bin/resample_soxr_convolve_play src/resample_soxr_convolve_play.c tmp/msconv.o tmp/fftconv.o tmp/alsaplay.o tmp/cprefresh.o tmp/cprefresh_ass.o -lsoxr -lsndfile -lfftw3 -lasound -lpthread -lm -lrt

resample_soxr_convolve_play: bin/resample_soxr_convolve_play

tmp/usbplay.o: src/usbplay.h src/usbplay.c src/cprefresh.h |tmp
	$(CC) $(CFLAGSNO) $(C4TIMER) -c -o tmp/usbplay.o src/usbplay.c

tmp/directread.o: src/directread.h src/directread.c |tmp
	$(CC) $(CFLAGSNO) -c -o tmp/directread.o src/directread.c

bin/resample_soxr_convolve_USB: src/version.h src/nf_io.h src/resample_soxr_convolve_USB.c src/msconv.h src/usbplay.h src/directread.h tmp/msconv.o tmp/fftconv.o tmp/usbplay.o tmp/directread.o tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGS) -o bin/resample_soxr_convolve_USB src/resample_soxr_convolve_USB.c tmp/msconv.o tmp/fftconv.o tmp/usbplay.o tmp/directread.o tmp/cprefresh.o tmp/cprefresh_ass.o -lsoxr -lsndfile -lfftw3 -lusb-1.0 -lpthread -lm -lrt

resample_soxr_convolve_USB: bin/resample_soxr_convolve_USB

bin/cat64: src/version.h src/cat64.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGS) -o bin/cat64 src/cat64.c  tmp/cprefresh.o tmp/cprefresh_ass.o -lsndfile -lrt

bin/shownfinfo: src/version.h src/shownfinfo.c src/nf_io.h  |bin
	$(CC) $(CFLAGS) -o bin/shownfinfo src/shownfinfo.c 

bin/music2nf: src/version.h src/nf_io.h src/music2nf.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGS) -o bin/music2nf src/music2nf.c  tmp/cprefresh.o tmp/cprefresh_ass.o -lsndfile -lrt

bin/improvedirect: src/version.h src/improvedirect.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -o bin/improvedirect tmp/cprefresh.o tmp/cprefresh_ass.o src/improvedirect.c -lrt -lm



# experimental, only compiled on demand
bin/clreg86: tmp src/clreg86.c src/cprefresh_x8664.s |bin 
	$(CC) $(CFLAGSNO) -c -o tmp/cprefresh_x8664.o src/cprefresh_x8664.s
	$(CC) $(CFLAGSNO) -o bin/clreg86 src/clreg86.c tmp/cprefresh_x8664.o

bin/clreg: tmp src/clreg.c src/cprefresh_aa64.s |bin 
	$(CC) $(CFLAGSNO) -c -o tmp/cprefresh_aa64.o src/cprefresh_aa64.s
	$(CC) $(CFLAGSNO) -o bin/clreg src/clreg.c tmp/cprefresh_aa64.o

# undocumented private version, stripped some code
bin/myplayhrt: src/version.h tmp/net.o src/myplayhrt.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -o bin/myplayhrt src/myplayhrt.c tmp/net.o tmp/cprefresh.o tmp/cprefresh_ass.o -lasound -lpthread -lrt

bin/by4: src/by4.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -o bin/by4 src/by4.c tmp/cprefresh.o tmp/cprefresh_ass.o

tmp/counttotick_aa64.o: src/counttotick_aa64.h src/counttotick_aa64.s |tmp
	$(CC) $(CFLAGSNO) -o tmp/counttotick_aa64.o -c src/counttotick_aa64.s

bin/myplayhrt_c4: src/myplayhrt_c4.c src/version.h tmp/net.o  tmp/cprefresh.o tmp/cprefresh_ass.o |bin                   
	$(CC) $(CFLAGSNO) -o bin/myplayhrt_c4 src/myplayhrt_c4.c tmp/net.o tmp/cprefresh.o tmp/cprefresh_ass.o -lasound -lpthread -lrt -lm

bin/bufhrt_c4: src/version.h tmp/net.o src/bufhrt_c4.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -o bin/bufhrt_c4 tmp/net.o tmp/cprefresh.o tmp/cprefresh_ass.o src/bufhrt_c4.c -lpthread -lrt -lm

bin/forimprove_c4: src/version.h tmp/net.o src/forimprove_c4.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -o bin/forimprove_c4 tmp/net.o tmp/cprefresh.o tmp/cprefresh_ass.o src/forimprove_c4.c -lpthread -lrt -lm

bin/readlat: src/readlat.c |bin
	$(CC) $(CFLAGS) -o bin/readlat src/readlat.c

bin/music2nf_c4: src/version.h src/nf_io.h src/music2nf_c4.c tmp/cprefresh.o tmp/cprefresh_ass.o |bin
	$(CC) $(CFLAGSNO) -o bin/music2nf_c4 src/music2nf_c4.c  tmp/cprefresh.o tmp/cprefresh_ass.o -lsndfile -lrt

c4: bin/by4 bin/myplayhrt_c4 bin/bufhrt_c4 bin/forimprove_c4 bin/music2nf_c4 bin/readlat

clean: 
	rm -rf src/version.h bin tmp

veryclean: clean
	rm -f *~ */*~ */*/*~

# private, for bin in distribution
bin86: 
	make veryclean
	make
	cd bin; \
	strip * ; \
	tar cvf frankl_stereo-$(VERSION)-bin-$(ARCH).tar * ; \
	gzip -9 frankl*.tar ; \
	mv frankl*gz ..
binPi: 
	make veryclean
	make REFRESH=VFP
	cd bin; \
	strip * ; \
	tar cvf frankl_stereo-$(VERSION)-bin-$(ARCH).tar * ; \
	gzip -9 frankl*.tar ; \
	mv frankl*gz ..

	

