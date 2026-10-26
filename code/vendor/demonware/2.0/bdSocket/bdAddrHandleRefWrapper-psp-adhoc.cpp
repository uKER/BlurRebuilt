// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdAddrHandleRefWrapper.h>

bdAddrHandleRefWrapper::bdAddrHandleRefWrapper()
{
}

bdAddrHandleRefWrapper::bdAddrHandleRefWrapper(bdAddrHandleRef handle)
: m_handle(handle)
{
}

bdBool bdAddrHandleRefWrapper::operator == (const bdAddrHandleRefWrapper &other) const
{
	return m_handle.getHash() == other.m_handle.getHash();
}

bdUInt bdAddrHandleRefWrapper::getHash(const bdAddrHandleRefWrapper &addr) const
{
	return addr.m_handle.getHash();
}

