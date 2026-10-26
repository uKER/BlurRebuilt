// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _DEBUG
#define CHECK_FOR_MEMORY_LEAKS
#endif

#ifdef CHECK_FOR_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <iostream>
#include <tchar.h>
#include <string>

#include <stdio.h>
#include <tchar.h>

#include "math.h"

#include "assert.h"
#include "crtdbg.h"

#include "windows.h"
#include <assert.h>

// mercury includes
#include "core/core.h"
#include "core/containers.h"
#include "system/serialise.h"
#include "system/xml/xml.h"
#include "system/misc/StringUtil.h"
#include "system/debug/log/DebugLog.h"

// audio tools common
#include "console.h"
#include "Debug.h"
#include "HelperFuncs.h"
#include "ErrorReport.h"

#include "WaveFile.h"
#include "samplerate.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers



//int PrintError(const char *format, ...);
//int PrintWarning(const char *format, ...);

extern bool g_bVerbose;
// TODO: reference additional headers your program requires here
