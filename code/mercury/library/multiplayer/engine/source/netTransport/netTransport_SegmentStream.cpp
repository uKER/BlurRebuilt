#include "stdafx.h"
#include "netTransport_Enums.h"
#include "netTransport_SegmentStream.h"
#include "netEnums/netEnums.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	Intervals::Intervals()
	{
		clear();
	}

	void Intervals::clear()
	{
		m_intervals.clear();
	}

	void Intervals::debugList(const char* header)
	{
		char temp[512];
		int count = sprintf_s(temp, sizeof(temp), header);
			
		bdLinkedList<Interval>::Position pos = m_intervals.getHeadPosition();
		while(pos)
		{
			const Interval& i = m_intervals.forward(pos);
			
			if(sizeof(temp) - count < 20)
				break;

			count += sprintf_s(temp + count, sizeof(temp) - count, "(%d, %d)-", i.m_min, i.m_max);
		}

		if(count > 2 && count < sizeof(temp))
			temp[count-1] = '.';

		temp[sizeof(temp)-21] = 0;
		temp[sizeof(temp)-22] = '.';
		temp[sizeof(temp)-23] = '.';
		temp[sizeof(temp)-24] = '.';
		LOGINFO(Debugging::g_logger, temp);
	}

	bool Intervals::insert(int min, int max)
	{
		// buggered!
		if(min < 0 || max <= min)
		{
			assert(!"invalid interval!");
			return false;
		}

	#if 0//defined(CFG_DEBUG)
		if(m_intervals.isEmpty())
		{
			netDebug::logInfo(netDebug::g_TransportLog, "<netIntervals::insert()> inserting (%d, %d) in empty list.", min, max);
		}
		else
		{
			char header[128];
			sprintf_s(header, sizeof(header), "<netIntervals::insert()> inserting (%d, %d) in list ", min, max);
			debugList(header);
		}
	#endif


		// completely empty. add us, and we're done
		if(m_intervals.isEmpty())
		{
			m_intervals.addHead(Interval(min, max));
			return true;
		}

		// remove intervals we've intersected with. they'll be 'swallowed'
		// by the new interval.
		// also, expend range to the border intervals.
		bdLinkedList<Interval>::Position pos = m_intervals.getHeadPosition();
		while(pos)
		{
			Interval& i = m_intervals.getAt(pos);

			// no intersection, go to the next interval
			if(i.m_min > max || i.m_max < min)
			{
				m_intervals.forward(pos);
			}
			else
			{
				// we have intersection.
				// expand to the interval's borders.
				if(i.m_min < min)
					min = i.m_min;

				if(i.m_max > max)
					max = i.m_max;

				// remove the overlapped interval.
				m_intervals.removeAt(pos);
			}
		}

		// find where to slot the interval in the sorted list.
		pos = m_intervals.getHeadPosition();
		while(pos)
		{
			Interval& i = m_intervals.getAt(pos);
		
			// we have precedence over that one.
			if(i.m_min > min)
			{
				assert(i.m_min > max);
				m_intervals.insertBefore(pos, Interval(min, max));
				return true;
			}
			m_intervals.forward(pos);
		}
		// add to tail
		m_intervals.addTail(Interval(min, max));

	#if 0//defined(CFG_DEBUG)
		debugList("<netIntervals::insert()> result list ");
	#endif
		return true;
	}

	int Intervals::continuityEnd() const
	{
		if(m_intervals.isEmpty())
			return 0;

		const Interval& head = m_intervals.getHead();

		assert(head.m_min >=0 && head.m_max > head.m_min);

		// missing interval [0, head.m_min].
		if(head.m_min > 0)
			return 0;

		// we have a continuous interval from [0, head.m_max].
		return head.m_max;
	}

	bool Intervals::missingInterval(int& start, int& end, int limit) const
	{
		// no interval recorded
		if(m_intervals.isEmpty())
		{
			start = 0;
			end = limit;
			return false;
		}

		const Interval& i = m_intervals.getHead();

		// we're missing [0, min[.
		if(i.m_min > 0)
		{
			start = 0;
			end = i.m_min;
			return true;
		}

		// we've read everything from [0, limit[.
		if(limit <= i.m_max)
			return false;
		
		// find where to slot the interval in the sorted list.
		bdLinkedList<Interval>::Position pos = m_intervals.getHeadPosition();
		m_intervals.forward(pos);

		if(!pos)
		{
			start = i.m_max;
			end = limit;
			return true;
		}

		const Interval& s = m_intervals.getAt(pos);
		start = i.m_max;
		end = s.m_min;
		return true;
	}

	// find if the tange [min, max] has been completed without gaps.
	bool Intervals::completed(int min, int max) const
	{
		if(max < min) 
			return false;

		// no interval recorded
		if(m_intervals.isEmpty())
			return false;
		
		// find where to slot the interval in the sorted list.
		bdLinkedList<Interval>::Position pos = m_intervals.getHeadPosition();
		while(pos)
		{
			const Interval& i = m_intervals.forward(pos);

			// over the range we query.
			if(i.m_max < min)
				return false;

			// we fit in that one.
			// all good.
			if(min >= i.m_min && max <= i.m_max)
				return true;
		}
		return false;
	}

	SegmentStream::SegmentStream(SegmentPoolRef segmentPool)
	: netStream(0)
	, m_segmentPool(segmentPool)
	{
		m_head = NULL;
		clear();
	}

	SegmentStream::~SegmentStream()
	{
		clear();
	}

	Segment* SegmentStream::segment(int ptr)
	{
		if(ptr < 0)
			return NULL;

		if(m_head == NULL)
		{
			m_head = m_segmentPool->acquire();
			m_head->m_next = NULL;
		}
		Segment* item = m_head;
		int segmentId = ptr / SEGMENT_SIZE;
		
		for(int i = 0; i < segmentId; i ++, item = item->m_next)
		{
			// allocate buffers if needed
			Segment* nextItem = item->m_next;
			if(nextItem == NULL)
			{
				nextItem = m_segmentPool->acquire();
				item->m_next = nextItem;
				m_limit += SEGMENT_SIZE;
			}
		}
		return item;
	}

	bool SegmentStream::readBytes_internal(void* ptr, int len)
	{
		if(!ok()) 
			return false;

		if(len <= 0) 
			return true;

		if(byteReadOverflow(len))
		{
			LOGFATAL(Debugging::g_logger, "<SegmentStream> read overflow.");
			setStatus(netStream::STATUS_FAILED_READ_OVERFLOW);
			return true;
		}

		if(!m_writeIntervals.completed(getReadByteCount(), getReadByteCount() + len))
		{
			LOGFATAL(Debugging::g_logger, "<SegmentStream> the data has gaps in it. cant read it.");
			setStatus(netStream::STATUS_FAILED_READ);
			return true;
		}

		Segment* seg = segment(getReadByteCount());
		if(seg == NULL) return NULL;
		
		int readptr		= (getReadByteCount() % SEGMENT_SIZE);
		int readlimit	= (SEGMENT_SIZE - readptr);
		char* readbuff	= seg->m_buffer + readptr;
		int toread		= (readlimit > len)? len : readlimit;

		if(toread > 0)
		{
			memcpy(ptr, readbuff, toread);
		}

		// push read pointer.
		int startReadPtr = getReadByteCount();
		int newReadPtr = startReadPtr + toread;
		assert(newReadPtr <= getWriteByteCount());
		setReadByteCount(newReadPtr);

		// read next batch
		if((len - toread) > 0)
		{	
			readBytes_internal(((char*)ptr) + toread, (len - toread));
		}
		return ok();
	}

	bool SegmentStream::writeBytes_internal(const void* ptr, int len)
	{
		if(!ok()) 
			return false;
		if(len <= 0) 
			return true;

		Segment* seg = segment(getWriteByteCount());
		if(seg == NULL) return NULL;

		int writeptr	= (getWriteByteCount() % SEGMENT_SIZE);
		int writelimit	= (SEGMENT_SIZE - writeptr);
		char* writebuff	= seg->m_buffer + writeptr;
		int towrite		= (writelimit > len)? len : writelimit;

		if(towrite > 0)
		{
			memcpy(writebuff, ptr, towrite);
		}
		
		// push write pointer.
		int startWritePtr = getWriteByteCount();
		int newWritePtr = startWritePtr + towrite;
		if(newWritePtr > getWriteByteLimit())
			setWriteByteLimit(newWritePtr);

		setWriteByteCount(newWritePtr);
		m_writeIntervals.insert(startWritePtr, newWritePtr);

		// read next batch
		if((len - towrite) > 0)
		{			
			return writeBytes_internal(((const char*)ptr) + towrite, (len - towrite));
		}
		return ok();
	}

	bool SegmentStream::completed(int bytes) const
	{
		return (m_writeIntervals.continuityEnd() >= bytes);
		// m_writeIntervals.completed(0, bytes); // same thing
	}

	void SegmentStream::clear()
	{
		m_writeIntervals.clear();

		while(m_head)
		{
			Segment* next = m_head->m_next;
			m_head->m_next = NULL;
			m_segmentPool->release(m_head);
			m_head = next;
		}
		m_head = NULL;

		netStream::clear();
		setWriteByteLimit(0);
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif