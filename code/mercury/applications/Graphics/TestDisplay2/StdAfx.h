#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_XENON)

#include <xtl.h>
#include <stdio.h>
#include <stdlib.h>

#elif defined(PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#else

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#include <Display2/Display2Includes.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

#endif // STDAFX_H
