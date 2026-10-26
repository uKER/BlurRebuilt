#ifndef __SYSTEM_LOG_FILESYSTEM_H__
#define __SYSTEM_LOG_FILESYSTEM_H__

// file system that is used by the debug logging library
namespace Sys
{
	class I_FileSystem;
}

namespace Debug
{

extern Sys::I_FileSystem *g_pFileSystem;

}

#endif
