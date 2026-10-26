// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PSP-INFRA XENON XBOX PS3 UNIX WII OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyConnection.h>

#include <bdCore/bdCrypto/bdCypher3Des.h>
#include <bdCore/bdCrypto/bdCryptoUtils.h>
#include <bdCore/bdCrypto/bdHMacSHA1.h>


#define BD_LOG_LEVEL	"bdLobby/bdLobbyConnection"

// TODO Think about moving this code into a bdLobbyConnectionConfig file ?

// Specifies the default size of the send/receive buffer for a stream connection
// This effectively limits the maximum message size that can be sent over a
// stream connection.
#define BD_DEFAULT_STREAM_BUFFER_SIZE (0xffffu)
#define BD_MESSAGE_HEADER_SIZE (10)
// Maximum message size that we can handle.
#define BD_GET_MAX_STREAM_MESSAGE_SIZE(s) ((s - BD_MESSAGE_HEADER_SIZE))

// Defend against non-threadsafe operations
bdMutex bdLobbyConnection::m_mutex;

bdLobbyConnection::bdLobbyConnection(bdCommonAddrRef addr,
									 bdLobbyConnectionListener *lconnectionListener)
:m_addr(addr),
 m_messageSize(0),
 m_status(BD_NOT_CONNECTED),
 m_connectionListener(lconnectionListener)
 #if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
 , m_messageCount(0)
#endif
{
	m_outgoingBuffer = new bdCircularBuffer<bdUByte8>(BD_DEFAULT_STREAM_BUFFER_SIZE, BD_NULL, false);
	BD_ASSERT(m_outgoingBuffer != 0, "Failed to allocate buffer");
	m_outgoingBufferSize = BD_DEFAULT_STREAM_BUFFER_SIZE;
	m_incomingBuffer = new bdCircularBuffer<bdUByte8>(BD_DEFAULT_STREAM_BUFFER_SIZE, BD_NULL, false);
	BD_ASSERT(m_incomingBuffer != 0, "Failed to allocate buffer");
	m_incomingBufferSize = BD_DEFAULT_STREAM_BUFFER_SIZE;

	const bdBool blocking = false;
	m_socket.create(blocking);


}

bdLobbyConnection::bdLobbyConnection(bdCommonAddrRef addr,
									 void *sendBuffer,
									 bdUWord sendBufSize,
									 void *recvBuffer,
									 bdUWord recvBufSize,
									 bdLobbyConnectionListener *lobbyService)
:m_addr(addr),
 m_messageSize(0),
 m_status(BD_NOT_CONNECTED),
 m_connectionListener(lobbyService)
#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
, m_messageCount(0)
#endif
{
	m_outgoingBuffer = new bdCircularBuffer<bdUByte8>(static_cast<bdUInt>(sendBufSize), reinterpret_cast<bdUByte8 *>(sendBuffer), false);
	m_outgoingBufferSize = sendBufSize;
	m_incomingBuffer = new bdCircularBuffer<bdUByte8>(static_cast<bdUInt>(recvBufSize), reinterpret_cast<bdUByte8 *>(recvBuffer), false);
	m_incomingBufferSize = recvBufSize;

	const bdBool blocking = false;
	m_socket.create(blocking);
}

#if (defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
bdLobbyConnection::bdLobbyConnection( bdCommonAddrRef addr,
									 const bdSecurityID &secID,
									 bdLobbyConnectionListener *lobbyService)
:m_addr(addr),
 m_messageSize(0),
 m_secID(secID),
 m_connectionListener(lobbyService)
{
	const bdBool blocking = false;
	m_socket.create(blocking);

	m_outgoingBuffer = new bdCircularBuffer<bdUByte8>(BD_DEFAULT_STREAM_BUFFER_SIZE, BD_NULL, false);
	BD_ASSERT(m_outgoingBuffer != 0, "Failed to allocate buffer");
	m_outgoingBufferSize = BD_DEFAULT_STREAM_BUFFER_SIZE;
	m_incomingBuffer = new bdCircularBuffer<bdUByte8>(BD_DEFAULT_STREAM_BUFFER_SIZE, BD_NULL, false);
	BD_ASSERT(m_incomingBuffer != 0, "Failed to allocate buffer");
	m_incomingBufferSize = BD_DEFAULT_STREAM_BUFFER_SIZE;

}
#elif (defined BD_PLATFORM_XENON)
bdLobbyConnection::bdLobbyConnection( bdAddr addr,
									bdLobbyConnectionListener *lobbyService)
:m_messageSize(0),
 m_connectionListener(lobbyService)
{
	m_serverAddr = addr;
	const bdBool blocking = false;
	m_socket.create(blocking);

	m_outgoingBuffer = new bdCircularBuffer<bdUByte8>(BD_DEFAULT_STREAM_BUFFER_SIZE, BD_NULL, false);
	BD_ASSERT(m_outgoingBuffer != 0, "Failed to allocate buffer");
	m_outgoingBufferSize = BD_DEFAULT_STREAM_BUFFER_SIZE;
	m_incomingBuffer = new bdCircularBuffer<bdUByte8>(BD_DEFAULT_STREAM_BUFFER_SIZE, BD_NULL, false);
	BD_ASSERT(m_incomingBuffer != 0, "Failed to allocate buffer");
	m_incomingBufferSize = BD_DEFAULT_STREAM_BUFFER_SIZE;
}

bdLobbyConnection::bdLobbyConnection( bdAddr addr,
									 const bdSecurityID &secID,
									 bdLobbyConnectionListener *lobbyService)
: m_messageSize(0),
  m_secID(secID),
  m_connectionListener(lobbyService)
{
	m_serverAddr = addr;
	const bdBool blocking = false;
	m_socket.create(blocking);

	m_outgoingBuffer = new bdCircularBuffer<bdUByte8>(BD_DEFAULT_STREAM_BUFFER_SIZE, BD_NULL, false);
	BD_ASSERT(m_outgoingBuffer != 0, "Failed to allocate buffer");
	m_outgoingBufferSize = BD_DEFAULT_STREAM_BUFFER_SIZE;
	m_incomingBuffer = new bdCircularBuffer<bdUByte8>(BD_DEFAULT_STREAM_BUFFER_SIZE, BD_NULL, false);
	BD_ASSERT(m_incomingBuffer != 0, "Failed to allocate buffer");
	m_incomingBufferSize = BD_DEFAULT_STREAM_BUFFER_SIZE;
}


bdLobbyConnection::bdLobbyConnection(bdAddr addr,
									 void *sendBuffer,
									 bdUWord sendBufSize,
									 void *recvBuffer, 
									 bdUWord recvBufSize,
									 bdLobbyConnectionListener *lobbyService)
									 :m_messageSize(0),
									 m_connectionListener(lobbyService)
{
	m_serverAddr = addr;
	const bdBool blocking = false;
	m_socket.create(blocking);
	
	m_outgoingBuffer = new bdCircularBuffer<bdUByte8>(static_cast<bdUInt>(sendBufSize), reinterpret_cast<bdUByte8 *>(sendBuffer), false);
	BD_ASSERT(m_outgoingBuffer != 0, "Failed to allocate buffer");
	m_outgoingBufferSize = sendBufSize;
	m_incomingBuffer = new bdCircularBuffer<bdUByte8>(static_cast<bdUInt>(recvBufSize), reinterpret_cast<bdUByte8 *>(recvBuffer), false);
	BD_ASSERT(m_incomingBuffer != 0, "Failed to allocate buffer");
	m_incomingBufferSize = recvBufSize;
}

#else // This only exists on non X* platforms

void bdLobbyConnection::setSessionKey(const bdUByte8 sessionKey[BD_ENCRYPTION_KEY_SIZE])
{
	bdMemcpy(m_sessionKey, sessionKey, BD_ENCRYPTION_KEY_SIZE);
	m_cypher.init(m_sessionKey, BD_ENCRYPTION_KEY_SIZE);
}
#endif // #if (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)

bdLobbyConnection::~bdLobbyConnection()
{
	m_socket.close();
	delete m_outgoingBuffer;
	delete m_incomingBuffer;
}

bdLobbyConnection::Status bdLobbyConnection::getStatus() const
{
	return m_status;
}

bdBool bdLobbyConnection::send(const bdUByte8 *message,
							   bdUInt messageSize,
							   const bdBool encrypt)
{
	
	bdBool result = false;
	if(messageSize > (BD_GET_MAX_STREAM_MESSAGE_SIZE(m_outgoingBufferSize)))
	{
		BD_ERR(BD_LOG_LEVEL , "Message too big for outgoing buffer");
	}
	else if(m_status == BD_CONNECTED || m_status == BD_CONNECTING)
	{
		// Message header:
		// 1: Byte8 encryption type
		// 2: UInt32 IV seed
		// 3: Byte8[] Encrypted message
		//     |_ 4 bytes = 0xDEADBEEF

		bdByte8 encryptionType = encrypt ? BD_SEND_ENCRYPTED : BD_SEND_PLAINTEXT;

#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
		if (encrypt)
		{
			bdHMacSHA1 hmac(m_sessionKey,  BD_ENCRYPTION_KEY_SIZE);
			bdUByte8 signature[4];

			// Prepare some size constants.
			const bdUInt cypherSize = BD_NEXT_MULTIPLE_OF_M(sizeof(signature) +
														    messageSize, 8);
			const bdUInt32 sizePrefix = static_cast<bdUInt32>(cypherSize + sizeof(encryptionType)
				+ sizeof(m_messageCount));
			const bdInt headerSize = sizeof(sizePrefix) + sizeof(encryptionType) + sizeof(m_messageCount);

			bdUInt offset = 0;

			// Check that there's enough buffer space for this message.
			pump();

			// Defend against non-threadsafe operations
			m_mutex.lock();

			if (m_outgoingBuffer->getTotalFreeSpace() < cypherSize + headerSize)
			{
				BD_INFO(BD_LOG_LEVEL, "Not enough buffer space for the message.");

				// Defend against non-threadsafe operations
				m_mutex.unlock();

				return false;
			}

			if (m_outgoingBuffer->getFreeBlockSize() < cypherSize + headerSize)
			{
				m_outgoingBuffer->alignUsedBlock();
				BD_ASSERT(m_outgoingBuffer->getFreeBlockSize() >= cypherSize + headerSize,
					"Outgoing buffer align failed.");
			}

			bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
			bdMemset(initialVector, BD_NULL, sizeof(initialVector));

			bdCryptoUtils::calculateInitialVector(m_messageCount, initialVector);

			// Put unencrypted portion of the message in.
			result = true;
			result = result && m_outgoingBuffer->appendBasicType(sizePrefix);
			result = result && m_outgoingBuffer->appendBasicType(encryptionType);
			result = result && m_outgoingBuffer->appendBasicType(m_messageCount);

			// Above operations should never fail.
			BD_ASSERT(result, "Failed to write into send buffer.");

			bdUByte8 *encBuffer = m_outgoingBuffer->getFreeBlockStart();
			bdUInt encBufferSize = m_outgoingBuffer->getFreeBlockSize();

			offset = 0;
			bdUInt signatureSize = sizeof(signature);

			// First write a fake signature (hmac). We will calculate the signature later, after
			// the buffer has been adjusted for padding
			result = result && bdBytePacker::appendBuffer(encBuffer, encBufferSize, offset, offset, signature, sizeof(signature));
			bdUInt payloadOffset = offset + 1;
			//result = result && bdBytePacker::appendBasicType(encBuffer, encBufferSize, offset, payloadOffset, message->getType());
			result = result && bdBytePacker::appendBuffer(encBuffer, encBufferSize, offset, offset,
				message, messageSize);

			// Adjust the buffer for padding.
			bdUInt padding = cypherSize - sizeof(signature) - messageSize;
			for (bdUInt i = 0; i < padding; i++)
			{
				bdByte8 cnt = m_messageCount;
				bdBytePacker::appendBasicType(encBuffer, encBufferSize,offset, offset, cnt);
			}

			// Calculate the hmac over the payload + padding and add it to the send buffer
			hmac.process(encBuffer + payloadOffset, messageSize + padding -1);

			hmac.getData(reinterpret_cast<bdUByte8*>(&signature), signatureSize);
			bdBytePacker::appendBuffer(encBuffer, encBufferSize, 0, offset, signature, sizeof(signature));

			// Above operations should never fail.
			BD_ASSERT(result, "Failed to write into send buffer.");

			bdUInt tmp = cypherSize;
			result = result && m_cypher.encrypt(initialVector,
												encBuffer,
												encBuffer,
												tmp);

			// Encryption should never fail.
			BD_ASSERT(result, "Encryption failed.");

			m_outgoingBuffer->protectBlock(cypherSize);

			m_messageCount++;

			// Defend against non-threadsafe operations
			m_mutex.unlock();

			// Preceding operations should never fail
			BD_ASSERT(result, "Failed to prepare the packet.");
		}
		else
#endif // XBoxes
		{
#if (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)

			encryptionType = BD_SEND_PLAINTEXT;

#endif // XBoxes
			//const bdUInt16 payloadSize = static_cast<bdUInt16>(message->getPayload()->getDataSize());
			const bdUInt32 sizePrefix = static_cast<bdUInt32>(messageSize + sizeof(encryptionType)) ;
			const bdUInt headerSize = 5;//sizeof(sizePrefix) + sizeof(encryptionType);

			// Check that there's enough buffer space for this
			pump();

			// Defend against non-threadsafe operations
			m_mutex.lock();

			if (m_outgoingBuffer->getTotalFreeSpace() < messageSize + headerSize)
			{
				BD_INFO(BD_LOG_LEVEL, "Not enough buffer space for the message.");

				// Defend against non-threadsafe operations
				m_mutex.unlock();

				return false;
			}

			if (m_outgoingBuffer->getFreeBlockSize() < messageSize + headerSize)
			{
				m_outgoingBuffer->alignUsedBlock();
				BD_ASSERT(m_outgoingBuffer->getFreeBlockSize() >= messageSize + headerSize,
					"Outgoing buffer align failed.");
			}

			result = true;
			result = result && m_outgoingBuffer->appendBasicType(sizePrefix);
			result = result && m_outgoingBuffer->appendBasicType(encryptionType);
			//const bdMessageType msgType = message->getType();
			//result = result && m_outgoingBuffer->appendBasicType(msgType);

			result = result && m_outgoingBuffer->appendBuffer(message, messageSize);

			// Defend against non-threadsafe operations
			m_mutex.unlock();

			// Preceding operations should never fail
			BD_ASSERT(result, "Failed to prepare the header.");
		}

	}

	pump();
	return result;
}

void bdLobbyConnection::sendRaw(void *buffer,
                              const bdUInt32 bufferSize)
{
	// Defend against non-threadsafe operations
	m_mutex.lock();

	m_outgoingBuffer->appendBuffer((bdUByte8*)buffer, bufferSize);

	// Defend against non-threadsafe operations
	m_mutex.unlock();

	pump();
}

bdBool bdLobbyConnection::getMessageToDispatch(bdUByte8 &type,
											   bdBitBufferRef &payload)
{

	if (m_status == BD_DISCONNECTED)
	{
		return false;
	}
	pump();
	bdBool result = false;

	if(m_status == BD_CONNECTED || m_status == BD_CONNECTING)
	{
		bdInt status = 0;
		bdBool receiveCalled = false;

		// If we don't have a full message in the buffer, attempt to get one.
		if (!receivedFullMessage())
		{
			if (m_incomingBuffer->getFreeBlockSize() > 0)
			{
				status = m_socket.recv(m_incomingBuffer->getFreeBlockStart(),
					m_incomingBuffer->getFreeBlockSize());
				receiveCalled = true;
				if (status > 0)
				{
					m_incomingBuffer->protectBlock(static_cast<bdUInt>(status));
#if defined BD_SEND_KEEP_ALIVE
					m_lastReceivedTimer.reset();
					m_lastReceivedTimer.start();
#endif
				}
			}
		}
		if (m_messageSize > BD_GET_MAX_STREAM_MESSAGE_SIZE(m_incomingBufferSize))
		{
			// TODO: Is it correct to kill the connection at this point or should
			// we simply discard the message ?
			// Have we already overrun our buffer ?
			BD_WARN(BD_LOG_LEVEL, "Message received is too large to fit in the receive buffer.");
			close();
			return false;
		}

		if(status <= 0 && (!receivedFullMessage()) && receiveCalled)
		{
			// handle error!
			switch(status)
			{
				case BD_NET_CONNECTION_CLOSED:
				{
					m_status = BD_DISCONNECTING;
					callListenersDisconnect();
					m_status = BD_DISCONNECTED;
				}
				break;
				case BD_NET_WOULD_BLOCK:
				{
					if(m_status == BD_CONNECTING)
					{
						if (m_socket.isConnected())
						{
							m_status = BD_CONNECTED;
							callListenersConnect(true);
						}
					}
				}
				break;
				case BD_NET_CONNECTION_RESET:
				{
					BD_INFO(BD_LOG_LEVEL, "Received 'connection reset'. Closing connection.");
					close();
				}
				break;
				case BD_NET_MSG_SIZE:
				BD_WARN(BD_LOG_LEVEL, "Couldn't receive message. Buffer too small?");
				break;
				case BD_NET_BLOCKING_CALL_CANCELED:
					//blocking call was canceled.
				break;
				case BD_NET_NOT_BOUND:
				break;
				case BD_NET_NOT_CONNECTED:
				break;
				default:
				{
					BD_WARN(BD_LOG_LEVEL, "net error. status : %i" , status);
				}
				break;
			}
		}
		else if(receivedFullMessage())
		{
			if(m_status == BD_CONNECTING)
			{
				m_status = BD_CONNECTED;
				callListenersConnect(true);
			}

			result = true;
			bdByte8 encryptionType = 0;
			result = result && m_incomingBuffer->removeBasicType(m_messageSize);
			result = result && m_incomingBuffer->removeBasicType(encryptionType);

			BD_ASSERT(result, "Preceeding operations should never fail.");

			if (encryptionType == BD_SEND_ENCRYPTED)
			{
#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
				bdUInt32 signature = 0;
				bdUInt plainTextSize = m_messageSize - (sizeof(encryptionType) + sizeof(m_messageCount));
				bdUInt plaintextHeaderSize = sizeof(bdUByte8) + sizeof(signature);
				bdUInt32 IVseed = 0;
				result = result && m_incomingBuffer->removeBasicType(IVseed);

				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVseed, initialVector);

				BD_ASSERT(plainTextSize <= m_incomingBuffer->getUsedBlockSize(), "Lost buffer alignment.");

				result = result && m_cypher.decrypt(initialVector,
					m_incomingBuffer->getUsedBlockStart(),
					m_incomingBuffer->getUsedBlockStart(),
					plainTextSize);

				BD_ASSERT(result, "Decryption failed."); // Should never happen


				result = result && m_incomingBuffer->removeBasicType(signature);
				if (signature != BD_SIGNATURE_CHECK)
				{
					BD_WARN(BD_LOG_LEVEL, "Decryption failed.");
					result = false;
				}
				else
				{
					bdUByte8 msgType = 0;
					result = result && m_incomingBuffer->removeBasicType(msgType);

					if (result)
					{
						type = msgType;
						payload = new bdBitBuffer(m_incomingBuffer->getUsedBlockStart(),
							(plainTextSize - plaintextHeaderSize) << 3);
					}
					m_incomingBuffer->freeBlock(plainTextSize - plaintextHeaderSize);
					m_messageSize = 0;
				}
#else // Not XBoxes
				BD_ASSERT(false, "Should never be called.");
#endif // XBoxes
			}
			else
			{
				bdUByte8 msgType = 0;
				bdUInt payloadSize = m_messageSize - (sizeof(encryptionType)+ sizeof(msgType));
				result = true;
				result = result && m_incomingBuffer->removeBasicType(msgType);
				if (result)
				{
					type = msgType;
					payload = new bdBitBuffer(m_incomingBuffer->getUsedBlockStart(),
											  payloadSize << 3);
				}
				m_incomingBuffer->freeBlock(payloadSize);
				m_messageSize = 0;
			}
		}
	}
	return result;
}

bdBool bdLobbyConnection::getMessageToDispatch(bdUByte8 &type,
											   bdByteBufferRef &payload)
{

	if (m_status == BD_DISCONNECTED)
	{
		return false;
	}
	pump();
	bdBool result = false;

	if(m_status == BD_CONNECTED || m_status == BD_CONNECTING)
	{
		bdInt status = 0;
		bdBool receiveCalled = false;

		// If we don't have a full message in the buffer, attempt to get one.
		if (!receivedFullMessage())
		{
			if (m_incomingBuffer->getFreeBlockSize() > 0)
			{
				status = m_socket.recv(m_incomingBuffer->getFreeBlockStart(),
					m_incomingBuffer->getFreeBlockSize());
				receiveCalled = true;
				if (status > 0)
				{
					m_incomingBuffer->protectBlock(static_cast<bdUInt>(status));
	#if defined BD_SEND_KEEP_ALIVE
					m_lastReceivedTimer.reset();
					m_lastReceivedTimer.start();
	#endif
				}
			}
		}
		if (m_messageSize > BD_GET_MAX_STREAM_MESSAGE_SIZE(m_incomingBufferSize))
		{
			// TODO: Is it correct to kill the connection at this point or should
			// we simply discard the message ?
			// Have we already overrun our buffer ?
			BD_WARN(BD_LOG_LEVEL, "Message received is too large to fit in the receive buffer.");
			close();
			return false;
		}

		if(status <= 0 && (!receivedFullMessage()) && receiveCalled)
		{
			// handle error!
			switch(status)
			{
				case BD_NET_CONNECTION_CLOSED:
				{
					m_status = BD_DISCONNECTING;
					callListenersDisconnect();
					m_status = BD_DISCONNECTED;
				}
				break;
				case BD_NET_WOULD_BLOCK:
				{
					if(m_status == BD_CONNECTING)
					{
						if(m_socket.isConnected())
						{
							m_status = BD_CONNECTED;
							callListenersConnect(true);
						}
					}
				}
				break;
				case BD_NET_CONNECTION_RESET:
				{
					BD_INFO(BD_LOG_LEVEL, "Received 'connection reset'. Closing connection.");
					close();
				}
				break;
				case BD_NET_MSG_SIZE:
				BD_WARN(BD_LOG_LEVEL, "Couldn't receive message. Buffer too small?");
				break;
				case BD_NET_BLOCKING_CALL_CANCELED:
				//blocking call was canceled.
				break;
				case BD_NET_NOT_BOUND:
				break;
				case BD_NET_NOT_CONNECTED:
				break;
				default:
				{
					BD_WARN(BD_LOG_LEVEL, "net error. status : %i" , status);
				}
				break;
			}
		}
		else if(receivedFullMessage())
		{
			if(m_status == BD_CONNECTING)
			{
				m_status = BD_CONNECTED;
				callListenersConnect(true);
			}

			result = true;
			bdByte8 encryptionType = 0;
			result = result && m_incomingBuffer->removeBasicType(m_messageSize);
			result = result && m_incomingBuffer->removeBasicType(encryptionType);

			BD_ASSERT(result, "Preceeding operations should never fail.");

			if (encryptionType == BD_SEND_ENCRYPTED)
			{
#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
				bdUInt32 signature = 0;
				bdUInt plainTextSize = m_messageSize - (sizeof(encryptionType) + sizeof(m_messageCount));
				bdUInt plaintextHeaderSize = sizeof(bdUByte8) + sizeof(signature);
				bdUInt32 IVseed = 0;
				result = result && m_incomingBuffer->removeBasicType(IVseed);

				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVseed, initialVector);

				BD_ASSERT(plainTextSize <= m_incomingBuffer->getUsedBlockSize(), "Lost buffer alignment.");

				result = result && m_cypher.decrypt(initialVector,
													m_incomingBuffer->getUsedBlockStart(),
                                                    m_incomingBuffer->getUsedBlockStart(),
                                                    plainTextSize);

				BD_ASSERT(result, "Decryption failed."); // Should never happen


				result = result && m_incomingBuffer->removeBasicType(signature);
				if (signature != BD_SIGNATURE_CHECK)
				{
					BD_WARN(BD_LOG_LEVEL, "Decryption failed.");
					result = false;
				}
				else
				{
					bdUByte8 msgType = 0;
					result = result && m_incomingBuffer->removeBasicType(msgType);

					if (result)
					{
						type = msgType;
						//payload = new bdBitBuffer(m_incomingBuffer->getUsedBlockStart(),
						//	(plainTextSize - plaintextHeaderSize) << 3);
						payload = new bdByteBuffer(m_incomingBuffer->getUsedBlockStart(),
												  (plainTextSize - plaintextHeaderSize),true );
					}
					m_incomingBuffer->freeBlock(plainTextSize - plaintextHeaderSize);
					m_messageSize = 0;
				}
#else // Not XBoxes
				BD_ASSERT(false, "Should never be called.");
#endif // XBoxes
			}
			else
			{
				bdUByte8 msgType = 0;
				bdUInt payloadSize = m_messageSize - (sizeof(encryptionType)+ sizeof(msgType));
				result = true;
				result = result && m_incomingBuffer->removeBasicType(msgType);
				if (result)
				{
					type = msgType;
					//payload = new bdBitBuffer(m_incomingBuffer->getUsedBlockStart(),
					//						  payloadSize << 3);
					payload = new bdByteBuffer(m_incomingBuffer->getUsedBlockStart(),
											  payloadSize, true);
				}
				m_incomingBuffer->freeBlock(payloadSize);
				m_messageSize = 0;
			}
		}
	}
	return result;
}

bdUInt32 bdLobbyConnection::getReceiveBufferSize() const
{
	return m_incomingBuffer->getTotalFreeSpace();
}

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON
bdBool bdLobbyConnection::connect(bdAuthInfo &authInfo)
#else
bdBool bdLobbyConnection::connect(bdAuthInfo&)
#endif
{
	m_status = BD_CONNECTING;
#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON
	setSessionKey(authInfo.m_sessionKey);
#endif

#if defined BD_PLATFORM_XBOX
	// FIXME , this only permits connections to Title Servers....
	bdBool result = false;
	const DWORD titleid = static_cast<DWORD>(m_addr->getTitleId());
	IN_ADDR server;

	result = XNetTsAddrToInAddr(&(m_addr->getXNAddr()), titleid, &m_secID, &server) == 0;

	bdAddr serverAddr(server.S_un.S_addr, m_addr->getPort());
	const bdSocketStatusCode status = m_socket.connect(serverAddr);
#elif defined BD_PLATFORM_XENON
	// FIXME , this only permits connections to Title Servers....
	const bdSocketStatusCode status = m_socket.connect(m_serverAddr);
#else
	const bdSocketStatusCode status = m_socket.connect(m_addr->getPublicAddr());
#endif

	switch(status)
	{
		case BD_NET_SUCCESS:
			m_status = BD_CONNECTED;
			callListenersConnect(true);
			return true;
		case BD_NET_WOULD_BLOCK:
			return true;
		default:
			close();
			return false;
	}
}


void bdLobbyConnection::disconnect()
{
	close();
}

void bdLobbyConnection::close()
{

	if(m_status == BD_CONNECTING)
	{
		callListenersConnect(false);
	}
	else if(m_status != BD_DISCONNECTED)
	{
		callListenersDisconnect();
	}
	m_status = BD_DISCONNECTED;
	m_socket.close();
}


void bdLobbyConnection::callListenersConnect(const bdBool success)
{
	if(success)
	{
#if defined BD_SEND_KEEP_ALIVE
		m_keepAliveTimer.reset();
		m_keepAliveTimer.start();
		m_lastReceivedTimer.reset();
		m_lastReceivedTimer.start();
#endif

		if (m_connectionListener)
		{
			m_connectionListener->onConnect(this);
		}
	}
	else
	{
		if (m_connectionListener)
		{
			m_connectionListener->onConnectFailed(this);
		}
	}
}


void bdLobbyConnection::callListenersDisconnect()
{
	if (m_connectionListener)
	{
		m_connectionListener->onDisconnect(this);
	}
}


bdBool bdLobbyConnection::pump()
{
	// Defend against non-threadsafe operations
	m_mutex.lock();

	while (m_outgoingBuffer->getUsedBlockSize() > 0 &&
		(m_status == BD_CONNECTED || m_status == BD_CONNECTING))
	{
		const bdInt sent = m_socket.send(m_outgoingBuffer->getUsedBlockStart(),
										m_outgoingBuffer->getUsedBlockSize());
		if (sent > 0)
		{
#if defined BD_SEND_KEEP_ALIVE
			m_keepAliveTimer.start();
#endif
			if (m_status == BD_CONNECTING)
			{
				m_status = BD_CONNECTED;
				callListenersConnect(true);
			}
			m_outgoingBuffer->freeBlock(static_cast<bdUInt>(sent));
		}
		else
		{
			switch(sent)
			{
				case BD_NET_SUBSYTEM_ERROR:
					// something very wrong.
					BD_WARN(BD_LOG_LEVEL, "net subsystem error!");
					close();
					break;
				case BD_NET_ADDRESS_INVALID:
					BD_INFO(BD_LOG_LEVEL, "Invalid address. Closing connection.");
					close();
					break;
				case BD_NET_ERROR:
					// unknown error..
					BD_WARN(BD_LOG_LEVEL, "unknown error.");
					close();
					break;
				case BD_NET_WOULD_BLOCK:
					// Is actually ok to happen. Means there is no buffer space at all, or something.
					//BD_WARN(BD_LOG_LEVEL, "would block.");
					break;
				case BD_NET_ADDRESS_IN_USE:
					// can't happen.. not binding
					break;
				case BD_NET_CONNECTION_RESET:
				case BD_NET_MSG_SIZE:
					close();
					break;
				case BD_NET_INVALID_HANDLE:
					BD_WARN(BD_LOG_LEVEL, "invalid handle.");
					close();
					break;
				case BD_NET_NOT_CONNECTED:
					/*
					if (m_status == BD_CONNECTING)
					{
						//BD_INFO(BD_LOG_LEVEL, "Failed sending because connection is still pending.");
					}
					else
					{
						close();
					}*/
					break;
				default:
					BD_WARN(BD_LOG_LEVEL, "unknown error.");
					close();
					break;
			}
			return m_status == BD_CONNECTED || m_status == BD_CONNECTING;
		}
	}
#if defined BD_SEND_KEEP_ALIVE
	if (m_keepAliveTimer.getElapsedTimeInSeconds() > BD_KEEP_ALIVE_PERIOD)
	{
		bdUInt32 keepAlive = 0;
		m_socket.send(&keepAlive, sizeof(keepAlive));
		m_keepAliveTimer.start();
	}
	if (m_lastReceivedTimer.getElapsedTimeInSeconds() > 3 * BD_KEEP_ALIVE_PERIOD)
	{
		BD_WARN(BD_LOG_LEVEL,"Connection timed out\n");
		close();
	}

#endif
	// Defend against non-threadsafe operations
	m_mutex.unlock();

	return true;
}

bdBool bdLobbyConnection::receivedFullMessage()
{
	// See if we need to shift
	if (m_incomingBuffer->getUsedBlockSize() < sizeof(m_messageSize) &&
		m_incomingBuffer->getFreeBlockSize() + m_incomingBuffer->getUsedBlockSize() < sizeof(m_messageSize))
	{
		m_incomingBuffer->alignUsedBlock();
	}


	if (m_incomingBuffer->getUsedBlockSize() >= sizeof(m_messageSize))
	{
		//m_messageSize = *(reinterpret_cast<bdUInt16*>(m_incomingBuffer->getUsedBlockStart()));
		bdUInt offset =0;
		bdBytePacker::removeBasicType(m_incomingBuffer->getUsedBlockStart(), m_incomingBuffer->getUsedBlockSize(), 0, offset, m_messageSize);
		offset++;
		// Special case: a message size of 0 indicates a reply from the server
		// following a keep alive message sent from the client.
#if defined BD_SEND_KEEP_ALIVE
		if (m_messageSize == 0)
		{
			bdUInt32 temp;
			m_incomingBuffer->removeBasicType(temp);
		}
#endif
	}

	if (m_messageSize > 0 && m_incomingBuffer->getUsedBlockSize() >= m_messageSize + sizeof(m_messageSize))
	{
		return true;
	}


	// See if we need to shift to be able to fit the whole message
	if (m_incomingBuffer->getUsedBlockSize() + m_incomingBuffer->getFreeBlockSize() <
			sizeof(m_messageSize) + m_messageSize)
	{
		m_incomingBuffer->alignUsedBlock();
	}

	return false;
}


