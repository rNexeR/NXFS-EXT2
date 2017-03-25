#include "device.h"

int read_block(void* buffer, uint32 block_number, uint32 size_of_block) {
  fseek(f, block_number*size_of_block, SEEK_SET);

  return ( fread(buffer, 1, size_of_block, f) == size_of_block );
}

int write_block(void* buffer, uint32 block_number, uint32 size_of_block) {
  fseek(f, block_number*size_of_block, SEEK_SET);

  return ( fwrite(buffer, 1, size_of_block, f) == size_of_block );
}

int device_write(void *buffer, uint32 size){

  /*if(size == SUPERBLOCK_SIZE_TO_SAVE){
    struct s_superblock sb = (struct s_superblock)(*buffer);
    printf("cantidad de inodes : %u\n",sb.s_inodes_count );
    printf("magic  : %u\n",sb.s_magic );
    printf("s_pad : %u\n",sb.s_pad );
  }*/
  int siz = fwrite(buffer, 1, size, f);
  // printf("size %d\n", siz);
  return ( siz == size );
}

int device_read(void* buffer, uint32 size){
  return ( fread(buffer, 1, size, f) == size );
}

void device_seek(uint64 offset){
  // printf("seek at %lu\n",offset );
  fseek(f, offset, SEEK_SET);
  uint32 cur_pos = ftell(f);
  if(offset != cur_pos)
    printf("-------> |||||||||||||||||||||| <------------------");
}

struct stat device_stat(){
  struct stat fileStat;
  stat(device_path,&fileStat);
  return fileStat;
}

int device_open(const char *path) {
  device_path = path;
  int path_len = strlen(path);
  printf("path %s\n", device_path);
  f = fopen(path, "r+");

  return (f != NULL);
}

void device_close(){
  fflush(f);
  fclose(f);
}

void device_flush(){
  fflush(f);
}
