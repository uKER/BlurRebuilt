// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdInAddr.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#ifdef BD_PS2_USE_LIBINSOCK
#include <libinsck.h>
#endif // BD_PS2_USE_LIBINSOCK

#ifdef BD_PS2_USE_LIBEENET
#include <libeenet.h>

#ifdef ip_opts
#undef ip_opts
#endif // ip_opts

#include <libeenet/netinet/in.h>
#include <libeenet/arpa/inet.h>
#endif // BD_PS2_USE_LIBEENET


bdInAddr::bdInAddr()
{
	inUn.m_iaddr = BD_IN_ADDR_NONE;
}

bdInAddr::bdInAddr(bdUInt32 addr)
{
	inUn.m_iaddr = addr;
}

bdInAddr::bdInAddr(const bdNChar8 * addr)
{
	fromString(addr);
}

void bdInAddr::fromString(const bdNChar8 *addr)
{
	m_inAddri = inet_addr(addr);
}

bdUWord bdInAddr::toString(bdNChar8 *const str, 
						   const bdUWord size) const
{
	in_addr addr;
	addr.s_addr = m_inAddri;

	bdBool ok = false;
	const bdNChar8 *const result = inet_ntoa(addr);	
	if(result)
	{
		ok = (size > bdStrlcpy(str, result, size));
	}

	bdUWord strLength = 0;
	if(ok)
	{
		strLength = bdStrlen(str);
	}

	return strLength;
}

