// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTitleUtilities/bdTimeStamp.h>

bdTimeStamp::bdTimeStamp()
:m_timeStamp(0)
{
}


bdBool bdTimeStamp::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_timeStamp);
	return ok;
}
		
bdUInt bdTimeStamp::sizeOf()
{
	return sizeof(bdTimeStamp);
}


