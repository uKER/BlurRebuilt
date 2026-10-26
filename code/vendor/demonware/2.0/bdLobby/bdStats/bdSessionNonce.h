// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_SESSION_NONCE_H
#define BD_SESSION_NONCE_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdSessionNonce : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdSessionNonce();

		/// Destructor.
		virtual ~bdSessionNonce();

		virtual void serialize(bdByteBuffer &buffer)const;

		virtual bdBool deserialize(bdByteBufferRef buffer);
	
		virtual bdUInt sizeOf();

		bdUInt64 m_nonce;

};

#endif // BD_SESSION_NONCE_H
