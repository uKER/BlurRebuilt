#ifndef __SYSTEM_SCOPEDTIMER_H__
#define __SYSTEM_SCOPEDTIMER_H__

namespace Time
{

#ifndef CFG_GOLD

#define SCOPED_TIMER(name) Time::C_ScopedTimer scopedTimer(name)

class C_ScopedTimer
{
public:
	C_ScopedTimer(const char* name, F32 logThresholdMs = 0.f);
	~C_ScopedTimer();

private:
	const U64				m_StartTime;
	const char*				m_Name;
	F32						m_LogThresholdMs;
};

#else

#define	SCOPED_TIMER

class C_ScopedTimer
{
public:
	C_ScopedTimer(const Util::C_String& name) {}
	~C_ScopedTimer() {}
};

#endif

} // namespace Time

#endif