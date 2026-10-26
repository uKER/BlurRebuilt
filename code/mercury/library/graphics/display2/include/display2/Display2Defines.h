#ifndef DISPLAY2_DEFINES_H
#define DISPLAY2_DEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Core.h>

#if PLATFORM_PC

	#define DEVICE_D3D11 1
	#define DEVICE_D3D11_PC 1
	#define DEVICE_PC 1

#elif PLATFORM_PS3 || PLATFORM_SPU

	#define DEVICE_GCM 1
	#define DEVICE_GCM_PS3 1
	
#endif

#if CFG_DEBUG
#define DISPLAY_DEBUG 1
#endif

#if !CFG_SPEED
#define TRACK_DISPLAY2_RESOURCES 1
#endif

#if !CFG_GOLD
#define TRACK_DISPLAY2_PROTECTED_MEMORY 1
#endif

#if !CFG_GOLD
//#define DONT_INLINE_DISPLAY2 1
#endif

#if !CFG_GOLD
//#define USE_GPU_TIMINGS2 1
#endif

#define SUCCESS(_status)			((HRESULT)(_status)>=0)
#define FAILURE(_status)			((HRESULT)(_status)<0)

#endif // DISPLAY2_DEFINES_H
