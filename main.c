#include "nxt2fs.h"
#include "device.h"

#include <fuse.h>
#include <stdio.h>

/*#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>*/

struct fuse_operations nxfs_oper;

/*int findStringPos(char buffer[], char searched[]){
	int size_of_buffer = strlen(buffer);
	int size_of_searched = strlen(searched);
	int searched_pos = 0;
	int first_position = -1;
	int f_found = 0;
	for (int i = 0; i < size_of_buffer; i++)
	{
		if(buffer[i] == searched[searched_pos]){
			searched_pos++;
			if(f_found == 0){
				f_found = 1;
				first_position = i;
				//printf("First char found at %d\n", i);
			}
			if(searched_pos == size_of_searched)
				break;
		}else if(f_found == 1){
			return -1;
		}
	}
	if(searched_pos == size_of_searched)
		return first_position;
	else
		return -1;
}

int substringCount(char buffer[], char searched, int size_of_buffer){
	int count = 0;
	for (int i = 0; i < size_of_buffer; i++){
		if(buffer[i] == searched)
			count++;
	}
	return count;
}

void parseBackUps(){
	struct stat fileStat;
	if(stat("backups.txt",&fileStat) < 0)    
		return 1;

	char buf[fileStat.st_size];

	int c, i = 0;
	FILE *file;
	file = fopen("backups.txt", "r");
	if (file) {
		while ((c = getc(file)) != EOF)
			buf[i++] = c;
		buf[i] = 0;
		fclose(file);
	}
	int backups_count = substringCount(buf, '\n', fileStat.st_size);
	printf("Number of lines: %d\n", backups_count);
	//printf("%s\n", buf);
	i = 0;
	int current_pos = 0;
	int limit = 0;
	int backups[backups_count];
	for (int i = 0; i < backups_count; i++)
	{
		current_pos += findStringPos(&buf[current_pos], "Backup superblock at ");
		current_pos += 21;
		limit = findStringPos(&buf[current_pos], ",");
		
		buf[current_pos + limit] = '\0';
		backups[i] = atoi(&buf[current_pos]);

		current_pos += limit + 1;
		current_pos += findStringPos(&buf[current_pos], "\n");
	}
}*/

int main(int argc, char *argv[]) {
	int i, fuse_stat;

	nxfs_oper.getattr = nxfs_get_attr;
	nxfs_oper.rmdir = nxfs_rmdir;
	nxfs_oper.mkdir = nxfs_mkdir;
	nxfs_oper.rename = nxfs_rename;
	nxfs_oper.truncate = nxfs_truncate;
	nxfs_oper.write = nxfs_write;
	nxfs_oper.release = nxfs_release;
	nxfs_oper.releasedir = nxfs_releasedir;
	nxfs_oper.unlink = nxfs_unlink;
	nxfs_oper.mknod = nxfs_mknod;
	nxfs_oper.create = nxfs_create;
	/*nxfs_oper.readlink = nxfs_readlink;
	nxfs_oper.getdir = NULL;
	
	
	
	nxfs_oper.symlink = nxfs_symlink;
	
	nxfs_oper.link = nxfs_link;
	nxfs_oper.chmod = nxfs_chmod;
	nxfs_oper.chown = nxfs_chown;
	
	nxfs_oper.utime = nxfs_utime;*/
	nxfs_oper.open = nxfs_open;
	nxfs_oper.read = nxfs_read;
	nxfs_oper.statfs = nxfs_statfs;
	/*
	nxfs_oper.flush = nxfs_flush;
	
	nxfs_oper.fsync = nxfs_fsync;*/
	nxfs_oper.opendir = nxfs_opendir;
	nxfs_oper.readdir = nxfs_read_dir;
	
	nxfs_oper.init = nxfs_init;

	nxfs_oper.utime = nxfs_utime;
	// nxfs_oper.create = nxfs_creat;

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

	fuse_stat = fuse_main(argc, argv, &nxfs_oper, NULL);

	printf("fuse_main returned %d\n", fuse_stat);

	return fuse_stat;
}
