// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_PERFORMANCE_VALUE_H
#define BD_PERFORMANCE_VALUE_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

class bdPerformanceValue : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdPerformanceValue();

		~bdPerformanceValue();

		virtual void  serialize(bdByteBuffer& buffer) const;

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// The team/user ID to which the performance value pertains
		bdEntityID m_entityID;

		/// A placing value representing the performance in the match
		bdInt64 m_performanceValue;
		
};


#endif // BD_PERFORMANCE_VALUE_H
