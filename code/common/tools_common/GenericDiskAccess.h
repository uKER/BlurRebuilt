#ifndef TU_GENERIC_DISK_ACCESS_H
#define TU_GENERIC_DISK_ACCESS_H

#include <fcntl.h> // for open()
#include <cerrno> // for errno 
#include <cstdio> // for perror()
#include <io.h>
#include <share.h>

#define TU_GENERIC_DISK_ACCESS_META_TAG 'META'
#define TU_GENERIC_DISK_ACCESS_ID(value) ((U32)(((value>>24)&0xff)|((value>>8)&0xff00)|((value<<8)&0xff0000)|((value<<24)&0xff000000)))
#define TU_GENERIC_DISK_ACCESS_MAX_BLOCK_DEPTH	256
#define TU_GENERIC_DISK_ACCESS_OPEN_FILE_FAIL	"Failed to open file."

#endif
