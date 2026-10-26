// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdLinkCode/bdLink.h>


#define BD_LOG_LEVEL "linkcode"


bdLink::bdLink()
: m_entityID(0)
, m_category(0)
{
	bdMemset(m_linkCode, BD_NULL, BD_MAX_LINK_CODE_LENGTH);
}

bdBool bdLink::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt64(m_entityID);
	ok = ok && buffer->readUInt32(m_category);

	bdUInt codeLength = 0;
	ok = ok && buffer->getStringLength(codeLength);
	ok = ok && buffer->readString(m_linkCode, BD_MAX_LINK_CODE_LENGTH);
	if (ok && codeLength >= BD_MAX_LINK_CODE_LENGTH)
	{
		BD_ERR(BD_LOG_LEVEL,
			"Length of received link code (%u) is not less than than length of buffer (%u).  Truncated link code: %s",
			codeLength, BD_MAX_LINK_CODE_LENGTH, m_linkCode);
		ok = false;
	}

	return ok;
}

bdUInt bdLink::sizeOf()
{
	return sizeof(bdLink);
}
