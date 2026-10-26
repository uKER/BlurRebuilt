#ifndef VEHICLE_STDAFX_H
#define VEHICLE_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif
 
#pragma warning(disable:4146)

#ifdef PLATFORM_SPU

#elif PLATFORM_PS3

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#define _stricmp strcasecmp

#else

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#ifndef PLATFORM_SPU
#include <Display/Core/Includes.h>
#endif

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

#include <Display.h>

#include "Vehicle/Utils/DebugLog.h"

#ifdef PLATFORM_PS3
	#define MAX_CHARS 260
#else
	#define MAX_CHARS 260
#endif

#if defined(CFG_SPEED) || defined(CFG_GOLD)
#define VEHICLE_MASTER_BUILD
#endif

#if /*defined(USER_CWRIGHT) && */defined(CFG_RELEASE) && !defined(VEHICLE_MASTER_BUILD) && (defined(PLATFORM_XENON) || defined(PLATFORM_WINDOWS) || defined(PLATFORM_PC))
	#define TURN_OFF_VEHICLE_OPTIMISATIONS
	//#define ENABLE_VEHICLE_SHADER_DAMAGE
#endif

//#define TURN_OFF_VEHICLE_OPTIMISATIONS


#endif // VEHICLE_STDAFX_H
