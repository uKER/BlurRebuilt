// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTitleUtilities/bdTitleStats.h>

bdTitleStats::bdTitleStats()
:m_numOnlineSessions(0),
 m_numTotalUsers(0),
 m_numOnlineUsers(0)
{
}


bdBool bdTitleStats::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_numOnlineSessions);
	ok = ok && buffer->readUInt32(m_numTotalUsers);
	ok = ok && buffer->readUInt32(m_numOnlineUsers);
	return ok;
}
		
bdUInt bdTitleStats::sizeOf()
{
	return sizeof(bdTitleStats);
}



