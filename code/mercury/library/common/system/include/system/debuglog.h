#ifndef __DEBUGLOG_H__
#define __DEBUGLOG_H__

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_SPU
#include "debug\log\DebugLogSPU.h"
#else

// this file contains the set of includes needed for the most basic debug logging functionality
// i.e. Debug::GetLogger, LOGINFO, LOGDEBUG, etc.

#include "debug\log\DebugLevel.h"
#include "debug\log\LocationInfo.h"
#include "debug\log\GlobalInterface.h"
#include "debug\log\DebugLogMacro.h"

#endif // PLATFORM_SPU

#endif // __DEBUGLOG_H__

