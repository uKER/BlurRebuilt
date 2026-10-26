#ifndef NIGHTWING_STDAFX_H
#define NIGHTWING_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

// [MarkC]
// This should only contain #includes to commonly used tech files
// DON'T ADD ANY INCLUDES WITHOUT TALKING TO MARK OR YOU BE IN TROUBLE!!!!!

#ifndef PLATFORM_SPU 

#ifdef _XBOX

#include <xtl.h>

#include <stdio.h>
#include <stdlib.h>

#elif defined (PLATFORM_PS3)

#define NO_DEBUG_CONSOLE

#include <limits.h>
#include <core/maths.h>
#include <wchar.h>

#elif defined (PLATFORM_SPU)
	// nothing for SPU

#else

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500 

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <mmreg.h>


#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <dsound.h>

#endif

#include <Display/Core/Includes.h>

#define SHOOTER
#include <core/core.h>
#include <core/utility/utility.h>
#include <core/utility/hash.h>
#include <core/utility/hashops.h>
#include <core/containers.h>
#include <system/DebugLog.h>
#include <system/debug/Debug.h>
#include <system/debug/log/LogManager.h>
#include <system/misc/StringUtil.h>
#include <system\memory.h>

#include <system/Serialise.h>
#include <system/serialise/TextSerialise.h>

#include <Btl/Allocators/pool.h>
#include <Btl/Containers/containers.h>
#include <Btl/Containers/algorithm.h>
#include <Btl/Utility/pair.h>
#include <Btl/Utility/string.h>

#include "SharedTech.h"

#include "GameLoggers.h"

#ifndef CFG_SPEED
//#define GAME_USE_CONSTRAINT_EDITOR
#endif

#ifndef CFG_GOLD
	#if !defined( CFG_SPEED ) || defined( USER_JASOND )
//	#define USE_RAYTRACE_STRESS_TEST
	#endif
#endif

// uncomment this to completely knock out the audio system and any dependant systems
#if defined( __INTEL_COMPILER ) // JASOND: TODO: Fix internal compiler error in generator new/delete to get PC audio back in game...
#define _DISABLE_AUDIO
// disable audio in ps3 debug builds
#elif defined (PLATFORM_PS3) || defined(PLATFORM_SPU)
#if defined(_DEBUG)
#define _DISABLE_AUDIO
#endif
#endif

#ifdef USER_JONC
#if defined (PLATFORM_PS3) || defined(PLATFORM_SPU)
//#define JONS_SPU_TEST
#endif
#endif


#endif // ndef platform SPU
#endif
