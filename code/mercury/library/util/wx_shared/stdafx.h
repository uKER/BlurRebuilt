#ifdef _MSC_VER_
#pragma once
#endif

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501
#define GDIPVER			0x0110

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/aui/aui.h"

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

#pragma warning(disable:4355)	// This used in initialiser list
#pragma warning(disable:4706)	// Assignment within conditional expression
#pragma warning(disable:4200)	// Zero sized array in struct

namespace Tools
{
	//extern Debug::Logger& g_Log;
}

#include <Xt/XtTool.h>