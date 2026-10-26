// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class for advertising games on a LAN. To be used in conjunction with
// a discovery client.

#ifndef BD_LAN_DISCOVERY_SERVER_H
#define BD_LAN_DISCOVERY_SERVER_H

#include <bdCore/bdCore.h>

#include <bdNet/bdLANDiscovery/bdLANDiscovery.h>

BD_REFERENCE(bdGameInfo);

class bdLANDiscoveryListener;

class bdLANDiscoveryServer
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Represents the various states of the discovery server.
		enum bdStatus
		{
			BD_IDLE,
			BD_PENDING,
			BD_ERROR
		};

		/// Creates a discovery server.
		bdLANDiscoveryServer();

		~bdLANDiscoveryServer();

		/// Starts the discovery server
		/// \param gameInfo[in] The game info to advertise.
		/// \param localAddr[in] The local IP address and port on which the server
		/// listens for incoming discovery broadcast. By default, the server
		/// will listen to any possible address
		/// \return - True if the server could be started
		///			- False otherwise
		bdBool start(const bdGameInfoRef gameInfo,
					 const bdInetAddr& localAddr = bdInetAddr::Any(),
					 const bdPort discoveryPort = BD_DEFAULT_DISCOVERY_PORT);

		/// Updates the discovery server.
		/// broadcasts and sends out replies. This function needs to be
		/// invoked regularly in order to process any incoming
		/// broadcasts and to send out replies.
		void update();

		/// Stops the discovery server
		void stop();

		/// Registers a listener for discovery events
		/// \param listener[in] : The listener to be registered
		void registerListener(bdLANDiscoveryListener *listener);

		/// Removes a previously registered listener
		/// \param listener[in] : The listener to be removed
		void unregisterListener(bdLANDiscoveryListener *listener);

		bdStatus getStatus() const;

		bdGameInfoRef getGameInfo();

	protected:

		/// Stores the game information to be sent out to discovery clients
		bdGameInfoRef m_gameInfo;

		// Socket for sending/receiving discovery messages
		bdDefaultSocket m_socket;

		/// List of all registered listeners
		bdFastArray<bdLANDiscoveryListener*> m_listeners;

		/// The current status of this class.
		bdStatus m_status;

#ifdef	BD_PLATFORM_PSP_ADHOC
		bdInt m_contextID;
#endif
};

#endif // BD_LAN_DISCOVERY_SERVER_H

