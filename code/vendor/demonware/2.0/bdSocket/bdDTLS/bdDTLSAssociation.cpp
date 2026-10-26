// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdDTLS/bdDTLSAssociation.h>


#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#include <bdCore/bdCrypto/bdHMacSHA1.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>
#include <bdCore/bdCrypto/bdECCKey.h>
#include <bdCore/bdSocket/bdCommonAddrInfo.h>


#include <bdSocket/bdAddressMap.h>
#include <bdSocket/bdSecurityKeyMap.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSCookieAck.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSCookieEcho.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSData.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSInit.h>

// define BD_NO_SHARED_SECRET_CALCULATION to skip the shared
// secret calculation. This calculation can take a long time on slower
// platforms.
//#define BD_NO_SHARED_SECRET_CALCULATION


#define BD_SOCKET_INIT_TIMEOUT		(1.f)
#define BD_SOCKET_COOKIE_TIMEOUT	(1.f)

#if defined BD_RELEASE
#	define BD_SOCKET_INIT_RESENDS		(5.f)
#	define BD_SOCKET_COOKIE_RESENDS		(5.f)
#else
#	define BD_SOCKET_INIT_RESENDS		(1000.f)
#	define BD_SOCKET_COOKIE_RESENDS		(1000.f)
#endif // RELEASE

#if defined BD_RELEASE
#	define BD_SOCKET_COOKIE_LIFE		(60)
#else
#	define BD_SOCKET_COOKIE_LIFE		(3600)
#endif // RELEASE

#define BD_LOG_LEVEL "bdSocket/dtls"

bdNonce<BD_SOCKET_COOKIE_KEY_SIZE> bdDTLSAssociation::m_cookieKey;

bdDTLSAssociation::bdDTLSAssociation(bdDefaultSocket *socket,
									 bdSecurityKeyMap *keyMap,
									 bdECCKey *ECCkey,
									 const bdAddr &addr,
									 bdAddrHandleRef addrHandle,
									 bdCommonAddrRef localCommonAddr,
									 bdAddressMap *addrMap,
									 bdFloat32 receiveTimeout)
: m_socket(socket),
  m_keyMap(keyMap),
  m_ECCKey(ECCkey),
  m_addr(addr),
  m_addrHandle(addrHandle),
  m_state(BD_DTLS_CLOSED),
  m_initResends(0),
  m_cookieResends(0),
  m_incomingSeqNums(-1),
  m_localCommonAddr(localCommonAddr),
  m_addrMap(addrMap),
  m_receiveTimeout(receiveTimeout)
{
	reset();
	bdDTLSAssociation::m_cookieKey.ensureCreated();
}

bdDTLSAssociation::~bdDTLSAssociation()
{
	m_ECCKey = BD_NULL;
	m_keyMap = BD_NULL;
	m_socket = BD_NULL;
}

void bdDTLSAssociation::connect()
{
	sendInit();
	m_state = BD_DTLS_COOKIE_WAIT;
}

bdDTLSAssociationStatus bdDTLSAssociation::getStatus()
{
	bdDTLSAssociationStatus status = BD_SOCKET_LOST;

	switch(m_state)
	{
		case BD_DTLS_CLOSED:
			status = BD_SOCKET_LOST;
			break;
		case BD_DTLS_COOKIE_WAIT:
		case BD_DTLS_COOKIE_ECHOED:
			status = BD_SOCKET_PENDING;
			break;
		case BD_DTLS_ESTABLISHED:
			status = BD_SOCKET_CONNECTED;
			break;
		default:
			BD_ERR(BD_LOG_LEVEL, "Bad state!");
			bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
			bdCommonAddrInfo::getBriefInfo(m_addrHandle->m_endpoint.getCommonAddr(), commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
			BD_INFO(BD_LOG_LEVEL, commonAddrInfo);
			break;
	}

	return status;
}


bdInt	bdDTLSAssociation::sendTo(const bdAddr &addr,
								  const void *data,
								  const bdUInt length)
{
	bdInt val = BD_NET_ERROR;

	switch(m_state)
	{
		case BD_DTLS_CLOSED:
		case BD_DTLS_COOKIE_WAIT:
		case BD_DTLS_COOKIE_ECHOED:
			BD_WARN(BD_LOG_LEVEL, "not connected. Cannot send.");
			break;
		case BD_DTLS_ESTABLISHED:
			{
				val = sendData(addr, data, length);				
			}
			break;
	}

	return val;
}

bdInt	bdDTLSAssociation::receiveFrom(bdAddr &addr,
									   const void *data,
									   const bdUInt size,
									   bdAddrHandleRef &addrHandle,
									   void *buffer,
									   const bdUInt bufferSize)
{
	bdInt val = BD_NET_WOULD_BLOCK;
	bdDTLSHeader header;
	bdUInt tmpUInt;

	if (header.deserialize(data, size, 0, tmpUInt))
	{
		const bdDTLSPacketTypes type = static_cast<bdDTLSPacketTypes>(header.getType());

		switch(type)
		{
			case BD_DTLS_INIT:
				val = handleInit(addr, data, size);
				break;
			case BD_DTLS_INIT_ACK:
				val = handleInitAck(addr, data, size);
				break;
			case BD_DTLS_COOKIE_ECHO:
				val = handleCookieEcho(addr, data, size);
				break;
			case BD_DTLS_COOKIE_ACK:
				val = handleCookieAck(data, size);
				break;
			case BD_DTLS_ERROR:
				val = handleError(addr, data, size);
				break;
			case BD_DTLS_DATA:
				val = handleData(addr, 
								 reinterpret_cast<const bdUByte8 *>(data), 
								 size, 
								 addrHandle, 
								 reinterpret_cast<bdUByte8 *>(buffer), 
								 bufferSize);
				break;
			default:
				val = BD_NET_WOULD_BLOCK;
				break;
		}
	}

	return val;
}

void bdDTLSAssociation::pump()
{
	switch(m_addrHandle->getStatus())
	{
		case bdAddrHandle::BD_ADDR_NOT_CONSTRUCTED:
			{
				bdNChar8 addrString[BD_ADDR_STRING_SIZE];
				m_addrHandle->getRealAddr().toString(addrString, BD_ADDR_STRING_SIZE);
				BD_WARN(BD_LOG_LEVEL, "Address not constructed (%s). Shouldn't be here. Closing.", addrString);
				m_state = BD_DTLS_CLOSED;
			}
			break;
		case bdAddrHandle::BD_ADDR_NOT_RESOLVED:
			{
				bdNChar8 addrString[BD_ADDR_STRING_SIZE];
				m_addrHandle->getRealAddr().toString(addrString, BD_ADDR_STRING_SIZE);
				BD_WARN(BD_LOG_LEVEL, "Address not resolved (%s). Shouldn't be here. Closing.", addrString);
				m_state = BD_DTLS_CLOSED;
			}
			break;
		case bdAddrHandle::BD_ADDR_ERROR:
			BD_WARN(BD_LOG_LEVEL, "Address error.");
			m_state = BD_DTLS_CLOSED;
			break;
		case bdAddrHandle::BD_ADDR_RESOLVED:
			// expected state. continue.
			break;
		case bdAddrHandle::BD_ADDR_UNRESOLVED:
			BD_INFO(BD_LOG_LEVEL, "Address unregistered. Closing DTLS.");
			m_state = BD_DTLS_CLOSED;
			break;
	}

	if(m_lastReceived.getElapsedTimeInSeconds() > m_receiveTimeout)
	{
		// timed out
		BD_INFO(BD_LOG_LEVEL, "DTLS receive timeout. Closing.");
		m_state = BD_DTLS_CLOSED;
	}
	else
	{
		switch(m_state)
		{
			case BD_DTLS_CLOSED:
				break;
			case BD_DTLS_COOKIE_WAIT:
				// check to see if we need to send another init packet
				if(m_initTimer.getElapsedTimeInSeconds() > BD_SOCKET_INIT_TIMEOUT)
				{
					sendInit();
				}
				break;
			case BD_DTLS_COOKIE_ECHOED:
				// check to see if we need to send another cookie echo packet
				if(m_cookieTimer.getElapsedTimeInSeconds() > BD_SOCKET_COOKIE_TIMEOUT)
				{
					sendCookieEcho(m_addr);
				}
				break;
			case BD_DTLS_ESTABLISHED:
				break;
		}
	}
}

bdAddrHandleRef bdDTLSAssociation::getAddrHandle()
{
	return m_addrHandle;
}

const bdSecurityID* bdDTLSAssociation::getLocalSecurityId()
{
	return &m_localId;
}

// protected:

bdInt bdDTLSAssociation::handleInit(const bdAddr &addr, const void *data, const bdUInt size)
{
	bdDTLSInit init;
	bdUInt tmpUInt;

	if (init.deserialize(data, size, 0, tmpUInt))
	{
		BD_INFO(BD_LOG_LEVEL, "handling init: m_localTag: %d, init->m_initTag: %d",
				m_localTag, init.getInitTag());

		sendInitAck(addr, init);

		m_lastReceived.start();
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "bad init packet.");
	}
	
	return BD_NET_WOULD_BLOCK;
}

bdInt bdDTLSAssociation::handleInitAck(const bdAddr &addr, const void *data, const bdUInt size)
{
	//If an INIT ACK is received by an endpoint in any state other than the
	//COOKIE-WAIT state, the endpoint should discard the INIT ACK chunk.
	//An unexpected INIT ACK usually indicates the processing of an old or
	//duplicated INIT chunk.
	if(m_state == BD_DTLS_COOKIE_WAIT)
	{
		// Use a temporary object here in case deserialize fails.
		bdDTLSInitAck initAck;
		bdUInt tmpUInt;

		if (initAck.deserialize(data, size, 0, tmpUInt))
		{
			m_initAck = initAck;

			if (m_initAck.getVtag() == m_localTag)
			{
				m_peerTag = m_initAck.getInitTag();

				BD_INFO(BD_LOG_LEVEL, "handling init ack: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

				sendCookieEcho(addr);

				m_lastReceived.start();

				m_state = BD_DTLS_COOKIE_ECHOED;
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Received init ack with invalid vtag. Ignoring");
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to deserialize init ack .Ignoring");
		}
	}

	return BD_NET_WOULD_BLOCK;
}

bdInt bdDTLSAssociation::handleCookieEcho(const bdAddr &addr, const void *data, const bdUInt size)
{
	bdBool result = false;
	bdDTLSCookieEcho cookieEcho;
	bdUInt tmpUInt;

	if (cookieEcho.deserialize(data, size, 0, tmpUInt))
	{
		if (!checkCookieValidity(addr, cookieEcho))
		{
			BD_WARN(BD_LOG_LEVEL, "Received bad cookie. Ignoring.");
			return BD_NET_WOULD_BLOCK;
		}

		const bdUInt16 localTag = cookieEcho.getCookie().getLocalTag();
		const bdUInt16 peerTag = cookieEcho.getCookie().getPeerTag();
		const bdUInt16 localTieTag = cookieEcho.getCookie().getLocalTieTag();
		const bdUInt16 peerTieTag = cookieEcho.getCookie().getPeerTieTag();

		bdSecurityID secID;
		bdCommonAddrRef commonAddr = new bdCommonAddr;

		bdMemcpy(secID.ab, cookieEcho.getSecID(), BD_SECURITY_ID_LENGTH);
		result = commonAddr->deserialize(m_localCommonAddr, cookieEcho.getCa());

		if(result)
		{
			switch(m_state)
			{
				case BD_DTLS_CLOSED:
					// expected state.
					m_localTag = localTag;
					m_peerTag = peerTag;
					
					if (!m_keyMap->contains(secID))
					{
						BD_WARN(BD_LOG_LEVEL, "secID not in keymap.");
						sendError(addr, secID, bdDTLSError::BD_DTLS_ERROR_BAD_SECID);
						m_state = BD_DTLS_CLOSED;
					}
					else if(calculateSharedKey(cookieEcho.getECCKey(), BD_ECC_EXPORTED_KEY_SIZE, secID))
					{
						// Remove any previous mapping for this common addr.
						bdBool removed = false;
						bdAddrHandleRef addrHandle;
						const bdBool found = m_addrMap->findAddrHandle(commonAddr, secID, addrHandle);
						if(found && addrHandle.notNull())
						{
							removed = m_addrMap->unregisterAddr(addrHandle);

							if (!removed)
							{
								// We don't expect this to happen very often.'
								BD_INFO(BD_LOG_LEVEL, "Failed to remove existing common addr mapping.");
							}
						}


						
						// Now get a new addr handle for the common addr.
						bdBool result = m_addrMap->getAddrHandle(commonAddr, secID, m_addrHandle);
						if (true == result)
						{
								
							m_addrHandle->setRealAddr(addr);
							m_addr = addr;

							BD_INFO(BD_LOG_LEVEL, "handling cookie echo (expected): m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

							sendCookieAck(addr, cookieEcho);
							m_state = BD_DTLS_ESTABLISHED;

							BD_INFO(BD_LOG_LEVEL, "DTLS established.");

							result = true;

							m_localId = secID;
						}
						else
						{
							// addr handle setup has failed.
							// we will return false from this function.
						}
					}
					else
					{
						BD_WARN(BD_LOG_LEVEL, "Failed to generate shared secret");
						result = false;
					}
					break;
				default:
					// unexpected state.
					//+------------+------------+---------------+--------------+-------------+
					//|  Local Tag | Peer's Tag | Local-Tie-Tag |Peer's-Tie-Tag|   Action/   |
					//|            |            |               |              | Description |
					//+------------+------------+---------------+--------------+-------------+
					//|    X       |     X      |      M        |      M       |     (A)     |
					//+------------+------------+---------------+--------------+-------------+
					//|    M       |     X      |      A        |      A       |     (B)     |
					//+------------+------------+---------------+--------------+-------------+
					//|    M       |     0      |      A        |      A       |     (B)     |
					//+------------+------------+---------------+--------------+-------------+
					//|    X       |     M      |      0        |      0       |     (C)     |
					//+------------+------------+---------------+--------------+-------------+
					//|    M       |     M      |      A        |      A       |     (D)     |
					//+======================================================================+
					//|       Table 2: Handling of a COOKIE ECHO when a TCB exists           |
					//+======================================================================+
					//
					//   Legend:
					//
					//      X - Tag does not match the existing TCB
					//      M - Tag matches the existing TCB.
					//      0 - No Tie-Tag in Cookie (unknown).
					//      A - All cases, i.e. M, X or 0.
					//

					if(
						m_localTag != localTag && m_peerTag != peerTag
						&&
						m_localTag == localTieTag && m_peerTag == peerTieTag
						)
					{
						//A) In this case, the peer may have restarted.  When the endpoint
						//   recognizes this potential 'restart', the existing session is
						//   treated the same as if it received an ABORT followed by a new
						//   COOKIE ECHO with the following exceptions:

						//   -  Any SCTP DATA Chunks MAY be retained (this is an implementation
						//      specific option).

						//   -  A notification of RESTART SHOULD be sent to the ULP instead of
						//      a "COMMUNICATION LOST" notification.

						//   All the congestion control parameters (e.g., cwnd, ssthresh)
						//   related to this peer MUST be reset to their initial values (see
						//   Section 6.2.1).

						//   After this the endpoint shall enter the ESTABLISHED state.

						//If the endpoint is in the SHUTDOWN-ACK-SENT state and recognizes
						//the peer has restarted (Action A), it MUST NOT setup a new
						//association but instead resend the SHUTDOWN ACK and send an ERROR
						//chunk with a "Cookie Received while Shutting Down" error cause to
						//its peer.

						if (!m_keyMap->contains(secID))
						{
							m_localTag = localTag;
							m_peerTag = peerTag;

							BD_WARN(BD_LOG_LEVEL, "secID not in keymap.");

							sendError(addr, secID, bdDTLSError::BD_DTLS_ERROR_BAD_SECID);
							m_state = BD_DTLS_CLOSED;
						}
						else if(calculateSharedKey(cookieEcho.getECCKey(), BD_ECC_EXPORTED_KEY_SIZE, secID))
						{
							reset();

							m_localTag = localTag;
							m_peerTag = peerTag;


							// our address could already be in the map because this is reconnect
							// logic. if it is just reuse
							// add their details to the address map.
							m_addrMap->getAddrHandle(commonAddr, secID, m_addrHandle);
							m_addrHandle->setRealAddr(addr);
							m_addr = addr;

							sendCookieAck(addr, cookieEcho);
							BD_INFO(BD_LOG_LEVEL, "handling cookie echo (A): m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

							m_state = BD_DTLS_ESTABLISHED;

							BD_INFO(BD_LOG_LEVEL, "DTLS established.");

							result = true;

							m_localId = secID;
						}
						else
						{
							BD_WARN(BD_LOG_LEVEL, "Failed to generate shared secret");
							result = false;
						}
					}
					else if(
							(m_localTag == localTag && m_peerTag != peerTag)
							||
							(m_localTag == localTag && !peerTieTag)
							)
					{
						//B) In this case, both sides may be attempting to start an association
						//   at about the same time but the peer endpoint started its INIT
						//   after responding to the local endpoint's INIT.  Thus it may have
						//   picked a new Verification Tag not being aware of the previous Tag
						//   it had sent this endpoint.  The endpoint should stay in or enter
						//   the ESTABLISHED state but it MUST update its peer's Verification
						//   Tag from the State Cookie, stop any init or cookie timers that may
						//   running and send a COOKIE ACK.

						BD_ASSERT(m_addrHandle->getStatus() == bdAddrHandle::BD_ADDR_RESOLVED, "Invalid addr handle!");

						m_peerTag = peerTag;

						BD_INFO(BD_LOG_LEVEL, "handling cookie echo (B): m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

						if (!m_keyMap->contains(secID))
						{
							BD_WARN(BD_LOG_LEVEL, "secID not in keymap.");
							sendError(addr, secID, bdDTLSError::BD_DTLS_ERROR_BAD_SECID);
							m_state = BD_DTLS_CLOSED;
						}
						else if(calculateSharedKey(cookieEcho.getECCKey(), BD_ECC_EXPORTED_KEY_SIZE, secID))
						{
							// our address could already be in the map, in which case reuse.
							// make sure we're not already in the map
							// add their details to the address map.
							m_addrMap->getAddrHandle(commonAddr, secID, m_addrHandle);
							m_addrHandle->setRealAddr(addr);
							m_addr = addr;

							sendCookieAck(addr, cookieEcho);
							m_state = BD_DTLS_ESTABLISHED;

							BD_INFO(BD_LOG_LEVEL, "DTLS established.");

							m_localId = secID;
							result = true;							
						}
						else
						{
							BD_WARN(BD_LOG_LEVEL, "Failed to generate shared secret.");
							result = false;
						}
					}
					else if(m_localTag != localTag && m_peerTag == peerTag)
					{
						//C) In this case, the local endpoint's cookie has arrived late.
						//   Before it arrived, the local endpoint sent an INIT and received an
						//   INIT-ACK and finally sent a COOKIE ECHO with the peer's same tag
						//   but a new tag of its own.  The cookie should be silently
						//   discarded.  The endpoint SHOULD NOT change states and should leave
						//   any timers running.

						BD_INFO(BD_LOG_LEVEL, "handling cookie echo (C): m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);
					}
					else if(m_localTag == localTag && m_peerTag == peerTag)
					{
						//D) When both local and remote tags match the endpoint should always
						//	enter the ESTABLISHED state, if it has not already done so. It
						//	should stop any init or cookie timers that may be running and send
						//	a COOKIE ACK.

						// No need to do shared key calculations here, because this branch is only entered
						// if some freak cookie re-send occurs. Calculations would've been done earlier.

						// Nonetheless, do some logging in case the above assumption is wrong.
						if (!(m_localId == secID))
						{
							BD_WARN(BD_LOG_LEVEL, "Received cookie echo in state D, but local secID doesn't match the cookie one. "
												  "This connection is unlikely to work.");
						}

						m_state = BD_DTLS_ESTABLISHED;

						BD_ASSERT(m_addrHandle->getStatus() == bdAddrHandle::BD_ADDR_RESOLVED, "Invalid addr handle!");

						BD_INFO(BD_LOG_LEVEL, "handling cookie echo (D): m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

						sendCookieAck(addr, cookieEcho);

						result = true;

						m_localId = secID;
					}
					else
					{
						//   any case not shown in above table, the cookie should be
						//   silently discarded.

						BD_INFO(BD_LOG_LEVEL, "handling cookie echo (E): m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);

						result = true;
					}
					break;
			}
		}
	}

	if(result)
	{
		m_lastReceived.start();
	}

	return BD_NET_WOULD_BLOCK;
}

bdInt bdDTLSAssociation::handleCookieAck(const void *data, const bdUInt size)
{
	bdDTLSCookieAck cookieAck;
	bdUInt tmpUInt;

	if (cookieAck.deserialize(data, size, 0, tmpUInt))
	{
		if (cookieAck.getVtag() == m_localTag)
		{			
			bdSecurityID secID;
			cookieAck.getSecID(secID);

			switch(m_state)
			{
				case BD_DTLS_CLOSED:
				case BD_DTLS_COOKIE_WAIT:
				{
					const bdUInt tempState = m_state;
					BD_INFO(BD_LOG_LEVEL, "Received cookie ack when in unexpected state %u. " , tempState);
				}					
				break;
				case BD_DTLS_COOKIE_ECHOED:
				{
					if(calculateSharedKey(cookieAck.getECCKey(), BD_ECC_EXPORTED_KEY_SIZE, secID))
					{
						m_lastReceived.start();
						m_state = BD_DTLS_ESTABLISHED;
						BD_INFO(BD_LOG_LEVEL, "handled cookie ack. DTLS established.");
						m_localId = secID;
					}
					else
					{
						bdNChar8 addrBuffer [BD_ADDR_STRING_SIZE];
						m_addr.toString(addrBuffer, BD_ADDR_STRING_SIZE);
						BD_WARN(BD_LOG_LEVEL, "Failed to calculate shared key for %s " , addrBuffer );
					}
				}
				break;
				case BD_DTLS_ESTABLISHED:
				{
					BD_INFO(BD_LOG_LEVEL, "Received cookie ack for already established DTLS, ignoring.");
				}
				break;
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Received cookie ack with invalid vtag. Ignoring");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to deserialize cookie ack. Ignoring");
	}

	return BD_NET_WOULD_BLOCK;
}

bdInt bdDTLSAssociation::handleError(const bdAddr &addr, const void *data, const bdUInt size)
{
	bdDTLSError error;
	bdUInt tmpUInt;

	if (error.deserialize(data, size, 0, tmpUInt))
	{
		if (error.getVtag() == m_localTag)
		{
			const bdUInt maxAddrLength = 22;
			bdNChar8 addrStr[maxAddrLength];

			BD_ASSERT(addr.toString(addrStr, maxAddrLength) != 0,
					"Buffer supplied does not fit an address");
			
			switch (error.getEtype())
			{
			case bdDTLSError::BD_DTLS_ERROR_BAD_SECID:
				BD_INFO(BD_LOG_LEVEL, "Received bad_security_id error reset "
					"from %s. Closing the association.", addrStr);
				m_state = BD_DTLS_CLOSED;
				break;
			default:
				BD_WARN(BD_LOG_LEVEL, "Received unrecognized error message from "
					"%s. Ignoring.", addrStr);
			}
		}
	}
	return BD_NET_WOULD_BLOCK;
}

bdInt bdDTLSAssociation::handleData(bdAddr &addr,
									const bdUByte8 *data,
									const bdUInt size,
									bdAddrHandleRef &addrHandle,
									bdUByte8 *buffer,
									const bdUInt bufferSize)
{
	bdInt val = BD_NET_WOULD_BLOCK;

	switch(m_state)
	{
		case BD_DTLS_CLOSED:
		case BD_DTLS_COOKIE_WAIT:
		case BD_DTLS_COOKIE_ECHOED:
		break;
		case BD_DTLS_ESTABLISHED:
		{
			if(m_addrHandle->m_endpoint.getCommonAddr().notNull())
			{
				bdDTLSData dataHeader;
				bdUInt dataHeaderSize;
				bdUInt payloadSize;

				if (dataHeader.deserialize(data, size, 0, dataHeaderSize,
						m_incomingSeqNums.getLastSequenceNumber(),
						m_sharedKey, buffer, bufferSize, payloadSize, &m_cypher))
				{
					if (verify(dataHeader))
					{
						val = payloadSize;
						addr = m_addr;
						addrHandle = m_addrHandle;
						m_lastReceived.start();
					}
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Receiving data before m_addrHandle is set!!");
			}
		}
		break;
	}

	return val;
}


void bdDTLSAssociation::sendInit()
{
	if(m_initResends++ <= BD_SOCKET_INIT_RESENDS)
	{
		bdSecurityID secID = m_addrHandle->m_endpoint.getSecID();

		bdDTLSInit init(m_localTag, secID);
		const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 buffer[bufferSize];
		bdUInt tmpUInt;

		BD_ASSERT(init.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize a packet.");

		m_socket->sendTo(m_addr, buffer, tmpUInt);

		m_initTimer.start();

		BD_INFO(BD_LOG_LEVEL, "sending init: m_localTag: %d", m_localTag);
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "sending init: too many retries, closing");
		m_state = BD_DTLS_CLOSED;
	}
}

void bdDTLSAssociation::sendInitAck(const bdAddr &addr, const bdDTLSInit &init)
{
	bdBool result = true;

	bdUInt16 peerTag = init.getInitTag();
	
	bdUInt16 localTag = 0;
	bdUInt16 peerTieTag = 0;
	bdUInt16 localTieTag = 0;

	switch(m_state)
	{
		case BD_DTLS_CLOSED:
			// expected state.
			m_peerTag = peerTag;
			localTag = m_localTag;
			break;
		case BD_DTLS_COOKIE_ECHOED:
			//For an endpoint that is in the COOKIE-ECHOED state it MUST populate
			//its Tie-Tags with the Tag information of itself and its peer.
			peerTieTag = m_peerTag;
			localTieTag = m_localTag;
			//break; continue into next case:
		case BD_DTLS_COOKIE_WAIT:	
			//This usually indicates an initialization collision, i.e., each
			//endpoint is attempting, at about the same time, to establish an
			//association with the other endpoint.
			//Upon receipt of an INIT in the COOKIE-WAIT or COOKIE-ECHOED state, an
			//endpoint MUST respond with an INIT ACK using the same parameters it
			//sent in its original INIT chunk (including its Initiation Tag,
			//unchanged).  These original parameters are combined with those from
			//the newly received INIT chunk.  The endpoint shall also generate a
			//State Cookie with the INIT ACK.  The endpoint uses the parameters
			//sent in its INIT to calculate the State Cookie.
			localTag = m_localTag;
			break;
		case BD_DTLS_ESTABLISHED:
			//Unless otherwise stated, upon reception of an unexpected INIT for
			//this association, the endpoint shall generate an INIT ACK with a
			//State Cookie.  In the outbound INIT ACK the endpoint MUST copy its
			//current Verification Tag and peer's Verification Tag into a reserved
			//place within the state cookie.  We shall refer to these locations as
			//the Peer's-Tie-Tag and the Local-Tie-Tag.  The outbound SCTP packet
			//containing this INIT ACK MUST carry a Verification Tag value equal to
			//the Initiation Tag found in the unexpected INIT.  And the INIT ACK
			//MUST contain a new Initiation Tag. Other parameters for the endpoint 
			//SHOULD be copied from the
			//existing parameters of the association into the INIT ACK and cookie.
			localTag = static_cast<bdUInt16>(bdTrulyRandom::getInstance()->getRandomUInt());
			peerTieTag = m_peerTag;
			localTieTag = m_localTag;
			break;
		default:
			result = false;
			break;
	};

	if(result)
	{
		bdSecurityID secID;
		init.getSecID(secID);
		bdDTLSInitAck initAck(peerTag, localTag, localTag, peerTag, localTieTag, peerTieTag,
			bdPlatformTiming::getLoResTimeStamp(), addr, secID);

		bdHMacSHA1 hmac(m_cookieKey.getData(), BD_SOCKET_COOKIE_KEY_SIZE);
		initAck.sign(hmac);

		const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 buffer[bufferSize];
		bdUInt tmpUInt;

		BD_ASSERT(initAck.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize a packet.");

		if(m_socket->sendTo(addr, buffer, tmpUInt) != static_cast<bdInt>(tmpUInt))
		{
			BD_WARN(BD_LOG_LEVEL, "problem with socket?");
		}

		BD_INFO(BD_LOG_LEVEL, "sending init ack: m_localTag/localTag/m_peerTag: %d/%d/%d", m_localTag, localTag, m_peerTag);
	}
}

void bdDTLSAssociation::sendCookieEcho(const bdAddr &addr)
{
	if(m_cookieResends++ <= BD_SOCKET_COOKIE_RESENDS)
	{		
		bdDTLSCookieEcho cookieEcho(m_peerTag, m_initAck, m_localCommonAddr, m_ECCKey);
		const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 buffer[bufferSize];
		bdUInt tmpUInt;

		// TODO, we should really handle the failure here more elegantly.
		BD_ASSERT(cookieEcho.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize a packet.");

		// send
		const bdInt sendRes = m_socket->sendTo(addr, buffer, tmpUInt);
		if (sendRes < 0)
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to send cookie echo");
		}
		else
		{
			BD_INFO(BD_LOG_LEVEL, "sending cookie echo: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);
			m_state = BD_DTLS_COOKIE_ECHOED;
		}
		m_cookieTimer.start();
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "sending cookie echo: too many retries, closing");
		m_state = BD_DTLS_CLOSED;
	}
}

void bdDTLSAssociation::sendCookieAck(const bdAddr &addr, const bdDTLSCookieEcho&)
{
	bdUInt keyLen = BD_ECC_EXPORTED_KEY_SIZE;
	bdUByte8 key[BD_ECC_EXPORTED_KEY_SIZE];
	if(!m_ECCKey->exportKey(key, keyLen) || keyLen != BD_ECC_EXPORTED_KEY_SIZE)
	{
		BD_WARN(BD_LOG_LEVEL, "problem with dh key");
	}
	
	const bdSecurityID& secID = m_addrHandle->m_endpoint.getSecID();

	bdDTLSCookieAck cookieAck(m_peerTag, key, secID);
	const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
	bdUByte8 buffer[bufferSize];
	bdUInt tmpUInt;

	BD_ASSERT(cookieAck.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize a packet.");

	m_socket->sendTo(addr, buffer, tmpUInt);

	BD_INFO(BD_LOG_LEVEL, "sending cookie ack: m_localTag/m_peerTag: %d/%d", m_localTag, m_peerTag);
}

void bdDTLSAssociation::sendError(const bdAddr &addr, const bdSecurityID &secID, const bdDTLSError::bdDTLSErrorType &type)
{
	bdDTLSError error(m_peerTag, type, secID);
	const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
	bdUByte8 buffer[bufferSize];
	bdUInt tmpUInt;

	BD_ASSERT(error.serialize(buffer, bufferSize, 0, tmpUInt), "Failed to serialize a packet.");

	m_socket->sendTo(addr, buffer, tmpUInt);

	BD_INFO(BD_LOG_LEVEL, "sending error: etype: %d", type);
}

bdInt bdDTLSAssociation::sendData(const bdAddr &addr, const void *data, const bdUInt length)
{
	bdInt val = BD_NET_ERROR;

	if(length < BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE)
	{
		const bdUInt outDataSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 outData[outDataSize];
		bdUInt packetLength;
		
		m_seqNum++;
		bdDTLSData dataPacket(m_peerTag, static_cast<bdUInt16>(m_seqNum.getValue()));

		if (!dataPacket.serialize(outData, outDataSize, 0, packetLength,
					m_seqNum, m_sharedKey, reinterpret_cast<const bdUByte8 *>(data), length, &m_cypher))
		{
			BD_WARN(BD_LOG_LEVEL, "Packet creation failed.");
			return val;
		}

		BD_ASSERT(packetLength < BD_MAX_DATAGRAM_SIZE, "overflow.");

		val = m_socket->sendTo(addr, outData, packetLength);
	}
	else
	{
		val = BD_NET_MSG_SIZE;
	}

	return val;
}

bdUInt bdDTLSAssociation::prepareData(const void *data, const bdUInt length, void *outData, const bdUInt outDataSize)
{
	bdUInt packetLength = 0;

	if(length < BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE && outDataSize == BD_MAX_DATAGRAM_SIZE)
	{
		m_seqNum++;
		bdDTLSData dataPacket(m_peerTag, static_cast<bdUInt16>(m_seqNum.getValue()));

		if (!dataPacket.serialize(outData, outDataSize, 0, packetLength,
					m_seqNum, m_sharedKey, reinterpret_cast<const bdUByte8 *>(data), length, &m_cypher))
		{
			BD_WARN(BD_LOG_LEVEL, "Packet creation failed.");
		}

		BD_ASSERT(packetLength < BD_MAX_DATAGRAM_SIZE, "overflow.");
	}

	return packetLength;
}

void bdDTLSAssociation::reset()
{
	m_initResends = 0;
	m_cookieResends = 0;

	m_seqNum = 0;

	m_localTag = static_cast<bdUInt16>(bdTrulyRandom::getInstance()->getRandomUInt());

	m_lastReceived.start();

	m_peerTag = 0;

	m_incomingSeqNums = bdSequenceNumberStore(-1);
}

bdBool bdDTLSAssociation::calculateSharedKey(const bdUByte8 *const pubKey,
											 const bdUInt keylen,
											 const bdSecurityID &secID)
{
	const bdUInt securityKeySize = BD_SECURITY_KEY_LENGTH;

	// get the security key
	bdSecurityKey securityKey;
	if(!m_keyMap->get(secID, securityKey))
	{
		BD_WARN(BD_LOG_LEVEL, "secid not in keymap.");
		return false;
	}

#ifdef BD_NO_SHARED_SECRET_CALCULATION
	bdUByte8 sharedSecret[securityKeySize];
	bdMemcpy(sharedSecret, securityKey.ab, securityKeySize);

#else //  BD_NO_SHARED_SECRET_CALCULATION

	const bdUInt ECCKeySize = BD_ECC_SHARED_SECRET_SIZE;

	const bdUInt sharedSecretLength = ECCKeySize + securityKeySize;
	bdUByte8 sharedSecret[sharedSecretLength];

	// generate the shared key
	if(!m_ECCKey->generateSharedSecret( pubKey, keylen, sharedSecret , sharedSecretLength))
	{
		BD_WARN(BD_LOG_LEVEL, "couldn't generate shared key.");
		return false;
	}

	bdMemcpy(sharedSecret+ECCKeySize, securityKey.ab, securityKeySize);
#endif //  BD_NO_SHARED_SECRET_CALCULATION

	// generate the shared key. this is a hash of both the dh key and 
	// the security key.
	bdUInt sharedKeySize = BD_SHARED_KEY_SIZE;
	bdHashTiger192 tigerHash;
	if(!tigerHash.hash(sharedSecret, sizeof(sharedSecret), m_sharedKey, sharedKeySize))
	{
		BD_WARN(BD_LOG_LEVEL, "unable to create hash.");
		return false;
	}

	// Once we have the shared key we can intialise the 3Des cypher
	m_cypher.init(m_sharedKey, BD_SHARED_KEY_SIZE);

	bdUInt32 sharedKeyAsUInt[sizeof(m_sharedKey) / sizeof(bdUInt32)];
	bdMemcpy(sharedKeyAsUInt, m_sharedKey, sizeof(sharedKeyAsUInt));
	BD_INFO(BD_LOG_LEVEL, "Shared key: %08x %08x %08x %08x %08x %08x", 
												sharedKeyAsUInt[0],
												sharedKeyAsUInt[1],
												sharedKeyAsUInt[2],
												sharedKeyAsUInt[3],
												sharedKeyAsUInt[4],
												sharedKeyAsUInt[5]);

	return true;
}

bdBool bdDTLSAssociation::verify(const bdDTLSData &dataPacket)
{
	bdSequenceNumber seqNum;
	bdBool result = false;

	if(dataPacket.getVtag() == m_localTag)
	{
		const bdSequenceNumber last = m_incomingSeqNums.getLastSequenceNumber();
		const bdSequenceNumber newSeqNum(last, dataPacket.getCounter(), sizeof(bdUInt16) << 3);
		
		switch(m_incomingSeqNums.check(newSeqNum))
		{
			case bdSequenceNumberStore::BD_SN_VALID_MUCH_LARGER:
				BD_INFO(BD_LOG_LEVEL, "New sequence number (%u) much larger than last (%u).", 
														newSeqNum.getValue(), 
														last.getValue());
				// sequence number is much large which is ok so continue into next case statement
			case bdSequenceNumberStore::BD_SN_VALID_SMALLER:	
			case bdSequenceNumberStore::BD_SN_VALID_LARGER:
				seqNum = newSeqNum;
				result = true;
				break;
			case bdSequenceNumberStore::BD_SN_INVALID_SMALLER:	
			case bdSequenceNumberStore::BD_SN_INVALID_DUPLICATE: 
			default:
				BD_WARN(BD_LOG_LEVEL, "invalid seq num.");
				result = false;
				break;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "vtag not the same as local tag.");
	}

	return result;
}

bdBool bdDTLSAssociation::checkCookieValidity(const bdAddr &peerAddr,
											  const bdDTLSCookieEcho &cookiePacket) const
{
	const bdUInt age = bdPlatformTiming::getLoResElapsedTime(cookiePacket.getCookie().getTimestamp(), bdPlatformTiming::getLoResTimeStamp());

	if (age < BD_SOCKET_COOKIE_LIFE)
	{
		if (cookiePacket.getCookie().getPeerAddr() == peerAddr)
		{
			bdHMacSHA1 hmac(m_cookieKey.getData(), BD_SOCKET_COOKIE_KEY_SIZE);
			const bdBool verifyHMAC = cookiePacket.getCookie().verify(hmac);

			if(!verifyHMAC)
			{
				BD_WARN(BD_LOG_LEVEL, "HMac Verification failed.");
			}

			return verifyHMAC;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Wrong peer address");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cookie expired. Age: %us", age);
	}
	
	return false;
}

