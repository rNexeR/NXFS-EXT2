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

#ifdef __cplusplus
extern "C" {
#endif

static struct s_superblock es;
static struct s_block_group_descriptor *groups_table;

static uint32 size_of_block;
static uint32 size_of_fragment;
static uint32 number_of_groups;

char* dir_path;
char* file_path;

void read_group_descriptors();
int read_sb();
struct s_inode read_inode(int);
int get_free_group_inode(int);
void read_block_bitmap(void* buffer, int group_number);
void read_inode_bitmap(void* buffer, int group_number);

int sfs_getattr(const char *path, struct stat *statbuf);
int sfs_open(const char *path, struct fuse_file_info *fileInfo);
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
void sfs_init(struct fuse_conn_info *conn);

int sfs_opendir(const char *path, struct fuse_file_info *fileInfo);

int sfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int sfs_rename(const char *path, const char *newpath);
int sfs_rmdir(const char *path);
int sfs_mkdir(const char *path, mode_t mode);

/*int sfs_readlink(const char *path, char *link, size_t size);
int sfs_mknod(const char *path, mode_t mode, dev_t dev);
int sfs_unlink(const char *path);
int sfs_symlink(const char *path, const char *link);
int sfs_link(const char *path, const char *newpath);
int sfs_chmod(const char *path, mode_t mode);
int sfs_chown(const char *path, uid_t uid, gid_t gid);
int sfs_truncate(const char *path, off_t newSize);
int sfs_utime(const char *path, struct utimbuf *ubuf);
int sfs_statfs(const char *path, struct statvfs *statInfo);
int sfs_flush(const char *path, struct fuse_file_info *fileInfo);
int sfs_release(const char *path, struct fuse_file_info *fileInfo);
int sfs_fsync(const char *path, int datasync, struct fuse_file_info *fi);
int sfs_releasedir(const char *path, struct fuse_file_info *fileInfo);
int sfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo);
int sfs_getdir(const char *path);*/

#ifdef __cplusplus
}
#endif

#endif //sfs_h