#include "SystemPch.h"

#include <system/serialise.h>

namespace Debug
{

// file system for debug logger
Sys::I_FileSystem *g_pFileSystem = &Sys::C_FileSystem::Instance();

}
