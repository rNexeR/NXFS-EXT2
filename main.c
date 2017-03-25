#include "nxt2fs.h"
#include "device.h"

#include <fuse.h>
#include <stdio.h>

struct fuse_operations fusx_oper;

int main(int argc, char *argv[]) {
	int i, fuse_stat;

	fusx_oper.getattr = fusx_get_attr;
	fusx_oper.rmdir = fusx_rmdir;
	fusx_oper.mkdir = fusx_mkdir;
	fusx_oper.rename = fusx_rename;
	fusx_oper.truncate = fusx_truncate;
	fusx_oper.write = fusx_write;
	fusx_oper.release = fusx_release;
	fusx_oper.releasedir = fusx_releasedir;
	fusx_oper.unlink = fusx_unlink;
	fusx_oper.mknod = fusx_mknod;
	fusx_oper.create = fusx_create;
	fusx_oper.open = fusx_open;
	fusx_oper.read = fusx_read;
	fusx_oper.statfs = fusx_statfs;
	fusx_oper.opendir = fusx_opendir;
	fusx_oper.readdir = fusx_read_dir;

	fusx_oper.init = fusx_init;

	fusx_oper.utime = fusx_utime;

	printf("Mounting FS.\n");
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
	}
	argc--;

	fuse_stat = fuse_main(argc, argv, &fusx_oper, NULL);

	printf("fuse_main returned %d\n", fuse_stat);

	return fuse_stat;
}
