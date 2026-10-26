// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
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
	if( m_handle.notNull() && other.m_handle.notNull() )
	{
		equal = m_handle->operator ==(*other.m_handle);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Unexpected Null addrHandle.");
	}

	return equal;
}

bdUInt BD_CALL bdAddrHandleRefWrapper::getHash(const bdAddrHandleRefWrapper &addr) const
{
	return addr.m_handle->m_addr.s_addr;
}

bdAddrHandleRef bdAddrHandleRefWrapper::getHandle() const
{
	return m_handle;
}
