#include "fuse_x.h"

/*TEST*/
void test()
{

    // byte buffer[size_of_block];

    // int result;

    // read_inode_bitmap(buffer, 0);
    // for (uint32 i = 0; i < 40; i++)

    // {

    //     result = bitmapGet(buffer, i);
    //     printf("Inode[%d]: %d\n", i, result);
    // }

    printf("next inode available: %d\n", get_free_inode(0));

    // uint32 indirect_blocks = indirect_blocks_count;
    // uint32 d_indirect_blocks = indirect_blocks * indirect_blocks;
    // uint32 t_indirect_blocks = d_indirect_blocks * indirect_blocks;

    printf("sizeof struct s_inode %lu\n", sizeof(struct s_inode));
    printf("sizeof es.inode %lu\n", es.s_inode_size);

    printf("Direct blocks %d\n", EXT2_NDIR_BLOCKS);
    printf("Indirect blocks %lu\n", indirect_blocks_count);
    printf("D_Indirect blocks %lu\n", d_indirect_blocks_count);
    printf("T_Indirect blocks %lu\n", t_indirect_blocks_count);
}

void* fusx_init(struct fuse_conn_info *conn)
{
    read_sb();
    read_group_descriptors();

    test();
    printf("GOT HERE!!!");
    return 0;
    //printf("\n\n");
}

int fusx_get_attr(const char *path, struct stat *statbuf)
{

    if (print_info){
        //printf("getattr %s\n", path);
    }
    memset(statbuf, 0, sizeof(struct stat));

    uint32 len = strlen(path);
    char path_copy[len + 1];
    strncpy(path_copy, path, len);
    path_copy[len] = 0;

    uint32 entry_inode = lookup_entry_inode(path_copy, ROOT_INO);
    if (entry_inode == 0)
    {
        //printf("entry_inode not found for %s\n", path);
        return -ENOENT;
    }

    struct s_inode *dir_inode = read_inode(entry_inode);

    statbuf->st_mode = dir_inode->i_mode;
    statbuf->st_nlink = dir_inode->i_links_count;
    statbuf->st_size = dir_inode->i_size;

    statbuf->st_uid = dir_inode->i_uid;
    statbuf->st_gid = dir_inode->i_gid;

    statbuf->st_blocks = dir_inode->i_blocks;

    if (print_info){
        //printf("success\n");
    }
    free(dir_inode);
    return 0;
}

int fusx_read_dir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo)
{

    if (print_info){
        //printf("read dir %s\n", path);
    }

    struct s_dir_entry2 *entry;

    struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;

    uint32 entry_inode = fh->f_inode;

    unsigned char block[size_of_block];
    struct s_inode *dir_inode = read_inode(entry_inode);

    uint32 current_block = 0;

    if (print_info){
        //printf("rd inode_direct[0] %lu\n", dir_inode->i_direct[0]);
    }
    if (read_inode_logic_block(block, *dir_inode, current_block++) != 0)
        return -ENOENT;

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

        if (entry->inode == 0)
        {
            //printf("rd-> error reading block\n");
            return -1;
        }

        if (print_info){
            //printf("rd-> \t%u %s\n", entry->inode, file_name);
        }
        filler(buf, file_name, NULL, 0);

        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }
    if (print_info){
        //printf("%u of %u\n", dir_inode->i_size, c_size);
    }
    // free(dir_inode);
    return 0;
}

int fusx_opendir(const char *path, struct fuse_file_info *fileInfo)
{

    if (print_info){
        //printf("opendir %s\n", path);
    }
    uint32 len = strlen(path);

    if ((len != 1) && path[0] != '/')
    {
        //printf("No permitido \n");
        return -EPERM;
    }

    char path_copy[len + 1];
    strncpy(path_copy, path, len);
    path_copy[len] = 0;
    uint32 entry_inode = lookup_entry_inode(path_copy, ROOT_INO);
    if (entry_inode > 0)
    {
        struct s_inode *dir_inode = read_inode(entry_inode);
        if (is_dir(dir_inode->i_mode))
        {
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
            //printf("No es directorio\n");
            free(dir_inode);
            return -EPERM;
        }
    }
    else
    {
        //printf("entry_inode not found for %s\n", path);
        return -ENOENT;
    }

    return -ENOENT;
}

int fusx_statfs(const char *path, struct statvfs *statInfo)
{

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

int fusx_open(const char *path, struct fuse_file_info *fileInfo)
{
    if (print_info){
        //printf("open %s\n", path);
    }
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
            struct s_inode *dir_inode = read_inode(entry_inode);
            if (is_file(dir_inode->i_mode))
            {

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

int fusx_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    if (print_info){
        //printf("read %s size %u offset %u\n", path, size, offset);
    }
    int bytes_to_read = size;

    struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;

    if (offset >= fh->f_size)
        return 0;

    if (offset + size > fh->f_size)
        bytes_to_read = fh->f_size - offset;

    uint32 logic_block_number = offset / size_of_block;
    uint32 number_of_blocks = bytes_to_read / size_of_block;
    if(number_of_blocks * size_of_block < size)
        number_of_blocks++;
    struct s_inode *inode = read_inode(fh->f_inode);

    for (unsigned int i = 0; i < number_of_blocks; i++)
    {
        char buff[size_of_block];
        read_inode_logic_block(buff, *inode, logic_block_number + i);

        // //printf("read {%s}\n", buff);

        memcpy(&buf[i * size_of_block], buff, size_of_block);
    }

    return bytes_to_read;
}

int fusx_release(const char *path, struct fuse_file_info *fileInfo)
{

    free((void *)fileInfo->fh);
    return 0;
}

int fusx_releasedir(const char *path, struct fuse_file_info *fileInfo)
{

    free((void *)fileInfo->fh);
    return 0;
}

int fusx_mkdir(const char *path, mode_t mode)
{

    uint32 len = strlen(path);
    char parent[len];
    char dir_name[len];

    parseNewEntry(path, parent, dir_name);

    // uint32 dir_name_len = strlen(dir_name);

    //printf("mkdir %s parent_dir %s mode %x\n", dir_name, parent, mode);

    uint32 parent_inode_number = lookup_entry_inode(parent, ROOT_INO);
    //printf("Parent number %d\n", parent_inode_number);

    struct s_inode *parent_inode = read_inode(parent_inode_number);

    // new inode to new dir
    int parent_inode_group, parent_inode_index;
    locate(parent_inode_number, es.s_inodes_per_group, &parent_inode_group, &parent_inode_index);

    long int new_inode = get_free_inode(parent_inode_group);
    //printf("Nuevo inode devuelto %d del grupo %d\n", new_inode, parent_inode_group);
    if (new_inode < 0)
        return -ENOENT;
    // ----

    struct s_inode s_new_inode;
    s_new_inode.i_mode = 0x4000;
    s_new_inode.i_uid = parent_inode->i_uid;
    s_new_inode.i_size = 0;
    s_new_inode.i_gid = parent_inode->i_gid;
    s_new_inode.i_links_count = 1;
    s_new_inode.i_blocks = n_512k_blocks_per_block;
    s_new_inode.i_flags = 0;
    for(int i=0; i<EXT2_NDIR_BLOCKS;i++)
        s_new_inode.i_direct[i]=0;
    s_new_inode.i_indirect = 0;
    s_new_inode.i_d_indirect = 0;
    s_new_inode.i_t_indirect = 0;

    /*int result = */add_entry(parent_inode, parent_inode_number, new_inode, dir_name, ENTRY_DIR);
    //printf("result: %d\n", result);

    //set new inode and first block as used in bitmap and save inode
    inode_bitmap_set(new_inode, 1);
    // block_bitmap_set(first_block,1);
    save_inode(s_new_inode,new_inode);

    // get new_inode_group to increment counter of inodes
    int new_inode_group, new_inode_index;
    locate(new_inode, es.s_inodes_per_group, &new_inode_group, &new_inode_index);

    groups_table[new_inode_group].bg_used_dirs_count++;

    char new_entry_name[ENTRY_BASE_SIZE];
    bzero(new_entry_name,ENTRY_BASE_SIZE);
    //dot entry for new dir
    strcpy(new_entry_name, ".");
    /*result = */add_entry(&s_new_inode, new_inode, new_inode, new_entry_name, ENTRY_DIR);
    //printf("result for dot (.): %d\n", result);
    //save_inode(s_new_inode,new_inode);

    //dot dot entry for new dir
    strcpy(new_entry_name, "..");
    /*result = */add_entry(&s_new_inode, new_inode, parent_inode_number, new_entry_name, ENTRY_DIR);
    //printf("result for dot (..): %d\n", result);
    //save_inode(s_new_inode,new_inode);
    save_meta_data();

    free(parent_inode);

    return 0;
}

int fusx_truncate(const char *path, off_t newSize)
{
    char *path_child = (char *)malloc(strlen(path) + 1);
    bzero(path_child, strlen(path) + 1);
    memcpy(path_child, path, strlen(path));
    // if (print_info)
    //     //printf("truncate %s newsize %lu\n", path_child, newSize);

    uint32 inode_index = lookup_entry_inode(path_child, ROOT_INO);
    // if (print_info)
    //     //printf("inode_index: %d\n", inode_index);
    struct s_inode *child_inode = read_inode(inode_index);

    // if (print_info)
    //     //printf("Llego hasta aqui\n");

    if (child_inode->i_size == newSize)
        return 0;

    if (child_inode->i_size < newSize)
        return -ENOENT;

    if (inode_index < 11)
    {
        //printf("inode not found \n");
        return -ENOENT;
    }

    // if (print_info)
    //     //printf("Llego hasta aqui 2\n");
    uint32 inode_blocks = child_inode->i_size / size_of_block;
    if (inode_blocks * size_of_block < child_inode->i_size)
        inode_blocks += 1;
    // uint32 new_block_size = newSize / size_of_block;
    // if (print_info)
        //printf("inode_blocks: %d\n", inode_blocks);

    if (newSize == 0)
    {
        for (uint32 i = 0; i < inode_blocks; ++i){
            //printf("free logic block %lu of %lu\n", i, inode_blocks);
            free_logic_block(child_inode, i);
        }

        if (print_info){
            //printf("free data blocks\n");
        }
        free_indirect_blocks(child_inode);
        if (print_info){
            //printf("free indirect blocks\n");
        }
        child_inode->i_indirect = 0;
        child_inode->i_d_indirect = 0;
        child_inode->i_t_indirect = 0;
        child_inode->i_size = 0;
        child_inode->i_blocks = 0;
    }

    save_inode(*child_inode, inode_index);
    if (print_info){
        //printf("Saved inode\n");
    }
    save_meta_data();
    if (print_info){
        //printf("Saved meta data\n");
    }
    free(path_child);
    free(child_inode);
    return 0;
}

int fusx_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    if (print_info){
        //printf("write %s size %lu offset %lu\n", path, size, offset);
    }
    unsigned int bytes_to_write = size;

    struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;

    if (print_info){
        //printf("write logic block 1\n");
    }
    uint32 logic_block_number = offset / size_of_block;
    uint32 number_of_blocks = size / size_of_block;
    if(number_of_blocks * size_of_block < size)
        number_of_blocks++;
    struct s_inode *inode = read_inode(fh->f_inode);

    for (unsigned int i = 0; i < number_of_blocks; i++)
    {

        // if (print_info)
            // //printf("\twrite logic block %d\n", logic_block_number + i);

        // bzero(buffer, size_of_block);
        char buffer[size_of_block];
        memcpy(buffer, &buf[i*size_of_block], size_of_block);
        // //printf("buffer to write {%s}\n", buffer);
        write_inode_logic_block(buffer, inode, logic_block_number + i, fh->f_inode);

        if (print_info){
            //printf("wrote\n", logic_block_number + i);
        }
    }

    if (offset + size > inode->i_size){
        uint32 n512_blocks = bytes_to_write / 512;
        if(n512_blocks*512 < bytes_to_write)
            n512_blocks++;
        inode->i_blocks += n512_blocks;
    }

    inode->i_size += bytes_to_write;

    save_inode(*inode, fh->f_inode);
    // print_inode_blocks(inode);
    free(inode);

    return bytes_to_write;
}

int fusx_rename(const char *path, const char *newpath)
{
    //printf("\n\nrename %s newPath %s\n", path, newpath);

    uint32 len = strlen(path);
    char parent[len];
    char old_entry_name[len];
    char new_entry_name[len];

    parseNewEntry(path, parent, old_entry_name);
    parseNewEntry(newpath, parent, new_entry_name);

    uint32 parent_inode_number = lookup_entry_inode(parent, ROOT_INO);
    //printf("Parent number %d\n", parent_inode_number);

    struct s_inode *parent_inode = read_inode(parent_inode_number);

    struct s_dir_entry2* old_entry = find_entry(*parent_inode, old_entry_name);
    struct s_inode* old_entry_inode = read_inode(old_entry->inode);

    remove_entry(parent_inode, parent_inode_number, old_entry_name);
    add_entry(parent_inode, parent_inode_number, old_entry->inode, new_entry_name, old_entry->file_type);

    save_inode(*parent_inode, parent_inode_number);
    save_meta_data();

    free(old_entry_inode);
    free(old_entry);
    free(parent_inode);
    return 0;
}

int fusx_rmdir(const char *path)
{
    //printf("rmdir %s\n", path);
    char path_copy[strlen(path)+1];
    strncpy(path_copy, path, strlen(path));
    path_copy[strlen(path)] = 0;
    uint32 parent_inode_number = lookup_entry_inode(path_copy,ROOT_INO);
    struct s_inode* parent_inode = read_inode(parent_inode_number);
    struct s_dir_entry2* last_entry = find_last_entry(*parent_inode);
    if(strcmp(last_entry->name, "..") == 0){
        fusx_unlink(path);
        return 0;
    }else
        return -EPERM;
}

int fusx_unlink(const char *path)
{
    //printf("unlink %s\n", path);
    //truncating inode
    fusx_truncate(path, 0);

    //free inode
    uint32 len = strlen(path);
    char parent[len];
    char child_name[len];
    parseNewEntry(path, parent, child_name);
    uint32 parent_inode_number = lookup_entry_inode(parent,ROOT_INO);
    //printf("Parent number %d\n", parent_inode_number);
    struct s_inode *parent_inode = read_inode(parent_inode_number);

    struct s_dir_entry2* entry_child = find_entry(*parent_inode,child_name);
    uint32 inode_index = entry_child->inode;
    //locate()
    remove_entry(parent_inode, parent_inode_number, child_name);
    inode_bitmap_set(inode_index,0);


    save_inode(*parent_inode, parent_inode_number);
    save_meta_data();
    //removing entry in parent inode
    free(parent_inode);
    free(entry_child);
    return 0;
}

int fusx_mknod(const char *path, mode_t mode, dev_t dev)
{
    //printf("mknod %s\n", path);
    return 0;
}

int fusx_create(const char *path, mode_t mode, struct fuse_file_info *fileInfo)
{
    //printf("create %s mode %x\n", path, mode);
    char parent_name[strlen(path)];
    char child_name[strlen(path)];
    parseNewEntry(path,parent_name,child_name);
    //printf("padre :%s hijo:%s\n",parent_name,child_name);
    uint32 parent_inode_number = lookup_entry_inode(parent_name, ROOT_INO);
    //printf("parent inode number %d\n", parent_inode_number);
    struct s_inode *inode = read_inode(parent_inode_number);
    int parent_inode_group, parent_inode_index;
    locate(parent_inode_number,es.s_inodes_per_group, &parent_inode_group,&parent_inode_index);
    long int new_inode = get_free_inode(parent_inode_group);
    //printf("Nuevo inode devuelto %d del grupo %d\n", new_inode, parent_inode_group);
    if (new_inode < 0)
        return -ENOENT;

    struct s_inode s_new_inode;
    s_new_inode.i_mode = mode;
    s_new_inode.i_uid = inode->i_uid;
    s_new_inode.i_size = 0;
    s_new_inode.i_gid = inode->i_gid;
    s_new_inode.i_links_count = 1;
    s_new_inode.i_blocks = 0;
    s_new_inode.i_flags = 0;
    for(unsigned int i=0; i<EXT2_NDIR_BLOCKS;i++)
        s_new_inode.i_direct[i]=0;
    s_new_inode.i_indirect = 0;
    s_new_inode.i_d_indirect = 0;
    s_new_inode.i_t_indirect = 0;

    struct s_file_handle *fh = (s_file_handle *)malloc(sizeof(s_file_handle));
    fh->f_inode = new_inode;
    fh->f_size = 0;
    fh->f_blocks_count = 0;
    fileInfo->fh = (uint64_t)fh;

    /*int result = */add_entry(inode, parent_inode_number, new_inode, child_name, ENTRY_FILE);
    //printf("result: %d\n", result);

    //set new inode and first block as used in bitmap and save inode
    inode_bitmap_set(new_inode, 1);
    //printf("result1: %d\n", result);
    // block_bitmap_set(first_block,1);
    save_inode(s_new_inode,new_inode);
    //printf("result2: %d\n", result);
    save_meta_data();
    //printf("result3: %d\n", result);
    free(inode);
    //int result = add_entry(*inode, parent_inode_number, child_name, mode, ENTRY_FILE);
    ////printf("result: %d\n", result);
    ////printf("fh index %lu\n",fileInfo->fh);
    //struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;
    ////printf("fh->inode %lu\n",fh->f_inode );
    return 0;
}

int fusx_utime(const char * path, struct utimbuf *times){
    //printf("utime %s\n", path);
    return 0;
}
