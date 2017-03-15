
CFLAGS = -g -Wall -DFUSE_USE_VERSION=26 `pkg-config fuse --cflags`
LINKFLAGS = -Wall `pkg-config fuse --libs`

all: bin/nxt2fs

clean:
	rm -rf bin obj

bin: 
	mkdir -p bin

bin/nxt2fs: bin obj/nxt2fs.o obj/bitmap.o obj/inodes.o obj/device.o obj/main.o obj/utils.o
	g++ -g -o bin/nxt2fs obj/* $(LINKFLAGS)

obj:
	mkdir -p obj

obj/main.o: obj main.c nxt2fs.h
	gcc -g $(CFLAGS) -c main.c -o $@

obj/nxt2fs.o: obj nxt2fs.c nxt2fs.h utils.h
	g++ -g $(CFLAGS) -c nxt2fs.c -o $@

obj/inodes.o: obj inodes.c inodes.h
	g++ -g $(CFLAGS) -c inodes.c -o $@

obj/bitmap.o: obj bitmap.c bitmap.h
	g++ -g $(CFLAGS) -c bitmap.c -o $@

obj/device.o: obj device.c device.h inodes.h bitmap.h
	g++ -g $(CFLAGS) -c device.c -o $@

obj/utils.o: obj utils.c utils.h
	g++ -g $(CFLAGS) -c utils.c -o $@