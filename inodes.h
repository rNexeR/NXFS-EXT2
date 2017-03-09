#ifndef inode_h
#define inode_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define uint16 unsigned short int
#define uint32 unsigned int
#define uint8 uint8_t

#define	EXT2_BAD_INO		 	1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 	2	/* Root inode */
#define EXT2_ACL_IDX_INO	 	3	/* ACL inode */
#define EXT2_ACL_DATA_INO	 	4	/* ACL inode */
#define EXT2_BOOT_LOADER_INO	5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 	6	/* Undelete directory inode */

void locate_inode(int inode_number, int inodes_per_group, int* inode_group, int* inode_offset);
int is_dir(uint16 i_mode);
int is_file(uint16 i_mode);

#ifdef __cplusplus
}
#endif

#endif