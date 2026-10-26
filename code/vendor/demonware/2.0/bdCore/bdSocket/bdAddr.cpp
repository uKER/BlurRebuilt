// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdAddr.h>

#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdCore/bdUtilities/bdBytePacker.h>

bdUInt bdAddr::serializedSize = 0;

bdAddr::bdAddr()
: m_port(BD_INVALID_PORT)
{

}

bdAddr::bdAddr(const bdAddr &other)
: m_address(other.m_address),
  m_port(other.m_port)
{
}

bdAddr::bdAddr(const bdInetAddr &address,
			   const bdPort port)
: m_address(address),
  m_port(port)
{
}

bdAddr::bdAddr(const bdNChar8 *socketAddress)
{
	set(socketAddress);
}

void bdAddr::set(const bdNChar8 *socketAddress)
{
	BD_ASSERT( (socketAddress != BD_NULL), "bdAddr::set(const bdNChar8 *socketAddress) \n"
				"Invalid socket address \n " );

	const bdNChar8 *const sep = bdStrchr(socketAddress, ':');
	
	if(sep)
	{
		const bdUInt pos = static_cast<bdUInt>((sep - socketAddress) + 1);
		const bdUInt strAddrSize = 16;
		bdNChar8 strAddr[strAddrSize];
		bdMemcpy(strAddr, socketAddress, strAddrSize < pos ? strAddrSize : pos);
		strAddr[strAddrSize < pos ? strAddrSize -1 : pos -1] = 0;
		m_address.set(bdInAddr(strAddr));
		m_port = static_cast<bdPort>(bdStrtoui32(sep+1, BD_NULL, 10));
	}
	else
	{
		set(socketAddress, 0);
	}
}

void bdAddr::set(const bdInetAddr &address,
				 const bdPort port)
{
	m_address.set(address);
	m_port = port;
}

bdBool bdAddr::operator ==(const bdAddr &other) const
{
	return (m_port == other.m_port) &&
		   (m_address == other.m_address);
}

bdBool bdAddr::operator !=(const bdAddr &other) const
{
	return (m_port != other.m_port) ||
		   (m_address != other.m_address);
}

bdBool bdAddr::operator < (const bdAddr &other) const
{
	const bdUInt thisHash = getHash();
	const bdUInt otherHash = other.getHash();
	return (thisHash < otherHash);
}

const bdInetAddr& bdAddr::getAddress() const
{
	return m_address;
}

bdInetAddr& bdAddr::getAddress()
{
	return m_address;
}

void bdAddr::setPort(const bdPort port)
{
	m_port = port;
}

bdPort bdAddr::getPort() const
{
	return m_port;
}

bdUWord bdAddr::toString(bdNChar8 *const str, 
						const bdUWord size) const
{
	bdUWord sizeLeft = size;
	bdUWord strLength = m_address.toString(str, size);
	sizeLeft = (strLength > size)?(0):(sizeLeft - strLength); 

	strLength += bdSnprintf(str+strLength, sizeLeft, ":%u", m_port);
	return strLength;
}

bdUInt bdAddr::getHash() const
{

	bdUInt hash = 0;
	bdNChar8 val[BD_MAX_DATAGRAM_SIZE];
	bdUInt len;
	BD_ASSERT(serialize(val, BD_MAX_DATAGRAM_SIZE, 0, len), "Failed to serialize.");

	for (bdUInt i = 0; i < len; i++) 
	{
		hash = 31*hash + *(val+ i);
	}
	return hash;
}

bdBool bdAddr::serialize(void *data,
						 const bdUInt size,
						 const bdUInt offset,
						 bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && m_address.serialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_port);

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdAddr::deserialize(const void* data,
						   const bdUInt size,
						   const bdUInt offset,
						   bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && m_address.deserialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_port);

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdUInt bdAddr::getSerializedSize() const
{
	if (serializedSize == 0)
	{
		BD_ASSERT(bdAddr::serialize(BD_NULL, BD_UINT16_MAX, 0, serializedSize)
			&& serializedSize > 0, "Failed to get serialized size.");
		BD_ASSERT(serializedSize < BD_MAX_DATAGRAM_SIZE, "Size is larger than a biggest datagram.");
	}
	return serializedSize;
}

#if defined BD_PLATFORM_PSP_ADHOC

bdBool bdAddr::isNull()
{
	return false;
}

bdBool bdAddr::notNull()
{
	return true;
}

#endif //BD_PLATFORM_PSP_ADHOC

