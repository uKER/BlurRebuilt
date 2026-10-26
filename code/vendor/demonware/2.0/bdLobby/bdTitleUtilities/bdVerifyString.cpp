// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTitleUtilities/bdVerifyString.h>

#define BD_PF_OK 0

bdVerifyString::bdVerifyString()
:m_verified(false)
{
}

bdBool bdVerifyString::deserialize(bdByteBufferRef buffer)
{
	bdUInt32 verifyResult = 0;
	const bdBool ok = buffer->readUInt32(verifyResult);
	if (ok && verifyResult == BD_PF_OK)
	{
		m_verified = true;
	}
	else
	{
		m_verified = false;
	}
	return ok;
}

bdUInt bdVerifyString::sizeOf()
{
	return sizeof(bdVerifyString);
}
