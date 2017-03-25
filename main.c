#include "kjext2fs.h"
#include "device.h"

#include <fuse.h>
#include <stdio.h>

struct fuse_operations kjfs_oper;

int main(int argc, char *argv[]) {
	int i, fuse_stat;

	kjfs_oper.getattr = kjfs_get_attr;
	kjfs_oper.rmdir = kjfs_rmdir;
	kjfs_oper.mkdir = kjfs_mkdir;
	kjfs_oper.rename = kjfs_rename;
	kjfs_oper.truncate = kjfs_truncate;
	kjfs_oper.write = kjfs_write;
	kjfs_oper.release = kjfs_release;
	kjfs_oper.releasedir = kjfs_releasedir;
	kjfs_oper.unlink = kjfs_unlink;
	kjfs_oper.mknod = kjfs_mknod;
	kjfs_oper.create = kjfs_create;
	kjfs_oper.open = kjfs_open;
	kjfs_oper.read = kjfs_read;
	kjfs_oper.statfs = kjfs_statfs;
	kjfs_oper.opendir = kjfs_opendir;
	kjfs_oper.readdir = kjfs_read_dir;
	
	kjfs_oper.init = kjfs_init;

	kjfs_oper.utime = kjfs_utime;

	printf("mounting file system...\n");
	for(i = 1; i < argc && (argv[i][0] == '-'); i++) {
		if(i == argc) {
			return (-1);
		}
	}

	// parseBackUps();
	
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

	fuse_stat = fuse_main(argc, argv, &kjfs_oper, NULL);

	printf("fuse_main returned %d\n", fuse_stat);

	return fuse_stat;
}
