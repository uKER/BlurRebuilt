#ifndef _CPUCOUNTERS_
#define _CPUCOUNTERS_

#ifndef CPU_COUNTERS_ENABLED
#define CPU_COUNTER_BEGIN(x) {((void)0);}
#define CPU_COUNTER_END(x) {((void)0);}
#else
#define CPU_COUNTER_BEGIN(x) m_aCPUCounters[x].Begin()
#define CPU_COUNTER_END(x) m_aCPUCounters[x].End()
#endif

#ifdef NAMED_COUNTERS_ENABLED
#define NAMED_CPU_COUNTER_BEGIN(x)  { if (g_bNamedCountersEnabled) System::Get()->GetNamedCpuCounters().BeginEvent(x, StringHash(x)); }
#define NAMED_CPU_COUNTER_END(x) { if (g_bNamedCountersEnabled) System::Get()->GetNamedCpuCounters().EndEvent(x, StringHash(x)); }
#else
#define NAMED_CPU_COUNTER_BEGIN(x) {((void)0);}
#define NAMED_CPU_COUNTER_END(x) {((void)0);}
#endif

#ifdef NAMED_COUNTERS_ENABLED
extern Debug::Bool g_bNamedCountersUseInstanceNames;
extern Debug::Bool g_bNamedCountersUseEntityNames;
extern bool g_bNamedCountersEnabled;
#endif

#ifdef NAMED_COUNTERS_ENABLED
#define OBJECT_CPU_COUNTER_BEGIN(x) \
	{ \
		if (g_bNamedCountersEnabled) \
		{ \
			if (g_bNamedCountersUseInstanceNames) \
			System::Get()->GetNamedCpuCounters().BeginEvent(x->GetObject()->Name(), x->GetObject()->m_uNameHash); \
			else \
			if (g_bNamedCountersUseEntityNames) \
				System::Get()->GetNamedCpuCounters().BeginEvent(x->EntityName(), x->m_uEntityNameHash); \
			else \
				System::Get()->GetNamedCpuCounters().BeginEvent(x->GroupName(), x->m_uGroupNameHash); \
		} \
	}
#else
#define OBJECT_CPU_COUNTER_BEGIN(x) {((void)0);}
#endif

#ifdef NAMED_COUNTERS_ENABLED
#define OBJECT_CPU_COUNTER_END(x) \
	{ \
		if (g_bNamedCountersEnabled) \
		{ \
			if (g_bNamedCountersUseInstanceNames) \
				System::Get()->GetNamedCpuCounters().EndEvent(x->GetObject()->Name(), x->GetObject()->m_uNameHash); \
			else \
			if (g_bNamedCountersUseEntityNames) \
				System::Get()->GetNamedCpuCounters().EndEvent(x->EntityName(), x->m_uEntityNameHash); \
			else \
				System::Get()->GetNamedCpuCounters().EndEvent(x->GroupName(), x->m_uGroupNameHash); \
		} \
	}
#else
#define OBJECT_CPU_COUNTER_END(x) {((void)0);}
#endif

#include "Common\StopWatch.h"

#ifdef NAMED_COUNTERS_ENABLED
#include "btl/containers/map.h"
#endif

namespace Audio
{
	enum CpuCounters
	{
		eCpu_EntireSystem=0,
		eCpu_MessageQueue,
		eCpu_FileStreamer,
		eCpu_ObjectManagerUpdateAndRender,
		eNumCpuCounters,
	};

////////////////////////////////////////////////////////////////////////////////
//	Counter
//		not thread safe
////////////////////////////////////////////////////////////////////////////////
#ifdef CPU_COUNTERS_ENABLED
	class Counter
	{
	public:
		Counter() : fTime(0.f), uCount(0) {}

		void Begin()
		{
			swTimer.Start();
		}

		void End(bool bContinueSampling = false)
		{
			fTime += swTimer.Time();
			if (!bContinueSampling)
				uCount++;
		}

		F32 Value() const
		{
			if (uCount)
			{
				F32 fVal = fTime / (F32)uCount;	// divide time by number of times end was called
				fTime = 0.f;
				uCount = 0;
				return fVal;
			}
			else
				return 0.f;
		}

	protected:
		StopWatch swTimer;
		mutable F32 fTime;
		mutable U32 uCount;
	};
#else
	class Counter
	{
	};
#endif	//#ifdef CPU_COUNTERS_ENABLED


#ifdef NAMED_COUNTERS_ENABLED
////////////////////////////////////////////////////////////////////////////////
//	NamedCounter
//		not thread safe
//	can call BeginEvent and EndEvent multiple times in a frame and will accumlate
//	timings. at the end of the frame call EndAllEvents.
////////////////////////////////////////////////////////////////////////////////
	class NamedCounter : public Counter
	{
	public:
		NamedCounter(const char* pchLabel) { strLabel = pchLabel; }
		void EndEvents() { uCount++; }

		Btl::String strLabel;
	};

	typedef Btl::Map<StringHash, NamedCounter*> C_NamedCounterHashMap; 

	class NamedCpuCounters
	{
	public:
		~NamedCpuCounters();
		
		// for recording events.
		void BeginEvent(const char* pchLabel, const StringHash hash);
		void EndEvent(const char* pchLabel, const StringHash hash); 
		void EndAllEvents();
		
		// for retrieving recorded data on events
		S32 GetNumCounters() const { return m_counterMap.Size();  }
		const Btl::String GetEventName(S32 index); 
		F32 GetEventCpuTime(S32 index); 

		// for clearing all tags/labels
		void Clear();

	private:
		NamedCounter* GetNamedCounter(const char* pchLabel, const StringHash hash); 

	private:
		C_NamedCounterHashMap m_counterMap;
	};
#else
	class NamedCpuCounters
	{
	};
#endif

}	// namespace Audio

#endif // _CPUCOUNTERS_
