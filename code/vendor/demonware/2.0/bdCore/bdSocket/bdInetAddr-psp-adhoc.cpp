// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdInetAddr.h>

#include <bdCore/bdUtilities/bdBytePacker.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

bdInetAddr BD_CALL bdInetAddr::Loopback()
{
	bdInetAddr addr;
	addr.set(static_cast<bdInAddr>(BD_IN_ADDR_LOOPBACK));
	return addr;
}

bdInetAddr BD_CALL bdInetAddr::Broadcast()
{
	bdInetAddr addr;
	addr.set(static_cast<bdInAddr>(static_cast<bdUInt64>(BD_IN_ADDR_BROADCAST)));
	return addr;
}

bdInetAddr BD_CALL bdInetAddr::Any()
{
	bdInetAddr addr(static_cast<bdInAddr>(static_cast<bdUInt64>(BD_IN_ADDR_ANY)));
	return addr;
}

bdInetAddr::bdInetAddr(void)
{
	m_addr.set(BD_IN_ADDR_ANY);
}

bdInetAddr::~bdInetAddr()
{
	m_addr.set(BD_IN_ADDR_NONE);
}

/// Copy constructor
bdInetAddr::bdInetAddr(const bdInetAddr &other)
{
	m_addr = other.m_addr;
}

bdInetAddr::bdInetAddr(const bdNChar8 *address)
{
	set(static_cast<bdInAddr>(address));
}

bdInetAddr::bdInetAddr(const bdUInt64 address)
{
	m_addr.set(address);
}

bdInetAddr::bdInetAddr(const bdUByte8 address[BD_IN_ADDR_SIZE])
{
	bdUInt64 addr = 0;
	bdMemcpy(&addr, address, BD_IN_ADDR_SIZE);
	m_addr.set(addr);
}


bdInetAddr::bdInetAddr(const bdInAddr inaddr)
: m_addr(inaddr)
{
}

void bdInetAddr::set(const bdInetAddr &other)
{
	m_addr = other.m_addr;
}

//void bdInetAddr::set(const bdChar8 *address)
//{
//	bdMemcpy(&m_addr.inUn, address, 6);
//}

void bdInetAddr::set(const bdInAddr inaddr)
{
	m_addr = inaddr;
}

bdBool bdInetAddr::operator ==(const bdInetAddr &other) const
{
	return m_addr.getAsUInt64() == other.m_addr.getAsUInt64();
}

bdBool bdInetAddr::operator !=(const bdInetAddr &other) const
{
	return !(*this == other);
}

bdBool bdInetAddr::operator < (const bdInetAddr &other) const
{
	return m_addr.getAsUInt64() < other.m_addr.getAsUInt64();
}

bdBool bdInetAddr::isValid() const
{
	// if this is equal to the default bdInetAddr
	// then it is invalid
	return !(*this == bdInetAddr());
}

bdBool bdInetAddr::isLoopback() const
{
	return *this == bdInetAddr::Loopback();
}

bdBool bdInetAddr::isBroadcast() const
{
	return *this == bdInetAddr::Broadcast();
}

bdUInt bdInetAddr::toUInt32() const
{
	return 0;
}

bdUWord bdInetAddr::toString(bdNChar8 *const str, 
							 const bdUWord size) const
{
	const bdUWord strLength = m_addr.toString(str,size);
	return strLength;
}

bdInAddr bdInetAddr::getInAddr() const
{
	return m_addr;
}

bdBool bdInetAddr::serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const
{
	newOffset = offset;
	return bdBytePacker::appendBuffer(data, size, newOffset, newOffset,
		m_addr.getAsPointer(), BD_IN_ADDR_SIZE);
}

bdBool bdInetAddr::deserialize(const void* data,
							   const bdUInt size,
							   const bdUInt offset,
							   bdUInt &newOffset)
{
	newOffset = offset;

	bdUByte8 tmp[BD_IN_ADDR_SIZE];
	const bdBool result = bdBytePacker::removeBuffer(data, size, newOffset, newOffset, tmp, BD_IN_ADDR_SIZE);
	m_addr.set(tmp);
	return result;
}
