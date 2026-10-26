#ifndef GAMESYS_DEBUGSYSTEMSPCH_H
#define GAMESYS_DEBUGSYSTEMSPCH_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#define _stricmp strcasecmp

#else

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

#endif // GAMESYS_DEBUGSYSTEMSPCH_H
