CC=cc
CFLAGS=-O2 -fnative-struct -fno-rtti -fno-exceptions -mcpu=pentium -march=i386

# Rule to make compiled resource (Windows)
%-rc.o: %.rc
	windres -i $< -o $@

untgz: untgz.o untgz-rc.o ../../libz.a
	$(CC) $(CFLAGS) -s -o untgz  untgz.o untgz-rc.o -L../.. -lz

untgz.o: untgz.c ../../zlib.h
	$(CC) $(CFLAGS) -c -I../.. untgz.c

../../libz.a:
	cd ../..; make

clean:
	rm -f untgz untgz.o *~
