// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE XBOX XENON PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdInetAddr.h>
#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>


#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdCore/bdUtilities/bdBytePacker.h>


bdInetAddr BD_CALL bdInetAddr::Loopback()
{
	bdInetAddr addr;
	addr.set(static_cast<bdUInt>(BD_IN_ADDR_LOOPBACK));
	return addr;
}

bdInetAddr BD_CALL bdInetAddr::Broadcast()
{
	bdInetAddr addr;
	addr.set(static_cast<bdUInt>(BD_IN_ADDR_BROADCAST));
	return addr;
}

bdInetAddr BD_CALL bdInetAddr::Any()
{
	bdInetAddr addr;
	addr.set(static_cast<bdUInt>(BD_IN_ADDR_ANY));
	return addr;
}

bdInetAddr::bdInetAddr(void)
{
	//m_addr.m_inAddri = BD_IN_ADDR_NONE;
}

bdInetAddr::~bdInetAddr()
{
	m_addr.m_inAddri = 0xDEADBEEF;
}

/// Copy constructor
bdInetAddr::bdInetAddr(const bdInetAddr &other)
{
	m_addr = other.m_addr;
}

bdInetAddr::bdInetAddr(const bdNChar8 *address)
{
	set(address);
}



bdInetAddr::bdInetAddr(const bdUInt address)
{
	set(address);
}

bdInetAddr::bdInetAddr(const bdInAddr inaddr)
: m_addr(inaddr)
{
}

void bdInetAddr::set(const bdInetAddr &other)
{
	m_addr = other.m_addr;
}

void bdInetAddr::set(const bdNChar8 *address)
{
	m_addr.fromString(address);
}

void bdInetAddr::set(const bdUInt address)
{
	m_addr.m_inAddri = address;
}

void bdInetAddr::set(const bdInAddr inaddr)
{
	m_addr = inaddr;
}

bdBool bdInetAddr::operator ==(const bdInetAddr &other) const
{
	return m_addr.m_inAddri == other.m_addr.m_inAddri;
}

bdBool bdInetAddr::operator !=(const bdInetAddr &other) const
{
	return !(*this == other);
}

bdBool bdInetAddr::operator < (const bdInetAddr &other) const
{
	const bdUInt addr1 = toUInt32();
	const bdUInt addr2 = other.toUInt32();
	return (addr1 < addr2);
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
	return m_addr.m_inAddri;
}

bdUWord bdInetAddr::toString(bdNChar8 *const str, 
							 const bdUWord size) const
{	
	bdUWord strLength = 0;
	if(isValid())
	{
		strLength = m_addr.toString(str, size);
	}
	return strLength;
}

bdInAddr bdInetAddr::getInAddr() const
{
	return m_addr;
}

bdBool bdInetAddr::serialize(void *data,
							 const bdUInt size,
							 const bdUInt offset,
							 bdUInt &newOffset) const
{
	const bdBool ok = bdBytePacker::appendBuffer(data,
												 size,
												 offset,
												 newOffset,
												 &m_addr.m_inAddri,
												 sizeof(m_addr.m_inAddri));
	return ok;
}

bdBool bdInetAddr::deserialize(const void* data,
							   const bdUInt size,
							   const bdUInt offset,
							   bdUInt &newOffset)
{
	const bdBool ok = bdBytePacker::removeBuffer(data,
												 size,
												 offset,
												 newOffset,
												 &m_addr.m_inAddri,
												 sizeof(m_addr.m_inAddri));
	return ok;
}

