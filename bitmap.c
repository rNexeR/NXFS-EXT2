#include "bitmap.h"
#include "stdio.h"

static int get  (byte,   byte);
static void set  (byte *, byte);
static void reset(byte *, byte);

void locate(int position, int positions_per_group, int* group, int* index){
	*group =  (position -1)/positions_per_group;
	*index = (position -1)%positions_per_group;
}

int bitmapGet(byte *bitmap, int pos) {
    return get(bitmap[pos/BIT], pos%BIT);
}

void bitmapSet(byte *bitmap, int pos) {
    set(&bitmap[pos/BIT], pos%BIT);
}

void bitmapReset(byte *bitmap, int pos) {
    reset(&bitmap[pos/BIT], pos%BIT);
}


static int get(byte a, byte pos) {
    return (a >> pos) & 1;
}

static void set(byte *a, byte pos) {
    *a |= 1 << pos;
}

static void reset(byte *a, byte pos) {
    *a &= ~(1 << pos);
}
