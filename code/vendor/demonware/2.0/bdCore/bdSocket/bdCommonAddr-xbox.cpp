// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdCommonAddr.h>
#include <bdCore/bdContainers/bdBitBuffer.h>
#include <bdCore/bdUtilities/bdBytePacker.h>

#define BD_INVALID_TITLE_ID (0)

bdCommonAddr::bdCommonAddr()
: m_port(0),
  m_titleId(BD_INVALID_TITLE_ID),
  m_isLoopback(false)
{

}

bdCommonAddr::bdCommonAddr(const XNADDR &addr, const bdPort port)
: m_addr(addr),
  m_port(port),
  m_titleId(BD_INVALID_TITLE_ID),
  m_isLoopback(true)
{
	calculateHash();
}

bdCommonAddr::bdCommonAddr(bdCommonAddrRef me, 
						   const XNADDR &addr, 
						   const bdPort port)
: m_addr(addr),
  m_port(port),
  m_titleId(BD_INVALID_TITLE_ID),
  m_isLoopback(false)
{
	calculateHash();
	if(me->operator == (*this))
	{
		m_isLoopback = true;
	}	
}

bdCommonAddr::bdCommonAddr(bdCommonAddrRef me, 
						   const TSADDR &addr, 
						   const bdPort port, 
						   const bdUWord titleId)
: m_addr(addr),
  m_port(port),
  m_titleId(titleId),
  m_isLoopback(false)
{
	BD_ASSERT(titleId != BD_INVALID_TITLE_ID, "Invalid title id!");

	calculateHash();
	if(me->operator == (*this))
	{
		m_isLoopback = true;
	}	
}

bdCommonAddr::~bdCommonAddr()
{
}

const XNADDR &bdCommonAddr::getXNAddr() const
{
	return m_addr;
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
	bdUInt offset = 0;
	bdBool result = true;

	result = result && bdBytePacker::appendBasicType(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset, m_addr);
	result = result && bdBytePacker::appendBasicType(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset, m_port);
	result = result && bdBytePacker::appendBasicType(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset, m_titleId);

	BD_ASSERT(result, "Unable to serialize common addr.");
}

void bdCommonAddr::serialize(bdBitBufferRef buffer) const
{
	const bdUInt tmpBufferSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
	bdUByte8 tmpBuffer[tmpBufferSize];

	serialize(tmpBuffer);

	buffer->writeBits(tmpBuffer, tmpBufferSize * 8);
}

bdBool bdCommonAddr::deserialize(bdCommonAddrRef me, const bdUByte8 buffer[BD_COMMON_ADDR_SERIALIZED_SIZE])
{
	bdUInt offset = 0;
	bdBool result = true;

	result = result && bdBytePacker::removeBasicType(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset, m_addr);
	result = result && bdBytePacker::removeBasicType(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset, m_port);
	result = result && bdBytePacker::removeBasicType(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset, m_titleId);

	BD_ASSERT(result, "Unable to deserialize common addr.");

	if(result)
	{
		calculateHash();

		if(me->operator == (*this))
		{
			m_isLoopback = true;
		}
		else
		{
			m_isLoopback = false;
		}
	}

	return result;
}

bdBool bdCommonAddr::deserialize(bdCommonAddrRef me, bdBitBufferRef buffer)
{
	const bdUInt tmpBufferSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
	bdUByte8 tmpBuffer[tmpBufferSize];

	if (buffer->readBits(tmpBuffer, tmpBufferSize * 8))
	{
		return deserialize(me, tmpBuffer);
	}
	else
	{
		return false;
	}
}

bdBool bdCommonAddr::operator == (const bdCommonAddr &other) const
{
	return this->getHash() == other.getHash();
}

bdBool bdCommonAddr::operator != (const bdCommonAddr &other) const
{
	return !(operator == (other));
}

bdBool bdCommonAddr::operator < (const bdCommonAddr &other) const
{
	return this->getHash() < other.getHash();
}

bdUWord bdCommonAddr::getTitleId() const
{
	return m_titleId;
}

void bdCommonAddr::setTitleId(bdUWord titleid)
{
	m_titleId = titleid;
}

bdPort bdCommonAddr::getPort() const
{
	return m_port;
}

// protected:

void bdCommonAddr::calculateHash()
{
	m_hash = 0;

	for (bdUInt i = 0; i < sizeof(m_addr.abEnet); i++) 
	{
		m_hash = 31*m_hash + m_addr.abEnet[i];
	}

}