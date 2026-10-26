#ifndef _stdwx_h_
#define _stdwx_h_

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#ifdef __VISUALC__
#   pragma warning(disable:4100)  
#endif

// shared container classes
#include <Core/Core.h>
#include <Core/Maths.h>
#include <core/maths/hvec/new/IncludeGeneral.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers/Vector.h>
#include <Core/Containers/List.h>
#include <Core/Containers/OpenHashTable.h>
#include <btl/containers/vector.h>
#include <btl/containers/openhashmap.h>
#include <btl/containers/list.h>
#include <btl/utility/string.h>

// logging and file access
#include <System/DebugLog.h>
#include <System/Debug/Log/DebugLog.h>
#include <System/Debug/Debug.h>

// string
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

namespace Editor
{
	extern Debug::Logger& g_Log;
}
#endif
