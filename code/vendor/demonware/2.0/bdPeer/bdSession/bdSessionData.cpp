// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdSession/bdSessionData.h>
#include <bdConnection/bdConnection.h>

bdSessionData::bdSessionData() :
m_hash(0)
{
}

bdSessionData::~bdSessionData()
{
}

void bdSessionData::serialize(bdBitBufferRef buffer)
{
	buffer->writeUInt32(m_hash);
}

bdBool bdSessionData::deserialize(bdBitBufferRef buffer)
{
	bdBool result = false;
	bdUInt32 temp = 0;
	if(buffer->readUInt32(temp))
	{
		m_hash = temp;
		result = true;
	}	
	return result;
}

bdConnectionRef bdSessionData::getConnection() const
{
	return m_connection;
}

void bdSessionData::setConnection(bdConnectionRef conn)
{
	m_connection = conn;
}


bdBool bdSessionData::equals (bdSessionDataRef other) const
{	
	bdBool result = false;

	result = (m_hash == other->m_hash);
	return result;

}

void bdSessionData::calculateLocalHash(bdArray<bdSessionDataRef> peers)
{
	bdUInt32 hash = 0;
	const bdUInt numPeers = peers.getSize();
	for(bdUInt i=0; i < numPeers; i++)
	{
		bdConnectionRef connection = peers[i]->getConnection();
		// include "(i+1) *" so that order matters
		hash += (i+1) * connection->getAddress()->getHash();
	}
	m_hash = hash;
}
