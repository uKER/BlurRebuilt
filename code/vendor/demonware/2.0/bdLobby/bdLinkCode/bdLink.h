// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: Represents a single link between an entityID and a server-generated
//			code within a category.  Returned by the lobby service.

#ifndef BD_LINK_H
#define BD_LINK_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>


class bdLink : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Constructor.
		bdLink();

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// The entity ID
		bdUInt64 m_entityID;

		/// The category
		bdUInt32 m_category;

		/// The link code
		bdNChar8 m_linkCode[BD_MAX_LINK_CODE_LENGTH];
};

#endif // BD_LINK_H
