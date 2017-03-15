#ifndef utils_h
#define utils_h

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

unsigned int findChar(char* source, char searched);
void getNewDir(const char* oldPath, char* parent_directory, char* name);

#ifdef __cplusplus
}
#endif

#endif //sfs_h