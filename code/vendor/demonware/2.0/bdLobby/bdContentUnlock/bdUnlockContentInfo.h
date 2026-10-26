// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_UNLOCKED_CONTENT_INFO_H
#define BD_UNLOCKED_CONTENT_INFO_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>


class bdUnlockContentInfo : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdUnlockContentInfo();
		
		~bdUnlockContentInfo();

		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		virtual bdUInt sizeOf();

		/// The content key
		bdUInt32 m_contentKey;

		/// The ID of the kit that last unlocked this content
		bdUInt64 m_hardwareID;
	
};

#endif // BD_UNLOCKED_CONTENT_INFO_H
