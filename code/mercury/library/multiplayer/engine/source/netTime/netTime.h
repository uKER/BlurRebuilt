#ifndef NETTIME_H
#define NETTIME_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netEnums/netEnums.h"
#include "System/Time/time.h"

typedef Time::C_Time netTime;
typedef Time::C_DecomposedTime netDate;

class netStopwatch
{
public:
	netStopwatch();
	~netStopwatch();
	
	void start();
	void stop();
	bool started() const { return m_started; }
	bool stopped() const { return !started(); }

	bool elapsed(netTime microseconds);
	bool ticked(netTime microseconds);
	netTime elapsedTime();

	netTime m_start;
	netTime m_now;
	bool    m_started;
};

class netClock
{
public:
	Time::C_Time now();		// time in microseconds (real or update depending on mode)
	U32 nowMilliseconds();	// time now in milliseconds
	F64 nowSeconds();		// time now in seconds
	
private:
	Time::C_Time m_realTime;		// real timing (off the performance counters).
	
private:
	static netClock* s_instance;
	netClock();
	virtual ~netClock();
public:
	static netClock* instance() { return s_instance; }
	static void createInstance() { assert(s_instance == NULL); s_instance = new netClock; }
	static void destroyInstance(){ if(s_instance) delete(s_instance); s_instance = NULL; }
};

#endif //NETTIME_H
