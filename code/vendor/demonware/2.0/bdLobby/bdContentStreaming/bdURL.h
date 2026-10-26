// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_URL_H
#define BD_URL_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>



class bdURL : public bdTaskResult
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdURL();

	~bdURL();

	virtual bdBool deserialize(bdByteBufferRef buffer);

	virtual bdUInt sizeOf();

	bdNChar8 m_url[BD_MAX_URL_SIZE];
	bdUInt16 m_serverType;
	bdNChar8 m_serverIndex[BD_MAX_SERVER_URL_SIZE];
};

#endif // BD_URL_H
