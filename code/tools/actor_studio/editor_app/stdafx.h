#ifndef _stdwx_h_
#define _stdwx_h_

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501

// for artery
#include <winsock2.h>

#include <windows.h>
#include <gdiplus.h>

// STL
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

// SYSTEM INCLUDES
#include <wx/wxprec.h>
#include <wx/regex.h>
#include <wx/grid.h>

#ifdef __VISUALC__
#   pragma warning(disable:4100)  
#endif

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
#include <Btl/Containers/map.h>
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
#include <System/Misc/StringUtil.h>
#include <System/Misc/BtlStringUtil.h>

#include <Misc_Shared/Variant/Variant.h>
#include <Misc_Shared/Variant/KeyValue.h>
#include <Misc_Shared/Factory/Factory.h>

#include <Tools_Shared/ToolsApplication.h>
// display library
#include <Display/Core/Includes.h>
#include <Display.h>

//persistent debug rendering
#include <Render/Debug/DebugRender.h>

// aftk rendering
#include "Aftk/Aftk.h"
#include "AftkRender.h"

#include <xt/xttool.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Definitions.h>

// APPLICATION INCLUDES
#include "Resource.h"
#include <EditorApp/Misc/Definitions.h>

//use wxflatnotebook from contrib in C_Frame
#define USE_WXFLATNOTEBOOK

namespace Editor
{
	extern Debug::Logger& g_Log;
	void AppYield();
}

#endif
