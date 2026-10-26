
#include "stdafx.h"
#include "netTransport_TrafficLogger.h"
#include "netTime/netTime.h"
#include "netEnums/netEnums.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	TrafficSample::TrafficSample()
	{
		m_limit = 0.0f;
		m_timeStart = netClock::instance()->now();
		m_timeEnd = netClock::instance()->now();
		memset(m_bytes, 0, sizeof(m_bytes));
	}

	bool TrafficSample::operator /= (int count)
	{
		if(count == 0)
			return false;

		m_limit /= count;

		for(int i = 0; i < TRAFFIC_TYPE_COUNT; i ++)
		{
			m_bytes[i] /= (float) count;
		}
		return true;
	}

	bool TrafficSample::operator += (const TrafficSample& other)
	{			
		if(other.m_timeEnd > m_timeEnd)
			m_timeEnd = other.m_timeEnd;

		if(other.m_timeStart < m_timeStart)
			m_timeStart = other.m_timeStart;

		m_limit += other.m_limit;
		
		for(int i = 0; i < TRAFFIC_TYPE_COUNT; i ++)
		{
			m_bytes[i] += other.m_bytes[i];
		}
		return true;
	}

	void TrafficSample::peak(const TrafficSample& other)
	{
		if(other.m_timeEnd > m_timeEnd)
			m_timeEnd = other.m_timeEnd;

		if(other.m_timeStart < m_timeStart)
			m_timeStart = other.m_timeStart;

		if(other.m_limit > m_limit)
		{
			m_limit = other.m_limit;
		}

		for(int i = 0; i < TRAFFIC_TYPE_COUNT; i ++)
		{
			if(other.m_bytes[i] > m_bytes[i])
			{
				m_bytes[i] = other.m_bytes[i];
			}
		}
	}

	bool TrafficSample::normalise()
	{
		float period = (float)(m_timeEnd - m_timeStart) / (1000.0f * 1000.0f);
		if(period < 1.0f)
			period = 1.0f;

		if(period > 0.000001f)
		{
			for(int i = 0; i < TRAFFIC_TYPE_COUNT; i ++)
			{
				m_bytes[i] = m_bytes[i] / period;
			}
		}
		return true;
	}

	void TrafficSample::setLimit(float limit)
	{
		m_limit = limit;
	}

	void TrafficSample::log(unsigned int bytes, int type)
	{
		if(type >= 0 && type < TRAFFIC_TYPE_COUNT)
		{
			m_bytes[type] += bytes;
			m_timeEnd = netClock::instance()->now();
		}
	}

	TrafficLogger::TrafficLogger()
	{
		clear();
	}

	TrafficLogger::~TrafficLogger()
	{
		clear();
	}
	
	void TrafficLogger::clear()
	{
		m_ptr = 0;
		m_samples[(m_ptr & HISTORY_SIZE)] = TrafficSample();
	}

	void TrafficLogger::poll()
	{
		m_ptr++;
		m_samples[(m_ptr % HISTORY_SIZE)] = TrafficSample();
	}

	void TrafficLogger::log(unsigned int bytes, int type)
	{
		m_samples[(m_ptr % HISTORY_SIZE)].log(bytes, type);
	}

	void TrafficLogger::getTraffic(U64 period, TrafficSample& accum) const
	{
		accum = TrafficSample();
		
		unsigned int count = (m_ptr > HISTORY_SIZE)? HISTORY_SIZE : m_ptr;

		U64 now = netClock::instance()->now();
		U64 timestamp = (now > period)? (now - period) : 0;
		
		int rejected=0;
		for(unsigned int i = 0; i < count; i ++)
		{
			const TrafficSample* s = &(m_samples[i]);
			
			if(s->m_timeEnd >= timestamp)
			{
				accum += *s;
			}
			else
			{
				rejected++;
			}
		}
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

