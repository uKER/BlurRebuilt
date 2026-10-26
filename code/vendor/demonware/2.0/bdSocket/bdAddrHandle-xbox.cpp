// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdAddrHandle.h>

#define BD_LOG_LEVEL "bdSocket/bdAddrHandle"

void bdAddrHandle::getSecID(bdSecurityID& secID) const
{
	secID = m_endpoint.getSecID();
}

const bdAddr bdAddrHandle::getRealAddr() const
{
	const bdInetAddr inetAddr(m_addr.s_addr);
	const bdAddr addr(inetAddr, m_port);
	return addr;
}

bdBool bdAddrHandle::operator == (const bdAddrHandle &other) const
{
	return m_endpoint == other.m_endpoint;
	//return m_addr.s_addr == other->m_addr.s_addr;
}

void bdAddrHandle::setRealAddr( const in_addr &realAddr, bdPort port)
{
	m_addr = realAddr;
	m_port = port;
}

bdAddrHandle::bdAddrHandle(bdCommonAddrRef ca, const bdSecurityID &secID)
: m_endpoint(ca, secID)
{
}

void bdAddrHandle::print()
{
	bdNChar8 real[16];
	getRealAddr().getAddress().toString( real, 15 );
	
	bdNChar8 local[16];
	XNADDR xlocal = m_endpoint.getCommonAddr()->getXNAddr();
	sprintf( local, "%u.%u.%u.%u", xlocal.ina.S_un.S_un_b.s_b1, 
									xlocal.ina.S_un.S_un_b.s_b2,
									xlocal.ina.S_un.S_un_b.s_b3,
									xlocal.ina.S_un.S_un_b.s_b4 );

	bdNChar8 pub[16];
	sprintf( pub, "%u.%u.%u.%u", xlocal.inaOnline.S_un.S_un_b.s_b1, 
									xlocal.inaOnline.S_un.S_un_b.s_b2,
									xlocal.inaOnline.S_un.S_un_b.s_b3,
									xlocal.inaOnline.S_un.S_un_b.s_b4 );

	BD_INFO(BD_LOG_LEVEL, "real %s local %s public %s secID %x %x %x %x", real, local, pub, 
							m_endpoint.getSecID().ab[0], 
							m_endpoint.getSecID().ab[1],
							m_endpoint.getSecID().ab[2],
							m_endpoint.getSecID().ab[3] );

}
