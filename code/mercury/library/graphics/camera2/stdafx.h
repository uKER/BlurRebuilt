#ifndef _CAMERA_STDAFX_H
#define _CAMERA_STDAFX_H
#pragma warning (push, 3)
#ifdef _MSC_VER
#pragma once
#endif

#pragma warning(disable:4146)

#if defined (PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#define _stricmp strcasecmp

#else

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <Core/Threads/Mutex.h>
#include <System/Debug/Debug.h>
#include <core/containers/sortedbinarytree.h>
#include <core/containers/containerSerialisation.h>
#include <core/containers/string.h>

#pragma warning (pop)

#endif // _CAMERA_STDAFX_H
