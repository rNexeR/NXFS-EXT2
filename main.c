#include "nxt2fs.h"
#include "device.h"

#include <fuse.h>
#include <stdio.h>

struct fuse_operations nxfs_oper;

int main(int argc, char *argv[]) {
	int i, fuse_stat;

	nxfs_oper.getattr = fs_get_attr;
	nxfs_oper.rmdir = fs_rmdir;
	nxfs_oper.mkdir = fs_mkdir;
	nxfs_oper.rename = fs_rename;
	nxfs_oper.truncate = fs_truncate;
	nxfs_oper.write = fs_write;
	nxfs_oper.release = fs_release;
	nxfs_oper.releasedir = fs_releasedir;
	nxfs_oper.unlink = fs_unlink;
	nxfs_oper.mknod = fs_mknod;
	nxfs_oper.create = fs_create;
	nxfs_oper.open = fs_open;
	nxfs_oper.read = fs_read;
	nxfs_oper.statfs = fs_statfs;
	nxfs_oper.opendir = fs_opendir;
	nxfs_oper.readdir = fs_read_dir;

	nxfs_oper.init = fs_init;

	nxfs_oper.utime = fs_utime;

	printf("mounting file system...\n");
	for(i = 1; i < argc && (argv[i][0] == '-'); i++) {
		if(i == argc) {
			return (-1);
		}
	}

	if (!device_open(realpath(argv[i], NULL)) ) {
		printf("Cannot open device file %s\n", argv[i]);
		return 1;
	}

	for(; i < argc; i++) {
		argv[i] = argv[i+1];
		//printf("->%s\n", argv[i]);
	}
	argc--;
	//printf("llego aqui\n");

	fuse_stat = fuse_main(argc, argv, &nxfs_oper, NULL);

	printf("fuse_main returned %d\n", fuse_stat);

	return fuse_stat;
}
