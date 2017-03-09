#include "nxt2fs.h"

void read_group_descriptors(){
	struct stat fileStat = device_stat();

	number_of_groups = fileStat.st_size / (es.s_blocks_per_group * size_of_block) + 1;
	printf("Number of groups: %d\n", number_of_groups);

	groups_table = (struct s_block_group_descriptor *) malloc(sizeof(s_block_group_descriptor)*number_of_groups);

	device_seek(1024 + size_of_block);
	printf("\n");

	for (int i = 0; i < number_of_groups; ++i)
	{
		device_read(&groups_table[i], sizeof(s_block_group_descriptor));

    /*printf("--|Block Group: %d\n", i);
    printf("block_bitmap: %u\n", groups_table[i].bg_block_bitmap);
    printf("inode_bitmap: %u\n", groups_table[i].bg_inode_bitmap);
    printf("inode_table: %u\n", groups_table[i].bg_inode_table);
    printf("free_blocks_count: %u\n", groups_table[i].bg_free_blocks_count);
    printf("free_inodes_count: %u\n", groups_table[i].bg_free_inodes_count);
    printf("used_dirs_count: %u\n", groups_table[i].bg_used_dirs_count);
    printf("\n\n");*/
	}
}

int read_sb(){
	device_seek(1024);
	device_read(&es, sizeof(s_superblock));

	size_of_block = 1024 << es.s_log_block_size;
	printf("Size of block %u\n", size_of_block);

	if(es.s_log_frag_size >= 0){
		size_of_fragment = 1024 << es.s_log_frag_size;
	}else{
		size_of_fragment = 1024 >> -es.s_log_frag_size;
	}
	printf("Size of fragment %u\n", size_of_fragment);

	return es.s_magic == 61267 ? 0 : -1;
}

void read_block_bitmap(void* buffer, int group_number){
	int block_number = groups_table[group_number].bg_block_bitmap;
	read_block(buffer, block_number, size_of_block);
}

void read_inode_bitmap(void* buffer, int group_number){
	int block_number = groups_table[group_number].bg_inode_bitmap;
	read_block(buffer, block_number, size_of_block);
}

struct s_inode read_inode(int inode_number){
	int inode_group, inode_index;
	locate_inode(inode_number, es.s_inodes_per_group, &inode_group, &inode_index);

	printf("group: %u offset: %u\n", inode_group, inode_index);

	struct s_inode inode;
	int offset = sizeof(s_inode)*inode_index;
	int base_pointer = groups_table[inode_group].bg_inode_table*size_of_block;

	device_seek(base_pointer + offset);
	device_read(&inode, sizeof(s_inode));

	printf("block: %u base: %d offset: %d\n", groups_table[inode_group].bg_inode_table, base_pointer, offset);
	printf("i_mode %x\n", inode.i_mode);
	printf("i_blocks %u\n", inode.i_blocks);

	for (int i = 0; i < EXT2_N_BLOCKS; ++i)
	{
		printf("d %d %u\n", i, inode.i_direct[i]);
	}

  // printf("d1 %u\n", inode.i_direct1);
  // printf("d2 %u\n", inode.i_direct2);
  // printf("d3 %u\n", inode.i_direct3);
  // printf("d4 %u\n", inode.i_direct4);
  // printf("d5 %u\n", inode.i_direct5);
  // printf("d6 %u\n", inode.i_direct6);
  // printf("d7 %u\n", inode.i_direct7);
  // printf("d8 %u\n", inode.i_direct8);
  // printf("d9 %u\n", inode.i_direct9);
  // printf("d10 %u\n", inode.i_direct10);
  // printf("d10 %u\n", inode.i_direct11);
  // printf("d12 %u\n", inode.i_direct12);

	printf("indirect %u\n", inode.i_indirect);
	printf("d_indirect %u\n", inode.i_d_indirect);
	printf("t_indirect %u\n", inode.i_t_indirect);

	return inode;
}

int get_free_group_inode(int group_number){
	byte buffer[size_of_block];
	read_inode_bitmap(buffer, group_number);
	int start = 0;
	if(group_number == 0)
		start = 10;
	return bitmapSearch(buffer, 0, size_of_block, start);
}

void list_root_directory_entries(){
	struct s_dir_entry2 *entry;

	unsigned char block[size_of_block];
	struct s_inode root_inode = read_inode(2);
  //fseek(f, root_inode.i_direct[0]*size_of_block, SEEK_SET);

	read_block(block, root_inode.i_direct[0], size_of_block);

	int size = 0;
  //int so_dir_entry = sizeof(s_dir_entry2) - sizeof(char*);

	printf("inode size: %d\n", root_inode.i_size);

  entry = (struct s_dir_entry2 *) block;           /* first entry in the directory */
	while(size < root_inode.i_size) {

		char file_name[EXT2_NAME_LEN+1];

		memcpy(file_name, entry->name, entry->name_len);

    file_name[entry->name_len] = 0;              /* append null char to the file name */

		printf("%u %s\n", entry->inode, file_name);

    entry = (struct s_dir_entry2*)((void*) entry + entry->rec_len);      /* move to the next entry */
		size += entry->rec_len;
	}

  /*for (int i = 0; i < root_inode.i_blocks/2; ++i)
  {
    printf("--|Reading block\n");
    int size = 0;
    int so_dir_entry = sizeof(s_dir_entry2) - sizeof(char*);

    printf("inode size: %d\n", root_inode.i_size);
    while(size < root_inode.i_size){
      struct s_dir_entry2 dir_entry;
      fread(&dir_entry, 1, so_dir_entry, f);
      
      if(dir_entry.inode == EXT2_BAD_INO || dir_entry.inode == 0)
        break;
      
      printf("inode %u\n", dir_entry.inode);
      printf("name_len %u\n", dir_entry.name_len);

      char name[dir_entry.rec_len - so_dir_entry + 1];
      dir_entry.name = (char*) malloc(255);

      fread(name, 1, dir_entry.name_len, f);
      name[dir_entry.name_len] = 0;
      memcpy(dir_entry.name, name, dir_entry.rec_len - so_dir_entry);

      printf("name: %s\n", dir_entry.name);

      free(dir_entry.name);
      size += dir_entry.rec_len;
    }
  }*/
}

void test(){

	byte buffer[size_of_block];
	read_inode_bitmap(buffer, 0);
	int result;
	for (int i = 0; i < es.s_inodes_per_group; i++)
  // for (int i = 0; i < 10; i++)
	{
    // bitmapSet(buffer, i);
		result = bitmapGet(buffer, i);
		if(result == 1)
			printf("Inode[%d]: %d\n", i, result);
	}

	printf("next available: %d\n", get_free_group_inode(0));
	printf("sizeof(s_inode): %d\n", sizeof(s_inode));

	printf("\tRoot Inode\n");
	list_root_directory_entries();
}




int sfs_getattr(const char *path, struct stat *statbuf){
	printf("getattr: %s\n", path);
	
	int res = 0;

	memset(statbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		statbuf->st_mode = S_IFDIR | 0755;
		statbuf->st_nlink = 2;
	} else if (strcmp(path+1, "test") == 0) {
		statbuf->st_mode = S_IFREG | 0444;
		statbuf->st_nlink = 1;
		statbuf->st_size = strlen("content");
	} else if (strcmp(path+1, "test2") == 0) {
		statbuf->st_mode = S_IFREG | 0444;
		statbuf->st_nlink = 1;
		statbuf->st_size = strlen("contenido");
	} else if(strcmp(path+1, "hello") == 0){
		statbuf->st_mode = S_IFDIR | 0755;
		statbuf->st_nlink = 2;
	} else{
		res = -ENOENT;
	}

	return res;
}

int sfs_open(const char *path, struct fuse_file_info *fileInfo){
	printf("open %s\n", path);
	return 0;
}

int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo){
	// printf("read %s\n", path);
	// printf("offset %d\n", offset);
	// printf("size %d\n", size);

	size_t len;
	(void) fileInfo;

	if(strcmp(path+1, "test") == 0){

		len = strlen("content");
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, "content" + offset, size);
		} else
		size = 0;
	} else if(strcmp(path+1, "test2") == 0){

		len = strlen("contenido");
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, "contenido" + offset, size);
		} else
		size = 0;
	}
	return 0;
}

int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo){
	printf("readdir %s\n", path);

	if (strcmp(path, "/") == 0){
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		filler(buf, "test", NULL, 0);
		filler(buf, "test2", NULL, 0);
		filler(buf, "hello", NULL, 0);
		return 0;
	}else if(strcmp(path, "/hello") == 0){
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		return 0;
	}
	return -ENOENT;
}

void sfs_init(struct fuse_conn_info *conn){
	printf("init\n");
	int status = read_sb();
	read_group_descriptors();

	test();
	printf("end init\n");
	dir_path = (char*) malloc(1);
	dir_path[0] = 0;
}

int sfs_opendir(const char *path, struct fuse_file_info *fileInfo){
	if(strcmp(path,dir_path) != 0){
		printf("opendir %s\n", path);
		int path_len = strlen(path);
		dir_path = (char*) malloc(path_len+1);
		strncpy(dir_path, path, path_len);
		dir_path[path_len] = 0;
		printf("new dir path %s\n", dir_path);
	}
	return 0;
}

