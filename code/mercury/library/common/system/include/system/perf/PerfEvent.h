#ifndef __SYSTEM_PERF_EVENT_H__
#define __SYSTEM_PERF_EVENT_H__

// header file for the CPU-only version of PERF_EVENT
// the PERF_EVENT macro will be overridden if mercury/graphics/display is included


#include <system/Perf/Profiler.h>

namespace Perf
{

class C_PerfEvent
{
public:
	
	C_PerfEvent(const char* pName)
	{
		BeginNamedEvent(pName);

		#ifdef ENABLE_PIX
		PIXBeginNamedEvent(0, pName);
		#endif
	}

	~C_PerfEvent()
	{
		EndNamedEvent();

		#ifdef ENABLE_PIX
		PIXEndNamedEvent();
		#endif
	}
};

}	// namespace Perf

// for SPU programs, this macros stores the name string in the ppudata section in main memory (instead of SPU memory)
#define SPU_PERF_EVENT(_name) static char __tmp_event_[] __attribute__ ((section ("ppudata"))) = _name; Perf::C_PerfEvent cpuEvent(__tmp_event_)

#ifndef PERF_EVENT
#if !defined(CFG_GOLD)
#ifdef PLATFORM_SPU
#define PERF_EVENT(_name) SPU_PERF_EVENT(_name)
#else
#define PERF_EVENT(_name) Perf::C_PerfEvent cpuEvent(_name)
#endif
#else
#define PERF_EVENT(_name)
#endif
#endif

#endif
