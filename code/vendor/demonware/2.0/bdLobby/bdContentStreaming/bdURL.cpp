// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdContentStreaming/bdURL.h>

bdURL::bdURL() : 
m_serverType(0)
{
	m_url[0] = 0x0;
}

bdURL::~bdURL()
{
}

bdBool bdURL::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readString(m_url,BD_MAX_URL_SIZE);
	ok = ok && buffer->readUInt16(m_serverType);
	ok = ok && buffer->readString(m_serverIndex,BD_MAX_SERVER_URL_SIZE);

	return ok;
}

bdUInt bdURL::sizeOf()
{
	return sizeof(bdURL);
}

