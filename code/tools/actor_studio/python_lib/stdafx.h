#ifndef _stdwx_h_
#define _stdwx_h_

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

// SYSTEM INCLUDES
#ifdef __VISUALC__
#   pragma warning(disable:4100)  
#endif

// for artery
#include <winsock2.h>

// shared container classes
#include <Core/Core.h>
#include <Core/Maths.h>
#include <core/maths/hvec/new/IncludeGeneral.h>
#include <Core/Utility/Utility.h>
#include <Core/Utility/Delegate.h>
#include <Core/Containers/Vector.h>
#include <Core/Containers/List.h>
#include <Core/Containers/OpenHashTable.h>
#include <Btl/Containers/vector.h>
#include <Btl/Containers/openhashmap.h>
#include <Btl/Containers/openhashset.h>
#include <Btl/Containers/list.h>
#include <Btl/Utility/string.h>
#include <Btl/Utility/Hashops.h>

// logging and file access
#include <System/DebugLog.h>
#include <System/Debug/Log/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Debug/I_DebugRender.h>
#include <System/Serialise.h>

#include <Misc_Shared/Factory/Factory.h>
#include <Tools_Shared/ToolsApplication.h>
// display library
#include <Display/Core/Includes.h>
#include <Display.h>

// aftk rendering
#include "Aftk/Aftk.h"
#include "AftkRender.h"

// string
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

// APPLICATION INCLUDES
#include <EditorApp/Misc/Definitions.h>

namespace Editor
{
	extern Debug::Logger& g_Log;
}
#endif
