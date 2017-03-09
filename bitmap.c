#include "bitmap.h"
#include "stdio.h"

/*int is_used(char* bitmap, int pos){
	char character = bitmap[pos/BITS];
	int position = pos-((pos/BITS)*BITS);

	return character & 1<<position;
}

void set_used(char* bitmap, int pos){
	int posArray = pos/BITS;
	int posadd = pos%BITS;

	printf("[%d/%d]: %d %d\n", pos, BITS, posArray, posadd);

	bitmap[posArray] = bitmap[posArray] | 1<<posadd;
}

void set_unused(char* bitmap, int pos){
	int posArray = pos/BITS;
	int posadd = pos%BITS;

	bitmap[posArray] = bitmap[posArray] & ~(1<<posadd);
}*/

static int get  (byte,   byte);
static void set  (byte *, byte);
static void reset(byte *, byte);

int bitmapGet(byte *bitmap, int pos) {
    return get(bitmap[pos/BIT], pos%BIT);
}

void bitmapSet(byte *bitmap, int pos) {
    set(&bitmap[pos/BIT], pos%BIT);
}

void bitmapReset(byte *bitmap, int pos) {
    reset(&bitmap[pos/BIT], pos%BIT);
}

int bitmapSearch(byte *bitmap, int n, int size, int start) {
    int i;
    for(i = start+1, size *= BIT; i < size; i++)
        if(bitmapGet(bitmap,i) == n)
            return i;
    return BITMAP_NOTFOUND;
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