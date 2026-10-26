// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMessaging/bdChannelInfo.h>

bdChannelInfo::bdChannelInfo()
: m_channelID(0),
  m_numSubscribers(0),
  m_maxSubscribers(0)
{
}

bdChannelInfo::~bdChannelInfo()
{
}

bdBool bdChannelInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt64(m_channelID);
	ok = ok && buffer->readUInt32(m_numSubscribers);
	ok = ok && buffer->readUInt32(m_maxSubscribers);
	return ok;
}

bdUInt bdChannelInfo::sizeOf()
{
	return sizeof(bdChannelInfo);
}
