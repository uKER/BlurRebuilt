// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The gateway to our network engine.

#ifndef BD_NET_H
#define BD_NET_H

#include <bdCore/bdCore.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>
#include <bdCore/bdSocket/bdInetAddr.h>
#include <bdSocket/bdSocketRouter.h>
#if defined BD_COMPILE_RELAY_SOCKET_ROUTER
#include <bdSocket/bdRelay/bdRelaySocketRouter.h>
#endif
#include <bdConnection/bdConnectionStore.h>
#include <bdSocket/bdNAT/bdNATTypeDiscoveryClient.h>
#include <bdSocket/bdNAT/bdIPDiscoveryClient.h>
#include <bdNet/bdGetHostByName/bdGetHostByName.h>
#include <bdNet/bdNetStartParams.h>
#include <bdNet/bdUPnP/bdUPnP.h>

#if (!defined(BD_PLATFORM_XBOX)) && (!defined(BD_PLATFORM_XENON)) && (!defined(BD_PLATFORM_PSP_ADHOC))
#include <bdNet/bdUPnP/bdUPnP.h>
#endif

class bdConnectionListener;


/// The gateway to our network engine.
/// This is the central access point for our network engine.
/// It must be started before any network messages can be sent/received.
class bdNetImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdNetStatus
		{
			BD_NET_PARAMS_CONFIG_ERROR = -4,
			BD_NET_BIND_FAILED = -3,
			BD_NET_ONLINE_FAILED = -2,
			BD_NET_INIT_FAILED = -1,
			BD_NET_STOPPED,
			BD_NET_PENDING,
			BD_NET_DONE
		};

		/// Start the net engine. This will perform NAT traversal and UPnP discovery
		/// if the m_online flag in bdNetStartParams has been set to true.
		/// pump() needs to be called until completion.
		/// \param[in] params : A structure containing various startup parameters
		/// \note If the m_socket member of this class is set you need to ensure
		///       that create() has been called on it before calling this function.
		///       If using the default socket create() will be called internally in
		///       this function.
		bdBool start(const bdNetStartParams& params = bdNetStartParams());

		/// Pumps the startup process of the network subsystem.
		void pump();

		/// Stop the net engine.
		/// Closes all open connections.
		/// \note This call will take a relatively long time (~1000ms) to
		/// complete as it does a controlled shutdown of the connections.
		void stop();

		/// Gets the current state of the net engine
		/// \return The current state
		bdNetStatus getStatus() const;

		/// Send as many of the queued packets as possible.
		/// This function will loop through all the live connections
		/// and try to send any queued packets.
		bdBool sendAll();

		/// Receive as many packets as possible.
		/// This will drive bdConnectionStore which tries to
		/// receive data from the listening socket and send it to the
		/// relevant connection.
		bdBool receiveAll();

		/// Takes all messages from the connections and tries to dispatch them
		/// to their relevant callback objects or registered interceptors
		void dispatchAll();

		/// Gets a pointer to the internal connection store.
		/// Applications can register connection listeners with the connection store
		/// in order to be notified about connection related events.
		/// \return A pointer to the connection store.
		bdConnectionStore *getConnectionStore();

		/// Retrieves the local common address used for this machine.
		/// A common address is used to uniquely identify a machine on the network.
		/// This function has to be called after bdNet::start();
		/// \return A reference to the local common address.
		bdCommonAddrRef getLocalCommonAddr() const;

		/// Gets a reference to the startup parameters that where provided.
		/// \return A reference to the startup parameters
		const bdNetStartParams& getParams();

		/// Gets a pointer to the internal socket router.
		/// \return A pointer to the internal socket router.
		bdSocketRouter* getSocketRouter();

	protected:

		/// Grant friendship to bdSingleton<bdNetImpl> so that it can
		/// construct the singleton instance of this class.
		friend class bdSingleton<bdNetImpl>;

		/// Default constructor.
		/// This is protected so that this class cannot be instantiated, except
		/// as a singleton.
		bdNetImpl();

		/// Virtual destructor.
		virtual ~bdNetImpl();

// These functions do not exist on all platforms.
#if (!defined(BD_PLATFORM_XBOX)) && (!defined(BD_PLATFORM_XENON)) && (!defined(BD_PLATFORM_PSP_ADHOC))
		bdBool findFreePort(bdAddr& addr);

		bdBool getBindAddr(bdAddr &addr);


#endif

	protected:

		/// Stores the startup parameters that were used
		bdNetStartParams m_params;

		/// A class to handle input and output of data to and from connections.
		bdConnectionStore m_connectionStore;

		bdSocketRouter *m_socketRouter;

		/// Internal status
		bdNetStatus m_status;

#if (!defined(BD_PLATFORM_XBOX)) && (!defined(BD_PLATFORM_XENON)) && (!defined(BD_PLATFORM_PSP_ADHOC))

		bdIPDiscoveryClient *m_ipDiscClient;
		bdNATTypeDiscoveryClient *m_natTypeDiscClient;
		bdGetHostByName *m_getHostByName;
		bdUPnP m_UPnP;
		bdArray<bdAddr> m_natTravAddrs;

		bdUInt m_currentNatTravAddrIndex;
		bdUInt m_currentNatTravHostIndex;

		bdUInt m_upnpCollisionRetryCount;

#endif




};

/// This class provides a point of access to the singleton instance.
/// Implementation details can be found in bdNetImpl.
class bdNet : public bdSingleton<bdNetImpl>
{
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	protected:
		/// Default constructor.
		bdNet();
};

#endif // BD_NET_H

