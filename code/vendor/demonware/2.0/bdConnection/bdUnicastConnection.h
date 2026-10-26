// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE : Represents a connection to a remote host.

#ifndef BD_UNICAST_CONNECTION_H
#define BD_UNICAST_CONNECTION_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdConnection.h>
#include <bdConnection/bdConnectionStatistics.h>

#include <bdConnection/bdWindow/bdUnreliableSendWindow.h>
#include <bdConnection/bdWindow/bdUnreliableReceiveWindow.h>
#include <bdConnection/bdPacket/bdChunk.h>

class bdMessageFrame;
class bdReliableSendWindow;
class bdReliableReceiveWindow;
class bdHMacSHA1;
class bdAddressMap;

BD_REFERENCE(bdUnicastConnection);
BD_REFERENCE(bdInitChunk);
BD_REFERENCE(bdInitAckChunk);
BD_REFERENCE(bdCookieEchoChunk);
BD_REFERENCE(bdMessage);

#define BD_MAX_MESSAGE_SIZE	(BD_MAX_PAYLOAD_SIZE - 20)

/// bdUnicastConnection represents a connection to a remote host.
class bdUnicastConnection : public bdConnection
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		virtual bdBool connect();

		// See documentation in base class bdConnection.
		virtual void disconnect();

		// See documentation in base class bdConnection.
		virtual bdBool send(const bdMessageRef message, const bdBool reliable = false);

		virtual Status getStatus() const;

		virtual void close();

		/// Retrieves the network statistics for this connection.
		/// \return A reference to the network statistics.
		bdConnectionStatistics& getStats();

	protected:

		/// Grant friendship to bdConnectionStore so that it can
		/// construct an instance of this class.
		friend class bdConnectionStore;
		friend class bdReference<bdUnicastConnection>;

		bdUnicastConnection(bdCommonAddrRef dest,
							bdAddressMap *addrMap);

		bdUnicastConnection(bdAddressMap *addrMap);

		/// Destructor.
		virtual ~bdUnicastConnection();

		// See documentation in base class bdConnection.
		virtual bdBool receive(const bdUByte8 * buffer,
							   const bdUInt32 bufferSize);


		// See documentation in base class bdConnection.
		virtual bdUInt getDataToSend(bdUByte8 *const buffer,
									 const bdUInt bufferSize);

		// See documentation in base class bdConnection.
		virtual bdBool getMessageToDispatch(bdMessageRef &message);

		/// Sets the status of this connection.
		/// \param status the status of the connection.
		void setStatus(const Status status);

	protected:

		bdBool handleData(bdChunkRef &chunk);

		bdBool handleSAck(bdChunkRef &chunk);

		bdBool handleInit(bdChunkRef &chunk);

		bdBool handleInitAck(bdChunkRef &chunk, const bdUInt32 vtag);

		bdBool handleCookieEcho(bdChunkRef &chunk, const bdUInt32 vtag);

		bdBool handleCookieAck(bdChunkRef &chunk, const bdUInt32 vtag);

		bdBool handleHeartbeat(bdChunkRef &chunk);

		bdBool handleHeartbeatAck(bdChunkRef &chunk);

		bdBool handleShutdown(bdChunkRef &chunk);

		bdBool handleShutdownAck(bdChunkRef &chunk);

		bdBool handleShutdownComplete(bdChunkRef &chunk);

		bdBool sendInit();

		bdBool sendInitAck(bdInitChunkRef chunk);

		bdBool sendCookieEcho(bdInitAckChunkRef chunk);

		bdBool sendCookieAck();

		bdBool sendHeartbeat(bdInitChunkRef chunk);

		bdBool sendHeartbeatAck(bdInitChunkRef chunk);

		bdBool sendShutdown();

		bdBool sendShutdownAck();

		bdBool sendShutdownComplete();

		void reset();

		void callListenersConnect(const bdBool success);

		void callListenersDisconnect();

		void callListenersReconnect();

		bdBool windowsEmpty() const;

	protected:

		enum bdUnicastConnectionStatus
		{
			BD_UC_CLOSED,
			BD_UC_COOKIE_WAIT,
			BD_UC_COOKIE_ECHOED,
			BD_UC_ESTABLISHED,
			BD_UC_SHUTDOWN_PENDING,
			BD_UC_SHUTDOWN_SENT,
			BD_UC_SHUTDOWN_RECEIVED,
			BD_UC_SHUTDOWN_ACK_SENT
		};

		class bdControlChunkStore
		{
			public:
				inline bdControlChunkStore(bdChunkRef chunk, bdBool lone = false)
				: m_chunk(chunk),
				m_lone(lone)
				{
				}

				bdChunkRef m_chunk;
				bdBool m_lone;
		};

	protected:

		/// Network statistics for this connection.
		bdConnectionStatistics m_stats;

		/// Message queue for outgoing reliable messages.
		bdReliableSendWindow *m_reliableSendWindow;

		/// Message queue for incoming reliable messages.
		bdReliableReceiveWindow *m_reliableRecvWindow;

		/// Message queue for outgoing, unreliable messages.
		bdUnreliableSendWindow m_unreliableSendWindow;

		/// Message queue for incoming unreliable messages.
		bdUnreliableReceiveWindow m_unreliableReceiveWindow;

		/// Queue of outgoing chunks. This queue isn't ordered or reliable.
		bdQueue<bdControlChunkStore> m_outQueue;

		/// Timer to keep track of the time since we last sent  a
		/// message.
		/// This is used to establish if a ping message should be sent in order
		/// to keep the connection alive.
		bdStopwatch m_sendTimer;

		/// Timer to keep track of the time since we last received a
		/// message. If we haven't received a message within 2*m_alivePeriod, we
		/// deem the connection as not established
		bdStopwatch m_receiveTimer;

		bdUnicastConnectionStatus m_state;

		bdUInt32 m_localTag;
		bdUInt32 m_peerTag;

		bdStopwatch m_initTimer;
		bdUByte8 m_initResends;
		bdStopwatch m_cookieTimer;
		bdUByte8 m_cookieResends;
		bdStopwatch m_shutdownTimer;
		bdUByte8 m_shutdownResends;

		bdStopwatch m_shutdownGuard;

		bdInitAckChunkRef m_initAckChunk;

		bdFloat32 m_smoothedRTT;
		bdFloat32 m_RTTVariation;

		bdAddressMap *m_addrMap;
};

#endif // BD_UNICAST_CONNECTION_H
