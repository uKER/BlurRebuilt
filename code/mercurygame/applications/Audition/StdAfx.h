#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PC)

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT	0x0501

#include <winsock2.h>
#include <windows.h>
#include <time.h>

#elif defined(PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#include <cell/pad.h>

#elif defined(PLATFORM_XENON)

#include <xtl.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#endif

#include <Display/Core/Includes.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

#include <Display.h>

#ifndef CFG_SPEED
#define ENABLE_MEMORY_TRACKER
// 
//#define MEMORY_TRACKER_TAG_ALLOCATION			1
//#define MEMORY_TRACKER_CALLSTACK				1
//#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS	1
//#define MEMORY_TRACKER_BLOCK_LOCATIONS			1
#endif

#endif // STDAFX_H
