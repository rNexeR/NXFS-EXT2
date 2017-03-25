
CFLAGS = -g -Wall -DFUSE_USE_VERSION=26 `pkg-config fuse --cflags`
LINKFLAGS = -Wall `pkg-config fuse --libs`

all: bin/fuse_x

clean:
	rm -rf bin obj

bin:
	mkdir -p bin

bin/fuse_x: bin obj/fuse_x.o obj/dev_ops.o obj/bitmap.o obj/inodes.o obj/device.o obj/main.o obj/utils.o
	g++ -g -o bin/fuse_x obj/* $(LINKFLAGS)

obj:
	mkdir -p obj

obj/main.o: obj main.c fuse_x.h
	gcc -g $(CFLAGS) -c main.c -o $@

obj/fuse_x.o: obj fuse_x.c fuse_x.h utils.h dev_ops.h
	g++ -g $(CFLAGS) -c fuse_x.c -o $@

obj/dev_ops.o: obj dev_ops.c dev_ops.h
	g++ -g $(CFLAGS) -c dev_ops.c -o $@

obj/inodes.o: obj inodes.c inodes.h
	g++ -g $(CFLAGS) -c inodes.c -o $@

obj/bitmap.o: obj bitmap.c bitmap.h
	g++ -g $(CFLAGS) -c bitmap.c -o $@

obj/device.o: obj device.c device.h inodes.h bitmap.h
	g++ -g $(CFLAGS) -c device.c -o $@

obj/utils.o: obj utils.c utils.h
	g++ -g $(CFLAGS) -c utils.c -o $@
