#ifndef AUDIO_AUDIOPCH_H
#define AUDIO_AUDIOPCH_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>

NOALIAS __forceinline void ReadWriteBarrier();

//	audio profiler stuff
#if !defined (PLATFORM_SPU) && !defined (CFG_GOLD)
#define AUDIO_ENABLE_PROFILING
#endif

#if defined (AUDIO_ENABLE_PROFILING)
#if defined (PLATFORM_PC)
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 
#include <winsock2.h>
#include <windows.h>
#endif // (PLATFORM_PC)
#endif // (AUDIO_ENABLE_PROFILING)

#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU) && !defined CFG_GOLD
#define NAMED_COUNTERS_ENABLED
#define CPU_COUNTERS_ENABLED
#endif

//#define AUDIO_USE_NEW_MATHS

//#ifndef MEMORY_TRACKER_PUSH
//#define MEMORY_TRACKER_PUSH
//#endif
//
//#ifndef MEMORY_TRACKER_POP
//#define MEMORY_TRACKER_POP()
//#endif


#if defined (PLATFORM_PS3) && !defined (_PS3)
#define _PS3
#include <sys/timer.h>
#endif


#ifdef PLATFORM_PC
#include <windows.h>

#define AUDIO_RESTRICT
#define AUDIO_INLINE	__forceinline

#endif

#ifdef PLATFORM_PS3 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <libsn.h>
#include <stdint.h>

#define AUDIO_RESTRICT __restrict
#define AUDIO_INLINE __attribute__((always_inline))

#endif // PLATFORM_PS3

//#include <system/debuglog.h>
#include <system/debug/debug.h>
//#include <system/memory/MemoryTracker.h>

#ifdef PLATFORM_SPU
#include "Source/SystemHeadersSPU.h"
#else
#include "Source/SystemHeaders.h"
#endif

#include <core/Maths.h>

//#include "Source/Core/CoreHeaders.h"

#endif // AUDIO_AUDIOPCH_H
