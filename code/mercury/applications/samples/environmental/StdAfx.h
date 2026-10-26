#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Core.h>

// Xenon includes
#if defined(PLATFORM_XENON)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <xtl.h>
#include <xgraphics.h>

#ifdef  __cplusplus
extern "C" {
#endif
_CRTIMP void __cdecl _wassert(__in_z const wchar_t * _Message, __in_z const wchar_t *_File, __in unsigned _Line);
#ifdef  __cplusplus
}
#endif
//#define assert(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )

// PC includes
#elif defined(PLATFORM_PC)

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#include <Display/Core/Includes.h>

// PS3 includes
#elif defined(PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>
#include <cell/pad.h>

#include <Display/Core/Includes.h>

// End specific includes
#endif

// Common includes
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>
#include <Display.h>
#include <Render.h>

//#include "Samples/System/Memory.h"
#include "Samples/System/Debug.h"

#endif // STDAFX_H
