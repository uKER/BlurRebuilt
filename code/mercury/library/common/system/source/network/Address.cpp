#include "SystemPch.h"

#ifdef PLATFORM_PS3
#include <system/network/platform.h>
#endif
#include <system/network/address.h>

#if !defined( COMPILER_VC )
#if defined( PLATFORM_PS3 )
#include <netdb.h>			// gethostbyname
#include <arpa/inet.h>		// inet_aton
#include <netex/net.h>
#include <netex/ifctl.h>
#else
#include <sys/unistd.h>		// gethostname
#include <netdb.h>			// gethostbyname
#include <arpa/inet.h>		// inet_aton
#endif
#endif

namespace Network
{

/// default constructor - initialize to 0.0.0.0
Address::Address()
{
	SetAddress(NULL);
}

/// Initialize an address using a string (either x.x.x.x or hostname format)
Address::Address(const char *address)
{
	if (address == 0 || !strcmp(address, "*"))
		SetAddress(NULL);
	else
		SetAddress(address);
}

/// copy constructor
Address::Address(const Address &rhs)
{
	m_IPAddr = rhs.m_IPAddr;
}

/// destructor
Address::~Address()
{
}

/// return the address as a platform-specific in_addr struct
struct in_addr Address::GetAddress() const
{
	return m_IPAddr[0];
}

#if defined( PLATFORM_PS3)
int GetPS3NetworkAdapterID()
{
	int id_list[10] = { 0 };
//#warning ========> sys_net_get_if_list has been removed from the PS3 network library!
//	if (sys_net_get_if_list(id_list,sizeof(id_list) / sizeof(int)) <= 0)
//		return -1;

	return id_list[0];
}

#endif

/// set the outward facing network ip address of the local host
LocalHost::LocalHost()
{
#if defined( PLATFORM_PS3 )
	// get the network address of the first network adapter
	int id = GetPS3NetworkAdapterID();
	if (id != -1)
	{
		m_IPAddr.Destroy();

		static struct in_addr tmpaddr;
//#warning ========> error: 'SYS_NET_CC_GET_ADDRESS' was not declared in this scope
//		sys_net_if_ctl(id, SYS_NET_CC_GET_ADDRESS, &tmpaddr, sizeof(tmpaddr));
		m_IPAddr.PushBack( tmpaddr );
		return;
	}

	SetAddress("0.0.0.0");
#else
	char namebuf[256];
	gethostname(namebuf, 256);
	SetAddress(namebuf);
#endif
}

/// Sets the IP address from a string representation of the numeric address, ie "127.0.0.1"
bool Address::SetIPAddress(const char *host)
{
	if (!host)
		return false;

#if defined ( COMPILER_VC )
	// microsoft implementation
	struct sockaddr_in addr;	
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = inet_addr(host);
	if(addr.sin_addr.s_addr == INADDR_NONE)
		return false;

	struct in_addr tmpaddr;
	memcpy(&tmpaddr, &addr.sin_addr, sizeof(struct in_addr));
	m_IPAddr.PushBack( tmpaddr );
#else
	// linux implementation
	struct in_addr l_addr;
	
	int ok = inet_aton(host, &l_addr);
	if ( !ok )
		return false;
	m_IPAddr.PushBack( l_addr );
#endif
	return true;
}

/// Specify a host name or numeric internet address
void Address::SetAddress(const char *host)
{	
	m_IPAddr.Destroy();

	// try to set as IP address
	if (SetIPAddress(host))
		return;

	// try to set as hostname
	struct hostent *hp;
	hp = gethostbyname(host);
	if (hp)
	{
		// success - copy the resulting host IP addresses
		for (struct in_addr **bptr = (struct in_addr **)hp->h_addr_list; *bptr != NULL; bptr++)
			m_IPAddr.PushBack( **bptr );
		return;
	}

	// fail - initialize to 0.0.0.0
	m_IPAddr.PushBack();
	memset((void *)&m_IPAddr[0], 0, sizeof(struct in_addr));
}

/// retrieve the hostname
/// MICROSOFT: The app needs to call WSAStartup(WINSOCK_VERSION, &WSADATA()) on initialization or this will return ""
Util::C_String GetHostName()
{
#ifndef CFG_GOLD
	char buffer[4096] = "";

#if defined( PLATFORM_PS3 )
	// get the name of the first network adapter
	int id = GetPS3NetworkAdapterID();
//#warning ==========>error: 'SYS_NET_CC_GET_DEVICE_NAME' was not declared in this scope
//	if (id != -1) sys_net_if_ctl(id, SYS_NET_CC_GET_DEVICE_NAME, &buffer, sizeof(buffer));
#else
	gethostname(buffer, 4096);
#endif
	
	return Util::C_String(buffer);

#else

	return Util::C_String("");
#endif
}

}
