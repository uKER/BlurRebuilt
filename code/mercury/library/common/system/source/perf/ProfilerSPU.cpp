#include <core/Core.h>
#include <core/Containers.h>
#include <system/DebugLog.h>
#include "System/Perf/ProfilerEnable.h"
#include "System/Perf/ProfilerBackend.h"
#include "system/scheduler/jobqueue.h"
#include "system/scheduler/jobqueue_ps3.h"
#include <cell/atomic.h>
#include <cell/spurs.h>
#ifdef PLATFORM_SPU
#include <cell/dma.h>
#endif

namespace Perf
{

#ifdef PLATFORM_SPU

static SPUData* const _pSPULocalBuffer = &Scheduler::_jqThisWorker->profilerInfo;
static int &  _iCurrentEvent = ((Scheduler::_jqWorker*)JQ_KERNEL_WORKER)->profilerInfo.iCurrentEvent;

int g_iDMAWaitTime = 0;

#ifdef ENABLE_MERCURY_PROFILER

inline void _FlushEventBuffer()
{
	if (_iCurrentEvent == 0)
		return;

	int &  _piCurrentEvent = ((Scheduler::_jqWorker*)JQ_KERNEL_WORKER)->profilerInfo.piCurrentEvent;
	int &  _pEventBuffer = ((Scheduler::_jqWorker*)JQ_KERNEL_WORKER)->profilerInfo.pEventBuffer;

	uint32_t temp[32] __attribute__((aligned(128)));
	U32 iEvent = cellAtomicAdd32(temp, (uint64_t)_piCurrentEvent, _iCurrentEvent);
	if (iEvent+_iCurrentEvent<ProfilerCommon::E_Profiler_MaxEvents)
	{
		cellDmaPut((void*)_pSPULocalBuffer->eventBuffer, iEvent*sizeof(Event) + (U32)_pEventBuffer, sizeof(Event)*_iCurrentEvent, 0, 0, 0);
		cellDmaWaitTagStatusAll(1 << 0);
	}

	_iCurrentEvent = 0;
}

inline void EmitEvent(const char *pszName, int iEnd)
{
	// add an event to the buffer
	Perf::Event &ev = _pSPULocalBuffer->eventBuffer[_iCurrentEvent];
	ev.name = pszName;
	U64 time = ~((uint32_t) spu_read_decrementer());
	int x = Scheduler::_jqThisWorker->Processor;
	int iCore = x + 3;
	ev.time = (time & 0xffffffffffffffULL) + ((U64)iCore << 56) + ((U64)iEnd << 63);
	ev.thread = x;
	_iCurrentEvent++;

	// if the buffer is full, DMA the contents to main memory and start again
	if (_iCurrentEvent >= EVENT_BUFFER_SIZE)
		_FlushEventBuffer();

	Perf::Event &ev2 = _pSPULocalBuffer->eventBuffer[_iCurrentEvent];
	ev2.name = NULL;
	ev2.time = 0xC000000000000000ULL + g_iDMAWaitTime;
	g_iDMAWaitTime = 0;
	ev2.thread = 0;
	_iCurrentEvent++;

	// if the buffer is full, DMA the contents to main memory and start again
	if (_iCurrentEvent >= EVENT_BUFFER_SIZE)
		_FlushEventBuffer();
}

void BeginNamedEvent(const char *pszName)
{
#ifndef CFG_SPEED
	if ((int)Scheduler::_jqThisWorker->PPUModuleAddress < (256*1024))
		DebugBreak();
#endif
	EmitEvent((char*)(pszName+(int)Scheduler::_jqThisWorker->PPUModuleAddress), 0);
}

void EndNamedEvent()
{
	EmitEvent(0, 1);
}

void FlushEventBuffer()
{
	_FlushEventBuffer();
}

#endif // ENABLE_MERCURY_PROFILER

#endif // PLATFORM_SPU

#ifdef PLATFORM_PS3

#include "system/Perf/ProfilerBackend.h"

// PPU side function to fill in the data structure
void SetSPUData(SPUData *pData)
{
	pData->iCurrentEvent = 0;
	pData->pEventBuffer = (int)Perf::g_pProfilerEvents;
	pData->piCurrentEvent = (int)&Perf::g_iProfilerCurrentEvent;
}

#endif // PLATFORM_PS3

} // namespace Perf

