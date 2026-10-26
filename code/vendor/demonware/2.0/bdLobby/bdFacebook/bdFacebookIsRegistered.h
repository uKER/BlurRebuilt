// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE:	The result set obtained following a call to
//			bdTwitter::isRegistered.

#ifndef BD_FACEBOOK_IS_REGISTERED_H
#define BD_FACEBOOK_IS_REGISTERED_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdFacebookIsRegistered : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdFacebookIsRegistered();

		virtual void serialize(bdByteBuffer& buffer) const;

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// Registration status: true if the user is registered, false otherwise.
		bdBool m_isRegistered;

};

#endif // BD_FACEBOOK_IS_REGISTERED_H
