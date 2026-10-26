// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdCommonAddr.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>

#include <bdCore/bdContainers/bdBitBuffer.h>
#include <bdCore/bdUtilities/bdBytePacker.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>

bdCommonAddr::bdCommonAddr()
: m_natType(BD_NAT_OPEN),
  m_hash(0),
  m_isLoopback(false)
{
}

bdCommonAddr::~bdCommonAddr()
{}

bdCommonAddr::bdCommonAddr(const bdArray<bdAddr> &localAddrs,
						   const bdAddr& publicAddr,
						   const bdNATType natType)
: m_localAddrs(localAddrs),
  m_publicAddr(publicAddr),
  m_natType(natType),
  m_isLoopback(true)
{
	BD_ASSERT(localAddrs.getSize() > 0, "Too few local addresses!");
	BD_ASSERT(localAddrs.getSize() <= BD_MAX_LOCAL_ADDRS, "Too many local addresses!");

	calculateHash();
}

bdCommonAddr::bdCommonAddr(bdCommonAddrRef me,
						   const bdArray<bdAddr> &localAddrs,
						   const bdAddr& publicAddr,
						   const bdNATType natType)
: m_localAddrs(localAddrs),
  m_publicAddr(publicAddr),
  m_natType(natType),
  m_isLoopback(false)
{
	BD_ASSERT(localAddrs.getSize() > 0, "Too few local addresses!");
	BD_ASSERT(localAddrs.getSize() <= BD_MAX_LOCAL_ADDRS, "Too many local addresses!");

	calculateHash();

	if(me && me->operator == (*this))
	{
		m_isLoopback = true;
	}
}

bdCommonAddr::bdCommonAddr(const bdAddr& publicAddr)
: m_publicAddr(publicAddr),
  m_natType(BD_NAT_OPEN),
  m_isLoopback(false)
{
	BD_ASSERT(publicAddr.getAddress().isValid(), "Address not valid!");

	m_localAddrs.pushBack(publicAddr);

	calculateHash();
}

bdUInt bdCommonAddr::getHash() const
{
	return m_hash;
}

bdBool bdCommonAddr::isLoopback() const
{
	return m_isLoopback;
}

bdNATType bdCommonAddr::getNATType() const
{
	return m_natType;
}


void bdCommonAddr::serialize(bdUByte8 buffer[BD_COMMON_ADDR_SERIALIZED_SIZE]) const
{
	const bdUInt bufferSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
	bdUInt offset = 0;
	bdBool status = true;

	const bdAddr invalidAddr;

	// Write local addresses
	for (bdUInt i=0; i < BD_MAX_LOCAL_ADDRS; i++)
	{
		if (i < m_localAddrs.getSize())
		{
			status = status && m_localAddrs[i].serialize(buffer, bufferSize, offset, offset);
		}
		else
		{
			status = status && invalidAddr.serialize(buffer, bufferSize, offset, offset);
		}
	}

	// Write public addresses
	status = status && m_publicAddr.serialize(buffer, bufferSize, offset, offset);

	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset,
			static_cast<bdByte8>(m_natType));

	BD_ASSERT(status && offset == BD_COMMON_ADDR_SERIALIZED_SIZE,
			"bdCommonAddr::serialize, wrong size.");
}

void bdCommonAddr::serialize(bdBitBufferRef buffer) const
{
	const bdUInt tmpBufferSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
	bdUByte8 tmpBuffer[tmpBufferSize];

	serialize(tmpBuffer);

	buffer->writeBits(tmpBuffer, tmpBufferSize * 8);
}

bdBool bdCommonAddr::deserialize(bdCommonAddrRef me,
								 const bdUByte8 buffer[BD_COMMON_ADDR_SERIALIZED_SIZE])
{
	const bdUInt bufferSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
	bdUInt offset = 0;
	bdBool status = true;

	m_localAddrs.clear();

	for (bdUInt i=0; i < BD_MAX_LOCAL_ADDRS; i++)
	{
		bdAddr localAddr;
		status = status && localAddr.deserialize(buffer, bufferSize, offset, offset);

		if (status && localAddr.getAddress().isValid())
		{
			m_localAddrs.pushBack(localAddr);
		}
	}

	status = status && m_publicAddr.deserialize(buffer, bufferSize, offset, offset);
	bdByte8 tmpByte = 0;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, tmpByte);

	if (status)
	{
		m_natType = static_cast<bdNATType>(tmpByte);
		calculateHash();

		if (me && me->operator == (*this))
		{
			m_isLoopback = true;
		}
		else
		{
			m_isLoopback = false;
		}
	}

	return status;
}

bdBool bdCommonAddr::deserialize(bdCommonAddrRef me,
								 bdBitBufferRef buffer)
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
	const bdAddr localAddr = m_localAddrs.getSize() > 0 ? m_localAddrs[0] : bdAddr();
	const bdAddr otherLocalAddr = other.m_localAddrs.getSize() > 0 ? other.m_localAddrs[0] : bdAddr();

	if (m_hash == other.m_hash)
	{
		const bdAddr *myAddr;
		const bdAddr *otherAddr;

		if (m_publicAddr.getAddress().isValid())
		{
			myAddr = &m_publicAddr;
		}
		else
		{
			myAddr = &localAddr;
		}

		if (other.m_publicAddr.getAddress().isValid())
		{
			otherAddr = &other.m_publicAddr;
		}
		else
		{
			otherAddr = &otherLocalAddr;
		}

		return (*myAddr) == (*otherAddr);
	}
	else
	{
		return false;
	}
}

bdBool bdCommonAddr::operator != (const bdCommonAddr &other) const
{
	return m_hash != other.m_hash;
}

bdBool bdCommonAddr::operator < (const bdCommonAddr &other) const
{
	return m_hash < other.m_hash;
}

const bdAddr& bdCommonAddr::getPublicAddr() const
{
	return m_publicAddr;
};

const bdArray<bdAddr>& bdCommonAddr::getLocalAddrs() const
{
	return m_localAddrs;
};

const bdAddr& bdCommonAddr::getLocalAddrByIndex(const bdUInt index) const
{
	return m_localAddrs[index];
};

// protected:

void bdCommonAddr::calculateHash()
{
	bdAddr addr;
	const bdAddr localAddr = m_localAddrs.getSize() > 0 ? m_localAddrs[0] : bdAddr();

	if(m_publicAddr.getAddress().isValid())
	{
		addr = m_publicAddr;
	}
	else
	{
		// no public address so try and use the first local address for the hash
		addr = localAddr;
	}

	const bdUWord dataSize = BD_ADDR_SIZE;
	bdUByte8 data[dataSize];

	bdUInt offset = 0;
	bdBool status = true;

	status = status && addr.serialize(data, dataSize, 0, offset);

	const bdUInt hashSize = BD_TIGER_HASH_SIZE;
	bdUByte8 hash[hashSize];

	bdUInt tmpHashSize = hashSize;

	bdHashTiger192 tiger;
	status = status && tiger.hash(data, dataSize, hash, tmpHashSize);
	status = status && bdBytePacker::removeBasicType(hash, hashSize, 0, offset, m_hash);

	BD_ASSERT(status, "Failed to calculate hash.");
}

