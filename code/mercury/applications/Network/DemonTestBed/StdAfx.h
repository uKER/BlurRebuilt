#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined( PLATFORM_XENON )

#include <xtl.h>
#include <xbdm.h>
#include <xmp.h>

#elif defined( PLATFORM_PC )

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 

#include <winsock2.h>
#include <windows.h>

#elif defined( PLATFORM_PS3 )

#include <limits.h>
#include <cell/atomic.h>
#include <string.h>
#include <wchar.h>
#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>
#include <cell/pad.h>

#endif

#include <Core/Core.h>
#include <Display/Core/Includes.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <system/serialise.h>
#include <Display.h>
#include <Render.h>

#include "Samples/System/Debug.h"

#include "ui/uilogger.h"
#include "Network/Core/NetLogger.h"

#endif // STDAFX_H

/*
#include <Core/Core.h>

#ifndef PLATFORM_SPU

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#include <cell/pad.h>

*/
