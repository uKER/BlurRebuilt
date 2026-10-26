// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON UNIX OSX IPHONE PSP-INFRA PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdInAddr.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#if defined BD_PLATFORM_WIN32

# pragma warning( push )
# pragma warning( disable: 4127) // warning in windows headers
#  include <winsock2.h>
# pragma warning( pop )

#elif defined BD_PLATFORM_UNIX

# include <sys/types.h>
# include <unistd.h>
# include <errno.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>

# include <signal.h>
# include <sys/ioctl.h>   /* _SGIAPI FIO* functions	*/

# include <netinet/in.h>
# include <net/if.h>

#include <stdio.h>
#elif defined BD_PLATFORM_PSP 
#include <pspnet_inet.h>
#include <pspnet/sys/socket.h>

#elif defined BD_PLATFORM_PS3

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif


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
#ifdef 	BD_PLATFORM_PSP
	m_inAddri = sceNetInetInetAddr(addr);
#else
	m_inAddri = inet_addr(addr);
#endif
}

bdUWord bdInAddr::toString(bdNChar8 *const str, 
						   const bdUWord size) const
{
#ifndef BD_PLATFORM_PSP
	in_addr addr;
	addr.s_addr = m_inAddri;
#endif

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
	const bdBool ok = (XNetInAddrToString(addr, str, (int)size) == 0);
#elif defined BD_PLATFORM_PSP
	const bdBool ok = (BD_NULL != sceNetInetInetNtop(SCE_NET_INET_AF_INET, &m_inAddri, str, size));

#else
	bdBool ok = false;
	const bdNChar8 *const result = inet_ntoa(addr);	
	if(result)
	{
		bdStrlcpy(str, result, size);
		ok = true;
	}
#endif

	bdUWord strLength = 0;
	if(ok)
	{
		strLength = bdStrlen(str);
	}
	return strLength;
}


