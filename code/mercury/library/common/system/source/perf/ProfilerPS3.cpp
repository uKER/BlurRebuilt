#include "SystemPch.h"

#include <sys/ppu_thread.h>
#include <sys/time_util.h>
#include <sys/dbg.h>
#include <cell/perf/performance.h>

#include "System/Perf/ProfilerEnable.h"
#include "System/Perf/ProfilerBackend.h"
#include "System/Perf/Profiler.h"

#define ENABLE_LV2_OS_TRACE

namespace Perf
{

#ifndef ENABLE_MERCURY_PROFILER

/// constructor
Profiler::Profiler()
{
}

// destructor
Profiler::~Profiler()
{
}

const char *Profiler::ThreadGetFriendlyName(const Event &e)
{
	return "";
}

#else

extern Debug::Logger &g_Log;

struct TraceHeader
{
	U8 tag;
	U8 length;
	U16 hardwareId;
	U32 time;
};

struct SchedulerTraceData : public TraceHeader
{
	U32 threadId;
	U8	incident;
	U8	reserved;
	U16	priority;
};

struct ControlTraceData : public TraceHeader
{
	U32	control;
	U32 reserved;
};

struct ThreadTraceData : public TraceHeader
{
	U32  processId;
	U32  threadId;
	U8   cName;
	U8   incident;
	U16  reserved;
	char name[1];	// can be up to 255 characters
};


/// constructor
Profiler::Profiler()
{
#ifdef ENABLE_LV2_OS_TRACE
	InitialiseTraceBuffer();

	AddLv2OSTrace();

	cellPerfStart();
#endif
}

// destructor
Profiler::~Profiler()
{
}

/// initialise libperf trace buffer
void Profiler::InitialiseTraceBuffer()
{
	int ret;

	m_iTraceCacheSize = 64*1024; // 64K OS trace buffer size

	// Setting the performance monitor parameters
	CellPerfCBEpmSetup setup;
	memset(&setup, 0, sizeof(CellPerfCBEpmSetup));
	setup.conf = CELL_PERF_SETUP_CONF_BUFFER_TYPE_EXTERNAL | CELL_PERF_SETUP_CONF_COUNTER8;
	setup.interval = 0xff;

	// Enabling CBE Performance Monitor module
	ret = cellPerfAddCBEpm(&setup);
	if (ret < 0)
	{
		LOGERROR( g_Log, "cellPerfAddCBEpm returns error (%08x)\n", ret);
		return;
	}
}

/// (re)add Lv2 OS trace buffers
void Profiler::AddLv2OSTrace()
{
	int ret;

	// Enabling Lv2 OS PPU scheduler trace
	ret = cellPerfAddLv2OSTrace(CELL_PERF_LV2_TRACE_TYPE_PPU_SCHEDULER,
								m_iTraceCacheSize,						// Trace Buffer size
								CELL_PERF_LV2_TRACE_MODE_STOP_AT_END,	// Trace mode
								&m_pSchedTraceCache);
	if (ret < 0)
	{
		LOGERROR( g_Log, "cellPerfAddLv2OSTrace returns error (%08x)\n", ret);
	}

	// Enabling Lv2 OS PPU thread trace
	ret = cellPerfAddLv2OSTrace(CELL_PERF_LV2_TRACE_TYPE_PPU_THREAD,
								m_iTraceCacheSize,						// Trace Buffer size
								CELL_PERF_LV2_TRACE_MODE_STOP_AT_END,	// Trace mode
								&m_pThreadTraceCache);
	if (ret < 0)
	{
		LOGERROR( g_Log, "cellPerfAddLv2OSTrace returns error (%08x)\n", ret);
	}
}

///	read trace data from libperf
void Profiler::ReadTraceData(bool bClearOnly)
{
#ifdef ENABLE_LV2_OS_TRACE
	m_ThreadMap.Clear();

	// stop tracing
	cellPerfStop();

	if (!bClearOnly)
	{
		ProcessTraceData(m_pSchedTraceCache);

		ProcessTraceData(m_pThreadTraceCache);
	}

	cellPerfDeleteAllLv2OSTrace();

	AddLv2OSTrace();

	cellPerfStart();
#endif
}

/// return a friendly name for the thread on an event
const char *Profiler::ThreadGetFriendlyName(const Event &e)
{
	int iCore = PROFILER_EVENT_CORE(e);
	if (iCore >=3)
		return "SPU Thread";
	else if (iCore == 0)
		return "Graphics";

	Ut::UtMap<U32,Util::C_String>::Iterator iter = m_ThreadMap.Find(e.thread);
	if (iter == m_ThreadMap.End())
		return ProfilerCommon::ThreadGetFriendlyName(e);

	return iter->GetValue();
}


/// copy the contents of the thread trace buffer into the event buffer
void Profiler::ProcessTraceData(U32 *pTraceCache)
{
	int ret;

	// get trace data
	CellPerfStatus status;

	ret = cellPerfGetCBEpmStatus(&status);
	if (ret < 0)
	{
		// dw - this always fails on test kits. removed to stop tty spamage.
		//LOGERROR( g_Log, "cellPerfGetCBEpmStatus returns an error (%08x)\n", ret);
		return;
	}

	// collect thread trace events from the trace buffer and copy them into the event buffer
	U8 *pTrace = ((U8*)pTraceCache) + 128; // first 128 bytes contains header
	U8 *pEndTrace = ((U8*)pTraceCache) + m_iTraceCacheSize;
	U64 curTime = status.startSystemTime;

	U64 highTime = 0;
	U64 lowTime = 0xffffffffffffffffULL;

	curTime -= 0x80000000ULL;
	while (pTrace < pEndTrace)
	{
		TraceHeader *p = (TraceHeader*)pTrace;
		U8 c = p->length;
		if (p->tag == 0xf0)		// control packet
		{
			// control packet
			ControlTraceData *pC = (ControlTraceData*)pTrace;
			if (pC->control == 2)
				break;	// encountered trace end event
		}
		else if (p->tag == 0)	// PPU scheduler trace
		{
			// found a PPU thread trace event
			SchedulerTraceData *pS = (SchedulerTraceData*)pTrace;

			static const int iLookup[] = { 0, 0, 1, 1, 1, 1 };

			// get an event
			int iEvent = Perf::g_iProfilerCurrentEvent++;
			if (iEvent>=E_Profiler_MaxEvents)
				break;

			// construct an event
			Event *pEv = &Perf::g_pProfilerEvents[iEvent];
			pEv->thread = pS->threadId;
			if (p->time < (curTime & 0xffffffffULL))
			{
				curTime += 0x100000000ULL;
				curTime &= 0xffffffff00000000ULL;
			}
			U64 longTime = (U64)p->time | (curTime & 0xffffffff00000000ULL);
			curTime = longTime;
			PROFILER_EVENT_SET(*pEv, longTime, p->hardwareId+1, iLookup[pS->incident]);
			pEv->name = (char*)-1;
		}
		else if (p->tag == 0x03)	// PPU thread trace
		{
			ThreadTraceData *pT = (ThreadTraceData*)pTrace;

			m_ThreadMap[pT->threadId] = Util::C_String(pT->name);
		}

		pTrace += (2 + c) * 4;
	}
}

#endif

};
