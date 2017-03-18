#ifndef _DEVICE_H_
#define _DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#include "structures.h"
#include "inodes.h"
#include "bitmap.h"

static const char *device_path;
static FILE *f;

int device_open(const char *path);
void device_close();
void device_flush();

int read_block(void* buffer, uint32 block_number, uint32 size_of_block);
int write_block(void* buffer, uint32 block_number, uint32 size_of_block);
int device_read(void* buffer, uint32 size);
void device_seek(uint32 offset);
int device_write(void *buffer, uint32 size);
struct stat device_stat();

#ifdef __cplusplus
}
#endif

#endif
