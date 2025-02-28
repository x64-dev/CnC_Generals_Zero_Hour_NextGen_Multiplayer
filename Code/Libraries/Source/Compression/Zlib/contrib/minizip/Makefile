CC=cc
CFLAGS=-O2 -fnative-struct -fno-rtti -fno-exceptions -mcpu=pentium -march=i386 -I../..

UNZ_OBJS = miniunz.o miniunz-rc.o unzip.o -L../.. -lz 
ZIP_OBJS = minizip.o minizip-rc.o zip.o   -L../.. -lz 

.c.o:
	$(CC) -c $(CFLAGS) $*.c

# Rule to make compiled resource (Windows)
%-rc.o: %.rc
	windres -i $< -o $@

all: miniunz minizip

miniunz:  $(UNZ_OBJS)
	$(CC) -s $(CFLAGS) -o $@ $(UNZ_OBJS)

minizip:  $(ZIP_OBJS)
	$(CC) -s $(CFLAGS) -o $@ $(ZIP_OBJS)

test:	miniunz minizip
	./minizip test readme.txt
	./miniunz -l test.zip
	mv readme.txt readme.old
	./miniunz test.zip

clean:
	/bin/rm -f *.o *~ minizip miniunz
