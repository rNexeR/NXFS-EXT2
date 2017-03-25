#include "nxt2fs.h"

/*METADATA*/
void read_group_descriptors()
{

    number_of_groups = 1 + (es.s_blocks_count - 1) / es.s_blocks_per_group;
    if (print_info)
    {
        printf("Number of groups: %d\n", number_of_groups);
    }

    groups_table = (struct s_block_group_descriptor *)malloc(sizeof(struct s_block_group_descriptor) * number_of_groups);
    Last_full_block_galaxy = (int *)malloc(sizeof(int)*number_of_groups);
    for(int i=0;i<number_of_groups;i++)
        Last_full_block_galaxy[i]=0;

    uint32 offset = size_of_block;
     if (size_of_block == 1024)
         offset += 1024;

    device_seek(offset);
    uint32 size_of_group_descriptor = sizeof(struct s_block_group_descriptor) * number_of_groups;
    printf("size of gd %u and list %u\n",sizeof(struct s_block_group_descriptor),size_of_group_descriptor);
    int result_read = device_read(groups_table, size_of_group_descriptor);
    printf("result %d\n",result_read );
    for (uint32 i = 0; i < number_of_groups; ++i)
    {

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

    n_512k_blocks_per_block = size_of_block / 512;

    return es.s_magic == 61267 ? 0 : -1;
}

int save_meta_data()
{

    device_seek(1024);
    device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
    //if (print_info)
        printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", 0, 1024, 0);

    uint32 offset_group_descriptor = size_of_block;
    if(size_of_block == 1024)
      offset_group_descriptor+=1024;

    uint32 size_of_group_descriptor = sizeof(struct s_block_group_descriptor) * number_of_groups;
    //long int var = 1024 + size_of_block;
    printf("number of groups: %d\n",number_of_groups );
  //  printf("offset %d - %lu\n", var, (unsigned)var);
    device_seek(offset_group_descriptor);
    printf("Paso \n");

    device_write(groups_table, size_of_group_descriptor);
    //if (print_info)
        printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", 0, 1024 + size_of_block + offset_group_descriptor, 1);

    uint32 offset = (1 * es.s_blocks_per_group) * size_of_block;
    device_seek(offset);
    device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
    //if (print_info)
        printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", 1, 1024 + offset, es.s_blocks_per_group);

    device_seek(offset + size_of_block);
    device_write((void*)groups_table, size_of_group_descriptor);
    //if (print_info)
        printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", 1, 1024 + offset + size_of_block, es.s_blocks_per_group + 1);

    for (int i = 3; i < number_of_groups; i *= 3)
    {
        uint32 block = (i * es.s_blocks_per_group);
        offset = block * size_of_block;
        device_seek(offset);
        device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
        //if (print_info)
            printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", i, offset, block);
        device_seek(offset + size_of_block);
        device_write((void*)groups_table, size_of_group_descriptor);
        //if (print_info)
            printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", i, offset + size_of_block, block + 1);
    }

    for (int i = 5; i < number_of_groups; i *= 5)
    {
        uint32 block = (i * es.s_blocks_per_group);
        offset = block * size_of_block;
        device_seek(offset);
        device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
        //if (print_info)
            printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", i, offset, block);
        device_seek(offset + size_of_block);
        device_write((void*)groups_table, size_of_group_descriptor);
        //if (print_info)
            printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", i, offset + size_of_block, block + 1);
    }

    for (int i = 7; i < number_of_groups; i *= 7)
    {
        uint32 block = (i * es.s_blocks_per_group);
        offset = block * size_of_block;
        device_seek(offset);
        device_write(&es, SUPERBLOCK_SIZE_TO_SAVE);
        //if (print_info)
            printf("Saved s_superblock: %d, offset bytes: %d, block: %d\n", i, offset, block);
        device_seek(offset + size_of_block);
        device_write((void*)groups_table, size_of_group_descriptor);
        //if (print_info)
            printf("Saved groups_table: %d, offset bytes: %d, block: %d\n", i, offset + size_of_block, block + 1);
    }

    return 0;
}

/*READ AND WRITE LOGICAL BLOCK*/
void read_t_indirect_block(void *buffer, uint32 t_indirect_block, uint32 logic_position)
{
    uint32 t_indirect_blocks[indirect_blocks_count];
    read_block(t_indirect_blocks, t_indirect_block, size_of_block);

    uint32 position = logic_position / (indirect_blocks_count);
    uint32 offset = logic_position - position * d_indirect_blocks_count;

    if (print_info)
        printf("reading t_indirect block number: %lu logic pos %lu position %lu offset %lu\n", t_indirect_blocks[position], logic_position, position, offset);

    read_d_indirect_block(buffer, t_indirect_blocks[position], offset);
}

void read_d_indirect_block(void *buffer, uint32 d_indirect_block, uint32 logic_position)
{

    uint32 d_indirect_blocks[indirect_blocks_count];
    read_block(d_indirect_blocks, d_indirect_block, size_of_block);

    uint32 position = logic_position / (indirect_blocks_count);
    uint32 offset = logic_position - position * indirect_blocks_count;

    if (print_info)
        printf("reading d_indirect block number: %lu logic pos %lu position %lu offset %lu\n", d_indirect_blocks[position], logic_position, position, offset);

    read_indirect_block(buffer, d_indirect_blocks[position], offset);
}

void read_indirect_block(void *buffer, uint32 indirect_block, uint32 logic_position)
{
    uint32 indirect_blocks[indirect_blocks_count];
    read_block(indirect_blocks, indirect_block, size_of_block);

    if (print_info)
        printf("reading indirect block number: %lu logic position %lu\n", indirect_blocks[logic_position], logic_position);
    read_block(buffer, indirect_blocks[logic_position], size_of_block);
}

int read_inode_logic_block(void *buffer, struct s_inode inode, uint32 logic_block_number)
{
    if (logic_block_number < EXT2_NDIR_BLOCKS)
    {
        if (inode.i_direct[logic_block_number] >= es.s_firts_inode)
            read_block(buffer, inode.i_direct[logic_block_number], size_of_block);
        else
            return -1;
    }
    else
    {
        logic_block_number -= EXT2_NDIR_BLOCKS;
        if (logic_block_number >= d_indirect_blocks_count && inode.i_t_indirect >= es.s_firts_inode)
            read_t_indirect_block(buffer, inode.i_t_indirect, logic_block_number - d_indirect_blocks_count);
        else if (logic_block_number >= indirect_blocks_count && inode.i_d_indirect >= es.s_firts_inode)
            read_d_indirect_block(buffer, inode.i_d_indirect, logic_block_number - indirect_blocks_count);
        else if (inode.i_indirect >= es.s_firts_inode)
            read_indirect_block(buffer, inode.i_indirect, logic_block_number);
        else
        {
            return -1;
        }
    }
    return 0;
}

void write_t_indirect_block(void *buffer, uint32 t_indirect_block, uint32 logic_position, uint32 inode_number)
{
    uint32 t_indirect_blocks[indirect_blocks_count];
    read_block(t_indirect_blocks, t_indirect_block, size_of_block);

    uint32 position = logic_position / (indirect_blocks_count);
    uint32 offset = logic_position - position * d_indirect_blocks_count;

    if (print_info)
        printf("writing t_indirect block number: %lu logic pos %lu position %lu offset %lu\n", t_indirect_blocks[position], logic_position, position, offset);

    if (t_indirect_blocks[position] == 0)
    {
        int group, index;
        locate(inode_number, es.s_inodes_per_group, &group, &index);
        uint32 new_block = get_free_block(group);
        if (new_block < 0)
        {
            if (print_info)
                printf("Error getting new block\n");
            return;
        }

        t_indirect_blocks[position] = new_block;
        write_block(t_indirect_blocks, t_indirect_block, size_of_block);
        block_bitmap_set(new_block, 1);
    }
    // printf("writing t_indirect block number: %lu logic pos %lu position %lu offset %lu\n", t_indirect_blocks[position], logic_position, position, offset);
    write_d_indirect_block(buffer, t_indirect_blocks[position], offset, inode_number);
}

void write_d_indirect_block(void *buffer, uint32 d_indirect_block, uint32 logic_position, uint32 inode_number)
{
    uint32 d_indirect_blocks[indirect_blocks_count];
    read_block(d_indirect_blocks, d_indirect_block, size_of_block);

    uint32 position = logic_position / (indirect_blocks_count);
    uint32 offset = logic_position - position * indirect_blocks_count;

    if (print_info)
        printf("writing d_indirect block number: %lu logic pos %lu position %lu offset %lu\n", d_indirect_blocks[position], logic_position, position, offset);

    if (d_indirect_blocks[position] == 0)
    {
        int group, index;
        locate(inode_number, es.s_inodes_per_group, &group, &index);
        uint32 new_block = get_free_block(group);
        if (new_block < 0)
        {
            if (print_info)
                printf("Error getting new block\n");
            return;
        }

        d_indirect_blocks[position] = new_block;
        write_block(d_indirect_blocks, d_indirect_block, size_of_block);
        block_bitmap_set(new_block, 1);
    }
    // printf("writing d_indirect block number: %lu logic pos %lu position %lu offset %lu\n", d_indirect_blocks[position], logic_position, position, offset);
    write_indirect_block(buffer, d_indirect_blocks[position], offset, inode_number);
}

void write_indirect_block(void *buffer, uint32 indirect_block, uint32 logic_position, uint32 inode_number)
{
    uint32 indirect_blocks[indirect_blocks_count];
    read_block(indirect_blocks, indirect_block, size_of_block);

    if (print_info)
        printf("writing indirect block number: %lu logic position %lu\n", indirect_blocks[logic_position], logic_position);

    if (indirect_blocks[logic_position] == 0)
    {
        int group, index;
        locate(inode_number, es.s_inodes_per_group, &group, &index);
        uint32 new_block = get_free_block(group);
        if (new_block < 0)
        {
            if (print_info)
                printf("Error getting new block\n");
            return;
        }

        indirect_blocks[logic_position] = new_block;
        write_block(indirect_blocks, indirect_block, size_of_block);
        block_bitmap_set(new_block, 1);
    }
    // printf("writing indirect block number: %lu logic position %lu\n", indirect_blocks[logic_position], logic_position);
    write_block(buffer, indirect_blocks[logic_position], size_of_block);
}

void write_inode_logic_block(void *buffer, struct s_inode* inode, uint32 logic_block_number, uint32 inode_number)
{
    int group, index;
    locate(inode_number, es.s_inodes_per_group, &group, &index);
    uint32 new_block = get_free_block(group);
    if (new_block < 0)
    {
        if (print_info)
            printf("Error getting new block\n");
        return;
    }

    if (logic_block_number < EXT2_NDIR_BLOCKS)
    {
        if (inode->i_direct[logic_block_number] == 0)
        {
            inode->i_direct[logic_block_number] = new_block;
            block_bitmap_set(new_block, 1);
        }
        write_block(buffer, inode->i_direct[logic_block_number], size_of_block);
    }
    else
    {
        logic_block_number -= EXT2_NDIR_BLOCKS;
        if (logic_block_number >= d_indirect_blocks_count)
        {
            if (inode->i_t_indirect == 0)
            {
                inode->i_t_indirect = new_block;
                block_bitmap_set(new_block, 1);
            }
            write_t_indirect_block(buffer, inode->i_t_indirect, logic_block_number - d_indirect_blocks_count, inode_number);
        }
        else if (logic_block_number >= indirect_blocks_count)
        {
            if (inode->i_d_indirect == 0)
            {
                inode->i_d_indirect = new_block;
                block_bitmap_set(new_block, 1);
            }
            write_d_indirect_block(buffer, inode->i_d_indirect, logic_block_number - indirect_blocks_count, inode_number);
        }
        else
        {
            if (inode->i_indirect == 0)
            {
                inode->i_indirect = new_block;
                block_bitmap_set(new_block, 1);
            }
            write_indirect_block(buffer, inode->i_indirect, logic_block_number, inode_number);
        }
    }
    save_inode(*inode, inode_number);
}

/*INODES*/
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
    {

        bitmapSet(buffer, inode_index);
    }
    else
        bitmapReset(buffer, inode_index);

    if (state)
    {
        es.s_free_inodes_count--;
        groups_table[inode_group].bg_free_inodes_count--;
    }
    else
    {
        es.s_free_inodes_count++;
        groups_table[inode_group].bg_free_inodes_count++;
    }

    write_block(buffer, groups_table[inode_group].bg_inode_bitmap, size_of_block);
    device_flush();
}

struct s_inode *read_inode(uint32 inode_number)
{
    int inode_group, inode_index;
    locate(inode_number, es.s_inodes_per_group, &inode_group, &inode_index);

    struct s_inode *inode = (struct s_inode *)malloc(sizeof(struct s_inode));
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
    int start = 0;
    if (group_number == 0)
        start = 12;
    int result;
    for (uint32 i = start; i < es.s_inodes_per_group; i++)
    {
        result = bitmapGet(buffer, i);
        if (!result)
            return i;
    }
    return -1;
}

uint32 get_free_inode(uint32 group_number)
{
    int new_inode = get_free_inode_in_group(group_number++);
    while (new_inode < 0 && group_number < number_of_groups)
    {
        new_inode = get_free_inode_in_group(group_number++);
    }
    return new_inode >= 0 ? ((group_number - 1) * es.s_inodes_per_group) + (uint32)new_inode + 1 : new_inode;
}

int save_inode(struct s_inode inode, uint32 index)
{
    int inode_group, inode_index;
    locate(index, es.s_inodes_per_group, &inode_group, &inode_index);
    char inode_buff[es.s_inode_size];
    memcpy(inode_buff, (void *)&inode, es.s_inode_size);
    int offset = es.s_inode_size * inode_index;
    uint32 base_pointer = groups_table[inode_group].bg_inode_table * size_of_block;

    device_seek(base_pointer + offset);
    return device_write(inode_buff, es.s_inode_size);
}

/*BLOCKS*/
void read_block_bitmap(void *buffer, int group_number)
{
    int block_number = groups_table[group_number].bg_block_bitmap;
    read_block(buffer, block_number, size_of_block);
}

void block_bitmap_set(uint32 inode_number, uint8 state)
{
    int block_group, block_index;
    locate(inode_number, es.s_blocks_per_group, &block_group, &block_index);

    if (print_info){
        printf("----------------------------------------------------\n");
        printf("block bitmap set %lu %u\n", inode_number, state);
        printf("before bb set, first av. block %lu\n", get_free_block(block_group));
        printf("block bitmap set %lu %u g: %d off: %d\n", inode_number, state, block_group, block_index);
    }

    byte buffer[size_of_block];
    read_block_bitmap(buffer, block_group);
    if (state)
        bitmapSet(buffer, block_index);
    else
        bitmapReset(buffer, block_index);

    if (state)
    {
        es.s_free_block_count--;
        groups_table[block_group].bg_free_blocks_count--;
    }
    else
    {
        char buffer2[size_of_block];
        bzero(buffer2, size_of_block);
        write_block(buffer2, inode_number, size_of_block);
        es.s_free_block_count++;
        groups_table[block_group].bg_free_blocks_count++;
    }

    write_block(buffer, groups_table[block_group].bg_block_bitmap, size_of_block);
    if (print_info)
    {
        printf("after bb set, first av. block %lu\n", get_free_block(block_group));
        printf("----------------------------------------------------\n");
    }
}

int get_free_block_in_group(uint32 group_number)
{
    byte buffer[size_of_block];
    read_block_bitmap(buffer, group_number);

    for (int i = Last_full_block_galaxy[group_number]; i < es.s_blocks_per_group; i++)
    {
        unsigned char galaxy = buffer[i/BIT];
        bool isFull = (galaxy^0xFF)==0;
        if (print_info){
            printf("i: %d, group_number: %d, isFull: %d\n",i,group_number, isFull);
            printf("Last_full_block_galaxy[group_number]: %d\n", Last_full_block_galaxy[group_number]);
        }
        if (isFull){
            i+=BIT-1;
            Last_full_block_galaxy[group_number]=i;
            continue;
        }else{
            if (print_info)
                printf("galaxy: %d\n", galaxy);
            if (bitmapGet(buffer, i) == 0)
                return i;
        }
    }
    return -1;
}

uint32 get_free_block(uint32 group_number)
{
    if (group_number<last_groupblock)group_number=last_groupblock;
    int new_block = get_free_block_in_group(group_number++);
    while (new_block < 0 && group_number <= number_of_groups)
    {
        new_block = get_free_block_in_group(group_number++);
    }
    return new_block >= 0 ? (group_number - 1) * es.s_blocks_per_group + (uint32)new_block + 1 : new_block;
    ;
}

void free_i_logic_block(uint32 i_indirect_block, uint32 logic_position)
{
    uint32 i_indirect_blocks[indirect_blocks_count];
    read_block(i_indirect_blocks, i_indirect_block, size_of_block);

    if (i_indirect_blocks[logic_position] != 0)
    {
        if (print_info)
            printf("free data Block: %d\n", i_indirect_blocks[logic_position]);
        block_bitmap_set(i_indirect_blocks[logic_position], 0);
    }
}

void free_d_logic_block(uint32 d_indirect_block, uint32 logic_position)
{
    uint32 d_indirect_blocks[indirect_blocks_count];
    read_block(d_indirect_blocks, d_indirect_block, size_of_block);

    uint32 position = logic_position / (indirect_blocks_count);
    uint32 offset = logic_position - position * d_indirect_blocks_count;

    free_i_logic_block(d_indirect_blocks[position], offset);
}

void free_t_logic_block(uint32 t_indirect_block, uint32 logic_position)
{
    uint32 t_indirect_blocks[indirect_blocks_count];
    read_block(t_indirect_blocks, t_indirect_block, size_of_block);

    uint32 position = logic_position / (indirect_blocks_count);
    uint32 offset = logic_position - position * d_indirect_blocks_count;

    free_d_logic_block(t_indirect_blocks[position], offset);
}

void free_logic_block(struct s_inode *child_inode, uint32 logic_block_number)
{
    if (logic_block_number < EXT2_NDIR_BLOCKS)
    {
        uint32 physical_data_block = child_inode->i_direct[logic_block_number];
        if (physical_data_block != 0)
        {
            if (print_info)
                printf("free data Block: %d\n", physical_data_block);
            block_bitmap_set(physical_data_block, 0);
            child_inode->i_direct[logic_block_number] = 0;
        }
    }
    else
    {
        logic_block_number -= EXT2_NDIR_BLOCKS;
        if (logic_block_number >= d_indirect_blocks_count)
            free_t_logic_block(child_inode->i_t_indirect, logic_block_number - d_indirect_blocks_count);
        else if (logic_block_number >= indirect_blocks_count)
            free_d_logic_block(child_inode->i_d_indirect, logic_block_number - indirect_blocks_count);
        else
            free_i_logic_block(child_inode->i_indirect, logic_block_number);
    }
}

void free_indirect_blocks(struct s_inode *child_inode)
{
    if (child_inode->i_indirect != 0)
        block_bitmap_set(child_inode->i_indirect, 0);

    uint32 cant_indirects = indirect_blocks_count;
    uint32 d_indirect_blocks[cant_indirects];
    if (child_inode->i_d_indirect != 0)
    {
        read_block(d_indirect_blocks, child_inode->i_d_indirect, size_of_block);

        for (uint32 i = 0; i < cant_indirects; ++i)
        {
            if (d_indirect_blocks[i] != 0)
            {
                // if (print_info)
                    printf("free indirect Block: %d\n", d_indirect_blocks[i]);
                block_bitmap_set(d_indirect_blocks[i], 0);
            }
        }

        if (print_info)
            printf("free d indirect Block: %d\n", child_inode->i_d_indirect);
        block_bitmap_set(child_inode->i_d_indirect, 0);
    }

    if (child_inode->i_t_indirect != 0)
    {
        uint32 t_indirect_blocks[cant_indirects];
        read_block(t_indirect_blocks, child_inode->i_t_indirect, size_of_block);

        for (uint32 i = 0; i < cant_indirects; ++i)
        {
            if (t_indirect_blocks[i] != 0)
            {
                read_block(d_indirect_blocks, t_indirect_blocks[i], size_of_block);
                for (uint32 j = 0; j < cant_indirects; ++j)
                {
                    if (d_indirect_blocks[j] != 0)
                    {
                        // if (print_info)
                            printf("free indirect Block: %d\n", d_indirect_blocks[j]);
                        block_bitmap_set(d_indirect_blocks[j], 0);
                    }
                }

                // if (print_info)
                    printf("free d indirect Block: %d\n", t_indirect_blocks[i]);
                block_bitmap_set(t_indirect_blocks[i], 0);
            }
        }
        // if (print_info)
            printf("free t indirect Block: %d\n", child_inode->i_t_indirect);
        block_bitmap_set(child_inode->i_t_indirect, 0);
    }
}

/*ENTRIES*/
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

        struct s_inode *current_inode = read_inode(current_inode_number);
        entry = find_entry(*current_inode, sub_path);
        free(current_inode);
        if (entry == NULL)
            return 0;
        if (hasChild)
            return lookup_entry_inode(path + dashPos, entry->inode);
        uint32 inode = entry->inode;
        free(entry);
        return inode;
    }

    return 0;
}

struct s_dir_entry2 *find_last_entry(struct s_inode inode)
{
    if (print_info)
        printf("Entro last entry\n");
    struct s_dir_entry2 *entry;
    unsigned char block[size_of_block];

    uint32 current_block = 0;

    int error = read_inode_logic_block(block, inode, current_block++);

    uint16 c_size = 0;

    entry = (struct s_dir_entry2 *)block;

    if (error<0)
        return NULL;

    while (c_size < inode.i_size)
    {

        if (c_size > 0 && c_size % size_of_block == 0){
            read_inode_logic_block(block, inode, current_block++);
            entry = (struct s_dir_entry2 *)block;
        }

        char file_name[EXT2_NAME_LEN + 1];

        memcpy(file_name, entry->name, entry->name_len);

        file_name[entry->name_len] = 0; /* append null char to the file name */

        if (print_info)
            printf("%lu %s %lu\n", entry->inode, file_name, c_size);

        c_size += entry->rec_len;

        if (c_size >= inode.i_size)
        {
            c_size -= entry->rec_len;
            break;
        }

        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }

    struct s_dir_entry2 *ret = (struct s_dir_entry2 *)malloc(sizeof(struct s_dir_entry2));
    ret->inode = entry->inode;
    ret->rec_len = entry->rec_len;
    ret->name_len = entry->name_len;
    ret->file_type = entry->file_type;
    strncpy(ret->name, entry->name, entry->name_len);
    ret->block_number = current_block - 1;
    ret->offset = c_size % size_of_block;
    if (print_info)
        printf("Salio last entry con %s\n", ret->name);
    return ret;
}

struct s_dir_entry2 *find_entry(struct s_inode inode, const char *entry_name)
{
    struct s_dir_entry2 *entry;
    unsigned char block[size_of_block];
    uint32 current_block = 0;
    if (read_inode_logic_block(block, inode, current_block++) != 0)
        return NULL;
    uint16 c_size = 0;
    entry = (struct s_dir_entry2 *)block;

    while (c_size < inode.i_size)
    {

        if (c_size > 0 && c_size % size_of_block == 0){
            read_inode_logic_block(block, inode, current_block++);
            entry = (struct s_dir_entry2 *)block;
        }

        if(entry->inode == 0)
            return NULL;

        char file_name[EXT2_NAME_LEN + 1];

        memcpy(file_name, entry->name, entry->name_len);

        file_name[entry->name_len] = 0; /* append null char to the file name */
                                        // if(print_info)
        if (print_info)
            printf("%lu {%s} {offset %lu} {offset + rec_len %lu}\n", entry->inode, file_name, c_size, c_size + entry->rec_len);

        c_size += entry->rec_len;

        if (strcmp(file_name, entry_name) == 0)
        {
            c_size -= entry->rec_len;
            struct s_dir_entry2 *return_entry = (struct s_dir_entry2 *)malloc(sizeof(struct s_dir_entry2));
            return_entry->inode = entry->inode;
            return_entry->rec_len = entry->rec_len;
            return_entry->name_len = entry->name_len;
            return_entry->file_type = entry->file_type;
            strcpy(return_entry->name, entry->name);
            return_entry->block_number = current_block - 1;
            return_entry->offset = c_size % size_of_block;
            printf("\n\n");
            return return_entry;
        }

        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }
    printf("\n\n");
    return NULL;
}

struct s_dir_entry2 *find_previous_entry(struct s_inode inode, const char *entry_name)
{
    struct s_dir_entry2 *entry;
    struct s_dir_entry2 *previous_entry = NULL;
    unsigned char block[size_of_block];
    uint32 current_block = 0;
    read_inode_logic_block(block, inode, current_block++);
    uint16 c_size = 0;
    entry = (struct s_dir_entry2 *)block;

    while (c_size < inode.i_size)
    {

        if (c_size > 0 && c_size % size_of_block == 0){
            read_inode_logic_block(block, inode, current_block++);
            entry = (struct s_dir_entry2 *)block;
        }

        char file_name[EXT2_NAME_LEN + 1];

        memcpy(file_name, entry->name, entry->name_len);

        file_name[entry->name_len] = 0; /* append null char to the file name */

        if (strcmp(file_name, entry_name) == 0)
        {
            if (previous_entry == NULL)
                return NULL;
            uint32 offset = (c_size % size_of_block) - previous_entry->rec_len;
            struct s_dir_entry2 *return_entry;
            if (offset >= 0)
            {
                return_entry = (struct s_dir_entry2 *)malloc(sizeof(struct s_dir_entry2));
                return_entry->inode = previous_entry->inode;
                return_entry->rec_len = previous_entry->rec_len;
                return_entry->name_len = previous_entry->name_len;
                return_entry->file_type = previous_entry->file_type;
                strcpy(return_entry->name, previous_entry->name);
                return_entry->block_number = current_block - 1;
                return_entry->offset = offset;
            }
            else
            {
                return_entry = find_entry(inode, previous_entry->name);
            }

            return return_entry;
        }
        c_size += entry->rec_len;
        previous_entry = entry;
        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }
    return NULL;
}

int add_entry(struct s_inode *parent_inode, uint32 parent_inode_number, int new_inode, char *entry_name, uint8 file_type)
{
    // printf("Entro add entry, new_inode: %d \n",new_inode);
    // printf("parent_inode_number: %d \n",parent_inode_number);
    uint32 entry_name_len = strlen(entry_name);
    // printf("new entry name len %lu\n", entry_name_len);

    struct s_dir_entry2 *parent_last_entry = find_last_entry(*parent_inode);
    // if (parent_last_entry)
    //     printf("last entry returned %s logic_block %lu offset %lu\n", parent_last_entry->name, parent_last_entry->block_number, parent_last_entry->offset);

    struct s_dir_entry2 new_entry;
    new_entry.inode = new_inode;
    new_entry.rec_len = size_of_block;
    new_entry.name_len = entry_name_len + 2;
    new_entry.file_type = (file_type == ENTRY_DIR)? 0x4000 : file_type;

    char buffer[size_of_block];
    bzero(buffer,size_of_block);

    // printf("parent_last_entry: %d\n", parent_last_entry);

    if (!parent_last_entry)
    {
        // read_inode_logic_block(buffer, *parent_inode, 0);
        // printf("Bloque virgen 0\n");
        parent_inode->i_size += size_of_block;

        parent_inode->i_blocks += n_512k_blocks_per_block;
        save_inode(*parent_inode, parent_inode_number);

        memcpy(&buffer, &new_entry, ENTRY_BASE_SIZE);
        strncpy(&buffer[ENTRY_BASE_SIZE], entry_name, new_entry.name_len);
        write_inode_logic_block(buffer, parent_inode, 0, parent_inode_number);
    }else if (size_of_block - (parent_last_entry->offset % size_of_block + ENTRY_BASE_SIZE + parent_last_entry->name_len) >= ENTRY_BASE_SIZE + entry_name_len + 2)
    {
        read_inode_logic_block(buffer, *parent_inode, parent_last_entry->block_number);
        // printf("cabe en el block number actual\n");
        uint32 new_rec_len = ENTRY_BASE_SIZE + parent_last_entry->name_len + 2;
        // printf("new rec_len %lu\n", new_rec_len);
        memcpy(&buffer[parent_last_entry->offset + sizeof(uint32)], &new_rec_len, sizeof(uint16));

        new_entry.rec_len = size_of_block - (parent_last_entry->offset + new_rec_len);

        memcpy(&buffer[parent_last_entry->offset + new_rec_len], &new_entry, ENTRY_BASE_SIZE);
        strncpy(&buffer[parent_last_entry->offset + new_rec_len + ENTRY_BASE_SIZE], entry_name, new_entry.name_len);
        write_inode_logic_block(buffer, parent_inode, parent_last_entry->block_number, parent_inode_number);
    }
    else
    {
        // printf("tengo que reservar un nuevo bloque para el\n");

        memcpy(buffer, &new_entry, ENTRY_BASE_SIZE);
        strncpy(&buffer[ENTRY_BASE_SIZE], entry_name, new_entry.name_len);

        parent_inode->i_size += size_of_block;

        parent_inode->i_blocks += n_512k_blocks_per_block;
        save_inode(*parent_inode, parent_inode_number);

        write_inode_logic_block(buffer, parent_inode, parent_last_entry->block_number + 1, parent_inode_number);
    }
    device_flush();
    free(parent_last_entry);
    return 0;
}

int take_left_entry(struct s_inode* inode, uint32 inode_number, struct s_dir_entry2* to_delete_entry){
    char buffer[size_of_block];
    read_inode_logic_block(buffer, *inode, to_delete_entry->block_number);

    struct s_dir_entry2 *next_entry = (struct s_dir_entry2*) &buffer[to_delete_entry->offset + to_delete_entry->rec_len];
    uint16 new_rec_len = to_delete_entry->rec_len + next_entry->rec_len;
    memcpy(&buffer[to_delete_entry->offset], next_entry, ENTRY_BASE_SIZE);
    memcpy(&buffer[to_delete_entry->offset + ENTRY_BASE_SIZE], next_entry->name, next_entry->name_len);
    memcpy(&buffer[to_delete_entry->offset + sizeof(uint32)], &new_rec_len, sizeof(uint16));
    write_inode_logic_block(buffer, inode, to_delete_entry->block_number, inode_number);
    return 0;
}

int take_right_entry(struct s_inode* inode, uint32 inode_number, struct s_dir_entry2* previous_entry){
    char buffer[size_of_block];
    read_inode_logic_block(buffer, *inode, previous_entry->block_number);

    struct s_dir_entry2 *target = (struct s_dir_entry2*) &buffer[previous_entry->offset + previous_entry->rec_len];
    uint16 new_rec_len = previous_entry->rec_len + target->rec_len;
    memcpy(&buffer[previous_entry->offset + sizeof(uint32)], &new_rec_len, sizeof(uint16));
    write_inode_logic_block(buffer, inode, previous_entry->block_number, inode_number);
    return 0;
}

int remove_entry(struct s_inode *parent_inode, uint32 parent_inode_number, char* entry_name){
    printf("remove entry starts\n");
    if(strcmp(entry_name, ".") == 0 || strcmp(entry_name, "..") == 0)
        return -EPERM;

    uint32 entry_name_len = strlen(entry_name);
    printf("new entry name len %lu\n", entry_name_len);

    int parent_inode_group, parent_inode_index;
    locate(parent_inode_number, es.s_inodes_per_group, &parent_inode_group, &parent_inode_index);

    struct s_dir_entry2* to_delete_entry = find_entry(*parent_inode, entry_name);
    struct s_dir_entry2* previous_entry = find_previous_entry(*parent_inode, entry_name);
    //struct s_dir_entry2* last_entry = find_last_entry(*parent_inode);

    printf("to_delete_entry: {b_number %lu} {offset: %lu} \n", to_delete_entry->block_number, to_delete_entry->offset);
    printf("previous_entry: {b_number %lu} {offset: %lu} \n", previous_entry->block_number, previous_entry->offset);

    if(!to_delete_entry){
        printf("entry not found\n");
        return -ENOENT;
    }else if(to_delete_entry->offset == 0 && to_delete_entry->rec_len != size_of_block){
        printf("first entry of block but not only one\n");
        return take_left_entry(parent_inode, parent_inode_number, to_delete_entry);
    }else if(to_delete_entry->offset == 0 && to_delete_entry->rec_len == size_of_block){
        printf("first entry of block ant only one\n");
        parent_inode->i_size -= size_of_block;
        parent_inode->i_blocks -= n_512k_blocks_per_block;
        free_logic_block(parent_inode, to_delete_entry->block_number);
    }else{
        printf("has previous_entry\n");
        return take_right_entry(parent_inode, parent_inode_number, previous_entry);
    }

    // save_inode(*parent_inode, parent_inode_number);

    return 0;

}

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

    uint32 indirect_blocks = indirect_blocks_count;
    uint32 d_indirect_blocks = indirect_blocks * indirect_blocks;
    uint32 t_indirect_blocks = d_indirect_blocks * indirect_blocks;

    printf("sizeof struct s_inode %lu\n", sizeof(struct s_inode));
    printf("sizeof es.inode %lu\n", es.s_inode_size);

    printf("Direct blocks %d\n", EXT2_NDIR_BLOCKS);
    printf("Indirect blocks %lu\n", indirect_blocks_count);
    printf("D_Indirect blocks %lu\n", d_indirect_blocks_count);
    printf("T_Indirect blocks %lu\n", t_indirect_blocks_count);
}

/*FUSE FUNCTIONS*/
void nxfs_init(struct fuse_conn_info *conn)
{
    int status = read_sb();
    read_group_descriptors();

    test();
    printf("\n\n");
}

int nxfs_get_attr(const char *path, struct stat *statbuf)
{

    // if (print_info)
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

    struct s_inode *dir_inode = read_inode(entry_inode);

    statbuf->st_mode = dir_inode->i_mode;
    statbuf->st_nlink = dir_inode->i_links_count;
    statbuf->st_size = dir_inode->i_size;

    statbuf->st_uid = dir_inode->i_uid;
    statbuf->st_gid = dir_inode->i_gid;

    statbuf->st_blocks = dir_inode->i_blocks;

    if (print_info)
        printf("success\n");
    free(dir_inode);
    return 0;
}

int nxfs_read_dir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo)
{

    if (print_info)
        printf("read dir %s\n", path);

    struct s_dir_entry2 *entry;

    struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;

    uint32 entry_inode = fh->f_inode;

    unsigned char block[size_of_block];
    struct s_inode *dir_inode = read_inode(entry_inode);

    uint32 current_block = 0;

    if (print_info)
        printf("rd inode_direct[0] %lu\n", dir_inode->i_direct[0]);
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
            printf("rd-> error reading block\n");
            return -1;
        }

        if (print_info)
            printf("rd-> \t%u %s\n", entry->inode, file_name);

        filler(buf, file_name, NULL, 0);

        entry = (struct s_dir_entry2 *)((void *)entry + entry->rec_len); /* move to the next entry */
    }
    if (print_info)
        printf("%u of %u\n", dir_inode->i_size, c_size);
    // free(dir_inode);
    return 0;
}

int nxfs_opendir(const char *path, struct fuse_file_info *fileInfo)
{

    if (print_info)
        printf("opendir %s\n", path);

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

    return -ENOENT;
}

int nxfs_statfs(const char *path, struct statvfs *statInfo)
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

int nxfs_open(const char *path, struct fuse_file_info *fileInfo)
{
    // if (print_info)
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

int nxfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    if (print_info)
        printf("read %s size %u offset %u\n", path, size, offset);

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

    for (int i = 0; i < number_of_blocks; i++)
    {
        char buff[size_of_block];
        read_inode_logic_block(buff, *inode, logic_block_number + i);

        // printf("read {%s}\n", buff);

        memcpy(&buf[i * size_of_block], buff, size_of_block);
    }

    return bytes_to_read;
}

int nxfs_release(const char *path, struct fuse_file_info *fileInfo)
{

    free((void *)fileInfo->fh);
    return 0;
}

int nxfs_releasedir(const char *path, struct fuse_file_info *fileInfo)
{

    free((void *)fileInfo->fh);
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
    printf("Parent number %d\n", parent_inode_number);

    struct s_inode *parent_inode = read_inode(parent_inode_number);

    // new inode to new dir
    int parent_inode_group, parent_inode_index;
    locate(parent_inode_number, es.s_inodes_per_group, &parent_inode_group, &parent_inode_index);

    int new_inode = get_free_inode(parent_inode_group);
    printf("Nuevo inode devuelto %d del grupo %d\n", new_inode, parent_inode_group);
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

    int result = add_entry(parent_inode, parent_inode_number, new_inode, dir_name, ENTRY_DIR);
    printf("result: %d\n", result);

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
    result = add_entry(&s_new_inode, new_inode, new_inode, new_entry_name, ENTRY_DIR);
    printf("result for dot (.): %d\n", result);
    //save_inode(s_new_inode,new_inode);

    //dot dot entry for new dir
    strcpy(new_entry_name, "..");
    result = add_entry(&s_new_inode, new_inode, parent_inode_number, new_entry_name, ENTRY_DIR);
    printf("result for dot (..): %d\n", result);
    //save_inode(s_new_inode,new_inode);
    save_meta_data();

    free(parent_inode);

    return 0;
}

int nxfs_truncate(const char *path, off_t newSize)
{
    char *path_child = (char *)malloc(strlen(path) + 1);
    bzero(path_child, strlen(path) + 1);
    memcpy(path_child, path, strlen(path));
    // if (print_info)
        printf("truncate %s newsize %lu\n", path_child, newSize);

    uint32 inode_index = lookup_entry_inode(path_child, ROOT_INO);
    if (print_info)
        printf("inode_index: %d\n", inode_index);
    struct s_inode *child_inode = read_inode(inode_index);

    if (print_info)
        printf("Llego hasta aqui\n");

    if (child_inode->i_size == newSize)
        return 0;

    if (child_inode->i_size < newSize)
        return -ENOENT;

    if (inode_index < 11)
    {
        printf("inode not found \n");
        return -ENOENT;
    }

    if (print_info)
        printf("Llego hasta aqui 2\n");
    uint32 inode_blocks = child_inode->i_size / size_of_block;
    if (inode_blocks * size_of_block < child_inode->i_size)
        inode_blocks += 1;
    uint32 new_block_size = newSize / size_of_block;
    if (print_info)
        printf("inode_blocks: %d\n", inode_blocks);

    if (newSize == 0)
    {
        for (uint32 i = 0; i < inode_blocks; ++i)
            free_logic_block(child_inode, i);

        // if (print_info)
            printf("free data blocks\n");
        free_indirect_blocks(child_inode);
        // if (print_info)
            printf("free indirect blocks\n");

        child_inode->i_indirect = 0;
        child_inode->i_d_indirect = 0;
        child_inode->i_t_indirect = 0;
        child_inode->i_size = 0;
        child_inode->i_blocks = 0;
    }

    save_inode(*child_inode, inode_index);
    if (print_info)
        printf("Saved inode\n");
    save_meta_data();
    if (print_info)
        printf("Saved meta data\n");
    free(path_child);
    free(child_inode);
    return 0;
}

/*NOT IMPLEMENTED, YET*/
int nxfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    if (print_info)
        printf("write %s size %lu offset %lu\n", path, size, offset);

    int bytes_to_write = size;

    struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;

    if (print_info)
        printf("write logic block 1\n");

    uint32 logic_block_number = offset / size_of_block;
    uint32 number_of_blocks = size / size_of_block;
    if(number_of_blocks * size_of_block < size)
        number_of_blocks++;
    struct s_inode *inode = read_inode(fh->f_inode);

    for (int i = 0; i < number_of_blocks; i++)
    {

        if (print_info)
            printf("write logic block %d\n", logic_block_number + i);

        // bzero(buffer, size_of_block);
        char buffer[size_of_block];
        memcpy(buffer, &buf[i*size_of_block], size_of_block);
        // printf("buffer to write {%s}\n", buffer);
        write_inode_logic_block(buffer, inode, logic_block_number + i, fh->f_inode);

        if (print_info)
            printf("wrote\n", logic_block_number + i);
    }

    if (offset + size > inode->i_size){
        uint32 n512_blocks = bytes_to_write / 512;
        if(n512_blocks*512 < bytes_to_write)
            n512_blocks++;
        inode->i_blocks += n512_blocks;
    }

    inode->i_size += bytes_to_write;

    save_inode(*inode, fh->f_inode);
    free(inode);

    return bytes_to_write;
}

int nxfs_rename(const char *path, const char *newpath)
{
    printf("\n\nrename %s newPath %s\n", path, newpath);

    uint32 len = strlen(path);
    char parent[len];
    char old_entry_name[len];
    char new_entry_name[len];

    parseNewEntry(path, parent, old_entry_name);
    parseNewEntry(newpath, parent, new_entry_name);

    uint32 parent_inode_number = lookup_entry_inode(parent, ROOT_INO);
    printf("Parent number %d\n", parent_inode_number);

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

int nxfs_rmdir(const char *path)
{
    printf("rmdir %s\n", path);
    char path_copy[strlen(path)+1];
    strncpy(path_copy, path, strlen(path));
    path_copy[strlen(path)] = 0;
    uint32 parent_inode_number = lookup_entry_inode(path_copy,ROOT_INO);

    struct s_inode* parent_inode = read_inode(parent_inode_number);
    struct s_dir_entry2* last_entry = find_last_entry(*parent_inode);
    if(strcmp(last_entry->name, "..") == 0){
        nxfs_unlink(path);
        return 0;
    }else
        return -EPERM;
}

int nxfs_unlink(const char *path)
{
    printf("unlink %s\n", path);
    //truncating inode
    nxfs_truncate(path, 0);

    //free inode
    uint32 len = strlen(path);
    char parent[len];
    char child_name[len];
    parseNewEntry(path, parent, child_name);
    uint32 parent_inode_number = lookup_entry_inode(parent,ROOT_INO);
    printf("Parent number %d\n", parent_inode_number);
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

int nxfs_mknod(const char *path, mode_t mode, dev_t dev)
{
    printf("mknod %s\n", path);
    return 0;
}

int nxfs_create(const char *path, mode_t mode, struct fuse_file_info *fileInfo)
{
    printf("create %s mode %x\n", path, mode);
    char parent_name[strlen(path)];
    char child_name[strlen(path)];
    parseNewEntry(path,parent_name,child_name);
    printf("padre :%s hijo:%s\n",parent_name,child_name);
    uint32 parent_inode_number = lookup_entry_inode(parent_name, ROOT_INO);
    printf("parent inode number %d\n", parent_inode_number);
    struct s_inode *inode = read_inode(parent_inode_number);
    int parent_inode_group, parent_inode_index;
    locate(parent_inode_number,es.s_inodes_per_group, &parent_inode_group,&parent_inode_index);
    int new_inode = get_free_inode(parent_inode_group);
    printf("Nuevo inode devuelto %d del grupo %d\n", new_inode, parent_inode_group);
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
    for(int i=0; i<EXT2_NDIR_BLOCKS;i++)
        s_new_inode.i_direct[i]=0;
    s_new_inode.i_indirect = 0;
    s_new_inode.i_d_indirect = 0;
    s_new_inode.i_t_indirect = 0;

    struct s_file_handle *fh = (s_file_handle *)malloc(sizeof(s_file_handle));
    fh->f_inode = new_inode;
    fh->f_size = 0;
    fh->f_blocks_count = 0;
    fileInfo->fh = (uint64_t)fh;

    int result = add_entry(inode, parent_inode_number, new_inode, child_name, ENTRY_FILE);
    printf("result: %d\n", result);

    //set new inode and first block as used in bitmap and save inode
    inode_bitmap_set(new_inode, 1);
    printf("result1: %d\n", result);
    // block_bitmap_set(first_block,1);
    save_inode(s_new_inode,new_inode);
    printf("result2: %d\n", result);
    save_meta_data();
    printf("result3: %d\n", result);
    free(inode);
    //int result = add_entry(*inode, parent_inode_number, child_name, mode, ENTRY_FILE);
    //printf("result: %d\n", result);
    //printf("fh index %lu\n",fileInfo->fh);
    //struct s_file_handle *fh = (struct s_file_handle *)fileInfo->fh;
    //printf("fh->inode %lu\n",fh->f_inode );
    return 0;
}

int nxfs_utime(const char * path, struct utimbuf *times){
    printf("utime %s\n", path);
    return 0;
}
