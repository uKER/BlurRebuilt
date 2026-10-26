// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE:	The result set obtained following a call to
//			bdTwitter::isRegistered.

#ifndef BD_TWITTER_IS_REGISTERED_H
#define BD_TWITTER_IS_REGISTERED_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdTwitterIsRegistered : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdTwitterIsRegistered();

		virtual void serialize(bdByteBuffer& buffer) const;

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// Registration status: true if the user is registered, false otherwise.
		bdBool m_isRegistered;

};

#endif // BD_TWITTER_IS_REGISTERED_H
