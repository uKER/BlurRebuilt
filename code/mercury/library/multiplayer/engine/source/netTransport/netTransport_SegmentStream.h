#ifndef NETTRANSPORT_SEGMENTSTREAM_H
#define NETTRANSPORT_SEGMENTSTREAM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netStream/netStream.h"
#include "netTransport_SegmentPool.h"
#include <bdCore/bdContainers/bdLinkedList.h>

namespace netTransport
{
	// records interval of data written to a stream.
	BD_REFERENCE(Intervals);
	class Intervals: public bdReferencable
	{
	public:
		Intervals();
		void clear();
		bool insert(int min, int max);
		bool completed(int min, int max) const;
		bool missingInterval(int& start, int& end, int limit) const;
		int continuityEnd() const;
		void debugList(const char* header);

		struct Interval
		{
			Interval(int min, int max)
			{ 
				assert(max >= min);
				m_min = min; m_max = max;
			}
			int m_min;
			int m_max;
		};
		// list of intervals we've registered
		bdLinkedList<Interval> m_intervals;
	};

	class SegmentStream: public netStream
	{
	public:
		// randomAcccess : allows to write runs of data at any point in the stream.
		// if data arrives out of order, then the stream will be in error status.
		SegmentStream(SegmentPoolRef segmentPool);
		~SegmentStream();

		virtual bool writeBytes_internal(const void* ptr, int len);
		virtual bool readBytes_internal(void* ptr, int len);
		virtual void clear();
		virtual bool completed(int bytes) const;
		
	protected:
		const Intervals& writeIntervals() const { return m_writeIntervals; }

	private:
		// record intervals of data written in the segmented stream.
		// this is used to determine if the stream is complete, or missing fragments.
		Intervals m_writeIntervals;

		// find a segment, given a pointer in the stream.
		Segment* segment(int ptr);

		// linked list of segments we use
		Segment* m_head;

		// the pool that allocates segments
		SegmentPoolRef m_segmentPool;
	};
}
#endif //NETTRANSPORT_SEGMENTSTREAM_H