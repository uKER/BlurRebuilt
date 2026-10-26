// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The result set obtained following a call to bdCounter::getCounterTotals.
//			Also used to represent (counterID, delta) when calling bdCounter::incrementCounters

#ifndef BD_COUNTER_VALUE_H
#define BD_COUNTER_VALUE_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdCounterValue : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Constructor.
		bdCounterValue();

		virtual void serialize(bdByteBuffer& buffer) const;

		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		virtual bdUInt sizeOf();

		/// The counter ID
		bdUInt32 m_counterID;

		/// The value of the counter; depending on context this may be a delta to add to the counter (bdCounter::incrementCounters)
		/// or the total value of the counter retrieved from the server (bdCounter::getCounterTotals)
		bdInt64 m_counterValue;

};

#endif // BD_COUNTER_VALUE_H
