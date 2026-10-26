#ifndef MULTIPLAYER_CONNGUID_H
#define MULTIPLAYER_CONNGUID_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netStream/netStream.h"
#include "netStream/netSerialise.h"

namespace netTransport
{
	enum
	{
		// assigned guids by a host
		CONN_GUID_ASSIGNED_FIRST = 0,
		CONN_GUID_ASSIGNED_LAST = 200,
		// special guids for communication
		CONN_GUID_BROADCAST = 240,
		CONN_GUID_HOST,
		CONN_GUID_CLIENT,
		CONN_GUID_INVALID,
	};

	class ConnGuid: public netSerialisable
	{
	public:
		ConnGuid(int guid=CONN_GUID_INVALID)
		{
			m_guid = guid;
		}

		virtual bool serialise(netStream* stream)
		{
			stream->writeType(m_guid);
			return stream->ok();
		}
		virtual bool deserialise(netStream* stream)
		{
			stream->readType(m_guid);
			return stream->ok();
		}

		operator int() const { return m_guid; }

		bool isNull() const { return m_guid == CONN_GUID_INVALID; }
		bool notNull() const { return m_guid != CONN_GUID_INVALID; }
		bool broadcast() const { return m_guid == CONN_GUID_BROADCAST; }
		bool assigned() const { return /*m_guid >= CONN_GUID_ASSIGNED_FIRST && */m_guid <= CONN_GUID_ASSIGNED_LAST; }

		bool operator <= (const ConnGuid& other) const
		{
			return (*this) < other || (*this) == other;
		}

		bool operator >= (const ConnGuid& other) const
		{
			return (*this) > other || (*this) == other;
		}

		bool operator < (const ConnGuid& other) const
		{
			return (m_guid < other.m_guid);
		}
		
		bool operator > (const ConnGuid& other) const
		{
			return (m_guid > other.m_guid);
		}
		
		bool operator == (const ConnGuid& other) const
		{
			return (m_guid == other.m_guid);
		}
		bool operator != (const ConnGuid& other) const { return !((*this) == other); }

		// reusable guids, range 0->255.
		unsigned char m_guid;
	};

	class ConnGuidGenerator
	{
	public:
		ConnGuidGenerator(const ConnGuid& first, const ConnGuid& last, U64 historyTimeout=(15 * 1000 * 1000));
		bool reset();
		bool release(ConnGuid guid);
		bool reserve(ConnGuid guid);
		ConnGuid generate();

	private:
		struct Record
		{
			// accuracy of timestamps (so we can use less memory).
			enum { TICKS_PER_SEC = 100 };

			enum
			{
				STATE_FREE,
				STATE_HISTORY,
				STATE_RESERVED,
			};			
			Record()
			{
				m_timestamp = 0;
				m_state = STATE_FREE;
			}
			union
			{
				U32 bits;
				struct
				{
					U32 m_timestamp:30;
					U32 m_state:2;	
				};
			};
		};
		Record* record(ConnGuid guid);
		ConnGuid guid(const Record* record);
	
		const ConnGuid m_guidFirst;
		const ConnGuid m_guidLast;
		const U64 m_historyTimeout;

		U64 m_tickBase;
		U32 timestamp();
		bool secondsElapsed(U32 timestamp, U32 seconds);

		enum { RECORD_COUNT = (CONN_GUID_ASSIGNED_LAST - CONN_GUID_ASSIGNED_FIRST + 1) };
		Record m_records[RECORD_COUNT];
	};
}
#endif //MULTIPLAYER_CONNGUID_H