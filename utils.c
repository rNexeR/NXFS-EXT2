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