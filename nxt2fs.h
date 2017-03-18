#ifndef nxt2fs_h
#define nxt2fs_h

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "device.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ROOT_INO 2
#define ENTRY_DIR 2
#define ENTRY_FILE 1
#define ENTRY_BASE_SIZE 8

static const int print_info = 0;

static struct s_superblock es;
static struct s_block_group_descriptor *groups_table;

static uint32 size_of_block;
static uint32 size_of_fragment;
static uint32 number_of_groups;
static uint32 n_512k_blocks_per_block;

static uint32 indirect_blocks_count;
static uint32 d_indirect_blocks_count;
static uint32 t_indirect_blocks_count;

char *dir_path;
char *file_path;

uint32 current_dir_inode;
uint32 current_file_inode;

void read_group_descriptors();
int read_sb();
struct s_inode* read_inode(uint32);
int get_free_group_inode(uint32);
int get_free_block_in_group(uint32 group_number);
int get_free_inode(uint32);
int get_free_block(uint32);

void read_t_indirect_block(void *, uint32, uint32);
void read_d_indirect_block(void *, uint32, uint32);
void read_indirect_block(void *buffer, uint32, uint32 logic_position);
int read_inode_logic_block(void *buffer, struct s_inode inode, uint32 logic_block_number);

void write_t_indirect_block(void *buffer, uint32, uint32 logic_block_number, uint32 inode_number);
void write_d_indirect_block(void *buffer, uint32, uint32 logic_block_number, uint32 inode_number);
void write_indirect_block(void *buffer, uint32, uint32 logic_block_number, uint32 inode_number);
void write_inode_logic_block(void *buffer, struct s_inode* inode, uint32 logic_block_number, uint32 inode_number);

void read_block_bitmap(void *buffer, int group_number);
void read_inode_bitmap(void *buffer, int group_number);
void inode_bitmap_set(int inode_number, uint8 state);
void block_bitmap_set(uint32 inode_number, uint8 state);

int nxfs_get_attr(const char *path, struct stat *statbuf);
uint32 lookup_entry_inode(char *path, uint32 current_inode_number);

struct s_dir_entry2* find_last_entry(struct s_inode inode);
struct s_dir_entry2* find_entry(struct s_inode inode, const char* entry_name);
struct s_dir_entry2 find_previous_entry(struct s_inode inode, char *entry_name);
int add_entry(struct s_inode parent_inode, uint32 parent_inode_number, char *entry_name, mode_t mode, uint8 file_type);

void nxfs_stat_entry(const char *path, struct stat *statbuf);
int nxfs_read_dir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
int nxfs_getattr(const char *path, struct stat *statbuf);
void nxfs_init(struct fuse_conn_info *conn);
int nxfs_opendir(const char *path, struct fuse_file_info *fileInfo);
int nxfs_statfs(const char *path, struct statvfs *statInfo);
int nxfs_open(const char *path, struct fuse_file_info *fileInfo);
int nxfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int nxfs_release(const char *path, struct fuse_file_info *fileInfo);
int nxfs_releasedir(const char *path, struct fuse_file_info *fileInfo);

int nxfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int nxfs_rename(const char *path, const char *newpath);
int nxfs_rmdir(const char *path);
int nxfs_mkdir(const char *path, mode_t mode);
int nxfs_truncate(const char *path, off_t newSize);
int nxfs_unlink(const char *path);
int nxfs_mknod(const char *path, mode_t mode, dev_t dev);
int nxfs_create(const char *, mode_t, struct fuse_file_info *);
int save_inode(struct s_inode inode, uint32 index);
int save_meta_data();
#ifdef __cplusplus
}
#endif

#endif //nxfs_h
