#include "nxt2fs.h"

//INIT
void read_group_descriptors()
{
    // struct stat fileStat = device_stat();

    // number_of_groups = fileStat.st_size / (es.s_blocks_per_group * size_of_block) + 1;
    number_of_groups = 1 + (es.s_blocks_count-1) / es.s_blocks_per_group;
    if (print_info)
    {
        printf("Number of groups: %d\n", number_of_groups);
    }

    groups_table = (struct s_block_group_descriptor *)malloc(sizeof(struct s_block_group_descriptor) * number_of_groups);

    uint32 offset = size_of_block;
    if (size_of_block == 1024)
        offset += 1024;

    device_seek(offset);
    printf("\n");
    // printf("size of block group %lu\n", sizeof(struct s_block_group_descriptor));

    for (uint32 i = 0; i < number_of_groups; ++i)
    {
        device_read(&groups_table[i], sizeof(struct s_block_group_descriptor));

        if (print_info)
        {
            printf("--|Block Group: %d\n", i);
            printf("block_bitmap: %u\n", groups_table[i].bg_block_bitmap);
            printf("inode_bitmap: %u\n", groups_table[i].bg_inode_bitmap);
            printf("inode_table: %u\n", groups_table[i].bg_inode_table);
            printf("free_blocks_count: %u\n", groups_table[i].bg_free_blocks_count);
            printf("free_inodes_count: %u\n", groups_table[i].bg_free_inodes_count);
            printf("used_dirs_count: %u\n", groups_table[i].bg_used_dirs_count);
            printf("\n\n");
        }
    }
}

int read_sb()
{
    device_seek(1024);
    device_read(&es, sizeof(s_superblock));

    size_of_block = 1024 << es.s_log_block_size;

    if (es.s_log_frag_size >= 0)
    {
        size_of_fragment = 1024 << es.s_log_frag_size;
    }
    else
    {
        size_of_fragment = 1024 >> -es.s_log_frag_size;
    }
    if (print_info)
    {
        printf("Size of block %u\n", size_of_block);
        printf("Size of fragment %u\n", size_of_fragment);
    }

    indirect_blocks_count = size_of_block / sizeof(uint32);
    d_indirect_blocks_count = indirect_blocks_count * indirect_blocks_count;
    t_indirect_blocks_count = d_indirect_blocks_count * indirect_blocks_count;

    return es.s_magic == 61267 ? 0 : -1;
}

//READ INODE BLOCKS
void read_t_indirect_block(void *buffer, uint32 t_indirect_block, uint32 logic_position)
{

    uint32 t_indirect_blocks[size_of_block / sizeof(uint32)];
    read_block(t_indirect_blocks, t_indirect_block, size_of_block);

    uint32 position = logic_position / (size_of_block / sizeof(uint32));
    uint32 offset = logic_position - position * d_indirect_blocks_count;

    printf("reading t_indirect block number: %lu logic pos %lu position %lu offset %lu\n", t_indirect_blocks[position], logic_position, position, offset);

    read_d_indirect_block(buffer, t_indirect_blocks[position], offset);
}

void read_d_indirect_block(void *buffer, uint32 d_indirect_block, uint32 logic_position)
{
    // printf("reading d_indirect block\n");

    uint32 d_indirect_blocks[size_of_block / sizeof(uint32)];
    read_block(d_indirect_blocks, d_indirect_block, size_of_block);

    uint32 position = logic_position / (size_of_block / sizeof(uint32));
    uint32 offset = logic_position - position * indirect_blocks_count;
    printf("reading d_indirect block number: %lu logic pos %lu position %lu offset %lu\n", d_indirect_blocks[position], logic_position, position, offset);

    read_indirect_block(buffer, d_indirect_blocks[position], offset);
}

void read_indirect_block(void *buffer, uint32 indirect_block, uint32 logic_position)
{
    uint32 indirect_blocks[size_of_block / sizeof(uint32)];
    read_block(indirect_blocks, indirect_block, size_of_block);
    printf("reading indirect block number: %lu logic position %lu\n", indirect_blocks[logic_position], logic_position);
    if (indirect_blocks[logic_position] >= es.s_firts_inode)
    {
        read_block(buffer, indirect_blocks[logic_position], size_of_block);
    }
}

void read_inode_logic_block(void *buffer, struct s_inode inode, uint32 logic_block_number)
{
    if (logic_block_number < EXT2_NDIR_BLOCKS)
        read_block(buffer, inode.i_direct[logic_block_number], size_of_block);
    else
    {
        logic_block_number -= EXT2_NDIR_BLOCKS;
        if (logic_block_number >= d_indirect_blocks_count)
            read_t_indirect_block(buffer, inode.i_t_indirect, logic_block_number - d_indirect_blocks_count);
        else if (logic_block_number >= indirect_blocks_count)
            read_d_indirect_block(buffer, inode.i_d_indirect, logic_block_number - indirect_blocks_count);
        else
            read_indirect_block(buffer, inode.i_indirect, logic_block_number);
    }
}

//BITMAPS
void read_block_bitmap(void *buffer, int group_number)
{
    int block_number = groups_table[group_number].bg_block_bitmap;
    read_block(buffer, block_number, size_of_block);
}

void read_inode_bitmap(void *buffer, int group_number)
{
    int block_number = groups_table[group_number].bg_inode_bitmap;
    read_block(buffer, block_number, size_of_block);
}

void inode_bitmap_set(uint32 inode_number, uint8 state)
{
    int inode_group, inode_index;
    locate(inode_number, es.s_inodes_per_group, &inode_group, &inode_index);

    byte buffer[size_of_block];
    read_inode_bitmap(buffer, inode_group);
    if (state)
        bitmapSet(buffer, inode_index);
    else
        bitmapReset(buffer, inode_index);
    write_block(buffer, groups_table[inode_group].bg_inode_bitmap, size_of_block);
}

void block_bitmat_set(uint32 inode_number, uint8 state)
{
    int block_group, block_index;
    locate(inode_number, es.s_blocks_per_group, &block_group, &block_index);

    byte buffer[size_of_block];
    read_inode_bitmap(buffer, block_group);
    if (state)
        bitmapSet(buffer, block_index);
    else
        bitmapReset(buffer, block_index);
    write_block(buffer, groups_table[block_group].bg_block_bitmap, size_of_block);
}

//INODES
struct s_inode* read_inode(uint32 inode_number)
{
    int inode_group, inode_index;
    locate(inode_number, es.s_inodes_per_group, &inode_group, &inode_index);

    struct s_inode* inode = (struct s_inode*) malloc(sizeof(struct s_inode));
    char inode_buff[es.s_inode_size];
    int offset = es.s_inode_size * inode_index;
    uint32 base_pointer = groups_table[inode_group].bg_inode_table * size_of_block;

    device_seek(base_pointer + offset);
    device_read(inode_buff, es.s_inode_size);
    memcpy(inode, inode_buff, sizeof(struct s_inode));

    if (print_info)
    {
        printf("block: %u base: %d offset: %d\n", groups_table[inode_group].bg_inode_table, base_pointer, offset);
        printf("i_mode %x\n", inode->i_mode);
        printf("i_blocks %lu\n", inode->i_blocks);

        for (int i = 0; i < EXT2_NDIR_BLOCKS; ++i)
        {
            printf("d %d %lu\n", i, inode->i_direct[i]);
        }

        printf("indirect %lu\n", inode->i_indirect);
        printf("d_indirect %lu\n", inode->i_d_indirect);
        printf("t_indirect %lu\n", inode->i_t_indirect);
    }

    return inode;
}

int get_free_inode_in_group(uint32 group_number)
{
    byte buffer[size_of_block];
    read_inode_bitmap(buffer, group_number);
    return bitmapSearch(buffer, 0, size_of_block, 0);
}

int get_free_block_in_group(uint32 group_number){
    byte buffer[size_of_block];
    read_block_bitmap(buffer, group_number);
    return bitmapSearch(buffer, 0, size_of_block, 0);
}

int get_free_inode(uint32 group_number){
    int new_inode = get_free_inode_in_group(group_number);
    int inode_group_offset = 1;
    while(new_inode < 0 && group_number + (inode_group_offset) <= number_of_groups){
        new_inode = get_free_inode_in_group(group_number + (inode_group_offset++));
    }
    return new_inode;
}

int get_free_block(uint32 group_number){
    int new_block = get_free_block_in_group(group_number);
    int block_group_offset = 1;
    while(new_block < 0 && group_number + (block_group_offset) <= number_of_groups){
        new_block = get_free_inode_in_group(group_number + (block_group_offset++));
    }
    return new_block;
}

//ENTRIES
uint32 lookup_entry_inode(char *path, uint32 current_inode_number)
{
    if (strlen(path) == 1 && strcmp(path, "/") == 0)
        return ROOT_INO;
    else
    {
        struct s_dir_entry2 *entry;
        path = path + 1;

        uint16 dashPos = findChar(path, '/');
        char sub_path[dashPos + 2];
        bzero(sub_path, dashPos + 1);
        strncpy(sub_path, path, dashPos);
        sub_path[dashPos + 1] = 0;

        if (print_info)
            printf("lookup path: %s subpath: %s\n", path, sub_path);

        uint8 hasChild = strlen(path) > strlen(sub_path);
        // unsigned char block[size_of_block];
        struct s_inode* current_inode = read_inode(current_inode_number);
        entry = find_entry(*current_inode, sub_path);
        free(current_inode);
        if(entry == NULL)
            return 0;
        if(hasChild)
            return lookup_entry_inode(path+dashPos, entry->inode);
        uint32 inode = entry->inode;
        free(entry);
        return inode;
    }
    // printf("lue entry_inode not found\n");
    return 0;
}

struct s_dir_entry2* find_last_entry(struct s_inode inode)
{
    struct s_dir_entry2 *entry;
    unsigned char block[size_of_block];

    uint32 current_block = 0;

    // read_block(block, dir_inode.i_direct[0], size_of_block);
    read_inode_logic_block(block, inode, current_block++);

    uint16 c_size = 0;

    entry = (struct s_dir_entry2 *)block;

    while (c_size < inode.i_size)
    {
        // char test;
        // fgets(&test, 1, stdin);
        if (c_size > 0 && c_size % size_of_block == 0)
            read_inode_logic_block(block, inode, current_block++);

        char file_name[EXT2_NAME_LEN + 1];

        memcpy(file_name, entry->name, entry->name_len);

        file_name[entry->name_len] = 0; /* append null char to the file name */
        // printf("%lu %s\n", entry->inode, file_name);

        c_size += entry->rec_len;

        if(c_size >= inode.i_size)
            break;

        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }

    struct s_dir_entry2* ret = (struct s_dir_entry2*) malloc(sizeof(struct s_dir_entry2));
    ret->inode = entry->inode;
    ret->rec_len = entry->rec_len;
    ret->name_len = entry->name_len;
    ret->file_type = entry->file_type;
    strncpy(ret->name, entry->name, entry->name_len);
    ret->block_number = current_block-1;
    ret->offset = c_size % size_of_block;

    return ret;
}

struct s_dir_entry2* find_entry(struct s_inode inode, const char* entry_name){
    struct s_dir_entry2 *entry;
    unsigned char block[size_of_block];
    uint32 current_block = 0;
    read_inode_logic_block(block, inode, current_block++);
    uint16 c_size = 0;
    entry = (struct s_dir_entry2 *)block;

    while (c_size < inode.i_size)
    {
        // char test;
        // fgets(&test, 1, stdin);
        if (c_size > 0 && c_size % size_of_block == 0)
            read_inode_logic_block(block, inode, current_block++);

        char file_name[EXT2_NAME_LEN + 1];

        memcpy(file_name, entry->name, entry->name_len);

        file_name[entry->name_len] = 0; /* append null char to the file name */
        printf("%lu %s\n", entry->inode, file_name);

        c_size += entry->rec_len;

        if (strcmp(file_name, entry_name) == 0)
        {
            struct s_dir_entry2 *return_entry = (struct s_dir_entry2 *)malloc(sizeof(struct s_dir_entry2));
            return_entry->inode = entry->inode;
            return_entry->rec_len = entry->rec_len;
            return_entry->name_len = entry->name_len;
            return_entry->file_type = entry->file_type;
            strcpy(return_entry->name,entry->name);
            return_entry->block_number = current_block-1;
            return_entry->offset = c_size % size_of_block;
            return return_entry;
        }

        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }
    return NULL;
}

struct s_dir_entry2* find_previous_entry(struct s_inode inode, const char* entry_name){
    struct s_dir_entry2 *entry;
    struct s_dir_entry2 *previous_entry = NULL;
    unsigned char block[size_of_block];
    uint32 current_block = 0;
    read_inode_logic_block(block, inode, current_block++);
    uint16 c_size = 0;
    entry = (struct s_dir_entry2 *)block;

    while (c_size < inode.i_size)
    {
        // char test;
        // fgets(&test, 1, stdin);
        if (c_size > 0 && c_size % size_of_block == 0)
            read_inode_logic_block(block, inode, current_block++);

        char file_name[EXT2_NAME_LEN + 1];

        memcpy(file_name, entry->name, entry->name_len);

        file_name[entry->name_len] = 0; /* append null char to the file name */
        // printf("%lu %s\n", entry->inode, file_name);

        c_size += entry->rec_len;

        if (strcmp(file_name, entry_name) == 0)
        {
            if(previous_entry == NULL)
                return NULL;
            uint32 offset = (c_size % size_of_block)-entry->rec_len;
            struct s_dir_entry2 *return_entry;
            if(offset>=0){
                return_entry = (struct s_dir_entry2 *)malloc(sizeof(struct s_dir_entry2));
                return_entry->inode = previous_entry->inode;
                return_entry->rec_len = previous_entry->rec_len;
                return_entry->name_len = previous_entry->name_len;
                return_entry->file_type = previous_entry->file_type;
                strcpy(return_entry->name,previous_entry->name);
                return_entry->block_number = current_block-1;
                return_entry->offset;
            }else{
                return_entry = find_entry(inode, previous_entry->name);
            }

            return return_entry;
        }
        previous_entry = entry;
        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }
    return NULL;
}

// void get_entry_stat

//TEST
void test()
{

    /*byte buffer[size_of_block];
	read_inode_bitmap(buffer, 0);
	int result;
	for (uint32 i = 0; i < es.s_inodes_per_group; i++)
  // for (int i = 0; i < 10; i++)
	{
    // bitmapSet(buffer, i);
		result = bitmapGet(buffer, i);
		if(print_info)
			printf("Inode[%d]: %d\n", i, result);
	}

	if(print_info){
		printf("next available: %d\n", get_free_group_inode(0));
		printf("sizeof(s_inode): %lu\n", sizeof(s_inode));

		printf("\tRoot Inode\n");
	}*/
    uint32 indirect_blocks = size_of_block / sizeof(uint32);
    uint32 d_indirect_blocks = indirect_blocks * indirect_blocks;
    uint32 t_indirect_blocks = d_indirect_blocks * indirect_blocks;

    printf("sizeof struct s_inode %lu\n", sizeof(struct s_inode));
    printf("sizeof es.inode %lu\n", es.s_inode_size);

    printf("Direct blocks %d\n", EXT2_NDIR_BLOCKS);
    printf("Indirect blocks %lu\n", indirect_blocks_count);
    printf("D_Indirect blocks %lu\n", d_indirect_blocks_count);
    printf("T_Indirect blocks %lu\n", t_indirect_blocks_count);
    // nxfs_truncate("/helloworld.asm",0);

    // struct s_inode* inode;
    // inode = read_inode(ROOT_INO);
    // struct s_dir_entry2* entry = find_last_entry(*inode);
    // printf("last entry of root: %lu %s\n", entry->inode, entry->name);

    // entry = find_entry(*inode, "test");
    // printf("last entry of root: %lu %s\n", entry->inode, entry->name);

    // entry = find_previous_entry(*inode, "test");
    // printf("last entry of root: %lu %s\n", entry->inode, entry->name);

    // free(inode);


    // char* str = "/folder";
    // printf("%s\n", str);
    // uint32 len = strlen(str);

    // char dir[len];
    // char name[len];
    // getNewDir(str, dir, name);
    // printf("path %s parent_dir %s name %s\n", str, dir, name);
}

//FUSE FUNCTIONS
void nxfs_init(struct fuse_conn_info *conn)
{
    int status = read_sb();
    read_group_descriptors();

    test();
    dir_path = (char *)malloc(1);
    file_path = (char *)malloc(1);
    dir_path[0] = 0;
    file_path[0] = 0;
    printf("\n\n");
}

int nxfs_get_attr(const char *path, struct stat *statbuf)
{
    if (print_info)
        printf("getattr %s\n", path);
    memset(statbuf, 0, sizeof(struct stat));

    uint32 len = strlen(path);
    char path_copy[len + 1];
    strncpy(path_copy, path, len);
    path_copy[len] = 0;

    uint32 entry_inode = lookup_entry_inode(path_copy, ROOT_INO);
    if (entry_inode == 0)
    {
        printf("entry_inode not found for %s\n", path);
        return -ENOENT;
    }

    // printf("entry_inode %lu\n", entry_inode);

    struct s_inode* dir_inode = read_inode(entry_inode);

    // printf("mode %lu\n", dir_inode.i_mode);
    // printf("links count \n", dir_inode.i_links_count);
    // printf("size \n", dir_inode.i_size);
    // printf("blocks \n", dir_inode.i_blocks);

    statbuf->st_mode = dir_inode->i_mode;
    statbuf->st_nlink = dir_inode->i_links_count;
    statbuf->st_size = dir_inode->i_size;
    //printf("file size: %lu\n", dir_inode.i_size);
    statbuf->st_uid = dir_inode->i_uid;
    statbuf->st_gid = dir_inode->i_gid;
    // statbuf->st_ino = 2;
    statbuf->st_blocks = dir_inode->i_blocks;

    if (print_info)
        printf("success\n");
    return 0;
}

int nxfs_read_dir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo)
{
    if (print_info)
        printf("read dir %s current_dir_inode %lu\n", path, current_dir_inode);
    // printf("FI_dir %lu\n", (uint64_t)fileInfo);
    struct s_dir_entry2 *entry;

    // uint32 len = strlen(path);
    // char path_copy[len+1];
    // strncpy(path_copy, path, len);
    // path_copy[len] = 0;

    // uint32 entry_inode = lookup_entry_inode(path_copy, ROOT_INO);
    // if(entry_inode < 0)
    // 	return -ENOENT;

    struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;
    // printf("fh->d_inode %lu fh->f_size %lu \n", fh->d_inode, fh->f_size);
    uint32 entry_inode = fh->f_inode;

    // printf("read entry, inode: %u\n", entry_inode);

    unsigned char block[size_of_block];
    struct s_inode* dir_inode = read_inode(entry_inode);
    //fseek(f, dir_inode.i_direct[0]*size_of_block, SEEK_SET);
    uint32 current_block = 0;

    // read_block(block, dir_inode.i_direct[0], size_of_block);
    read_inode_logic_block(block, *dir_inode, current_block++);

    uint32 c_size = 0;

    entry = (struct s_dir_entry2 *)block; /* first entry in the directory */

    while (c_size < dir_inode->i_size)
    {
        if (c_size > 0 && c_size % size_of_block == 0)
            read_inode_logic_block(block, *dir_inode, current_block++);

        char file_name[EXT2_NAME_LEN + 1];

        memcpy(file_name, entry->name, entry->name_len);

        file_name[entry->name_len] = 0; /* append null char to the file name */

        c_size += entry->rec_len;
        if (print_info)
            printf("rd-> \t%u %s\n", entry->inode, file_name);
        // filler(buf, file_name, NULL, c_size - entry->rec_len);
        filler(buf, file_name, NULL, 0);

        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }
    if (print_info)
        printf("%u of %u\n", dir_inode->i_size, c_size);
    free(dir_inode);
    return 0;
}

int nxfs_opendir(const char *path, struct fuse_file_info *fileInfo)
{
    if (print_info)
        printf("opendir %s\n", path);
    // printf("FI_dir %lu\n", (uint64_t)fileInfo);
    // if (strlen(path) == 1 && strcmp(path, "/") == 0)
    // {
    //     current_dir_inode = ROOT_INO;
    //     struct s_inode dir_inode = read_inode(ROOT_INO);
    //     struct s_file_handle *fh = (s_file_handle *)malloc(sizeof(s_file_handle));
    //     fh->f_inode = ROOT_INO;
    //     fh->f_size = dir_inode.i_size;
    //     fileInfo->fh = (uint64_t)fh;
    //     printf("root ino\n");
    //     return 0;
    // }
    // else
    // {
    uint32 len = strlen(path);

    if ((len != 1) && path[0] != '/')
    {
        printf("No permitido \n");
        return -EPERM;
    }

    char path_copy[len + 1];
    strncpy(path_copy, path, len);
    path_copy[len] = 0;
    uint32 entry_inode = lookup_entry_inode(path_copy, ROOT_INO);
    if (entry_inode > 0)
    {
        struct s_inode* dir_inode = read_inode(entry_inode);
        if (is_dir(dir_inode->i_mode))
        {
            current_dir_inode = entry_inode;
            struct s_file_handle *fh = (s_file_handle *)malloc(sizeof(s_file_handle));
            fh->f_inode = entry_inode;
            fh->f_size = dir_inode->i_size;
            fh->f_blocks_count = dir_inode->i_blocks;
            fileInfo->fh = (uint64_t)fh;
            free(dir_inode);
            return 0;
        }
        else
        {
            printf("No es directorio\n");
            free(dir_inode);
            return -EPERM;
        }
    }
    else
    {
        printf("entry_inode not found for %s\n", path);
        return -ENOENT;
    }
    // }
    return -ENOENT;
}

int nxfs_statfs(const char *path, struct statvfs *statInfo)
{
    //printf("statfs(path=%s)\n", path);

    statInfo->f_bsize = size_of_block;                      /* filesystem block size */
    statInfo->f_frsize = size_of_fragment;                  /* fragment size */
    statInfo->f_blocks = es.s_blocks_count * size_of_block; /* size of fs in f_frsize units */
    statInfo->f_bfree = es.s_free_block_count;              /* # free blocks */
    statInfo->f_bavail = es.s_free_block_count;             /* # free blocks for unprivileged users */
    statInfo->f_files = es.s_inodes_count;                  /* # inodes */
    statInfo->f_ffree = es.s_free_inodes_count;             /* # free inodes */
    statInfo->f_favail = es.s_free_inodes_count;            /* # free inodes for unprivileged users */
    statInfo->f_fsid = es.s_uuid;                           /* filesystem ID */
    statInfo->f_namemax = EXT2_NAME_LEN;                    /* maximum filename length */

    return 0;
}

int nxfs_open(const char *path, struct fuse_file_info *fileInfo)
{
    if (print_info)
        printf("open %s\n", path);

    if (strlen(path) == 1 && strcmp(path, "/") == 0)
        return -EPERM;
    else
    {
        uint32 len = strlen(path);

        if ((len != 1) && path[0] != '/')
            return -EPERM;

        char path_copy[len + 1];
        strncpy(path_copy, path, len);
        path_copy[len] = 0;
        uint32 entry_inode = lookup_entry_inode(path_copy, ROOT_INO);
        if (entry_inode > 0)
        {
            struct s_inode* dir_inode = read_inode(entry_inode);
            if (is_file(dir_inode->i_mode))
            {
                current_file_inode = entry_inode;
                // printf("--> file inode %lu\n", entry_inode);
                struct s_file_handle *fh = (s_file_handle *)malloc(sizeof(s_file_handle));
                fh->f_inode = entry_inode;
                fh->f_size = dir_inode->i_size;
                fh->f_blocks_count = dir_inode->i_blocks;
                fileInfo->fh = (uint64_t)fh;
                free(dir_inode);
                return 0;
            }
            else{
                free(dir_inode);
                return -EPERM;
            }
        }
        else
        {
            return -ENOENT;
        }
    }
    return -ENOENT;
}

int nxfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    if (print_info)
        printf("read %s size %u offset %u\n", path, size, offset);
    //printf("current_file_inode %lu\n", current_file_inode);

    int bytes_to_read = size;

    struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;
    // printf("fh->d_inode %lu fh->f_size %lu \n", fh->f_inode, fh->f_size);

    if (offset >= fh->f_size)
        return 0;

    if (offset + size > fh->f_size)
        bytes_to_read = fh->f_size - offset;

    uint32 logic_block_number = offset / size_of_block;
    uint32 number_of_blocks = size / size_of_block;
    struct s_inode* inode = read_inode(fh->f_inode);

    // printf("logic_block_number %lu number_of_blocks %lu size_in_blocks %lu\n", logic_block_number, number_of_blocks, inode.i_blocks);

    for (int i = 0; i < number_of_blocks; i++)
    {
        char buff[size_of_block];
        read_inode_logic_block(buff, *inode, logic_block_number + i);
        //printf("|%s|\n", buff);
        memcpy(&buf[i * size_of_block], buff, size_of_block);
    }

    // if(strcmp(path+1, "test") == 0){

    // 	len = strlen("content");
    // 	if (offset < len) {
    // 		if (offset + size > len)
    // 			size = len - offset;
    // 		memcpy(buf, "content" + offset, size);
    // 	} else
    // 	size = 0;
    // } else if(strcmp(path+1, "test2") == 0){

    // 	len = strlen("contenido");
    // 	if (offset < len) {
    // 		if (offset + size > len)
    // 			size = len - offset;
    // 		memcpy(buf, "contenido" + offset, size);
    // 	} else
    // 	size = 0;
    // }
    return bytes_to_read;
}

int nxfs_release(const char *path, struct fuse_file_info *fileInfo)
{
    // printf("release %s\n", path);
    free((void *)fileInfo->fh);
    return 0;
}

int nxfs_releasedir(const char *path, struct fuse_file_info *fileInfo)
{
    // printf("releaseDir %s\n", path);
    free((void *)fileInfo->fh);
    return 0;
}

//NOT IMPLEMENTED, YET
int nxfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    printf("write %s size %lu offset %lu\n", path, size, offset);
    return size;
}

int nxfs_rename(const char *path, const char *newpath)
{
    printf("rename %s newPath %s\n", path, newpath);
    return 0;
}

int nxfs_rmdir(const char *path)
{
    printf("rmdir\n");
    return 0;
}

int nxfs_mkdir(const char *path, mode_t mode)
{
    uint32 len = strlen(path);
    char parent[len];
    char dir_name[len];

    parseNewEntry(path, parent, dir_name);

    uint32 dir_name_len = strlen(dir_name);

    printf("mkdir %s parent_dir %s mode %x\n", dir_name, parent, mode);

    uint32 parent_inode_number = lookup_entry_inode(parent, ROOT_INO);

    if(parent_inode_number == 0){
        return -EPERM;
    }

    int parent_inode_group, parent_inode_index;
    locate(parent_inode_number, es.s_inodes_per_group, &parent_inode_group, &parent_inode_index);

    int new_inode = get_free_inode(parent_inode_group);
    if(new_inode < 0)
        return -ENOENT;

    int new_inode_group, new_inode_index;
    locate(new_inode, es.s_blocks_per_group, &new_inode_group, &new_inode_index);

    int new_block = get_free_block(new_inode_group);
    if(new_block < 0)
        return -ENOENT;

    struct s_inode* parent_inode = read_inode(parent_inode_number);

    struct s_dir_entry2* parent_last_entry = find_last_entry(*parent_inode);

    struct s_dir_entry2 new_entry;
    new_entry.inode = new_inode;
    new_entry.rec_len = size_of_block;
    // new_entry

    if(parent_last_entry->offset % size_of_block >= ENTRY_BASE_SIZE + dir_name_len + 1){
        char buffer[size_of_block];
        read_inode_logic_block(buffer, *parent_inode, parent_last_entry->block_number);

        uint32 new_rec_len = ENTRY_BASE_SIZE + parent_last_entry->name_len + 2;
        memcpy(&buffer[parent_last_entry->offset + ENTRY_BASE_SIZE],  &new_rec_len, sizeof(uint32));


    }else{
        // ask for new block entries
    }


    return 0;
}

int save_inode(struct s_inode inode, uint32 index){
  int inode_group, inode_index;
  locate(index, es.s_inodes_per_group, &inode_group, &inode_index);
  // char inode_buff[es.s_inode_size];
  // memcpy(inode_buff,(void *)&inode, es.s_inode_size);
  int offset = es.s_inode_size * inode_index;
  uint32 base_pointer = groups_table[inode_group].bg_inode_table * size_of_block;

  device_seek(base_pointer + offset);
  return device_write(&inode, es.s_inode_size);
}

int save_meta_data(){
  //saving in group 0
  device_seek(1024);
  device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
  printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", 0, 1024, 0);

  uint32 offset_group_descriptor = size_of_block;
  uint32 size_of_group_descriptor = sizeof(struct s_block_group_descriptor)*number_of_groups;

  //saveing group descriptor 0
  device_seek(1024+size_of_block+offset_group_descriptor);
  device_write(&groups_table, size_of_group_descriptor);
  printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", 0, 1024+size_of_block+offset_group_descriptor, 1);
  
  //saving in group 1
  uint32 offset = (1 * es.s_blocks_per_group)*size_of_block;
  device_seek(1024+offset);
  device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
  printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", 1, 1024+offset,es.s_blocks_per_group);
  //saveing group descriptor 1
  device_seek(1024+offset+size_of_block);
  device_write(&groups_table, size_of_group_descriptor);
  printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", 1, 1024+offset+size_of_block,es.s_blocks_per_group+1);
  //saving in power of 3
  for(int i =3; i< number_of_groups;i*=3){
    uint32 block = (i * es.s_blocks_per_group);
    offset = block*size_of_block;
    device_seek(1024+offset);
    device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
    printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", i, offset,block);
    device_seek(1024+offset+size_of_block);
    device_write(&groups_table, size_of_group_descriptor);
    printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", i, offset+size_of_block,block+1);
  }

  //saving in power of 5
  for(int i =5; i< number_of_groups;i*=5){
    uint32 block = (i * es.s_blocks_per_group);
    offset = block*size_of_block;    device_seek(1024+offset);
    device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
    printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", i, offset,block);
    device_seek(1024+offset+size_of_block);
    device_write(&groups_table, size_of_group_descriptor);
    printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", i, offset+size_of_block,block+1);
  }

  //saving in power of 7
  for(int i =7; i< number_of_groups;i*=7){
    uint32 block = (i * es.s_blocks_per_group);
    offset = block*size_of_block;
    device_seek(1024+offset);
    device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
    printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", i, offset,block);
    device_seek(1024+offset+size_of_block);
    device_write(&groups_table, size_of_group_descriptor);
    printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", i, offset+size_of_block,block+1);
  }

  return 0;
}

void free_i_logic_block(uint32 i_indirect_block,uint32 logic_position){
    uint32 i_indirect_blocks[size_of_block / sizeof(uint32)];
    read_block(i_indirect_blocks, i_indirect_block, size_of_block);

    if (i_indirect_blocks[logic_position]!=0)
    {
        printf("free data Block: %d\n", i_indirect_blocks[logic_position]);
        block_bitmat_set(i_indirect_blocks[logic_position],0);
    }
}

void free_d_logic_block(uint32 d_indirect_block,uint32 logic_position){
    uint32 d_indirect_blocks[size_of_block / sizeof(uint32)];
    read_block(d_indirect_blocks, d_indirect_block, size_of_block);

    uint32 position = logic_position / (size_of_block / sizeof(uint32));
    uint32 offset = logic_position - position * d_indirect_blocks_count;

    free_i_logic_block(d_indirect_blocks[position], offset);
}

void free_t_logic_block(uint32 t_indirect_block,uint32 logic_position){
    uint32 t_indirect_blocks[size_of_block / sizeof(uint32)];
    read_block(t_indirect_blocks, t_indirect_block, size_of_block);

    uint32 position = logic_position / (size_of_block / sizeof(uint32));
    uint32 offset = logic_position - position * d_indirect_blocks_count;

    free_d_logic_block(t_indirect_blocks[position], offset);
}

void free_logic_block(struct s_inode *child_inode, uint32 logic_block_number){
    if (logic_block_number < EXT2_NDIR_BLOCKS){
        uint32 physical_data_block = child_inode->i_direct[logic_block_number];
        if (physical_data_block!=0)
        {
            printf("free data Block: %d\n", physical_data_block);
            block_bitmat_set(physical_data_block,0);
            child_inode->i_direct[logic_block_number] = 0;
        }
    }
    else
    {
        logic_block_number -= EXT2_NDIR_BLOCKS;
        if (logic_block_number >= d_indirect_blocks_count)
            free_t_logic_block(child_inode->i_t_indirect,logic_block_number - d_indirect_blocks_count);
        else if (logic_block_number >= indirect_blocks_count)
            free_d_logic_block(child_inode->i_d_indirect,logic_block_number - indirect_blocks_count);
        else
            free_i_logic_block(child_inode->i_indirect,logic_block_number);
    }
}

void free_indirect_blocks(struct s_inode *child_inode){
    if(child_inode->i_indirect!=0)
        block_bitmat_set(child_inode->i_indirect,0);

    uint32 cant_indirects = size_of_block/sizeof(uint32);
    uint32 d_indirect_blocks[cant_indirects];
    if(child_inode->i_d_indirect!=0){
        read_block(d_indirect_blocks, child_inode->i_d_indirect, size_of_block);

        for (int i = 0; i < cant_indirects; ++i){
            if (d_indirect_blocks[i]!=0){
                printf("free indirect Block: %d\n",d_indirect_blocks[i]);
                block_bitmat_set(d_indirect_blocks[i],0);
            }
        }

        printf("free d indirect Block: %d\n",child_inode->i_d_indirect);
        block_bitmat_set(child_inode->i_d_indirect,0);
    }

    if(child_inode->i_t_indirect!=0){
        uint32 t_indirect_blocks[cant_indirects];
        read_block(t_indirect_blocks,child_inode->i_t_indirect,size_of_block);

        for (int i = 0; i < cant_indirects; ++i)
        {
            if (t_indirect_blocks[i]!=0)
            {
                read_block(d_indirect_blocks,t_indirect_blocks[i],size_of_block);
                for (int j = 0; j < cant_indirects; ++j)
                {
                    if (d_indirect_blocks[j]!=0)
                    {
                        printf("free indirect Block: %d\n",d_indirect_blocks[j]);
                        block_bitmat_set(d_indirect_blocks[j],0);
                    }
                }
                
                printf("free d indirect Block: %d\n",t_indirect_blocks[i]);
                block_bitmat_set(t_indirect_blocks[i],0);
            }
        }
        printf("free t indirect Block: %d\n",child_inode->i_t_indirect);
        block_bitmat_set(child_inode->i_t_indirect,0);
    }
}

int nxfs_truncate(const char *path, off_t newSize)
{
    char* path_child = (char*)malloc(strlen(path)+1);
    bzero(path_child,strlen(path)+1);
    memcpy(path_child,path,strlen(path));
    printf("truncate %s newsize %lu\n", path_child, newSize);

    uint32 inode_index = lookup_entry_inode(path_child,ROOT_INO);
    printf("inode_index: %d\n", inode_index);
    struct s_inode *child_inode = read_inode(inode_index);

    printf("Llego hasta aqui\n");

    if (child_inode->i_size == newSize)
        return 0;

    if (child_inode->i_size<newSize)
        return -ENOENT;

    if (inode_index<11)
    {
        printf("inode not found \n");
        return -ENOENT;
    }

    printf("Llego hasta aqui 2\n");
    uint32 inode_blocks = child_inode->i_size/size_of_block;
    if (inode_blocks*size_of_block < child_inode->i_size)
        inode_blocks+=1;
    uint32 new_block_size = newSize/size_of_block;
    printf("inode_blocks: %d\n",inode_blocks);

    if (newSize == 0)
    {
        for (uint32 i = 0; i < inode_blocks; ++i)
            free_logic_block(child_inode,i);

        printf("free data blocks\n");
        free_indirect_blocks(child_inode);
        printf("free indirect blocks\n");

        child_inode->i_indirect = 0;
        child_inode->i_d_indirect = 0;
        child_inode->i_t_indirect = 0;
        child_inode->i_size = 0;
    }

    save_inode(*child_inode,inode_index);
    printf("Saved inode\n");
    save_meta_data();
    printf("Saved meta data\n");
    free(path_child);
    return 0;
}

int nxfs_unlink(const char *path)
{
    printf("unlink %s\n", path);
    return 0;
}

int nxfs_mknod(const char *path, mode_t mode, dev_t dev)
{
    printf("mknod %s\n", path);
    return 0;
}

int nxfs_create(const char *path, mode_t mode, struct fuse_file_info *fileInfo)
{
    printf("create %s mode %x\n", path, mode);
    return 0;
}
