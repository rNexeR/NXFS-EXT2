#include "utils.h"

unsigned int findChar(char* source, char searched){
	unsigned int len = strlen(source);
	for (int i = 0; i < len; i++)
	{
		if(source[i] == searched)
			return i;
	}
	return -1;
}

void parseNewEntry(const char* oldPath, char* parent_directory, char* name){
	unsigned int len = strlen(oldPath);
	bzero(parent_directory, len);
	bzero(name, len);

	unsigned int i = len-1;
	while(oldPath[i] != '/')
		i--;
	
	strncpy(parent_directory, oldPath, i);
	if(strlen(parent_directory) == 0)
		strncpy(parent_directory, oldPath, i+1);
	strcpy(name, &oldPath[i+1]);
}