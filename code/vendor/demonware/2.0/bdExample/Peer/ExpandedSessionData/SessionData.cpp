// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "SessionData.h"

SessionData::SessionData()
{
}

void SessionData::serialize(bdBitBufferRef buffer)
{
	const bdUInt32 size = m_connectedPeerAddrs.getSize();
	buffer->writeUInt32(size);
	for(bdUInt i=0; i<size ; i++)
	{
		m_connectedPeerAddrs[i]->serialize(buffer);
	}
}

bdBool SessionData::deserialize(bdBitBufferRef buffer)
{
	m_connectedPeerAddrs.clear();

	bdUInt32 size = 0;
	bdBool result = buffer->readUInt32(size);
	bdCommonAddrRef me = bdNet::getInstance()->getLocalCommonAddr();
	for(bdUInt i=0; (true == result)&& (i<size) ; i++)
	{
		bdCommonAddrRef addr = new bdCommonAddr();
		result = addr->deserialize(me, buffer);
		if(true == result)
		{
			m_connectedPeerAddrs.pushBack(addr);
		}
	}
	return result;
}

bdBool SessionData::equals(bdSessionDataRef other) const
{
	bdBool result = false;
	SessionDataRef otherSessionData = reference_cast<SessionData>(other);
	if(otherSessionData->m_connectedPeerAddrs.getSize() == m_connectedPeerAddrs.getSize())
	{
		result = true;
	}
	return result;
}

void SessionData::calculateLocalHash(bdArray<bdSessionDataRef> peers)
{
	const bdUInt32 size = peers.getSize();
	m_connectedPeerAddrs.clear();
	for(bdUInt i=0; i<size ; i++)
	{
		SessionDataRef otherSessionData = reference_cast<SessionData>(peers[i]);
		m_connectedPeerAddrs.pushBack(otherSessionData->getConnection()->getAddress());
	}
}
