#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PC)

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <time.h>

#elif defined(PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>
#include <string.h>
#include <wchar.h>
#include <cell/pad.h>

#endif

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>
// [Blur2PC] umbrellas the game code assumes are visible (the engine UI PCH includes them too)
#include <btl/Containers/containers.h>
#include <btl/utility/string.h>
#include <misc_shared/Messaging/GameMessaging.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

#include <Display.h>

#endif // STDAFX_H
