// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_NET_START_PARAMS_H
#define BD_NET_START_PARAMS_H

#include <bdCore/bdCore.h>
#include <bdNet/bdGetHostByName/bdGetHostByNameConfig.h>
#include <bdNet/bdUPnP/bdUPnPConfig.h>

#if !defined BD_PLATFORM_XBOX && ! defined BD_PLATFORM_XENON
# define BD_DEFAULT_PORT (3074)
#else
# define BD_DEFAULT_PORT (1000)
#endif

class bdNetStartParams
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdNetStartParams();

#if defined BD_PLATFORM_WII
		// Sets m_onlineGame to false
		// and sets a dummy socket as the socket.
		void setLoopbackMode();

		// Whether we are using dummy sockets or not
		bdBool m_isDummy;
#endif

		/// Whether or not start an online game. An online game needs
		/// to contact the NAT server to find its public IP address
		/// and NAT Type.
		bdBool m_onlineGame;

		/// The port the engine listens on.
		bdPort m_gamePort;

		/// The socket the engine uses to send and receive data.
		/// \note: This socket will be deleted when bdNet::stop is called.
		bdDefaultSocket *m_socket;

		/// The address of the NAT traversal server.
		bdArray<bdString> m_natTravHosts;

		/// The port for the NAT traversal server.
		bdPort m_natTravPort;

		/// An array of IP addresses that this machine listens on.
		/// If more than one IP address is given the m_socket will be bound
		/// to address "0.0.0.0" but if there is only 1 IP address the socket
		/// will be bound to that address. This facilitates running several
		/// different instances on the same port but different addresses which
		/// is often needed for dedicated servers.
		bdArray<bdInetAddr> m_localAddresses;

#if (!defined(BD_PLATFORM_XBOX)) && (!defined(BD_PLATFORM_XENON)) && (!defined(BD_PLATFORM_PSP_ADHOC))
		/// The config info for a DNS lookup (gethostbyname)
		bdGetHostByNameConfig m_hostNameLookupConfig;

		/// The configuration information for the UPnP code.
		/// This class will work correctly using the default settings,
		bdUPnPConfig m_UPnPConfig;

#endif
#if defined BD_COMPILE_RELAY_SOCKET_ROUTER
		/// When set to true, indicates that bdNet will use a bdRelaySocket router
		bdBool m_useRelaySocketRouter;
#endif
		/// When set to false indicates that the bdNet code should bind to a
		/// specific IP address, the one specified in m_localAddresses[0].
		bdBool m_useAnyIP;

};



#endif // BD_NET_START_PARAMS_H
