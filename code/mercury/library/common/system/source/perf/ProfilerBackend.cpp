#include "SystemPch.h"

#include "system/Perf/ProfilerEnable.h"

#include "System/Perf/Profiler.h"
#include "System/Time/Time.h"
#include "System/Memory/MemoryTracker.h"
#include "System/Perf/ProfilerBackend.h"
#include "Btl/Containers/vector.h"
#ifdef PLATFORM_PS3
#include <sys/ppu_thread.h>
#include <sys/time_util.h>
#include <sys/dbg.h>
#include <cell/atomic.h>
#if CELL_SDK_VERSION < 0x270000
#include "ppu_intrinsics_gcc.h"
#else
#include "ppu_intrinsics.h"
#endif
#endif


#include "System/Debug/Variables.h"

namespace Perf
{

U32 g_iProfilerCurrentEvent = 0;
int g_iProfilerNumEvents = -1;
Event *g_pProfilerEvents = NULL;


Debug::Logger &g_Log = Debug::GetLogger("system.perf");

/// constructor
EventList::EventList(Event *pEvents, int nEvents)
	: m_pEvents(pEvents)
	, m_nEvents(nEvents)
{
}

/// array-style accessor
Event &EventList::operator [](int i)
{
	return m_pEvents[i];
}

/// returns the number of events in the list
int EventList::Size() const
{
	return m_nEvents;
}




// constructor
CounterList::CounterList()
{
}


Profiler *g_pProfiler = NULL; // for debugging

#ifndef ENABLE_MERCURY_PROFILER

ProfilerCommon::ProfilerCommon()
{
	g_pProfilerEvents = NULL;
	g_iProfilerCurrentEvent = E_Profiler_MaxEvents;
}

ProfilerCommon::~ProfilerCommon()
{
}

Profiler &ProfilerCommon::Instance()
{
	return Ut::Singleton<Profiler>::Instance();
}

EventList ProfilerCommon::GetEventList()
{
	EventList rtn(0,0);
	return rtn;
}

S32 ProfilerCommon::GetCounterValue(const Util::C_String &name)
{
	return 0;
}

const CounterList ProfilerCommon::GetCounterList(E_CounterType type)
{
	return CounterList();
}

void ProfilerCommon::EndFrame()
{
}

void ProfilerCommon::NamedEvent(const char *pszName, int iEnd)
{
}

void ProfilerCommon::EmitEvent(const char *pszName, U64 iTime, U32 iThread, U8 iCore, U8 iEnd)
{
}

void ProfilerCommon::RegisterCallback(EndFrameCallbackPtr callback, void* /*pParam*/)
{
}

void ProfilerCommon::RegisterOnlyOnceCallback(EndFrameCallbackPtr callback, void* /*pParam*/)
{
}

void ProfilerCommon::UnregisterCallback(EndFrameCallbackPtr callback, void* /*pParam*/)
{
}

int ProfilerCommon::GetCurrentFrameIndex()
{
	return 0;
}


#else

/// constructor
ProfilerCommon::ProfilerCommon()
{
	m_iFrame = 0;
	m_iLastStartEvent = 0;
	m_iLastEndEvent = 0;
	g_pProfiler = (Profiler*)this;

	g_pProfilerEvents = new Event[E_Profiler_MaxEvents];
	g_iProfilerCurrentEvent = 0;

	#if defined(PLATFORM_PS3)
	__lwsync();
	#else
	// apparently unnecessary on pc.. jkc
	//ReleaseLockBarrier();
	#endif

	g_iProfilerNumEvents = E_Profiler_MaxEvents;
}

/// destructor
ProfilerCommon::~ProfilerCommon()
{
	delete [] g_pProfilerEvents;
	g_pProfilerEvents = NULL;
	g_iProfilerNumEvents = 0;
}

/// retrieve singleton instance
Profiler &ProfilerCommon::Instance()
{
	return Ut::Singleton<Profiler>::Instance();
}

/// return a reference to a list of events
EventList ProfilerCommon::GetEventList()
{
	EventList rtn(&g_pProfilerEvents[m_iLastStartEvent], m_iLastEndEvent - m_iLastStartEvent);
	return rtn;
}

/// return the current value of a specific counter
S32 ProfilerCommon::GetCounterValue(const Util::C_String &name)
{
	return 0;
}

/// return a reference to a list of counters
const CounterList ProfilerCommon::GetCounterList(E_CounterType type)
{
	return CounterList();
}

/// notify the profile that this is the end of the current frame
void ProfilerCommon::EndFrame()
{
	m_Mutex.Wait();

#if defined( PLATFORM_PS3 )
	// copy PS3 trace buffer contents into main event buffer
	((Profiler*)this)->ReadTraceData(m_Callbacks.Size() + m_OnceCallbacks.Size() == 0);
#endif

	int iLast = g_iProfilerCurrentEvent;
	if (iLast > E_Profiler_MaxEvents)
		iLast = E_Profiler_MaxEvents;

	// set the last frame's start and end event indices
	m_iLastStartEvent = 0;
	m_iLastEndEvent = iLast; 

	// run all callbacks to allow them to process the data
	for (int i=0; i<m_Callbacks.Size(); i++)
		m_Callbacks[i].m_pFunction(this, m_Callbacks[i].m_pParam);

	// run all only once callbacks to allow them to process the data
	for (int i=0; i<m_OnceCallbacks.Size(); i++)
		m_OnceCallbacks[i].m_pFunction(this, m_OnceCallbacks[i].m_pParam);

	m_OnceCallbacks.Clear();

	// restart at the beginning of the buffer
	g_iProfilerCurrentEvent = 0;
	m_iFrame++;

	m_Mutex.Release();
}

/// return the index of the current frame
int ProfilerCommon::GetCurrentFrameIndex()
{
	return m_iFrame;
}

/// return the index of this callback in the callback array (or -1 if it doesn't exist)
int ProfilerCommon::FindCallbackIdx(EndFrameCallbackPtr callback, void* pParam)
{
	for (int i=0; i<m_Callbacks.Size(); i++)
	{
		if ((m_Callbacks[i].m_pFunction == callback) && (m_Callbacks[i].m_pParam == pParam))
			return i;
	}

	return -1;
}

/// return the index of this only once callback in the callback array (or -1 if it doesn't exist)
int ProfilerCommon::FindOnlyOnceCallbackIdx(EndFrameCallbackPtr callback, void* pParam)
{
	for (int i=0; i<m_OnceCallbacks.Size(); i++)
	{
		if ((m_OnceCallbacks[i].m_pFunction == callback) && (m_OnceCallbacks[i].m_pParam == pParam))
			return i;
	}

	return -1;
}

/// register a callback that will be called during the EndFrame function
void ProfilerCommon::RegisterCallback(EndFrameCallbackPtr callback, void* pParam)
{
	int idx = FindCallbackIdx(callback, pParam);
	if (idx == -1)
		m_Callbacks.PushBack(CallbackData(callback, pParam));
}

/// register a only once callback that will be called during the EndFrame function
void ProfilerCommon::RegisterOnlyOnceCallback(EndFrameCallbackPtr callback, void* pParam)
{
	int idx = FindOnlyOnceCallbackIdx(callback, pParam);
	if (idx == -1)
		m_OnceCallbacks.PushBack(CallbackData(callback, pParam));
}

/// remove a callback
void ProfilerCommon::UnregisterCallback(EndFrameCallbackPtr callback, void* pParam)
{
	int idx = FindCallbackIdx(callback, pParam);
	m_Callbacks.EraseNonOrdered(idx);
}


#endif

}; // namespace Perf

