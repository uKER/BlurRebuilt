// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdPlayer/bdPlayer.h>

bdPlayer::bdPlayer()
: m_id(0)
{	
}

bdPlayer::~bdPlayer()
{
}

void bdPlayer::setId(const bdPlayerId id)
{
	m_id = id;
}

bdPlayerId bdPlayer::getId() const
{
	return m_id;
}


void bdPlayer::setAddr(const bdCommonAddrRef addr)
{
	m_addr = addr;
}

bdCommonAddrRef bdPlayer::getAddr() const
{
	return m_addr;
}

bdBool bdPlayer::isLocal() const
{	
	const bdBool isLocal = m_addr->isLoopback();
	return isLocal;
}

void bdPlayer::serialize(bdBitBufferRef)
{
}

bdBool bdPlayer::deserialize(bdBitBufferRef)
{
	return true;
}


