#ifndef RESOURCEUTILSPCH_438575A0_6EE6_4C6B_86E4_420C6EDD8250_H
#define RESOURCEUTILSPCH_438575A0_6EE6_4C6B_86E4_420C6EDD8250_H

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

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

#endif // RESOURCEUTILSPCH_438575A0_6EE6_4C6B_86E4_420C6EDD8250_H
