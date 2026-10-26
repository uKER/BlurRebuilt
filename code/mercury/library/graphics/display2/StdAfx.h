#ifndef DISPLAY2_STDAFX_H
#define DISPLAY2_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#elif defined(PLATFORM_SPU)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#else

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#include <Display2/Display2Includes.h>

#include <Core/Core.h>
#include <System/DebugLog.h>

#if !defined(PLATFORM_SPU)
#include <Core/Maths.h>
#include <Core/Containers.h>
#include <System/Debug/Debug.h>
#endif

#include <Display2/Display2Logger.h>

#endif // DISPLAY2_STDAFX_H
