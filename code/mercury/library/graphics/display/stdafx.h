#ifndef DISPLAY_STDAFX_H
#define DISPLAY_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#pragma warning(disable:4146)

#if defined(PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#define _stricmp strcasecmp

#elif !PLATFORM_SPU

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#include <Display/Core/Includes.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>
#include <Btl\Containers\vector.h>

#if !PLATFORM_SPU
#include "Display/Device/DeviceDefines.h"
#include "Display/Utils/DebugLog.h"
#include "Display/Utils/Globals.h"
#include "Display/Utils/Perf.h"
#endif

#endif // DISPLAY_STDAFX_H
