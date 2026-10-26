// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_NAT_TRAV_CLIENT_H
#define BD_NAT_TRAV_CLIENT_H

#include <bdCore/bdCore.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>

#include <bdSocket/bdPacketInterceptor.h>
#include <bdSocket/bdNAT/bdNAT.h>
#include <bdSocket/bdNAT/bdNATTravClientData.h>

class bdNATTravListener;
class bdNATTraversalPacket;
class bdServiceBandwidthArbitrator;

// Nat traversal works according to the following table.
// F - full cone
// M - Moderate
// S - Strict
// X - Can Connect
// 0 - Can't connect
//
//     F    M     S
//    ----------------
// F | X |  X  |  X  |
//   |---------------|
// M | X |  X  |  0  |
//   |---------------|
// S | X |  0  |  0  |
//   |---------------|

/// NAT Traversal client code.
///
/// This class attempts to establish a communication channel with a remote 
/// host, possibly traversing multiple NAT devices. It is aware of possibility 
/// that the remote host is on the same LAN as the local host, behind one NAT.
///
/// This is meant to run on clients, driven by \a bdSocketRouter.
class bdNATTravClient : public bdPacketInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Defines the possible states of a bdNATTravClient.
		enum bdNATTravClientStatus
		{
			BD_NAT_TRAV_UNINITIALIZED,
			BD_NAT_TRAV_INITIALIZED,
			BD_NAT_TRAV_RUNNING
		};

		/// Basic constructor.
		/// When instantiated using this constructor, init() must be called 
		/// before this class can be used.
		bdNATTravClient();

		/// Default destructor.
		virtual ~bdNATTravClient();

		/// Initialize this class.
		/// This function must be called if the basic constructor was used to 
		/// create the class.
		/// \param socket[in] The socket used for sending packets.
		/// \param bandArb[in] A pointer to an instance of bandwidth 
		///					   arbitrator, shared between connection services 
		///					   (such as QoS). Pointer must be valid for the
		///					   entire lifetime of this class instance.
		/// \param localCommonAddr[in] The local common addr.
		/// \return
		///   - True if the class was initialized correctly.
		///   - False otherwise.
		bdBool init(bdDefaultSocket *socket,
					bdServiceBandwidthArbitrator *bandArb,
					bdCommonAddrRef localCommonAddr);

		/// Perform traversal to a remote host.
		/// \param remote[in] Remote host traversal should try to connect.
		/// \param listener[in] Traversal result listener.
		/// \param throttle[in] Set to true to use bandwidth limiting on this 
		///						connection (through \a bdServiceBandwidthArbitrator).
		/// \return
		///   - True if the connect attempt was successfully started.
		///   - False otherwise.
		bdBool connect(bdCommonAddrRef remote, 
					   bdNATTravListener *listener,
					   bdBool throttle = false);

		/// Pumps active tasks & keeps connections to STUN servers alive.
		/// This function should be called every frame.
		void pump();

		/// Stops all activity in this class.
		/// This will reset the class to the uninitialized state.
		/// After this function has been called, it is safe to call init again.
		/// \return 
		///   - True if the quit was error free.
		///   - False otherwise.
		bdBool quit();

		/// Derived from \a bdPacketInterceptor.
		virtual bdBool acceptPacket(bdDefaultSocket *socket, bdAddr addr, void *data, const bdUInt size, const bdUByte8 type);

		/// Abandons traversal to a particular remote host.
		/// \param remote[in] Address of the remote host we no longer want to connect to.
		void cancelConnect(bdCommonAddrRef remote);

		/// Get the status of this class.
		/// \return The status of the class.
		bdNATTravClientStatus getStatus();

		/// Set the introducers that this class should use.
		/// Invoking this while the NAT Trav client is running will 
		/// result in undefined behaviour. 
		/// It is intended for use when switching from LAN
		/// to online mode.
		/// \param introducers[in] An array of introducers to send keep alives 
		///						   to. The first element in the array points to
		///						   the introducer we will use for introduction
		///						   requests. i.e. The first element must point 
		///						   to a working introducer.
		/// \return 
		///  - True if the introducers were successfully set.
		///  - False otherwise.
		bdBool setupIntroducers(const bdArray<bdAddr> &introducers);

		/// Update the local common addr that this class uses.
		/// Once the local common addr is altered, all new DTLS 
		/// associations will use it, whereas existing ones will use
		/// the common addr they were created with. 
		/// One effect of this is that connecting to a remote endpoint
		/// to which a DTLS already exists will result is a second DLTS,
		/// if the local common addr differs.
		/// \param newLocalCommonAddr[in] The new local common addr.
		/// \return 
		///  - True if the new local common addr was successfully set.
		///  - False otherwise.
		bdBool updateLocalCommonAddr(const bdCommonAddrRef newLocalCommonAddr);

		/// Check if we are currently doing NAT Traversal to this remote host
		bdBool isConnectInProgress(const bdCommonAddrRef remote) const;

	protected:

		/// Handle incoming packet.
		/// \param addr[in] Address the packet came from.
		/// \param packet[in] Parsed packet.
		void receiveFrom(const bdAddr &addr, bdNATTraversalPacket &packet);


		/// Send keep-alives to all introducers.
		/// \return True, if successful.
		bdBool sendKeepAlive();

		/// Send stage 1 traversal packet.
		/// \param data[in] Request we're sending it for.
		/// \return True, if successful.
		bdBool sendStage1(bdNATTravClientData &data);

		/// Send stage 2 traversal packet.
		/// \param data[in] Request we're sending it for.
		/// \return True, if successful.
		bdBool sendStage2(bdNATTravClientData &data);

		/// Send stage 3 traversal packet.
		/// \param data[in] Request we're sending it for.
		/// \return True, if successful.
		bdBool sendStage3(bdNATTravClientData &data);

		/// Compute an HMAC signature based on an identifier and a pair of addresses.
		/// \param identifier[in] The identifier.
		/// \param src[in] First address.
		/// \param dest[in] Second address.
		/// \param data[in] Buffer to receive an HMAC signature based on the identifier
		///  and a pair of addresses.
		/// \return true on success, false otherwise
		bdBool doHMac(const bdUInt32 identifier, 
					  const bdAddr &src, 
					  const bdAddr &dest,
					  bdUByte8 data[BD_NAT_TRAV_HMAC_SIZE]) const;

	protected:

		/// Our bandwidth arbitrator.
		bdServiceBandwidthArbitrator *m_bandArb;

		/// All known introducers.
		bdArray<bdAddr> m_introducers;

		/// Socket for outgoing data.
		bdDefaultSocket *m_socket;

		// Map of the address to where we are sending the packets to
		// data required to callback.
		bdHashMap<bdUInt32, bdNATTravClientData> m_callbacks;

		/// Timer for sending keep alives to introducers.
		bdStopwatch m_keepAliveTimer;

		/// Our random secret key for signing requests.
		bdUByte8 m_secretKey[BD_ECC_SHARED_SECRET_SIZE];

		/// The status of this Nat traversal client.
		bdNATTravClientStatus m_status;

		bdCommonAddrRef m_localCommonAddr;
};

#endif // BD_NAT_TRAV_CLIENT_H
