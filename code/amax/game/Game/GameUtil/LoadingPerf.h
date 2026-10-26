#ifndef GAME_LOADINGPERF_H
#define GAME_LOADINGPERF_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef CFG_GOLD
//#define ENABLE_LOAD_PERF
#endif

#ifdef ENABLE_LOAD_PERF

// this system is a bit rubbish 

namespace LoadPerf
{
	// C_Event
	class C_Event
	{
	public:
		C_Event(char* pName);
		~C_Event();
		void Begin();
		void End();
		C_Event* Push(char* name);
		C_Event* Pop();
		void Dump(const Util::C_String& prefix);
		C_Event* GetParent() const;
		const Util::C_String& GetName() const;

	private:
		Util::C_String m_Name;
		Time::C_Time m_TimeBegin;
		Time::C_Time m_TimeEnd;

		C_Event* m_pParent;
		Ut::Vector<C_Event*> m_ChildEvents;	
	};

	bool ProfilingStarted();

	void Push(char* name);
	void Pop();
	void Pop(char* name);

	// C_ScopedEvent
	class C_ScopedEvent
	{
	public:
		C_ScopedEvent(char* name);
		~C_ScopedEvent();
	};

} // namespace LoadPerf

#define LOADING_PERF(X) LoadPerf::C_ScopedEvent loadPerf(X)

#else

namespace LoadPerf
{

inline void Push(char* name) {}
inline void Pop() {}
inline void Pop(char* name) {}

// C_ScopedEvent
class C_ScopedEvent
{
public:
	C_ScopedEvent(char* name) {}
	~C_ScopedEvent() {}
};

} // namespace LoadPerf

#define LOADING_PERF(X) {}

#endif

#endif // GAME_LOADINGPERF_H
