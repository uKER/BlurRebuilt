// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdCommonAddr.h>
#include <bdCore/bdContainers/bdBitBuffer.h>

bdCommonAddr::bdCommonAddr()
: m_isLoopback(false),
  m_hash(0)
{

}

bdCommonAddr::bdCommonAddr(const bdAddr &addr)
: m_addr(addr),
  m_isLoopback(true),
  m_hash(0)
{
	calculateHash();
}

bdCommonAddr::bdCommonAddr(bdCommonAddrRef me, const bdAddr &addr)
: m_addr(addr),
  m_isLoopback(false),
  m_hash(0)
{
	calculateHash();
	if(me->operator == (*this))
	{
		m_isLoopback = true;
	}	
}

void bdCommonAddr::setAddr(const bdAddr &addr)
{
	m_addr = addr;
	calculateHash();
	m_isLoopback = false;
}

bdCommonAddr::~bdCommonAddr()
{
}


bdUInt32 bdCommonAddr::getHash() const
{
	return m_hash;
}

bdBool bdCommonAddr::isLoopback() const
{
	return m_isLoopback;
}

void bdCommonAddr::serialize(bdUByte8 buffer[BD_COMMON_ADDR_SERIALIZED_SIZE]) const
{
	bdUInt newOffset;
	BD_ASSERT(m_addr.serialize(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, 0, newOffset),
		"Failed to serialize the address.");
}

void bdCommonAddr::serialize(bdBitBufferRef buffer) const
{
	bdUByte8 tmp[BD_COMMON_ADDR_SERIALIZED_SIZE];
	serialize(tmp);
	buffer->writeBits(tmp, BD_COMMON_ADDR_SERIALIZED_SIZE * 8);
}

bdBool bdCommonAddr::deserialize(bdCommonAddrRef me, const bdUByte8 buffer[BD_COMMON_ADDR_SERIALIZED_SIZE])
{
	bdUInt newOffset;
	BD_ASSERT(m_addr.deserialize(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, 0, newOffset),
		"Failed to deserialize the address.");

	calculateHash();

	if(me && me->operator == (*this))
	{
		m_isLoopback = true;
	}
	else
	{
		m_isLoopback = false;
	}
	return true;
}

bdBool bdCommonAddr::deserialize(bdCommonAddrRef me, bdBitBufferRef buffer)
{
	bdUByte8 tmp[BD_COMMON_ADDR_SERIALIZED_SIZE];
	if (buffer->readBits(tmp, BD_COMMON_ADDR_SERIALIZED_SIZE * 8))
	{
		return deserialize(me, tmp);
	}
	else
	{
		return false;
	}
}

bdBool bdCommonAddr::operator == (const bdCommonAddr &other) const
{
	return (this->getHash() == other.getHash())
		&& (this->m_addr == other.m_addr);
}

bdBool bdCommonAddr::operator != (const bdCommonAddr &other) const
{
	return !(operator == (other));
}

bdBool bdCommonAddr::operator < (const bdCommonAddr &other) const
{
	return this->getHash() < other.getHash();
}

const bdAddr& bdCommonAddr::getAddr() const
{
	return m_addr;
}

// protected:

void bdCommonAddr::calculateHash()
{
	m_hash = m_addr.getHash();
}
