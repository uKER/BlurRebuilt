#ifndef DISPLAY_UTILS_PERF_H
#define DISPLAY_UTILS_PERF_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"
#include "system/perf/Profiler.h"

#ifdef USE_GCM_P3X
#include "Display/Utils/GCM/libp3x.h"
#endif

#ifdef USE_GCM_HUD
#define USE_GCM_HUD_MARKERS
#endif

#ifdef PLATFORM_PC
#include <Core/Maths.h>
#endif

namespace Display
{

#if PLATFORM_PS3
extern THREAD_LOCAL_STORAGE int g_iOwnerThreadRefCount;
#endif
// C_GPUEvent

class C_GPUEvent
{
public:
	
	C_GPUEvent(const char* pName)
	{
		Perf::BeginNamedEvent(pName);

		#ifdef USE_D3D_PIX
		#if defined (PLATFORM_PC)
			WCHAR wName[256];
			memset(wName, 0, sizeof(wName));
			mbstowcs(wName, pName, Maths::Min(strlen(pName), sizeof(wName)/sizeof(wName[0])));
			D3DPERF_BeginEvent(0, wName);
		#endif
		#endif

		#ifdef USE_GCM_HUD_MARKERS
		if (g_iOwnerThreadRefCount)
		{
			cellGcmSetPerfMonPushMarker(gCellGcmCurrentContext, pName);
		}
		#endif

		#ifdef USE_GCM_P3X
		#if 1
		if(p3x_is_capturing())
			p3x_begin_named_event(0, pName);
		#else
		if(p3x_is_capturing())
			p3x_name_next_event(gCellGcmCurrentContext->current, pName);
		#endif
		#endif
	}

	~C_GPUEvent()
	{
		Perf::EndNamedEvent();

		#ifdef USE_D3D_PIX
		#if defined (PLATFORM_PC)
			D3DPERF_EndEvent();
		#endif
		#endif

		#ifdef USE_GCM_HUD_MARKERS
		if (g_iOwnerThreadRefCount)
		{
			cellGcmSetPerfMonPopMarker (gCellGcmCurrentContext);
		}
		#endif

		#ifdef USE_GCM_P3X
		#if 0
		if(p3x_is_capturing())
			p3x_end_named_event();
		#endif
		#endif
	}

	#ifdef USE_GCM_HUD_MARKERS
	static __thread U32		m_HUDMarkerDepth;
	#endif
};

}; // namespace Display

#ifndef PLATFORM_SPU
	#if defined(USE_D3D_PIX) || defined(USE_GCM_HUD) || defined(USE_GCM_P3X)
		#undef PERF_EVENT
		#define PERF_EVENT(_name) Display::C_GPUEvent gpuEvent(_name)
	#else
		#include <system/perf/PerfEvent.h>
	#endif
#endif // PLATFORM_SPU

#endif // DISPLAY_UTILS_PERF_H
