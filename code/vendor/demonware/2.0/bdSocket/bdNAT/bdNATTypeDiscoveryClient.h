// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to detect the NAT type that a game instance is behind.

#ifndef BD_NAT_TYPE_DISCOVERY_CLIENT_H
#define BD_NAT_TYPE_DISCOVERY_CLIENT_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdNAT/bdPacket/bdNATTypeDiscoveryPacket.h>
#include <bdSocket/bdNAT/bdNATTypeDiscoveryConfig.h>

class bdNATTypeDiscoveryPacketReply;

/// A class that uses a a simplified version of the RFC 3489 NAT type discovery
/// to detect the NAT type between an instance and the internet proper.
/// An instance running directly on the Internet will be detected as an having
/// an open NAT.
class bdNATTypeDiscoveryClient
{

	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Various states through which the class transitions during operation.
		enum bdNATTypeDiscoveryClientState
		{
			BD_NTDCS_UNINITIALIZED = 0,
			BD_NTDCS_RUN_TEST_1,
			BD_NTDCS_RUN_TEST_2,
			BD_NTDCS_RUN_TEST_3,
			BD_NTDCS_FINI,
			BD_NTDCS_ERROR
		};

		/// Basic constructor.
		bdNATTypeDiscoveryClient();

		/// Basic destructor.
		~bdNATTypeDiscoveryClient();

		/// Start the discovery process.
		/// NAT type discovery can only be run once. To run again
		/// you must call quit() and then init() again. This ensures
		/// that the test is accurate as a new randomly chosen port will be
		/// used for every run.
		/// \param clientAddr [in] The IP address to bind to for testing.
		/// \param serverAddr [in] The address of the server to run the tests
		///                        against.
		/// \param config     [in] Optional parameter to configure how the 
		///                        detection behaves.
		/// \note
		/// It is not necessary to alter the default config unless fine grained 
		/// control of the NAT type discovery process is required. The class 
		/// will behave correctly using the default values, and setting bad 
		/// values in this function may actually degrade the accuracy of any 
		/// tests.
		/// \return
		///  - True, if the discovery was successfully initialized.
		///  - False otherwise.
		bdBool init(bdDefaultSocket *socket,
					const bdAddr &serverAddr,
					bdNATTypeDiscoveryConfig config = bdNATTypeDiscoveryConfig());

		/// Pump the discovery client.
		/// The client should be pumped every frame while discovery
		/// is running.
		void pump(bdAddr fromAddr, bdUByte8 data[],	const bdInt dataSize);

		/// Stop & clean up the discovery client.
		/// This should be called once the discovery process is finished
		/// or if an error is encountered. Once this is called it is
		/// safe to call init() again.
		/// \return
		///  - True, if quitting was error free.
		///  - False otherwise.
		bdBool quit();

		/// Simple check on whether the discovery process is still running.
		/// \return
		///  - True, if discovery is still running.
		///  - False otherwise.
		bdBool isRunning() const;

		/// Return a descriptor describing the discovered NAT.
		/// \return a descriptor describing the discovered NAT.
		bdNATType getNATType() const;

		/// Get the status of an discovery client.
		/// \return The current status of the discovery client.
		bdNATTypeDiscoveryClientState getStatus() const;

		/// Accessor for the nat type discovery client config.
		/// \param config[out] The config that the nat type discovery client is using. 
		void getConfig(bdNATTypeDiscoveryConfig &config) const;

	protected:

		bdBool sendForTest1();

		bdBool sendForTest2();

		bdBool sendForTest3();

		void handleResponse(const bdAddr &addr, const bdNATTypeDiscoveryPacketReply &reply);

		bdBool sendNATTypeDiscoveryPacket(	bdNATTypeDiscoveryPacket::bdNATTypeDiscoveryPacketRequest packetType,
											const bdAddr &serverAddr);

		void receiveReplies(bdAddr fromAddr, bdUByte8 data[], const bdInt dataSize);

		void pumpActiveTest();


	protected:

		bdDefaultSocket *m_socket;

		// The address at which the STUN server is located
		bdAddr m_serverAddr1;

		// The alternate address used by the STUN server
		bdAddr m_serverAddr2;

		bdAddr m_mappedAddr;

		bdNATType m_NATType;

		bdNATTypeDiscoveryClientState m_state;

		bdUByte8 m_resends;

		bdStopwatch m_timer;

		bdNATTypeDiscoveryConfig m_config;
};


#endif // BD_NAT_TYPE_DISCOVERY_CLIENT_H
