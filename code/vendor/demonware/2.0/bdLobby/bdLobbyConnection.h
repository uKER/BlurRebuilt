// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PSP-INFRA XENON XBOX PS3 UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE : Represents a stream (TCP) connection.

#ifndef BD_LOBBY_CONNECTION_H
#define BD_LOBBY_CONNECTION_H

#include <bdCore/bdCore.h>
#include <bdCore/bdSocket/bdStreamSocket.h>
#include <bdCore/bdContainers/bdCircularBuffer.h>
#include <bdCore/bdCrypto/bdCypher3Des.h>
#include <bdLobby/bdAuthTicket.h>
#include <bdLobby/bdLobbyConnectionListener.h>

#include <bdCore/bdThread/bdMutex.h>

// 2 bytes total message size + 1 byte encryption type
#define BD_STREAM_HEADER_SIZE 3

// 
#define BD_SEND_PLAINTEXT 0
#define BD_SEND_ENCRYPTED 1
#define BD_SIGNATURE_CHECK (0xDEADBEEF)


// Sends keep alive messages to the LSG. If no reply is received
// within 2 * BD_KEEP_ALIVE_PERIOD, the stream connection will be
// closed. This is used to perform end-to-end connectivity tests
#define BD_SEND_KEEP_ALIVE

#if defined BD_SEND_KEEP_ALIVE
/// Xbox platforms do not support TCP keep alive packets. Hence we send
/// our own keep alive messages to the LSG 
#define BD_KEEP_ALIVE_PERIOD 40
#endif


/// A TCP connection to the lobby server.
class bdLobbyConnection : public bdReferencable
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Defines the possible states of a bdConnection.
		enum Status
	{
		BD_NOT_CONNECTED,
		BD_CONNECTING,
		BD_CONNECTED,
		BD_DISCONNECTING,
		BD_DISCONNECTED
	};

	/// Create a connection to addr.
	/// \param addr[in]	The address of the lobby service to connect to. 
	/// \param lobbyService[in] A pointer to the lobby service, used to
	///		call back in the event of connections/disconnections
	bdLobbyConnection(bdCommonAddrRef addr,
		bdLobbyConnectionListener *lobbyService);

	/// Create a connection to host. Using this constructor, the connection uses
	/// the memory as specified by sendBuffer and recvBuffer resp.
	/// It is the client's responsibility to free buffers
	/// \param addr[in]			The \b host to connect to. 
	/// \param sendBuffer[in]	A pointer to a chunk of memory to be used as send buffer
	/// \param sendBufSize[in]	size of the send buffer
	/// \param recvBuffer[in]	A pointer to a chunk of memory to be used as receive buffer
	/// \param recvBufSize[in]	size of the receive buffer
	bdLobbyConnection(bdCommonAddrRef addr,
		void *sendBuffer,
		bdUWord sendBufSize,
		void *recvBuffer, 
		bdUWord recvBufSize,
		bdLobbyConnectionListener *lobbyService);


#if (defined BD_PLATFORM_XBOX)
	/// \note XNetRegisterKey must be called before a stream connection
	/// is established.
	bdLobbyConnection(	bdCommonAddrRef addr, 
		const bdSecurityID &secID,
		bdLobbyConnectionListener *lobbyService);
#elif (defined BD_PLATFORM_XENON)
	/// \note XNetRegisterKey must be called before a stream connection
	/// is established.
	bdLobbyConnection(	bdAddr addr, 
		bdLobbyConnectionListener *lobbyService);

	/// \note XNetRegisterKey must be called before a stream connection
	/// is established.
	/// deprecated - use bdLobbyConnection(bdAddr,bdLobbyConnectionListener*) instead
	bdLobbyConnection(bdAddr addr, 
		const bdSecurityID &secID,
		bdLobbyConnectionListener *lobbyService);

	bdLobbyConnection(  bdAddr addr, 
		void *sendBuffer,
		bdUWord sendBufSize,
		void *recvBuffer, 
		bdUWord recvBufSize,
		bdLobbyConnectionListener *lobbyService);

#else // this only exists on non X* platforms
	/// Sets the connection's encryption key
	/// \param sessionKey[in] : The session key that will be used to 
	/// encrypt the data on this connection
	void setSessionKey(const bdUByte8 sessionKey[BD_ENCRYPTION_KEY_SIZE]);
#endif //#if (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)

	/// Destructor.
	~bdLobbyConnection();

	Status getStatus() const;

	/// Sends a message.
	///	\param message[in] The message to be sent. This function does not actually
	/// send the message, but rather enqueues it.
	/// \param encrypt[in] Flag to indicate whether the message should be encrypted
	/// or not
	/// \return True: the message was successfully enqueued. 
	virtual bdBool send(const bdUByte8 *message,
		bdUInt messageSize,
		const bdBool encrypt = false);

	/// \todo 
	///  Does this need to be publicly accessible ? It seems kinda dangerous. 
	///  Also it's only used internally in 1 place, might be better to encapsulate
	///  that onConnect logic in a less easily abusable function ?
	void sendRaw(void *buffer, 
		const bdUInt32 bufferSize);

	bdBool getMessageToDispatch(bdUByte8 &type, 
		bdBitBufferRef &payload);

	bdBool getMessageToDispatch(bdUByte8 &type, 
		bdByteBufferRef &payload);

	/// Gets the number of byte this connection can receive at once. 
	/// \return The maximum number of bytes this connection can receive.
	bdUInt32 getReceiveBufferSize() const;

	/// Tries to establish a connection. The underlying TCP socket is set to
	/// be non-blocking and this function returns immediately.
	bdBool connect(bdAuthInfo &authInfo);

	/// Disconnects the connections
	void disconnect();

	/// Closes the connection and calls any registered connections listeners.
	void close();

	bdBool pump();

protected:

	void callListenersConnect(const bdBool success);

	void callListenersDisconnect();

	bdBool receivedFullMessage();

protected:

	bdCommonAddrRef m_addr;

	bdCircularBuffer<bdUByte8> *m_incomingBuffer;
	bdCircularBuffer<bdUByte8> *m_outgoingBuffer;

	bdUInt m_incomingBufferSize;
	bdUInt m_outgoingBufferSize;

	bdUInt32 m_messageSize;

	bdStreamSocket m_socket;

	Status m_status;

	bdLobbyConnectionListener *m_connectionListener;

#if (defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
	bdSecurityID m_secID;

#elif (defined BD_PLATFORM_XENON)
	bdSecurityID m_secID;
	bdAddr m_serverAddr;
#else

	bdCypher3Des m_cypher;
	bdUByte8 m_sessionKey[BD_ENCRYPTION_KEY_SIZE];

	// Counter that is incremented for every message received. Used as a
	// seed for the IV vector
	bdUInt32 m_messageCount;


#endif // XBoxes

#if defined BD_SEND_KEEP_ALIVE

	// For timing the transmission of keep-alive packets
	bdStopwatch m_keepAliveTimer;

	// A timer that records the last time data was
	// received from the server
	bdStopwatch m_lastReceivedTimer;
#endif
	// Defend against non-threadsafe operations
	static bdMutex m_mutex;
};

#endif // BD_LOBBY_CONNECTION_H


