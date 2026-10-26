// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE : Consistent ordered and deterministic dispatching to peers.

#ifndef BD_DELIVERY_MANAGER_H
#define BD_DELIVERY_MANAGER_H

#include <bdCore/bdCore.h>

#include <bdPeer/bdSession/bdSessionInterceptor.h>
#include <bdPeer/bdSession/bdSessionListener.h>
#include <bdPeer/bdDelivery/bdDeliveryInterceptor.h>

class bdSession;
class bdDeliveryManagerListener;
class bdDeliveryLogger;
class bdDeliveryInitializer;

#define BD_DELIVERY_MGR_ACTIVATE_MSG		(0u)
#define BD_DELIVERY_MGR_DEACTIVATE_MSG		(1u)
#define BD_PLAYER_MGR_JOIN_MSG				(2u)
#define BD_PLAYER_MGR_LEAVE_MSG				(3u)
#define BD_ROUND_START_MSG					(4u)
#define BD_ROUND_END_MSG					(5u)
#define BD_COMMAND_MSG						(6u)
#define BD_GAME_STATE_HASH_MSG			(7u)
#define BD_DELIVERY_MGR_MAX_MESSAGE_TYPE	(7u) // this is correct, saves a -1 on ranged reads and writes

/// Provides consistent ordered and deterministic dispatching of messages to 
/// peers in a bdSession. Given the same bdDeliveryInterceptors and 
/// bdDeliveryLoggers on all peers, the bdDeliveryManager ensures that if you 
/// have received a message so will all other peers. It also guarantees the 
/// order which all messages are dispatched to the bdDeliveryInterceptors and 
/// bdDeliveryLoggers across all peers. A deterministic peer-to-peer system 
/// can be created by sending all messages though a bdDeliveryManager and 
/// accepting messages from it.
///
/// In order to dispatch messages deterministically across all peers a 
/// bdDeliveryManager can only dispatch after it receiving 
/// messages from all other bdDeliveryManagers in the system (including 
/// itself). This lock-step method is not well suited to real-time systems
/// as relative latencies between the peers will affect the rate at which 
/// messages can be dispatched. To counteract this the bdDeliveryManager 
/// provides latency compensation to ensure real-time systems can run smoothly. 
/// Latency compensation is configured via the message limit passed to the 
/// constructor. 
///
/// In order to reduce bandwidth a bdDeliveryManager can be active or inactive.
/// Before dispatching a bdDeliveryManager must receive a messages from all
/// \b active peers. However it will send messages to all peers. This allows
/// peers that don't provide inputs to the system to spectate without active 
/// peers having to wait for their messages.
class bdDeliveryManager : public bdSessionInterceptor,		  						  
						  public bdSessionListener,
						  public bdDeliveryInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor.
		/// \param messageLimit[in] The latency compensation message limit. 
		bdDeliveryManager(const bdUInt messageLimit = 1);

		/// Destructor.
		~bdDeliveryManager();

		/// \param session[in] A pointer to the bdSession whose peers messages
		///		will be sent to and received from.
		void init(bdSession *const session);

		void quit();

		/// Register a bdDeliveryInterceptor. Interceptors are used to pass 
		/// delivered messages to other classes.
		/// \param interceptor[in]	A pointer to the interceptor to register.
		void registerInterceptor(bdDeliveryInterceptor *const interceptor);

		/// Unregister a particular bdDeliveryInterceptor.
		/// \param interceptor[in] A pointer to the interceptor to unregister.
		void unregisterInterceptor(bdDeliveryInterceptor *const interceptor);

		/// Register a bdDeliveryManagerListener. Listeners are used to inform other 
		/// classes of bdDeliveryManager related events.
		/// \param listener[in] A pointer to the listener to be registered.
		void registerListener(bdDeliveryManagerListener *const listener);

		/// Unregister a particular bdDeliveryManagerListener.
		/// \param listener[in] A pointer to the interceptor to unregister.
		void unregisterListener(bdDeliveryManagerListener *const listener);

		/// Register a bdDeliveryLogger. Loggers are used to peek at
		/// delivered messages after they have been dispatched.
		/// \param logger[in]	A pointer to the bdDeliveryLogger to 
		///		register.
		void registerLogger(bdDeliveryLogger *const logger);

		/// Unregister a particular bdDeliveryLogger.
		/// \param logger[in] A pointer to the logger to unregister.
		void unregisterLogger(bdDeliveryLogger *const logger);

		void registerInitializer(bdDeliveryInitializer *const initializer);

		void unregisterInitializer(bdDeliveryInitializer *const initializer);

		/// Get the active state of this delivery manager.
		/// An active bdDeliveryManager is one whose messages other peers will 
		/// wait for before dispatching.
		/// \return 
		/// - True if active.
		/// - False if inactive.
		bdBool isActive() const;		

		/// Become an active peer. Sends a message to the session host 
		/// requesting activation of this peer. Activation is asynchronous
		/// to find out when the activation occurs poll isActive or 
		/// register a bdDeliveryManagerListener.
		/// \return 
		/// - True if the activate request was sent to the host.
		/// - False if already active or the request could not be sent.
		bdBool activate() const;

		/// Become an inactive peer. Deactivation is A-synchronous
		/// to find out when the deactivation occurs poll isActive or 
		/// register a bdDeliveryManagerListener.
		void deactivate();

		/// Add a message to be sent to all peers on the next call to send.
		/// \param msg[in] The message to send.
		void addMessage(bdMessage& msg);

		/// Check that it is safe to call bdDeliveryManager::send.
		/// The bdDeliveryManager can only send if:
		/// - it is active (see bdDeliveryManager::activate)
		/// - it has not already sent up to message limit beyond the current 
		///		dispatch
		/// - it is in sync with the bdDeliveryManagers of the all peers
		/// - the underlying session is consistent
		///
		/// \return 
		/// - True if the above conditions are met.
		bdBool okToSend() const;

		/// Send all added messages to all peers. This method calls 
		/// bdDeliveryManager::okToSend before attempting to send all added
		/// messages to all peers in the underlying bdSession.
		/// \return 
		/// - True if the send succeeded, this is no indication of delivery.
		/// - False if bdDeliveryManager::okToSend returned false or the 
		///		sending	failed.
		bdBool send();

		/// Get the underlying bdSession.
		/// \return A pointer to the bdSession to which this bdDeliveryManager
		/// communicates.
		bdSession* getSession();
		
	protected:

		virtual bdBool acceptSession(const bdReceivedMessage& receivedMsg);

		virtual bdBool acceptDelivery(const bdDeliveryMessage& receivedMsg);

		virtual void onSessionConnectSuccess();

		virtual void onSessionConnect(const bdConnectionRef connection);

		virtual void onSessionDisconnect(const bdConnectionRef connection);

		virtual void onSessionStatusChange(const bdSession::bdSessionStatus previous,
										   const bdSession::bdSessionStatus current);

		void handlePendingJoin(const bdReceivedMessage& pendingJoin);

		void handleInitData(const bdReceivedMessage& initData);

		void handleActivateRequest(const bdReceivedMessage& activateRequest);

		void handleBatchMessage(const bdReceivedMessage& batchMessage);

		void handleRecoveryMessage(const bdReceivedMessage& recoveryMessage);

		void handleInitComplete(const bdReceivedMessage& initCompleteMessage);

		void handleActivate(const bdMessageRef activateMessage);

		void handleDeactivate(const bdCommonAddrRef addr);

		void addToWindow(const bdDeliveryMessage& deliveryMessage);		

		bdBool tryAddToWindow(const bdDeliveryMessage& deliveryMessage);		

		void processWindow();		
	
		static bdMessageRef getBatchMessage(const bdBitBufferRef buffer);

		void dispatch(const bdDeliveryMessage& message) const;
		
		bdUInt getRecoveryIndex() const;

		void cleanWindow(const bdCommonAddrRef addr);

		void processPendingDeactivations();

		void createOutgoingMessage();

		bdConnectionRef findSessionConnection(const bdCommonAddr& addr) const;

		bdCommonAddrRef findActiveAddr(const bdCommonAddr& addr) const;

		bdBool findMessage(const bdUInt index,
						   const bdCommonAddrRef fromAddr) const;

		bdMessageRef createRecoveryMessage() const;	

		void sendInitData(const bdConnectionRef connection);

		bdBool sendToJoined(const bdMessageRef message) const;

	protected:

		class bdPendingJoin
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				bdPendingJoin(const bdConnectionRef connection,
							  const bdUInt32 joinAt);

				~bdPendingJoin();

				bdConnectionRef m_connection;
				bdUInt32 m_joinAt;
		};

	protected:

		friend class bdDeliveryManagerInfo;

		enum bdDeliveryStatus
		{
			BD_DELIVERY_NOT_JOINED,
			BD_DELIVERY_JOINING,
			BD_DELIVERY_JOINED,
			BD_DELIVERY_ACTIVE
		};

		bdSession *m_session;

		bdDeliveryStatus m_status;

		bdUInt m_messageLimit;

		bdUInt32 m_outgoingMsgNum;

		bdUInt32 m_dispatchNum;

		bdMessageRef m_outgoingMessage;
		
		bdArray<bdCommonAddrRef> m_activePeers;

		bdArray<bdConnectionRef> m_joinedPeers;

		bdLinkedList<bdPendingJoin> m_pendingJoins;

		bdArray<bdCommonAddrRef> m_pendingDeactivations;

		typedef bdArray<bdDeliveryMessage> bdMessageBatch;
		bdMessageBatch *m_messageWindow;

		bdLinkedList<bdDeliveryMessage> m_joiningBuffer;

		bdFastArray<bdDeliveryInterceptor*> m_interceptors;

		bdFastArray<bdDeliveryManagerListener*> m_listeners;

		bdFastArray<bdDeliveryLogger*> m_loggers;

		bdFastArray<bdDeliveryInitializer*> m_initializers;
};

#endif // BD_DELIVERY_MANAGER_H
