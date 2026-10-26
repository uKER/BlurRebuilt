// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdAddrHandleRefWrapper.h>

#define BD_LOG_LEVEL "bdSocket/bdAddrHandle"

bdAddrHandleRefWrapper::bdAddrHandleRefWrapper()
{
}

bdAddrHandleRefWrapper::bdAddrHandleRefWrapper(bdAddrHandleRef handle)
: m_handle(handle)
{
}

bdBool bdAddrHandleRefWrapper::operator == (const bdAddrHandleRefWrapper &other) const
{
	bdBool equal = false;
	if(m_handle.notNull() && other.m_handle.notNull())
	{
		equal = (*m_handle == *other.m_handle);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Unexpected Null addrHandle.");
	}

	return equal;
	
}

bdUInt bdAddrHandleRefWrapper::getHash(const bdAddrHandleRefWrapper &addr) const
{
	bdEndpointHashingClass hash;
	const bdUInt hashVal = hash.getHash(addr.m_handle->m_endpoint);
	return hashVal;
}

bdAddrHandleRef bdAddrHandleRefWrapper::getHandle() const
{
	return m_handle;
}
