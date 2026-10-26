// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdNetStartParams.h>

bdNetStartParams::bdNetStartParams()
: 
#if defined BD_PLATFORM_WII
  m_isDummy(false),
#endif
  m_onlineGame(false),
  m_gamePort(BD_DEFAULT_PORT),
  m_socket(BD_NULL),
#if defined BD_COMPILE_RELAY_SOCKET_ROUTER
  m_useRelaySocketRouter(false),
#endif
  m_useAnyIP(true)  
{
	m_natTravHosts.pushBack("stun.us.demonware.net");
	m_natTravHosts.pushBack("stun.eu.demonware.net");
	m_natTravHosts.pushBack("stun.jp.demonware.net");
	m_natTravHosts.pushBack("stun.au.demonware.net");
	m_natTravPort = 3074;
}

#if defined BD_PLATFORM_WII
void bdNetStartParams::setLoopbackMode()
{
	m_onlineGame = false;
	bdDefaultSocket *dummy = new bdDummySocket();
	m_socket = dummy;
	m_useAnyIP = false;
	m_localAddresses.pushBack("127.0.0.1");
	m_isDummy = true;
}
#endif

