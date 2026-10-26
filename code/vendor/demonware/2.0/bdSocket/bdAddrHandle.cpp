// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdAddrHandle.h>

bdAddrHandle::bdAddrHandle(bdCommonAddrRef ca, const bdSecurityID &secID)
: m_endpoint(ca, secID),
  m_status(BD_ADDR_NOT_RESOLVED)
{
}

bdBool bdAddrHandle::operator == (const bdAddrHandle &other) const
{
	bdBool equal = m_endpoint == other.m_endpoint;
	// Including a check for equality of m_realAddrs is asymmetric with bdAddrHandleRefWrapper::getHash, which 
	// generates a hash on only the endpoint. When fetching from a bdHashMap with bdAddrHandleRef as a key,
	// the hash may match but the subsequent test for equality may fail, resulting in unintuitive behaviour
	// equal = equal && m_realAddr == other.m_realAddr;
	return equal;
}

void bdAddrHandle::setRealAddr(const bdAddr &realAddr)
{
	m_realAddr = realAddr;
	m_status = BD_ADDR_RESOLVED;
}

void bdAddrHandle::getSecID(bdSecurityID& secID) const
{
	secID = m_endpoint.getSecID();
}

const bdAddr bdAddrHandle::getRealAddr() const
{
	return m_realAddr;
}

void bdAddrHandle::setStatus(const bdAddrHandleStatus &status)
{
	m_status = status;
}

bdAddrHandle::bdAddrHandleStatus bdAddrHandle::getStatus() const
{
	return m_status;
}

