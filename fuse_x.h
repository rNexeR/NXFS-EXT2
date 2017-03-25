
#ifdef __cplusplus
extern "C" {
#endif

#include "dev_ops.h"

//FUSE FUNCTION IMPLEMENTATIONS
int fusx_get_attr(const char *path, struct stat *statbuf);
void fusx_stat_entry(const char *path, struct stat *statbuf);
int fusx_read_dir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
int fusx_getattr(const char *path, struct stat *statbuf);
void* fusx_init(struct fuse_conn_info *conn);
int fusx_opendir(const char *path, struct fuse_file_info *fileInfo);
int fusx_statfs(const char *path, struct statvfs *statInfo);
int fusx_open(const char *path, struct fuse_file_info *fileInfo);
int fusx_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int fusx_release(const char *path, struct fuse_file_info *fileInfo);
int fusx_releasedir(const char *path, struct fuse_file_info *fileInfo);

int fusx_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int fusx_rename(const char *path, const char *newpath);
int fusx_rmdir(const char *path);
int fusx_mkdir(const char *path, mode_t mode);
int fusx_truncate(const char *path, off_t newSize);
int fusx_unlink(const char *path);
int fusx_mknod(const char *path, mode_t mode, dev_t dev);
int fusx_create(const char *, mode_t, struct fuse_file_info *);

int fusx_utime(const char *, struct utimbuf *);
#ifdef __cplusplus
}
#endif
