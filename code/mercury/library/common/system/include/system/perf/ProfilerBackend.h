#ifndef __SYSTEM_PROFILER_BACKEND_H__
#define __SYSTEM_PROFILER_BACKEND_H__

#include <core/utility/utility.h>
#include <core/utility/singleton.h>
#include <core/containers/string.h>

/// this header file is to be included by .cpp files that want to read back and display generated profiling
/// .cpp files that just want to generate profiling data should include Profiler.h
namespace Perf
{

class Profiler;
class ProfilerCommon;

/// A single uncompressed event
struct Event
{
    const char *name;	// name of the event
    U32	    thread;		// thread id of the event
//	U8	    core;		// top 7 bits of 'time': processor id for the event
//	U8	    end;		// top bit of 'time': 0 = start of this event, 1 = end of this event
    U64	    time;		// time the event occurred
};

// for profiler.h
extern U32 g_iProfilerCurrentEvent;
extern int g_iProfilerNumEvents;
extern Event *g_pProfilerEvents;

/// accessor macros
#define PROFILER_EVENT_CORE(X) ((U8)((((X).time)>>56)&0x3f))
#define PROFILER_EVENT_TIME(X) ((U64)(((X).time) & 0xffffffffffffffULL))
#define PROFILER_EVENT_END(X)  ((U8)(((X).time& 0x8000000000000000ULL) >> 63))
#define PROFILER_EVENT_COUNTER(X)  ((((X).time& 0xC000000000000000ULL) == 0x4000000000000000ULL))
#define PROFILER_EVENT_EXTRAINFO(X)  ((((X).time& 0xC000000000000000ULL) == 0xC000000000000000ULL))
#define PROFILER_EVENT_SET(X,TIME,CORE,END) ((X).time = ((TIME) & 0xffffffffffffffULL) + ((U64)(CORE)<<56) + ((U64)(END)<<63))
#define PROFILER_COUNTER_SET(X) ((X).time = 0x4000000000000000ULL)

/// A single counter
struct Counter
{
    const char *name;
    U32	    value;
};

/// counter type enumerator
enum E_CounterType
{
    E_CounterType_User,
    E_CounterType_Event,
    E_CounterType_Memory,
};

/// list of events
class EventList
{
	friend class Profiler;
	friend class ProfilerCommon;
public:
	/// constructor
	EventList(Event *pEvents, int nEvents);

	/// array-style accessor
	Event &operator [](int i);

	/// returns the number of events in the list
	int Size() const;

private:
	Event	*m_pEvents;
	int		m_nEvents;
};

/// list of counters
class CounterList
{
	friend class Profiler;
	friend class ProfilerCommon;
public:

protected:
	/// constructor
	CounterList();
};

typedef void (*EndFrameCallbackPtr)(ProfilerCommon *pProfiler, void* pParam);

class CallbackData
{
	CallbackData(EndFrameCallbackPtr pFunction, void* pParam) : m_pFunction(pFunction), m_pParam(pParam) {};
	friend class Profiler;
	friend class ProfilerCommon;

	EndFrameCallbackPtr m_pFunction;
	void*				m_pParam;
};

/// profiler manager class
class ProfilerCommon : public Ut::Singleton<Profiler>
{
public:
    enum {
        E_Profiler_MaxEvents		= 64*1024,
		E_Profiler_EventOverflow	= 32*1024,
    };


	/// constructor
			ProfilerCommon();

	/// destructor
	virtual ~ProfilerCommon();

	/// retrieve singleton instance
	static Profiler &Instance();

    /// return a reference to a list of events
    EventList GetEventList();

    /// return the current value of a specific counter
    S32 GetCounterValue(const Util::C_String &name);

    /// return a reference to a list of counters
    const CounterList GetCounterList(E_CounterType type);

    /// notify the profile that this is the end of the current frame
    void EndFrame();

	/// mark a perf event
	void NamedEvent(const char *pszName, int iEnd);

	/// emit a perf event
	void EmitEvent(const char *pszName, U64 iTime, U32 iThread, U8 iCore, U8 iEnd);

	/// return a friendly name for the thread on an event
	virtual const char *ThreadGetFriendlyName(const Event &e);

	/// register a callback that will be called during the EndFrame function
	void RegisterCallback(EndFrameCallbackPtr callback, void* pParam = NULL);

	/// register a only once callback that will be called during the EndFrame function
	void RegisterOnlyOnceCallback(EndFrameCallbackPtr callback, void* pParam = NULL);


	/// remove a callback
	void UnregisterCallback(EndFrameCallbackPtr callback, void* pParam = NULL);

	/// return the index of the current frame
	int GetCurrentFrameIndex();

protected:
	/// return the index of this callback in the callback array (or -1 if it doesn't exist)
	int FindCallbackIdx(EndFrameCallbackPtr callback, void* pParam);
	/// return the index of this only once callback in the callback array (or -1 if it doesn't exist)
	int FindOnlyOnceCallbackIdx(EndFrameCallbackPtr callback, void* pParam);

	/// mutex
	MT::Mutex		m_Mutex;
	/// last frame's start event index
	int				m_iLastStartEvent;
	int				m_iLastEndEvent;
	// current frame GUID
	int				m_iFrame;

	Ut::Vector<CallbackData>	m_Callbacks;
	Ut::Vector<CallbackData>	m_OnceCallbacks;
};

/// initialise server that allows the PC profiler client to connect to this app
int InitialiseNetworkConnection();

} // namespace Perf

// include the correct implementation of the platform-specific profiler
#if defined(PLATFORM_PS3)
#include "ProfilerPS3.h"
#else
#include "ProfilerGeneric.h"
#endif

#endif
