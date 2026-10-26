// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_RELAY_SOCKET_ROUTER_H
#define BD_RELAY_SOCKET_ROUTER_H

#include <bdSocket/bdSocketRouter.h>

#include <bdSocket/bdRelay/bdRelayPacket.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayForward.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayFinal.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayRouteAdd.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayRouteRemove.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayerInfo.h>

#include <bdSocket/bdRelay/bdRelayRouteManager.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#include <bdSocket/bdAddrHandleRefWrapper.h>

BD_REFERENCE(bdRelaySocketRouter);

#define BD_RELAY_BITRATE_CALC_ALPHA (0.5f)
#define BD_RELAY_BITRATE_CALC_PERIOD (1.5f)

#define BD_BUFFER_INFO_REQUESTS
#define BD_BUFFER_INFO_REPLIES
#define BD_BUFFER_ROUTE_ADD
#define BD_BUFFER_ROUTE_REMOVE
#define BD_BUFFER_RELAYED_MESSAGES

#define BD_INFO_REQUEST_TTL 1
#define BD_INFO_REPLIES_TTL 1
#define BD_ROUTE_ADD_TTL 1
#define BD_ROUTE_REMOVE_TTL 1
#define BD_RELAYED_MESSAGES_TTL 1

// This is not a timeout for the connection. It is used to keep track of active connections
#define BD_RELAY_CONNECTION_TIMEOUT (2.f * BD_RELAY_BITRATE_CALC_PERIOD)

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
// This is for xbox only since the underlying direct connection will not time out for 1.5 minutes
// we need a faster way to decide that a direct connection is dead and switch to a relayer.
// This value translates into a direct connection timeout of BD_DIRECT_MAX_MISSED_REPLIES * 
// BD_RELAY_BITRATE_CALC_PERIOD seconds
#define BD_DIRECT_MAX_MISSED_REPLIES (9u)
#endif


class bdRelaySocketRouter 
	: public bdSocketRouter
{
public:
	
	BD_DECLARE_NEW_AND_DELETE_OPERATORS;

	/// Basic Constructor
	bdRelaySocketRouter();

	/// Virtual Destructor
	virtual ~bdRelaySocketRouter();

	/// Checks the state of all dtls connections and rebuilds the routes map
	/// as needed. This drives the relay socket router, and should be pumped 
	/// every frame.
	virtual void pump();

	/// Disconnect all connections from the socket Router.
	/// This will reset the class to the uninitialized state.
	/// After this function has been called, it is safe to call init again.
	/// \return
	///   - True if the quit was error free.
	///   - False otherwise.
	virtual bdBool quit();

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON

	/// Connect to the addrHandle
	/// \param addrHandle[in]	The address to connect to
	/// \return					True if connection was started successfully
	virtual bdBool connect(bdAddrHandleRef &addrHandle );

#endif

	/// Close an established secure association. This will only close the 
	/// direct connection. There may still exist relayed connections which 
	/// can be used.
	/// \param addrHandle [in] The address identifying the association to
	///                        be closed.
	/// \return
	///  - True, if the disconnect process was successfully started.
	///  - False otherwise.
	virtual bdBool disconnect(bdAddrHandleRef &addrHandle, bdBool allowDisconnectBroadcast = true);

	/// Get the status of an individual association. If a relay connection exists
	/// the status returned is connected. Otherwise the status of the direct connection
	/// is returned.
	/// \param addrHandle [in] The address to identify the association.
	/// \return The status of the underlying relayed or direct connection.
	virtual bdDTLSAssociationStatus getStatus(const bdAddrHandleRef addrHandle);

	/// Send data to addrHandle.
	/// \param addrHandle[in]	The handle to send the data to.
	/// \param data[in]		A pointer to the data. <B> See note below. </B>
	/// \param length[in]	The length of the data.
	/// \return An signed integer representing:
	///  - If positive, the amount of amount of data sent. This may be larger than 
	///		the length passed in since a relay header may have been added.
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
	virtual bdInt sendTo(	const bdAddrHandleRef addrHandle,
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
	
	/// Sets our relayer score to a value between 1 and BD_MAX_RELAYER_LOAD. This load 
	/// is sent to all peers connected to us and is used when deciding which relayer to use.
	/// A value of BD_MAX_RELAYER_LOAD will cause will mean that we are not used as a relayer.
	/// \param load[in]		Our new load
	void setRelayerLoad( bdUInt32 load );

	/// Gets our current relayer stats. Should be used to determine our load. The average
	/// bitrate values and numRelayConnections are recomputed every BD_RELAY_BITRATE_CALC_PERIOD.
	/// The bitrate values are a weighted average of the previous bitrates.
	/// \param averageRelayBitrate[out]		The average bits relayed per second.
	/// \param averageTotalBitrate[out]		The average total bits per second received.
	/// \param load[out]					The current load setting.
	/// \param numRelayConnections[out]		The number of unique relay connections seen 
	///										in the past 2 BD_RELAY_BITRATE_CALC_PERIODs.
	/// \return								true if the socket route state is initialized.
	bdBool getRelayerStats(	bdUInt32 &averageRelayBitrate, 
							bdUInt32 &averageTotalBitrate, 
							bdUInt32 &load,
							bdUInt32 &numRelayConnections );

	/// Virtual function to determine whether an bdSocketRouter pointer points
	/// to a bdRelaySocketRouter or not.
	/// \return true if relay is enabled and false otherwise	
	virtual bdBool isRelayEnabled() const;

	/// Debug Functions
	
	/// Prints the current routes table
	/// Meant for debug and reporting purposes
	void print();

	/// Serializes the current routes tables into the buffer.
	/// Meant for debug and reporting purposes
	bdUInt serializeRoutes(void * buffer, const bdUInt bufferSize ) const 
	{ return m_relayRouteManager.serializeRoutes(buffer, bufferSize); }

	/// Sets the maximum number of connections that a node will relay
	/// Note, this value should allow for bidirectional connections. Set to -1 to remove the limit on relayed connections
	/// \param numConnections[in]		The number of connections allowed to relay
	void setMaxRelayedConnections(bdInt numConnections);

protected:

	/// Creates a dtls packet and then modifies it before sending to indicate 
	/// that this is a relay packet.
	/// \param destEndpoint[in]		The endpoint that we are sending to
	/// \param data[in]				A pointer to a buffer containing the data to send.
	/// \param length[in]			The size of the data to send.
	bdInt sendRelayPacket( const bdEndpoint destEndpoint,
							void *data,
							const bdUInt length );

	/// Pumps the bitrateCalcTimer and computes the bitRates if needed
	void pumpTimer();

	/// Pumps the relayConnections array and removes any stale connections
	void pumpRelayConnections();

	/// Processes a Relay Packet. Checks the type and calls the appropriate sub relayer.
	/// \param addrHandle[in]		The addrHandle that sent the packet.
	/// \param data[in]				A pointer to a buffer containing the decrypted packet.
	/// \param bytesTransferred[in]	The number of bytes received.
	/// \param receiving[out]		Whether we are done receiving or not.
	/// \return An signed integer representing:
	///  - If positive, the amount of amount of data read into buffer.
	///  - If negative then an error from bdSocketStatusCode.				
	bdInt processRelayPacket(	bdAddrHandleRef &addrHandle,
								void *data, 
								const bdUInt bytesTransferred,
								bdBool &receiving,
								void *gameData,
								bdUInt& gameDataRead);

	/// Processes a Relay Forward Packet. If we are connected to the destination we forward
	/// the packet, otherwise we inform the sender that we are not connected to that destination.
	/// \param addrHandle[in]		The addrHandle that sent the packet.
	/// \param data[in]				A pointer to a buffer containing the decrypted packet.
	/// \param length[in]			The number of bytes received.
	/// \return An signed integer representing whether an error occured.
	bdInt processRelayForward(		bdAddrHandleRef &addrHandle,
									void *data, 
									const bdUInt length );

	/// Processes a Relay Final Packet. Extract the payload and change the addrHandle to
	/// make it seem like the packet was sent directly.
	/// \param addrHandle[out]		The addrHandle that sent the packet through the relayer.
	/// \param data[in]				A pointer to a buffer containing the decrypted packet.
	/// \param length[in]			The number of bytes received.
	/// \return An signed integer representing:
	///  - If positive, the amount of amount of data read into buffer.
	///  - If negative then an error from bdSocketStatusCode.
	bdInt processRelayFinal(		bdAddrHandleRef &addrHandle,
									void *data, 
									const bdUInt length,
									void *gameData);

	/// Processes a Relay Route Add Packet. This informs us about who someone we are connected to 
	/// is connected to. 
	/// \param addrHandle[in]		The addrHandle that sent the packet.
	/// \param data[in]				A pointer to a buffer containing the decrypted packet.
	/// \param length[in]			The number of bytes received.
	/// \return An signed integer representing whether an error occured.
	bdInt processRelayRouteAdd(		bdAddrHandleRef &addrHandle,
									void *data, 
									const bdUInt length );

	/// Processes a Relay Route Remove Packet. This informs us about who someone we are connected to 
	/// is no longer connected to. 
	/// \param addrHandle[in]		The addrHandle that sent the packet.
	/// \param data[in]				A pointer to a buffer containing the decrypted packet.
	/// \param length[in]			The number of bytes received.
	/// \return An signed integer representing whether an error occured.
	bdInt processRelayRouteRemove(  bdAddrHandleRef &addrHandle,
									void *data, 
									const bdUInt length );

	/// Processes a Relay Info Request Packet. Fill in the latency requests and send a reply 
	/// back immediately so that they can compute their latency to us.
	/// \param addrHandle[in]		The addrHandle that sent the packet.
	/// \param data[in]				A pointer to a buffer containing the decrypted packet.
	/// \param length[in]			The number of bytes received.
	/// \return An signed integer representing whether an error occured.
	bdInt processRelayInfoRequest(  bdAddrHandleRef &addrHandle,
									void *data, 
									const bdUInt length );

	/// Processes a Relay Info Reply Packet. Set the returned vlues in the relay route manager.
	/// \param addrHandle[in]		The addrHandle that sent the packet.
	/// \param data[in]				A pointer to a buffer containing the decrypted packet.
	/// \param length[in]			The number of bytes received.
	/// \return An signed integer representing whether an error occured.
	bdInt processRelayInfoReply(	bdAddrHandleRef &addrHandle,
									void *data, 
									const bdUInt length);

	/// Send length bytes from buffer to each connected peer.
	/// \param buffer[in]		Contains the data to send
	/// \param length[in]		How much data to send
	void broadcastPacket( bdUByte8* buffer, const bdUInt length, const bdEndpoint *dontSend = BD_NULL);

	/// Tell all directly connected peers that we are now connected to newEndpoint
	/// \param newEndpoint[in]	The endpoint that we just connected to.
	void broadcastNewRoute( const bdEndpoint &newEndpoint );

	/// Tell all directly connected peers that we are no longer connected to deadEndpoint
	/// \param deadEndpoint[in]	The endpoint that we just disconnected from.
	void broadcastRemovedRoute( const bdEndpoint &deadEndpoint );

	/// Tell recipientEndpoint that we are no longer connected to the deadEndpoint.
	/// \param recipientEndpoint[in]	The receipient of the removed route message.
	/// \param deadEndpoint[in]			The endpoint that we are no longer connected to.
	void sendRemovedRoute( const bdEndpoint &recipientEndpoint, const bdEndpoint &deadEndpoint );

	/// Tell the given endpoint about everyone that we are connected to.
	/// \param endpoint[in]		The recipiend of the relay route add message.
	/// \param buffered[in]		Controls whether the route add messages are buffered or sent immediately.
	void sendConnections( const bdEndpoint &endpoint, bdBool buffered = false );

	/// Ask for relayer info (load and latencies for relayed endpoints) from all peers we are 
	/// directly connected to.
	void broadcastInfoRequests();

	/// Buffer any route messages for sending on the next client call to sendTo
	//void bufferRouteMessages(bdHashMap<bdAddrHandleRef, routePacket>& packetList, bdUByte8* buffer, const bdUInt packetLen,  const bdInt timeToLive, const bdEndpoint *newEndpoint);
	void bufferRouteMessages(bdHashMap<bdAddrHandleRef,  bdFastArray<routePacket> >& packetList, bdUByte8* buffer, const bdUInt packetLen,  const bdInt timeToLive, const bdEndpoint *newEndpoint);

	/// Write a buffered message to the outgoing packet if there is space available
	/// One for each type
	bdInt writeRelayInfoMessages(bdUByte8*& buffer, bdRelayerInfo& packetList, const bdUInt size, bdAddrHandleRef addrHandle);
	bdInt writeRouteRemoveMessages(bdUByte8*& buffer, bdArray<bdRelayRouteRemove>& packetList, bdUInt size);
	bdInt writeRouteAddMessages(bdUByte8*& buffer, bdArray<bdRelayRouteAdd>& packetList, bdUInt size);

	bdInt writeRelayData(const bdAddrHandleRef addrHandle, bdInt totalSize, bdUByte8*& buffer);

	void flushRelayMessages(const bdAddrHandleRef addr);

	/// Send the game packet stored in data. Attempts to append any
	/// buffered relay related messages to the game packet. Any outstanding packets
	/// are then sent in the minimum number of seperate packets.
	bdInt sendWithBuffering(const bdAddrHandleRef addrHandle,
							void *data,
							const bdUInt length);

	void bufferRelayInfoMessages(const bdRelayerInfo& packet);
	void bufferRouteAddMessages(bdRelayRouteAdd packet, const bdEndpoint *dontSend);
	void bufferRouteRemoveMessages(bdRelayRouteRemove packet, const bdEndpoint *dontSend);
	
	bdBool checkRouteFowardSlotAvailable(bdRelayForward& forwardPacket);
	void broadcastNewRouteSlot();
	bdBool isRelaying(const bdEndpoint& endpoint);



protected:

	/// Manages the relay routes
	bdRelayRouteManager m_relayRouteManager;

	/// An array to keep track of active relayed connections. New connections are added to 
	/// this list when they come in and time out if no activity is seen on them for 
	/// 2 * BD_RELAY_BITRATE_CALC_PERIOD seconds.
	bdArray <bdRelayForward> m_relayConnections;

	/// Timer used to determine when to calculate bitrate stats
	bdStopwatch m_bitrateCalcTimer;

	/// Keep track of relay bitrate
	bdUInt32 m_bytesForwarded;
	bdUInt32 m_averageRelayBitrate;

	/// Keep track of total bitrate
	bdUInt32 m_bytesReceived;
	bdUInt32 m_averageBitrate;

	/// Our Relayer Load
	bdUInt32 m_relayerLoad;

	/// A list of pending info requests. Requests that don't receive a reply by the next
	/// time we send a request have their latencies artificially raised.
	bdHashMap<bdEndpoint, bdUInt64, bdEndpointHashingClass> m_pendingInfoReplies;
	bdHashMap<bdAddrHandleRefWrapper,  bdRelayerInfo, bdAddrHandleRefWrapper> m_infoPackets;
	bdHashMap<bdAddrHandleRefWrapper,  bdArray<bdRelayerInfo>, bdAddrHandleRefWrapper> m_routeInfoPackets;
	bdHashMap<bdAddrHandleRefWrapper,  bdArray<bdRelayRouteAdd>, bdAddrHandleRefWrapper> m_routeAddPackets;
	bdHashMap<bdAddrHandleRefWrapper,  bdArray<bdRelayRouteRemove>, bdAddrHandleRefWrapper> m_routeRemovePackets;


	bdUInt32 m_sendTokens;
	bdUInt32 m_lastStatusIndex;
	bdUInt32 m_lastRelayStatusIndex;
	bdInt m_maxRelayedConnections;
#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
	bdHashMap<bdEndpoint, bdUInt, bdEndpointHashingClass> m_missedInfoReplies;
#endif

};

#endif // BD_RELAY_SOCKET_ROUTER_H
