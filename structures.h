#include <stdint.h>

#define uint16 unsigned short int
#define uint32 unsigned int
#define uint8 uint8_t
#define EXT2_NAME_LEN 255
#define EXT2_N_BLOCKS 12

struct s_superblock{
	uint32 s_inodes_count;
	uint32 s_blocks_count;
	uint32 s_r_blocks_count;
	uint32 s_free_block_count;
	uint32 s_free_inodes_count;
	uint32 s_first_data_block;
	uint32 s_log_block_size;
	uint32 s_log_frag_size;
	uint32 s_blocks_per_group;
	uint32 s_flags_per_group;
	uint32 s_inodes_per_group;
	uint32 s_mtime;
	uint32 s_wtime;
	uint16 s_mnt_count;
	uint16 s_max_mnt_count;
	uint16 s_magic;
	uint16 s_state;
	uint16 s_errors;
	uint16 s_minor_rev_level; //?
	uint32 s_lastcheck;
	uint32 s_checkinterval;
	uint32 s_creator_os;
	uint32 s_rev_level;
	uint16 s_def_resuid;
	uint16 s_def_resgid;
	uint32 s_firts_inode;
	uint16 s_inode_size;
};

struct s_block_group_descriptor{
	uint32 bg_block_bitmap;
	uint32 bg_inode_bitmap;
	uint32 bg_inode_table;
	uint16 bg_free_blocks_count;
	uint16 bg_free_inodes_count;
	uint16 bg_used_dirs_count;
	uint16 bg_pad;
	uint32 bg_reserved1;
	uint32 bg_reserved2;
	uint32 bg_reserved3;
};

struct s_inode{
	uint16 i_mode;
	uint16 i_uid;
	uint32 i_size;
	uint32 i_atime;
	uint32 i_ctime;
	uint32 i_mtime;
	uint32 i_dtime;
	uint16 i_gid;
	uint16 i_links_count;
	uint32 i_blocks;
	uint32 i_flags;
	uint32 i_osd1;
	uint32 i_direct[EXT2_N_BLOCKS];
	uint32 i_indirect;
	uint32 i_d_indirect;
	uint32 i_t_indirect;
	uint32 i_generation;
	uint32 i_file_acl;
	uint32 i_dir_acl;
	uint32 i_faddr;
	uint32 i_osd2_1;
	uint32 i_osd2_2;
	uint32 i_osd2_3;
};

struct s_dir_entry2 {
	uint32	inode;			/* Inode number */
	uint16	rec_len;		/* Directory entry length */
	uint8	name_len;		/* Name length */
	uint8	file_type;
	char	name[EXT2_NAME_LEN];	/* File name */
};