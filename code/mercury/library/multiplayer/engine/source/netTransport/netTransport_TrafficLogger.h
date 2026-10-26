#ifndef NETTRANSPORT_TRAFFICLOGGER_H
#define NETTRANSPORT_TRAFFICLOGGER_H

#ifdef _MSC_VER
#pragma once
#endif


#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include "netTransport_Enums.h"

namespace netTransport
{
	class TrafficSample
	{
	public:
		TrafficSample();
		
		void log(unsigned int bytes, int type);
		void setLimit(float limit);

		bool normalise();
		void peak(const TrafficSample& other);

		float limit() const { return m_limit; }

		bool operator += (const TrafficSample& other);
		bool operator /= (int count);

		float m_limit;

		U64 m_timeStart;
		U64 m_timeEnd;
		float m_bytes[TRAFFIC_TYPE_COUNT];
	};
			
	BD_REFERENCE(TrafficLogger);
	class TrafficLogger: public bdReferencable
	{
	public:
		TrafficLogger();
		~TrafficLogger();

		void clear();
		void poll();
		void log(unsigned int bytes, int type);
		void getTraffic(U64 period, TrafficSample& accum) const;
				
		unsigned int m_ptr;
		enum { HISTORY_SIZE = 32 };
		TrafficSample m_samples[HISTORY_SIZE];
	};
}
#endif //NETTRANSPORT_TRAFFICLOGGER_H