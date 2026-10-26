#pragma once

//C and C++ includes
#include <stdio.h>
#include <string>

//Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

//Mercury Core Lib
#include <core/core.h>
#include <core/containers/string.h>
#include <core/containers/list.h>
#include <core/maths.h>
#include <Btl/Containers/vector.h>
#include <Btl/Containers/set.h>
#include <Btl/Utility/string.h>

//Mercury System Lib
#include <system/debuglog.h>

//Mercury Tools Shared Lib
#include <tools_shared/ToolsShared.h>

#include "source/Options.h"

namespace Debug
{
	class I_LogSubscriber;
}

extern Debug::Logger& g_log;
extern Debug::I_LogSubscriber* g_pConsoleSubscriber;

extern S32 g_exitCode;

