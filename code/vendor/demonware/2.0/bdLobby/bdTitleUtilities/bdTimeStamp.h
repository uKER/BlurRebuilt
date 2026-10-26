// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_TIME_STAMP_H
#define BD_TIME_STAMP_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

/// Container class for storing a UTC time stamp
class bdTimeStamp : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Constructor.
		bdTimeStamp();

		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		virtual bdUInt sizeOf();

		/// UTC timestamp in seconds since the epoch
		bdUInt32 m_timeStamp;
};

#endif // BD_TIME_STAMP_H
