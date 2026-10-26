// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The result set obtained following a call to bdGroup::getGroupCounts.

#ifndef BD_GROUP_COUNT_H
#define BD_GROUP_COUNT_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdGroupCount : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Constructor.
		bdGroupCount();

		virtual void serialize(bdByteBuffer& buffer) const;

		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		virtual bdUInt sizeOf();

		/// The group ID
		bdUInt32 m_groupID;

		/// The value of the counter
		bdUInt32 m_groupCount;

};

#endif // BD_GROUP_COUNT_H
