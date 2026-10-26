// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_IP_DISCOVERY_CLIENT_H
#define BD_IP_DISCOVERY_CLIENT_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdNAT/bdIPDiscoveryConfig.h>

class bdIPDiscoveryClient
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Various states through which the class transitions during 
		/// operation.
		enum bdIPDiscoveryClientStatus
		{
			BD_IP_DISC_UNINITIALIZED = 0,
			BD_IP_DISC_RUNNING,
			BD_IP_DISC_SUCCESS,
			BD_IP_DISC_FAIL,
			BD_IP_DISC_ERROR
		};

		/// Constructor.
		bdIPDiscoveryClient();

		/// Destructor
		~bdIPDiscoveryClient();

		/// Initialize the class.
		/// This starts the IP discovery process.
		/// \note While this class is running no other class/code 
		///       can use the socket.
		/// \param socket [in] The socket instance to use.
		/// \param server [in] The addr of the IP discovery server.
		/// \param config [in] Optional configuration info for the class.
		/// \return:
		///  - True if IP discovery was successfully started.
		///  - False otherwise.
		bdBool init(bdDefaultSocket *socket,
					const bdAddr &server,
					bdIPDiscoveryConfig config = bdIPDiscoveryConfig());

		/// Pumps the class, and processes any replies received.
		void pump(bdAddr fromAddr, bdUByte8 data[],	const bdInt dataSize);

		/// Called to reset the class to it's initial state.
		/// When called: 
		///  - during discovery it will abandon discovery.
		///  - after discovery is complete it will reset the class 
		///    to it's uninitialized state.
		/// \note This function will always revert the class to a state where
		///       init can safely be invoked again.
		void quit();

		/// Returns the public IP address and port.
		/// This function only returns a meaningful value after the discovery 
		/// has been completed.
		/// \return: The public IP address and port to which the local address 
		///          is mapped.
		const bdAddr& getPublicAddress() const;

		/// Accessor for the internal status of the IP discovery client.
		bdIPDiscoveryClientStatus getStatus() const;

	protected:

		/// Send an IP discovery packet to the configured server address.
		/// \return:
		///  - True if the packet was successfully sent.
		///  - False otherwise.
		bdBool sendIPDiscoveryPacket();

	protected:

		// The socket to use when discovering our external address.
		bdDefaultSocket *m_socket;

		// The address of the server used for IP Discovery.
		bdAddr m_serverAddr;

		// The external/public address to which our local address has been 
		// mapped by the NAT.
		bdAddr m_publicAddr;

		// Timer to track how long IP discovery has been running
		bdStopwatch m_timer;

		// The internal status of the IP discovery client
		bdIPDiscoveryClientStatus m_status;

		// The configuration info.
		bdIPDiscoveryConfig m_config;

		// Counter to track how many retries have been attempted.
		bdUInt m_retries;

};

#endif // BD_IP_DISCOVERY_CLIENT_H
