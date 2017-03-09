#ifndef _BITMAP_H_
#define _BITMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*#define BITS sizeof(char)*8

int is_used(char* bitmap, int pos);
void set_used(char* bitmap, int pos);
void set_unused(char* bitmap, int pos);*/

#define BIT (8*sizeof(byte))
#define BITMAP_NOTFOUND -1

typedef unsigned char byte;

int bitmapGet   (byte *, int);
void bitmapSet   (byte *, int);
void bitmapReset (byte *, int);
int  bitmapSearch(byte *, int, int, int);

#ifdef __cplusplus
}
#endif

#endif