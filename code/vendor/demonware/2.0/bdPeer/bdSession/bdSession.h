// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Maintenance of peer-to-peer connections.

#ifndef BD_SESSION_H
#define BD_SESSION_H

#include <bdCore/bdCore.h>

#include <bdConnection/bdConnection.h>
#include <bdConnection/bdConnectionListener.h>
#include <bdConnection/bdMessage/bdReceivedMessage.h>

#include <bdConnection/bdDispatch/bdDispatchInterceptor.h>

BD_REFERENCE(bdMessage);
BD_REFERENCE(bdConnection);
BD_REFERENCE(bdSessionData);

class bdSessionHandler;
class bdSessionListener;
class bdSessionInterceptor;

#define BD_DEFAULT_SESSION_RELIABILITY (true)

//#define BD_BUFFER_MESSAGES_WHEN_NOT_READY

#ifdef BD_BUFFER_MESSAGES_WHEN_NOT_READY
#define BD_MESSAGE_BUFFER_SIZE (20)
#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY

/// bdSession creates and monitors the connections in a peer-to-peer system.
/// bdSession ensures that the connections between peers form a fully
/// connected connection graph, that is each peer is connected to all others
/// (including itself). bdSession can be queried to ensure that the graph is
/// fully connected, and if not will remain "not ready" until the graph again
/// becomes fully connected. Session traffic can be intercepted by registering
/// a bdSessionInterceptor. Traffic can be sent to a session via one of the
/// bdSession::send methods, all messages sent through the session are
///	reliable.
///
/// \note	A particular instance of this class may participate in a session as
///			the \a host or a \a peer. The host is considered to also be a peer,
///			but has some additional responsibilities, e.g. deciding if new peers
///			can join the session (see bdSessionHandler). The host can be
///			changed at any time using bdSession::setHost, but steps should be
///			taken to ensure it is set consistently on all peers. Hence the
///			host is usually only changed when the current host disconnects from
///			the session. The peer that starts a session, is automatically
///			designated as the host.
class bdSession :	public bdDispatchInterceptor,
					public bdConnectionListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Defines the states that a particular bdSession object may be in.
		/// Once this instance has successfully connected to a session
		/// it will alternate between the "ready" and "not ready" states,
		/// depending on connection events.
		enum bdSessionStatus
		{
			BD_SESSION_NOT_CONNECTED,
			BD_SESSION_CONNECTING_TO_HOST,
			BD_SESSION_CONNECTED_TO_HOST,
			BD_SESSION_CONNECTING_TO_PEERS,
			BD_SESSION_NOT_READY,
			BD_SESSION_READY,
			BD_SESSION_CONNECTING_TO_HOST_FAILED,
			BD_SESSION_CONNECTING_TO_PEERS_FAILED,
			BD_SESSION_SHUTTING_DOWN
		};

		/// Defines the possible roles of an instance of this class. These are
		/// "peer" or "host". A host has all the capabilities of a peer, but
		/// serves as the contact point for any new peers that want to join.
		/// A peer can also become of host at any time, due to host migration.
		enum bdSessionRole
		{
			BD_SESSION_HOST,
			BD_SESSION_PEER
		};

		/// Constructor.
		/// \param	sessionHandler[in]	A handler that determines whether peers
		///								are allowed to join the session. This
		///								will only be called on the session host.
		bdSession(bdSessionHandler *const sessionHandler = BD_NULL);

		/// Destructor. Removes this peer from the session by closing all
		/// connections.
		~bdSession();

		/// Register a bdSessionInterceptor. Interceptors are used to pass
		/// session messages to other classes.
		/// \param interceptor[in]	A pointer to the interceptor to register.
		void registerInterceptor(bdSessionInterceptor *const interceptor);

		/// Unregister a bdSessionInterceptor interceptor.
		/// \param interceptor[in] A pointer to the interceptor to unregister.
		void unregisterInterceptor(bdSessionInterceptor *const interceptor);

		/// Joins or creates a session.
		/// If \a hostEndpoint is local a session will be set up with this
		/// instance as the host. If \a hostEndpoint is remote request the
		/// join session sequence is started with a join request being sent
		/// to the hostEndpoint.
		/// \param hostAddr[in]	The address of the host session to join.
		/// \param secID[in]
		/// \param userData[in]		A pointer to a bit buffer containing
		///							additional data that should be sent along
		///							with the join request.
		/// \return
		/// - True, if the session join was initiated.
		/// - False, otherwise.
		bdBool join(bdCommonAddrRef hostAddr,
					const bdSecurityID &secID,
					const bdSecurityKey &secKey,
					bdBitBuffer *const userData = BD_NULL);

		/// [Deprecated] Disconnect from a session. Once this method completes, the
		/// process of disconnecting all peers will have started, but will require several pumps before completion.
		/// Session status will be set to BD_SESSION_NOT_CONNECTED although peers are not yet fully disconnected.
		/// This method is deprecated, please use bdSession::leaveGraceful() instead.
		void leave();

		/// Gracefully disconnect from a session. After calling this, call pumpShutdown()
		/// until the session status is BD_SESSION_NOT_CONNECTED. At that point,
		/// it will be safe to unregister the security key associated with the session.
		void leaveGraceful();

		/// Register a bdSessionListener. Listeners are used to inform other
		/// classes of bdSession related events.
		/// \param listener[in] A pointer to the listener to be registered.
		void registerListener(bdSessionListener *const listener);

		/// Unregister a particular bdSessionListener.
		/// \param listener[in] A pointer to the listener to be unregistered.
		void unregisterListener(bdSessionListener *const listener);

		/// Get the session status. If the session is not ready, algorithms
		/// that use the session should be paused. A session is ready when
		/// the connection graph is fully connected.
		/// \return The current session status.
		bdSessionStatus getStatus() const;

		/// Get the role of this session instance. Can be used to determine
		/// if we are currently the session host.
		/// \return the role of this session instance.
		bdSessionRole getRole() const;

		/// Get the number of peers connected to the session. Can be used with
		/// bdSession::getConnection to iterate the connections to all peers
		/// in the session.
		/// \return	The number of peers, including this instance and the host.
		bdUInt getNumPeers() const;

		/// Get the connection to a peer in the session.
		/// \param index[in] The index of the required connection.
		/// \return The \a index'th connection.
		bdConnectionRef getConnection(const bdUInt index) const;

		/// Set the session host. Care should be taken when changing the session
		/// host to ensure it is set to the same peer on all peers in the session.
		/// \param index[in] The index of the peer to be made host.
		void setHost(const bdUInt index);

		/// Get connection to the session host.
		/// \return	The connection to the host.
		bdConnectionRef getHost() const;

		/// Send a bdMessage to all peers in the session. Messages should
		/// only be sent to the session while the session is "ready" otherwise
		/// the number of peers is in flux and we cannot guarantee the message
		/// will reach all peers. The message will be sent reliably.
		/// \note Messages will also be sent to the local peer.
		/// \param	msg[in]	The message to send.
		/// \param reliable[]
		/// \return
		/// - True, if the message was successfully sent to all peers.
		///   This is not an indication of receipt.
		/// - False, otherwise.
		bdBool send(bdMessageRef msg,
					const bdBool reliable = BD_DEFAULT_SESSION_RELIABILITY);

		/// Send a bdMessage to a particular peer. The message is only sent if
		/// there is a peer in the session connected via \a peerConnection.
		/// The message will be sent reliably.
		/// \param peerConnection[in]	The connection via which the message
		///								should be sent.
		///	\param msg[in] The message to send.
		/// \param reliable[]
		/// \return
		/// - True, if the peerConnection connects to a peer in the session and
		///	  the message was sent. This is not an indication of receipt.
		/// - False, otherwise.
		bdBool send(bdConnectionRef peerConnection,
				    bdMessageRef msg,
					const bdBool reliable = BD_DEFAULT_SESSION_RELIABILITY); //const;

		/// Send a message to the session host.
		/// The message will be sent reliably.
		/// \param msg[in]	The message to send.
		/// \param reliable[]
		/// \return
		/// - True, if the message was sent successfully. This is not an
		///   indication of receipt.
		/// - False, otherwise.
		bdBool sendHost(bdMessageRef msg,
						const bdBool reliable = BD_DEFAULT_SESSION_RELIABILITY); //const;

		/// Sends a message to all peers forcing them to sync their peer list to that 
		/// of the host. 
		/// Call this if the session had been BD_SESSION_NOT_READY for an 
		/// unacceptable period of time indicating a perminent partial network split 
		/// (be sure to account for connection timeouts). Soon after
		/// the peers receive the hosts message the session status should return to BD_SESSION_READY or
		/// split into multiple sessions or disband. 
		/// Upon receipt of the message the peers will either disconnect any unwanted peers
		/// or call the bdSessionListener::onForceLeave method of all registered listsners. 
		/// Receiving a bdSessionListener::onForceLeave callback is an indicator that the 
		/// peer should leave the session by calling bdSession::leave.
		/// \return
		/// - True, if the message was successfully sent to all peers.
		///   This is not an indication of receipt.
		/// - False, otherwise.
		bdBool sendUpdatePeerList();

		/// Get the index of the given peer in the \c m_peers array.
		/// This does a linear search, i.e., is O(n).
		/// \param	connection[in]	The address of the peers to search for.
		/// \param	index[out]	Receives the index of the peer in \c m_peers.
		/// \return
		///		- True, if the peer was found.
		///		- False, if the peer was not found.
		bdBool getPeerIndex(const bdConnectionRef connection,
							bdUInt& index) const;

		bdUInt getLocalPeerIndex() const;

		bdBool readyToConnect() const;

		bdArray<bdSessionDataRef>& getSessionData();

		/// To be called after leaveGraceful() until the session's status 
		/// becomes BD_SESSION_NOT_CONNECTED. This will ensure that all peer connections 
		/// are fully and gracefully shut down.
		void pumpShutdown();

	protected:


		bdBool connectToLocalHost(const bdSecurityID& secID);

		bdBool connectToRemoteHost(const bdCommonAddrRef hostAddr,
								   const bdSecurityID& secID);

		bdBool connectToLocalPeer(const bdSecurityID& secID);

		bdBool startConnect(bdConnectionRef& connection,
							const bdCommonAddrRef addr,
							const bdSecurityID& secID,
							const bdNChar8 *const connectionDesc);

		bdBool createJoinRequest(bdBitBuffer *const userData);

		/// Check the consistency of the \c m_peers hashes, and set the status
		/// appropriately.
		void checkSessionConsistency();

		/// Recompute our hash of the \c m_peers array and write it into our own
		/// entry in \c m_peers.
		void doLocalHash();

		/// Send consistency updates to the session peers.
		void sendConsistencyUpdates();

		/// Set the status of the this session object.
		/// \param	status[in]	The status to set the session to.
		void setStatus(const bdSessionStatus status);

		void setRole(const bdSessionRole role);

		/// Overridden from bdConnectionListener interface.
		virtual void onConnect(bdConnectionRef connection);

		/// Overridden from bdConnectionListener interface.
		virtual void onConnectFailed(bdConnectionRef connection);

		/// Overridden from bdConnectionListener interface.
		virtual void onDisconnect(bdConnectionRef connection);

		/// Overridden from bdDispatchInterface.
		virtual bdBool accept(const bdReceivedMessage& receivedMsg);

		/// Handle a join request message. Should only be called on the session
		/// host. This may result in the \c m_sessionHandler being called. Only
		/// one join can be handled at a time.
		/// \param	joinRequest	The message.
		void handleJoinRequest(const bdReceivedMessage& joinRequest);

		/// Handle a join reply message. If we are allowed to join, this will
		/// try to establish connections to all the peers in the session.
		/// \param	joinReply	The message.
		void handleJoinReply(const bdReceivedMessage& joinReply);

		/// Handle a consistency message. This kind of message contains a hash
		/// of the sender's view of all the other peers.
		/// \param	consistencyMsg	The message.
		void handleConsistencyMessage(const bdReceivedMessage& consistencyMsg);

		/// Handle a peer list update message. 
		/// The message will only be acted upon if it is from the host, and may result
		/// in the disconnection of some peer or the calling of 
		/// bdSessionListsner::onForceLeave.
		/// \param	updatePeerListMsg	The message.
		void handleUpdatePeerList(const bdReceivedMessage& updatePeerListMsg);

		void processJoinRequest(const bdReceivedMessage& joinRequest);

		void processPendingJoinRequest();

		/// Start to shutdown any existing connections and reset internal
		/// state. After this completes, connections will not be fully shutdown
		/// until the connection has been pumped several times to
		/// propagate graceful disconnect messages.
		void cleanup();

		/// Start shutting down any existing connections.
		void cleanupGraceful();

	protected:

		/// The connection to host.
		bdConnectionRef m_hostConnection;

		/// A local loopback connection.
		bdConnectionRef m_localConnection;

		/// References the connection of a joining peer. Only one peer
		/// can join at a time.
		bdConnectionRef m_joiningPeer;

		/// An sorted array of peer data structures.
		/// \note There is currently no need to explicitly sort this array as
		/// only one peer is allowed to join at a time.
		bdArray<bdSessionDataRef> m_peers;

		/// The interceptors that are registered with this session. Whenever
		/// session traffic arrives, each of these will be called until one of
		/// them accepts it.
		bdFastArray<bdSessionInterceptor*> m_interceptors;

		/// The registered connection listeners. Each of these will be called
		/// with any session events that occur.
		bdFastArray<bdSessionListener*> m_listeners;

		/// A pointer to the handler for new connection requests.
		bdSessionHandler* m_handler;

		/// Indicates whether we are currently a peer or the session host.
		bdSessionRole m_role;

		/// Indicates the current state of the session.
		bdSessionStatus m_status;

		/// The index of the entry in \c m_peers that represents this peer.
		bdUInt m_localPeerIndex;

		/// Used only whilst in a connecting state. This enables us to detect
		/// asynchronously when connections have been set up (or not) and to
		/// change the peer state appropriately and call back to the application.
		bdArray<bdConnectionRef> m_pendingConnections;

		bdLinkedList<bdReceivedMessage> m_pendingJoinRequests;

		bdSecurityID m_secID;
		bdSecurityKey m_secKey;

		bdMessageRef m_sessionJoinRequest;

		bdBool m_registeredKey;
		bdSecurityID m_gameSecID;
		bdSecurityKey m_gameSecKey;

#ifdef BD_BUFFER_MESSAGES_WHEN_NOT_READY

		class bdBufferedMessage
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				enum bdDestination
				{
					BD_TO_UNKNOWN,
					BD_TO_SPECIFIC,
					BD_TO_HOST,
				};

				bdBufferedMessage();

				bdBufferedMessage(const bdMessageRef message,
								  const bdDestination destination,
								  const bdBool reliable);

				bdDestination getDestination() const;

				bdMessageRef getMessage() const;

				bdBool getReliable() const;

				void addConnection(const bdConnectionRef connection);

				bdArray<bdConnectionRef>& getConnections();

			protected:

				bdMessageRef m_message;
				bdDestination m_destination;
				bdArray<bdConnectionRef> m_sendTo;

				// TODO: don't need this is we only ever buffer reliable messages
				bdBool m_reliable;
		};

		bdBool addToBuffer(const bdBufferedMessage& message);
		void sendBufferedMessages();
		bdBool send(bdBufferedMessage& message);

		// TODO: used something with less overhead than bdArray
		bdArray<bdBufferedMessage>  m_messageBuffer;

#endif // BD_BUFFER_MESSAGES_WHEN_NOT_READY

};

#endif // BD_SESSION_H

