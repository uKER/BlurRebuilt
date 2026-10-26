// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#define PLATFORM_PC
#define AUDIO_RESTRICT
#define AUDIO_INLINE	__forceinline

#include <windows.h>

//#pragma optimize ("", off)
 
#ifdef _DEBUG
#define DISPLAY_PROFILE_INFO
#define CHECK_FOR_MEMORY_LEAKS
#ifdef CHECK_FOR_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

//#include "common\types.h"

#include <iostream>
#include <tchar.h>
#include <string>

#include "windows.h"
#include <assert.h>

// mercury includes
#include "core/core.h"
#include "core/containers.h"
#include "system/serialise.h"
#include "system/xml/xml.h"
#include "system/misc/StringUtil.h"
#include "system/debug/log/DebugLog.h"

// audio libary includes

#include "WaveBank.h"
#include "file.h"
#include "Common\StopWatch.h"

// utility includes
#include "xmaencoder.h"
#include "XmlParser.h"
#include "WaveFile.h"
#include "EndianSwitch.h"
#include "exception.h"
//#include "XmaFile.h"

// audio tools common
#include "console.h"
#include "Debug.h"
#include "HelperFuncs.h"
#include "ErrorReport.h"

// secret rabbit sample rate converter code
#include "samplerate.h"

#include "systemheaders.h"

#include "WaveBundler.h"