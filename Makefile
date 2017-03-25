
CFLAGS = -g -Wall -DFUSE_USE_VERSION=26 `pkg-config fuse --cflags`
LINKFLAGS = -Wall `pkg-config fuse --libs`

all: bin/kjext2fs

clean:
	rm -rf bin obj

bin: 
	mkdir -p bin

bin/kjext2fs: bin obj/kjext2fs.o obj/bitmap.o obj/inodes.o obj/device.o obj/main.o obj/utils.o
	g++ -g -o bin/kjext2fs obj/* $(LINKFLAGS)

obj:
	mkdir -p obj

obj/main.o: obj main.c kjext2fs.h
	gcc -g $(CFLAGS) -c main.c -o $@

obj/kjext2fs.o: obj kjext2fs.c kjext2fs.h utils.h
	g++ -g $(CFLAGS) -c kjext2fs.c -o $@

obj/inodes.o: obj inodes.c inodes.h
	g++ -g $(CFLAGS) -c inodes.c -o $@

obj/bitmap.o: obj bitmap.c bitmap.h
	g++ -g $(CFLAGS) -c bitmap.c -o $@

obj/device.o: obj device.c device.h inodes.h bitmap.h
	g++ -g $(CFLAGS) -c device.c -o $@

obj/utils.o: obj utils.c utils.h
	g++ -g $(CFLAGS) -c utils.c -o $@