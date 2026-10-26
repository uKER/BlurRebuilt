#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>

#if defined(PLATFORM_PC)

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#elif defined(PLATFORM_PS3)

#include <limits.h>
#include <cell/atomic.h>
#include <string>

#endif

#include <Display/Core/Includes.h>

#include <core/maths.h>
#include <core/Containers.h>
#include <btl/Containers/containers.h>
#include <core/utility/endian.h>
#include <system/debug/debug.h>
#include <system/serialise.h>
#include <core/threads.h>
#include <system/debuglog.h>
#include <system/memory.h>

#include "ui/uilogger.h"

#include <Display.h>
#include <Display/Utils/DebugLog.h>
#include <Display/device/Device.h>

#ifdef PLATFORM_PS3

#include <wchar.h>

#endif

#endif // STDAFX_H

