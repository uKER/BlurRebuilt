
#include "stdafx.h"
#include "netTransport_ConnGuid.h"
#include "./netTransport_Enums.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	ConnGuidGenerator::ConnGuidGenerator(const ConnGuid& first, const ConnGuid& last, U64 historyTimeout)
	: m_guidFirst(first)
	, m_guidLast(last)
	, m_historyTimeout(historyTimeout)
	{
		assert(first >= ConnGuid(CONN_GUID_ASSIGNED_FIRST));
		assert(last <= ConnGuid(CONN_GUID_ASSIGNED_LAST));
		assert(last >= first);

		reset();
	}

	bool ConnGuidGenerator::reset()
	{
		m_tickBase = Time::GetPerformanceCounter();

		for(int i = 0; i < RECORD_COUNT; i ++)
		{
			m_records[i] = Record();
		}
		return true;
	}

	ConnGuidGenerator::Record* ConnGuidGenerator::record(ConnGuid guid)
	{
		if(guid < m_guidFirst || guid > m_guidLast)
			return NULL;

		int index = (guid.m_guid - m_guidFirst.m_guid);
		return &(m_records[index]);
	}

	ConnGuid ConnGuidGenerator::guid(const ConnGuidGenerator::Record* record)
	{
		if(record < &m_records[0] || record >= &m_records[RECORD_COUNT])
			return ConnGuid();

		int index = (record - m_records);
		return ConnGuid(m_guidFirst.m_guid + index);
	}

	bool ConnGuidGenerator::secondsElapsed(U32 timestamp, U32 seconds)
	{
		return timestamp > (seconds * Record::TICKS_PER_SEC);
	}

	U32 ConnGuidGenerator::timestamp()
	{
		return (U32)(((Time::GetPerformanceCounter() - m_tickBase) * (U64)Record::TICKS_PER_SEC) / (1000L * 1000L));
	}

	bool ConnGuidGenerator::release(ConnGuid guid)
	{
		if(guid < m_guidFirst || guid > m_guidLast)
			return false;

		Record* r = record(guid);
		assert(r != NULL);

		switch(r->m_state)
		{
		default:
			{
				break;
			}
		case Record::STATE_FREE:
			{
				LOGWARN(Debugging::g_logger, "<ConnGuidGenerator::release()> guid slot (%d) already free.", guid.m_guid);
				break;
			}
		case Record::STATE_HISTORY:
			{
				LOGWARN(Debugging::g_logger, "<ConnGuidGenerator::release()> guid slot (%d) already history.", guid.m_guid);
				break;
			}
		}
		
		LOGINFO(Debugging::g_logger, "<ConnGuidGenerator::release()> guid slot (%d) is now HISTORY.", guid.m_guid);
				
		r->m_timestamp = timestamp();
		r->m_state = Record::STATE_HISTORY;
		return true;
	}

	bool ConnGuidGenerator::reserve(ConnGuid guid)
	{
		if(guid < m_guidFirst || guid > m_guidLast)
			return false;

		Record* r = record(guid);
		assert(r != NULL);

		switch(r->m_state)
		{
		default:
			{
				break;
			}
		case Record::STATE_RESERVED:
			{
				LOGINFO(Debugging::g_logger, "<ConnGuidGenerator::reserve()> guid slot (%d) RESERVED actualised.", guid.m_guid);
				break;
			}
		}
		
		LOGINFO(Debugging::g_logger, "<ConnGuidGenerator::reserve()> guid slot (%d) is now RESERVED.", guid.m_guid);
				
		r->m_timestamp = timestamp();
		r->m_state = Record::STATE_RESERVED;
		return true;
	}

	ConnGuid ConnGuidGenerator::generate()
	{
		Record* historySlot=NULL;
		Record* freeSlot=NULL;
		Record* reservedSlot=NULL;
		Record* slot=NULL;

		for(int i = m_guidFirst.m_guid; i <= m_guidLast.m_guid; i ++)
		{
			ConnGuid guid(i);

			// find if guid been in use recently
			Record* r = record(guid);
			assert(r != NULL);
			
			switch(r->m_state)
			{
			default:
				{
					break;
				}
			case Record::STATE_FREE:
				{
					freeSlot = r;
					break;
				}
			case Record::STATE_HISTORY:
				{
					if(historySlot == NULL || r->m_timestamp < historySlot->m_timestamp)
						historySlot = r;
					break;
				}
			case Record::STATE_RESERVED:
				{
					if(reservedSlot == NULL || r->m_timestamp < reservedSlot->m_timestamp)
						reservedSlot = r;
					break;
				}
			}
			if(freeSlot != NULL)
			{
				break;
			}

			// 20 minutes history. reuse.
			if(historySlot && secondsElapsed(historySlot->m_timestamp, (U32)(m_historyTimeout / (1000 * 1000))))
			{
				break;
			}
		}

		if(freeSlot)
		{
			ConnGuid g = guid(freeSlot);
			reserve(g);
			return g;
		}

		if(historySlot)
		{
			ConnGuid g = guid(historySlot);
			reserve(g);
			return g;
		}

		LOGWARN(Debugging::g_logger, "<ConnGuidGenerator::generate()> run out of guids.");
		return ConnGuid(CONN_GUID_INVALID);
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

