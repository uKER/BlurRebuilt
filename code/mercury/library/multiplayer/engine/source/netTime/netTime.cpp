#include "stdafx.h"
#include "netTime/netTime.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netStopwatch::netStopwatch()
{
	stop();
}

netStopwatch::~netStopwatch()
{
}

void netStopwatch::start()
{
	m_started = true;
	if(netClock::instance())
		m_start	= m_now = netClock::instance()->now();
}

void netStopwatch::stop()
{
	m_started = false;
	if(netClock::instance())
		m_start = m_now = netClock::instance()->now();
}

bool netStopwatch::ticked(netTime microseconds)
{
	// first immediate tick.
	if(!started()) 
	{
		start();
		return true;
	}

	netTime time = elapsedTime();
	
	if(time > microseconds)
	{
		start();
		return true;
	}
	else
	{
		return false;
	}
}

bool netStopwatch::elapsed(netTime microseconds)
{
	if(!started())
		return false;

	netTime time = elapsedTime();
	return (time > microseconds);
}

netTime netStopwatch::elapsedTime()
{
	if(!netClock::instance())
		return 0;

	m_now = netClock::instance()->now();
	assert(m_now >= m_start);
	return (m_now - m_start);
}

netClock* netClock::s_instance = NULL;

netClock::netClock()
: m_realTime(Time::GetPerformanceCounter())
{}

netClock::~netClock()
{}

U32 netClock::nowMilliseconds()
{
	Time::C_Time t = now() / 1000;
	return (U32)(t & (0xFFFFFFFF));
}

F64 netClock::nowSeconds()
{
	F64 seconds = (now() / (1000.0f * 1000.0f));
	return seconds;
}

Time::C_Time netClock::now()
{
	m_realTime = Time::GetPerformanceCounter();
	return m_realTime;
}
	
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

