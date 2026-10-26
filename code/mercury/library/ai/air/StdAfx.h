#ifndef _AIR_STDAFX_H
#define _AIR_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>
#include <core/Maths.h>
#include <System/DebugLog.h>
#include <core/containers/string.h>
#include <System/Debug/Debug.h>


#if defined (PLATFORM_PS3)
// PS3 system header files
#include <stdio.h>
#include <stdlib.h>


#elif defined (PLATFORM_PC)
// PC system header files
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 
#else
#endif

#include <btl/containers/vector.h>
#include <btl/containers/list.h>
#include <btl/Allocators/pool.h>

#include <string>
#include <vector>
#include <map>

// air includes... here
#include <air.h>

#endif  //_AIR_STDAFX_H
