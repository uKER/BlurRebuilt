#ifndef __SYSTEM_PROFILER_SPU_H__
#define __SYSTEM_PROFILER_SPU_H__

#include "system/Perf/ProfilerBackend.h"

namespace Perf
{

#define EVENT_BUFFER_SIZE 16

// data structure to be passed to the SPU which contains info for it to emit profiling data
struct SPUData
{
	int iCurrentEvent;
	int piCurrentEvent;
	int pEventBuffer;
	Perf::Event eventBuffer[EVENT_BUFFER_SIZE];
};

// PPU side function to fill in the data structure
void SetSPUData(SPUData *pData);

} // namespace Perf

#endif
