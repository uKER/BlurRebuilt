// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The result set obtained following a call to bdMatchMaking::createSession.

#ifndef BD_SESSION_CREATE_RESULT_H
#define BD_SESSION_CREATE_RESULT_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

/// The result set obtained following a call to bdMatchMaking::createSession.
class bdSessionID  : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Constructor.
		bdSessionID();

		// Copy constructor
		bdSessionID(const bdSessionID &other);

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual void serialize(bdByteBuffer &buffer)const;

		virtual bdUInt sizeOf();

		/// Security ID assigned by the server to the new session.
		bdSecurityID m_sessionID;

};

#endif // BD_SESSION_CREATE_RESULT_H
