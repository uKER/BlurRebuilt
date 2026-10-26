#pragma once

#define _CRT_SECURE_NO_DEPRECATE
//Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500 
#include <windows.h>
#include <winsock2.h>

#include <string.h>

//Mercury Core lib
#include <core/core.h>
#include <core/containers/list.h>
#include <core/containers/string.h>
#include <core/maths.h>

//Mercury Btl lib
#include "btl/containers/vector.h"

//Mercury System lib
#include <system/xml/Xml.h>
#include <system/serialise.h>

// system includes
#include <system/debuglog.h>

namespace Tools
{
	extern Debug::Logger& g_log;
}
