#ifndef __SYSTEM_SIMPLETIMER_H__
#define __SYSTEM_SIMPLETIMER_H__

namespace Time
{

#ifndef CFG_GOLD

class C_SimpleTimer
{
public:
	C_SimpleTimer()	: m_StartTime( Time::GetPerformanceCounter() )
	{		
	}

	float Stop() const
	{
		const C_Time endTime	= Time::GetPerformanceCounter();
		const C_Time deltaTime	= endTime - m_StartTime;

		return (float)deltaTime / 1000.f;
	}

private:
	const C_Time m_StartTime;
};

#else

class C_SimpleTimer
{
public:
	C_SimpleTimer() {}
	float Stop() const { return 0.f; }
};

#endif

} // namespace Time

#endif