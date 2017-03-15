#include "inodes.h"

// void locate_inode(int inode_number, int inodes_per_group, int* inode_group, int* inode_offset){
// 	*inode_group =  (inode_number -1)/inodes_per_group;
// 	*inode_offset = (inode_number -1)%inodes_per_group;
// }

int is_dir(uint16 i_mode){
	if(i_mode >= 0x4000 && i_mode < 0x5000)
		return 1;
	return 0;
}

int is_file(uint16 i_mode){
	if(i_mode >= 0x8000 && i_mode < 0x9000)
		return 1;
	return 0;
}