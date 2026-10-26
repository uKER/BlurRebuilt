// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

//PURPOSE: The discovery client is used to discover any games that are hosted on a LAN.
// A registered listener is informed in the event a game has been discovered.
// To be used in conjunction with a discovery server.

#ifndef BD_LAN_DISCOVERY_CLIENT_H
#define BD_LAN_DISCOVERY_CLIENT_H

#include <bdCore/bdCore.h>

#include <bdNet/bdLANDiscovery/bdLANDiscovery.h>

#define BD_MAX_DISCOVERY_RESPONSE_SIZE (1500u)

BD_REFERENCE(bdGameInfo);

class bdLANDiscoveryListener;

class bdLANDiscoveryClient
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdStatus
		{
			BD_IDLE,
			BD_PENDING,
			BD_ERROR
		};

		bdLANDiscoveryClient();

		virtual ~bdLANDiscoveryClient();

		/// Start a discovery process on a LAN
		/// \param titleID[in] The title ID of the game to be discovered
		/// \param timeout[in] The amount of time in seconds to wait for a reply
		/// \param addr[in] The address to which the discovery message should be sent
		///				By default the message is sent to a broadcast address
		/// \return - True, the discovery message could be sent
		///			- False, an error occurred
		bdBool discover(const bdUInt titleID,
						const bdFloat32 timeout,
						const bdInetAddr& addr = bdInetAddr::Broadcast(),
						const bdPort discoveryPort = BD_DEFAULT_DISCOVERY_PORT);

		/// Updates the state of the discovery process
		void update();

		/// Gets the status of the discovery process.
		/// \return - BD_IDLE : no discovery in progress
		///			- BD_PENDING : discovery in progress
		///			- BD_ERROR : the discovery process failed
		bdStatus getStatus() const;

		/// Registers a listener with the discovery client
		/// The listener is informed in the event a game has been discovered
		/// \param listener[in] A pointer to the listener object
		void registerListener(bdLANDiscoveryListener *listener);

		/// Removes a previously registered listener
		/// \param listener[in] A pointer to the listener to be removed
		void unregisterListener(bdLANDiscoveryListener *listener);

		void fireOnDiscoveryListeners(bdGameInfoRef gameInfo) const;

	protected:

		// The amount of time to wait for a discovery response
		bdFloat32	m_timeout;

		bdStopwatch m_timer;

		// Socket for sending/receiving discovery messages
		bdDefaultSocket m_socket;

		// A nonce for uniquely identifying discovery requests/replies
		bdUByte8 m_nonce[BD_NONCE_SIZE];

		// Array of registered listener objects
		bdFastArray<bdLANDiscoveryListener*> m_listeners;

		/// The current status of the discovery process
		bdStatus m_status;

#ifdef BD_PLATFORM_PSP_ADHOC
		bdInt m_contextID;
#endif

};

#endif // BD_LAN_DISCOVERY_CLIENT_H

