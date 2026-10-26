// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Provides a NAT traversed & encrypted socket-like API.

#ifndef BD_SOCKET_ROUTER_H
#define BD_SOCKET_ROUTER_H

#include <bdCore/bdCore.h>

#include <bdSocket/bdSecurityKeyMap.h>
#include <bdSocket/bdAddressMap.h>
#include <bdSocket/bdSocketRouterConfig.h>


#if !defined BD_PLATFORM_PSP_ADHOC

#include <bdSocket/bdDTLS/bdDTLSAssociation.h>
#include <bdSocket/bdQoS/bdQoSProbe.h>

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 

#include <bdCore/bdCrypto/bdECCKey.h>
#include <bdSocket/bdServiceBandwidthArbitrator.h>
#include <bdSocket/bdNAT/bdNATTravClient.h>
#include <bdSocket/bdSecurityKeyMapListener.h>

#endif

#else

enum bdDTLSAssociationStatus
{
	BD_SOCKET_IDLE,
	BD_SOCKET_PENDING,
	BD_SOCKET_CONNECTED,
	BD_SOCKET_LOST
};

#endif // !BD_PLATFORM_PSP_ADHOC


class bdPacketInterceptor;

/// Socket-like interface to NAT traversed & encrypted association.
/// This class can be used to create a NAT traversed & encrypted
/// association that can then be used send & receive data unreliably
/// between instances.
#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON && !defined BD_PLATFORM_PSP_ADHOC
class bdSocketRouter : public bdNATTravListener,
					   public bdSecurityKeyMapListener
#else
class bdSocketRouter
#endif //#ifndef BD_PLATFORM_PSP_ADHOC
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Internal status of the socket router.
		enum bdSocketRouterStatus
		{
			BD_SOCKET_ROUTER_UNINITIALIZED,
			BD_SOCKET_ROUTER_INITIALIZED,
			BD_SOCKET_ROUTER_ERROR
		};

		/// Basic constructor.
		/// A instance created with this constructor cannot be used until
		/// init() has been called in it.
		bdSocketRouter();

		/// Virtual destructor
		virtual ~bdSocketRouter();

		/// Initialize the class.
		/// If the class was created using the basic constructor or if quit()
		/// has been called then this function must be run before the class
		/// can be used.
		/// \param socket [in] The socket instance to use.
		/// \param localCommonAddr [in] The local common addr of this instance.
		/// \param config [in] Optional configuration info for the class.
		/// \return
		///  - True, if the class was successfully initialized.
		///  - False otherwise.
		bdBool init(bdDefaultSocket *socket,
					const bdCommonAddrRef localCommonAddr,
					const bdSocketRouterConfig &config = bdSocketRouterConfig());

		/// Pump the assorted associations.
		/// This drives the socket router, and should be pumped every frame.
		virtual void pump();

		/// Stops all activity in this class.
		/// This will reset the class to the uninitialized state.
		/// After this function has been called, it is safe to call init again.
		/// \return
		///   - True if the quit was error free.
		///   - False otherwise.
		virtual bdBool quit();

		/// Start the association setup process.
		/// This is an asymmetric process as NAT traversal can take of the
		/// order of tens of seconds to complete or fail.
		/// \param addrHandle [in] The address to connect to.
		/// \return
		///  - True, if the connect process was successfully started.
		///  - False otherwise.
		/// \note The connection process is asymmetric, bdSocketRouter::getStatus(handle)
		///		can be polled to find when the process either fails or succeeds.
		///		Both pump() & receiveFrom() must be called regularly in order to drive
		///		the setup process. Also, at the very least, the connectee must be 
		///		actively calling pump() & receiveFrom().
		/// \note Keep-alives must be sent in order to maintain the association. 
		///		The calling code is responsible for ensuring that these get sent, however
		///		no special handling is required on the receivers end. The act of sending 
		///		the data is sufficient to keep the association alive.
		virtual bdBool connect(bdAddrHandleRef &addrHandle);

		/// Close an established secure association.
		/// \param addrHandle [in] The address identifying the association to
		///                        be closed.
		/// \return
		///  - True, if the disconnect process was successfully started.
		///  - False otherwise.
		virtual bdBool disconnect(bdAddrHandleRef &addrHandle, bdBool allowDisconnectBroadcast = true);

		/// Get the status of an individual association.
		/// \param addrHandle [in] The address to identify the association.
		/// \return The status of the underlying association matching addrHandle.
		virtual bdDTLSAssociationStatus getStatus(const bdAddrHandleRef addrHandle);

		/// Get the internal status of an socket router.
		/// \return The status of the socket router.
		bdSocketRouterStatus getStatus();

		/// Send data to addr.
		/// \param addrHandle[in]	The handle to send the data to.
		/// \param data[in]		A pointer to the data. <B> See note below. </B>
		/// \param length[in]	The length of the data.
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data sent.
		///  - If negative then an error from bdSocketStatusCode.
		/// \note	The data passed to this function must be in a certain format.
		///			It is possible to send both encrypted & unencrypted data in one
		///			packet. The special formatting indicates which parts of the buffer
		///			are to be encrypted and which need not be.
		///			The <B>first 2 bytes in buffer</B> represent the size of the
		///			encrypted portion. The size of the unencrypted data is then
		///			determined to be the remainder of the data. 
		///			The size information should be written into the 
		///			buffer using bdBytePacker::appendBasicType() & bdUInt16.
		virtual bdInt sendTo(const bdAddrHandleRef addrHandle,
							 void *data,
							 const bdUInt length);

		/// Receive data.
		/// Make sure size is greater than the max packet size or data will be
		/// lost.
		/// \param addrHandle[out] The address handle of the sender.
		/// \param data[out]	A pointer to a buffer into which the data read
		///						from the socket will be read. <B> See note below. </B>
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data read into buffer.
		///  - If negative then an error from bdSocketStatusCode.
		/// \note	The data received from this function is in a certain format.
		///			It is possible to transmit both encrypted & unencrypted data in one
		///			packet. The special formatting indicates how much of the buffer
		///			is encrypted and how much is not. 
		///			The <B>first 2 bytes in the buffer</B> represent the size of the 
		///			encrypted portion. This size information should be read 
		///			from the buffer using bdBytePacker::removeBasicType() & bdUInt16.
		///			The size of the unencrypted data is then determined to be the
		///			remainder of the data.
		virtual bdInt receiveFrom(	bdAddrHandleRef &addrHandle,
									void *data,
									const bdUInt size,
									bool& didErrorOccur
									);

		/// Registers an interceptor with the router.
		/// \param interceptor[in]	The interceptor to be registered with this
		///							router.
		void registerInterceptor(bdPacketInterceptor *const interceptor);

		/// Removes an existing interceptor from the router.
		/// If the same interceptor has been registered multiple times, this
		/// call will remove all instances.
		/// \param interceptor[in]	The interceptor to be removed from this
		///							router.
		void unregisterInterceptor(bdPacketInterceptor *const interceptor);


// This data exists on all platforms except PSP ad hoc.
#if !defined BD_PLATFORM_PSP_ADHOC

// Furthermore this data exists on all platforms except XBox & Xbox 360[aka Xenon]
#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 
		// Implemented from bdNATTravListener
		// see base class description.
		virtual void onNATAddrDiscovery(bdCommonAddrRef remote,
										const bdAddr &realAddr);

		// Implemented from bdNATTravListener
		// see base class description.
		virtual void onNATAddrDiscoveryFailed(bdCommonAddrRef remote);

		// Implemented from bdSecurityKeyMapListener
		// see base class description.
		virtual void onSecurityKeyRemove(const bdSecurityID &id);

		/// Setup the introducer array that this class should use.
		/// This sets up the introducers that are used during NAT traversal.
		/// This function will overwrite any existing introducers that are
		/// setup.
		/// \param introducers[in]	An array of introducers addrs.
		/// \return :
		///  - True if the introducers were correctly setup.
		///  - False otherwise.
		/// \note The 0th entry, if it exists in the introducers array, 
		///       is the only one used for making outgoing NAT traversal requests
		///       so the calling code should ensure that the 0th entry works.
		///       Connections are kept open to all introducers in order that
		///       incoming NAT traversal requests can be accepted from any of them.
		bdBool setupIntroducers(const bdArray<bdAddr> &introducers);

#endif

		/// Returns a pointer to the QoS prober object. The pointer is only good
		/// while this object instance is valid.
		bdQoSProbe* getQoSProber();

// Finally this data exists on the PSP ad hoc platform when socket broadcast is turned on
#elif defined BD_SOCKET_DO_BROADCAST 

		/// Flush the buffer.
		/// This is only implemented on PSP in ad-hoc mode, it ensures data is sent
		/// immediately rather than during the next frame.
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data sent.
		///  - If negative then an error from bdSocketStatusCode.
		bdInt flushBuffer();
#endif // #if defined BD_SOCKET_DO_BROADCAST 

		/// Accessor for the socket router config.
		/// \param config[out] The config that the socket router is using. 
		void getConfig(bdSocketRouterConfig &config) const;

		/// Accessor for the address map used by the socket router.
		/// \return A pointer to the address map.
		bdAddressMap* getAddressMap();

		/// Gets a pointer to the internal security key map.
		/// \return A pointer to the internal security key map.
		bdSecurityKeyMap* getKeyMap();

		/// Alter the local common addr.
		/// \param newLocalCommonAddr[in]	The new local common addr.
		/// \return :
		///  - True if the new local common addr was correctly setup.
		///  - False otherwise.
		bdBool updateLocalCommonAddr(const bdCommonAddrRef newLocalCommonAddr);

		/// Accessor for the currently configured local common addr.
		/// \return A ref to the current local common addr.
		bdCommonAddrRef getLocalCommonAddr() const;

		/// Virtual function to determine whether an bdSocketRouter pointer points
		/// to a bdRelaySocketRouter or not.
		/// \return true if relay is enabled and false otherwise
		virtual bdBool isRelayEnabled() const;

	protected:

		/// Helper function which processes a received packet.
		/// \param addrHandle[out]		The addrHandle we received from.
		/// \param addr[in]				The real addr that we received from.
		/// \param data[out]			A pointer to a buffer where the data will go.
		/// \param size[in]				The size of the buffer.
		/// \param bytesTransfered[in]	The number of bytes received from the raw socket.
		/// \param receiving[out]		Whether we are done receiveing.
		/// \return A signed integer representing:
		///  - If positive, the amount of amount of data read into buffer.
		///  - If negative then an error from bdSocketStatusCode.
		bdInt processPacket(bdAddrHandleRef &addrHandle,
							bdAddr &realAddr, 
							void *data,
							const bdUInt size,
							const bdUInt bytesTransferred,
							bdBool &receiving );
		
		/// Helper function which processes socket receive errors
		/// \param realAddr[in]			The real Addr that we received from.
		/// \param val[in]				The error received from the socket.
		/// \param resetCount[out]		The number of connection resets we have received.
		/// \param receiving[out]		Whether we are done receiving.
		void processError( bdAddr &realAddr, bdInt val, bdUInt &resetCount, bdBool &receiving);

		/// Pointer to the socket used by this class.
		bdDefaultSocket *m_socket;

		/// Container for the currently registered interceptors.
		bdFastArray<bdPacketInterceptor*> m_interceptors;

		// The status of the socket router.
		bdSocketRouterStatus m_status;

		/// The config of the socket router.
		bdSocketRouterConfig m_config;

		/// The local common address.
		bdCommonAddrRef m_localCommonAddr;

		/// The address map that the socket router uses.
		bdAddressMap m_addrMap;

		/// A store of security key/id pairs
		bdSecurityKeyMap m_keyStore;


// This data exists on all platforms except PSP ad hoc.
#if !defined BD_PLATFORM_PSP_ADHOC 
	
		/// The QoS probing class that the socket router manages.
		bdQoSProbe m_qosProber;

// Furthermore this data exists on all platforms except XBox & Xbox 360[aka Xenon]
#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 

		/// The NAT trav client used by the socket router.
		bdNATTravClient m_natTrav;

		/// The encryption key used by the socket router.
		bdECCKey m_ECCKey;

		/// The bandwidth arbitrator used by the socket router.
		bdServiceBandwidthArbitrator m_qosBandwidth;

		// Map of real addrs to sockets.
		bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass> m_dtls;
#endif

		/// Transient container to store addr handle -> common addr mappings that are
		/// not yet fully setup.
		bdHashMap<bdEndpoint, bdAddrHandleRef, bdEndpointHashingClass> m_endpointToAddrMap;

// Finally this data exists on the PSP ad hoc platform when socket broadcast is turned on.
#elif defined BD_SOCKET_DO_BROADCAST 

		bdUByte8 *m_buffer;
		bdUInt m_bufferOffset;
		bdPort m_destPort;
		bdInetAddr m_localAddr;

#endif 
	private:

		/// Helper Function to iterate through associations and try to see if the
		/// data is on a valid bdSecurityID/bdAddr
		/// \param data[out] data buffer pointer to receive data on
		/// \param size[in] size of data buffer
		/// \param secID[in] bdSecurityID associated with DTLS association  
		/// \param realAddr[in] bdAddr associated with DTLS association 
		/// \param addrHandle[in] the bdAddrHandleRef to receive data on
		/// \param val[out] return value from receiveFrom function 
		/// \param checkSecID[in] bdBool whether to check on sec ID or not (defaults to true)
		/// \return :
		///  - True if the data was successfully received
		///  - False otherwise
		bdBool tryDecryptPacket(void *data,
									  const bdUInt& size, 
									  const bdSecurityID& secID, 
									  bdAddr& realAddr,
									  bdAddrHandleRef& addrHandle,
									  bdInt& val,
									  bdBool checkSecID = true);
		/// Helper Function to iterate through associations and try to see if the
		/// connection exists
		/// \param data[out] data buffer pointer to receive data on
		/// \param size[in] size of data buffer
		/// \param secID[in] bdSecurityID associated with DTLS association  
		/// \param realAddr[in] bdAddr associated with DTLS association 
		/// \param addrHandle[in] the bdAddrHandleRef to receive data on
		/// \param val[out] return value from receiveFrom function 
		/// \param checkSecID[in] bdBool whether to check on sec ID or not (defaults to true)
		/// \return :
		///  - True if the data was successfully received
		///  - False otherwise
		bdBool tryToFindConnection(void *data,
									  const bdUInt& size, 
									  const bdSecurityID& secID, 
									  bdAddr& realAddr,
									  bdAddrHandleRef& addrHandle,
									  bdInt& val,
									  bdBool checkSecID = true);
};
#endif // BD_SOCKET_ROUTER_H
