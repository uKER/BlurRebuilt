// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The result set obtained following a call to bdGroup::getGroupCounts.

#ifndef BD_GROUP_ID_H
#define BD_GROUP_ID_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdGroupID : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Constructor.
		bdGroupID();

		virtual void serialize(bdByteBuffer& buffer) const;

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// The group IDs
		bdUInt32 m_groupID;


};

#endif // BD_GROUP_ID_H
