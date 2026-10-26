// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdNATTravClient.h>

#include <bdCore/bdCrypto/bdHMacSHA1.h>
#include <bdCore/bdSocket/bdCommonAddrInfo.h>

#include <bdSocket/bdNAT/bdPacket/bdNATTraversalPacket.h>
#include <bdSocket/bdNAT/bdNATTravListener.h>
#include <bdSocket/bdServiceBandwidthArbitrator.h>

#define BD_NAT_TRAV_MAX_STAGE_1_TRIES	(3)
#define BD_NAT_TRAV_MAX_STAGE_2_TRIES	(4)
#define BD_NAT_TRAV_MAX_STAGE_3_TRIES	(3)
#define BD_NAT_TRAV_TIMEOUT				(.9f)

#define BD_NAT_TRAV_KEEP_ALIVE_TIMEOUT	(15.f)

/// Maximum time in seconds any one request is allowed to be processed for.
#define BD_NAT_TRAV_STARVATION_TIMEOUT  (20.f)

#define BD_LOG_LEVEL "bdSocket/nat"


bdNATTravClient::bdNATTravClient()
: m_status(BD_NAT_TRAV_UNINITIALIZED)
{
}

bdNATTravClient::~bdNATTravClient()
{
	bdHashMap<bdUInt32, bdNATTravClientData>::Iterator it = m_callbacks.getIterator();
	while(it)
	{
		bdNATTravClientData data = m_callbacks.getValue(it);
		const bdUInt tmpSize = BD_COMMON_ADDR_INFO_SIZE;
		bdNChar8 tmp[tmpSize];

		bdCommonAddrInfo::getBriefInfo(data.m_remote, tmp, tmpSize);

		BD_INFO(BD_LOG_LEVEL, "NAT traversal to %s failed.", tmp);

		data.callOnNATAddrDiscoveryFailed(data.m_remote);

		m_callbacks.next(it);
	}
	m_callbacks.releaseIterator(it);
	m_callbacks.clear();

}

bdBool bdNATTravClient::init(bdDefaultSocket *socket,
							 bdServiceBandwidthArbitrator *bandArb,
							 bdCommonAddrRef localCommonAddr)
{
	bdBool result = true;

	if(BD_NAT_TRAV_UNINITIALIZED != m_status)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "Cannot call init multiple times.");
	}
	else if(BD_NULL == socket)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid socket pointer is required by this class");
	}
	else if (BD_NULL == bandArb)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid service bandwidth arbitrator pointer is required by this class");
	}
	else if (localCommonAddr.isNull())
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid local common addr ref is required by this class");
	}
	else
	{
		m_socket = socket;
		m_bandArb = bandArb;
		m_localCommonAddr = localCommonAddr;
		result = sendKeepAlive();
		bdTrulyRandom::getInstance()->getRandomUByte8(m_secretKey, sizeof(m_secretKey));
		m_status = BD_NAT_TRAV_INITIALIZED;

	}
	return result;
}

bdBool bdNATTravClient::connect(bdCommonAddrRef remote,
								bdNATTravListener *listener,
								bdBool throttle)
{
	bdBool success = true;

	if(BD_NAT_TRAV_UNINITIALIZED == m_status)
	{
		success = false;
		BD_WARN(BD_LOG_LEVEL, "Cannot call connect until this class has been initialized.");
	}
	else
	{
		const bdUInt32 identifier = remote->getHash();
		if (!m_callbacks.containsKey(identifier))
		{
			bdNATTravClientData data(m_localCommonAddr, remote, listener);

			data.m_throttled = throttle;
			data.m_age.start();

			bdNChar8 destAddr[BD_COMMON_ADDR_INFO_SIZE];
			bdCommonAddrInfo::getInfo(remote, destAddr, BD_COMMON_ADDR_INFO_SIZE);
			BD_INFO(BD_LOG_LEVEL, "Starting NAT trav to %s", destAddr);

			if(false == sendStage1(data))
			{
				BD_WARN(BD_LOG_LEVEL, "Initial stage 1 send failed. Continuing anyway");
			}

			// put data in map. (data was changed by previous sendStage1 function)
			if(!m_callbacks.put(identifier, data))
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to put address in map.");
				bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
				bdCommonAddrInfo::getBriefInfo(remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
				BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

				success = false;
			}
			else
			{
				m_status = BD_NAT_TRAV_RUNNING;
			}
		}
		else
		{
			bdHashMap<bdUInt32, bdNATTravClientData>::Iterator it = m_callbacks.getIterator(identifier);
			// There's already a connect request pending...
			bdNATTravClientData &data = m_callbacks.getValue(it);
			if (data.m_secondaryListener == BD_NULL)
			{
				data.m_secondaryListener = listener;
			}
			else
			{
				bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
				bdCommonAddrInfo::getBriefInfo(remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);

				BD_WARN(BD_LOG_LEVEL, "Third connect request to %s. Ignoring.", commonAddrInfo);
				success = false;
			}
			m_callbacks.releaseIterator(it);
		}
	}

	return success;
}

void bdNATTravClient::cancelConnect(bdCommonAddrRef remote)
{
	if(remote.notNull())
	{
		const bdUInt32 identifier = remote->getHash();
		bdNATTravClientData callback;
		if (m_callbacks.remove(identifier, callback))
		{
			callback.callOnNATAddrDiscoveryFailed(remote);
		}
		else
		{
			bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
			bdCommonAddrInfo::getBriefInfo(remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
			BD_WARN(BD_LOG_LEVEL, "Connect canceled to unknown remote host %s. Ignoring.", commonAddrInfo);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Connect canceled to null common addr ref.");
	}
}


void bdNATTravClient::receiveFrom(const bdAddr &addr, bdNATTraversalPacket &packet)
{
	bdUInt tmpInt;
	const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
	bdUByte8 buffer[bufferSize];

	switch(packet.getType())
	{
		// requesting us to send the introduction packet onto the destination.
		case BD_NAT_TRAV_INTRO_REQ:
		{
			BD_WARN(BD_LOG_LEVEL, "Received server packet in client code.");
		}
		break;
		// requesting us to send the packet back to the src address.
		case BD_NAT_TRAV_INTRO_FORWARD:
		{
			const bdAddr dest = packet.getAddrSrc();
			packet.setType(BD_NAT_TRAV_INTRO_REPLY);

			const bdBool serialized = packet.serialize(buffer, bufferSize, 0, tmpInt);
			BD_ASSERT(serialized, "Cannot serialze the packet.");

			/// \todo We can probably perform a much more accurate size test here
			///  since we should know the serialized size of bdNATTraversalPacket
			const bdInt sendResult = m_socket->sendTo(dest, buffer, tmpInt);
			const bdUWord dstSize = BD_ADDR_STRING_SIZE;
			bdNChar8 dst[dstSize];
			dest.toString(dst, dstSize);
			if(sendResult > 0)
			{
				BD_INFO(BD_LOG_LEVEL, "sent INTRO REPLY to %s", dst);
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to send INTRO REPLY to %s. Socket error %i", dst, sendResult);
			}
		}
		break;
		// reply to our original introduction request
		case BD_NAT_TRAV_INTRO_REPLY:
		{
			BD_INFO(BD_LOG_LEVEL, "Received NAT Trav reply.");

			const bdUInt32 identifier = packet.getIdentifier();
			// check hmac
			bdUInt hmacLength = BD_NAT_TRAV_HMAC_SIZE;
			bdUByte8 hmacBuffer[BD_NAT_TRAV_HMAC_SIZE];

			const bdBool hmacRes = doHMac(identifier, packet.getAddrSrc(), packet.getAddrDest(), hmacBuffer);

			if(hmacRes && bdMemcmp(hmacBuffer, packet.getHMAC(), hmacLength) == 0)
			{
				const bdUWord strSize = BD_ADDR_STRING_SIZE;
				bdNChar8 src[strSize];
				addr.toString(src, strSize);
				bdNChar8 lookup[strSize];
				packet.getAddrDest().toString(lookup, strSize);

				BD_INFO(BD_LOG_LEVEL, "discovered addr for %s: %s", lookup, src);

				// look up addr and do callback.
				bdNATTravClientData data;
				if(m_callbacks.remove(identifier, data))
				{
					data.callOnNATAddrDiscovery(data.m_remote, addr);
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL, "Failed to find identifier in callback table. "
											"This is ok if we have already received a reply"
											" for this search.");
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Packet was tampered with, discarding.");
			}
		}
		break;
		case BD_NAT_TRAV_KEEP_ALIVE:
		{
			// not required to do anything. ignore.
		}
		break;
		// requesting us to send the packet back to the src address.
		case BD_NAT_TRAV_INTRO:
		{

			// 2 phase process
			//  - lookup our local common addr
			//  - check to see if the packet identifier corresponds to
			//    our local common addr hash.
			if(packet.getIdentifier() == m_localCommonAddr->getHash())
			{
				// identifier is the same as our local common addr hash
				// so the discovery packet was destined for us.

				const bdAddr dest = addr;
				packet.setType(BD_NAT_TRAV_INTRO_REPLY);

				const bdBool serialized = packet.serialize(buffer, bufferSize, 0, tmpInt);
				if(false == serialized)
				{
					BD_ASSERT(false, "Cannot serialze the packet.");
				}
				else
				{
					const bdInt sendResult = m_socket->sendTo(dest, buffer, tmpInt);

					const bdUWord dstSize = BD_ADDR_STRING_SIZE;
					bdNChar8 dst[dstSize];
					dest.toString(dst, dstSize);
					if(sendResult > 0)
					{
						BD_INFO(BD_LOG_LEVEL, "sent INTRO REPLY to %s", dst);
					}
					else
					{
						BD_ERR(BD_LOG_LEVEL, "Failed to send INTRO REPLY to %s. Socket error %i", dst, sendResult);
					}
				}
			}
			else
			{
				// identifier was different so we just happen to have the same
				// local ip address as someone else but we aren't who the requester
				// was looking for.
				const bdUWord srcSize = BD_ADDR_STRING_SIZE;
				bdNChar8 src[srcSize];
				addr.toString(src, srcSize);
				BD_INFO(BD_LOG_LEVEL, "ignored request from %s", src);
			}
		}
		break;
	}
}

void bdNATTravClient::pump()
{
	if(BD_NAT_TRAV_UNINITIALIZED != m_status)
	{
		// Do keep alive
		if(m_keepAliveTimer.getElapsedTimeInSeconds() > BD_NAT_TRAV_KEEP_ALIVE_TIMEOUT)
		{
			sendKeepAlive();
		}

		// dead traversal attempts that need to be removed
		bdQueue<bdUInt32> toRemove;

		// Resend any NAT traversal packets that have timed out without a reply.
		bdHashMap<bdUInt32, bdNATTravClientData, bdAddrHash>::Iterator it = m_callbacks.getIterator();
		while(it)
		{
			const bdUInt32 &identifier = m_callbacks.getKey(it);
			bdNATTravClientData &data = m_callbacks.getValue(it);

			if (data.m_throttled && data.m_age.getElapsedTimeInSeconds() > BD_NAT_TRAV_STARVATION_TIMEOUT)
			{
				const bdUInt bufferSize = 100;
				bdNChar8 buffer[bufferSize];
				data.m_remote->getPublicAddr().toString(buffer, bufferSize);
				BD_WARN(BD_LOG_LEVEL, "Request to %s has been pending for too long (%f seconds). "
									"Allocated bandwidth inconsistent with request rate.", buffer , data.m_age.getElapsedTimeInSeconds());
				data.callOnNATAddrDiscoveryFailed(data.m_remote);
				toRemove.enqueue(identifier);
			}
			else if(data.m_lastSent.getElapsedTimeInSeconds() > BD_NAT_TRAV_TIMEOUT)
			{
				BD_INFO(BD_LOG_LEVEL, "%f seconds since last send, retrying.." , data.m_lastSent.getElapsedTimeInSeconds());
				switch(data.m_state)
				{
					case bdNATTravClientData::BD_NTCDS_STAGE_1:
					{
						// check how many times we have tried NAT trav in state 1.
						if(data.m_tries > BD_NAT_TRAV_MAX_STAGE_1_TRIES)
						{
							// we've tried stage 1 enough, if we have a valid public
							// address and know an introducer then move onto stage 2.
							if(data.m_remote->getPublicAddr().getAddress().isValid() && !m_introducers.isEmpty())
							{
								BD_INFO(BD_LOG_LEVEL, "Stage 1 failed. Starting stage 2.");
								bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
								bdCommonAddrInfo::getBriefInfo(data.m_remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
								BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

								data.m_tries = 0;
								data.m_state = bdNATTravClientData::BD_NTCDS_STAGE_2;

								sendStage2(data);
							}
							else
							{
								// failed
								BD_WARN(BD_LOG_LEVEL, "Address lookup failed. (Stage 1 failed. No public address so finished.)");
								bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
								bdCommonAddrInfo::getBriefInfo(data.m_remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
								BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

								data.callOnNATAddrDiscoveryFailed(data.m_remote);
								toRemove.enqueue(identifier);
							}
						}
						else
						{
							BD_INFO(BD_LOG_LEVEL, "Request timed out. Retrying. (stage 1)");
							bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
							bdCommonAddrInfo::getBriefInfo(data.m_remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
							BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

							sendStage1(data);
						}
					}
					break;
					case bdNATTravClientData::BD_NTCDS_STAGE_2:
					{
						if(data.m_tries++ > BD_NAT_TRAV_MAX_STAGE_2_TRIES)
						{
							BD_INFO(BD_LOG_LEVEL, "Stage 2 failed. Starting stage 3.");
							bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
							bdCommonAddrInfo::getBriefInfo(data.m_remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
							BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

							data.m_tries = 0;
							data.m_state = bdNATTravClientData::BD_NTCDS_STAGE_3;

							sendStage3(data);
						}
						else
						{
							BD_INFO(BD_LOG_LEVEL, "Request timed out. Retrying. (stage 2)");
							bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
							bdCommonAddrInfo::getBriefInfo(data.m_remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
							BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

							sendStage2(data);
						}
					}
					break;
					case bdNATTravClientData::BD_NTCDS_STAGE_3:
					{
						// check how many times we have tried NAT trav in state 3.
						if(data.m_tries > BD_NAT_TRAV_MAX_STAGE_3_TRIES)
						{
							// failed
							BD_INFO(BD_LOG_LEVEL, "NAT traversal failed. (Stage 3 failed)");
							bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
							bdCommonAddrInfo::getBriefInfo(data.m_remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
							BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

							data.callOnNATAddrDiscoveryFailed(data.m_remote);
							toRemove.enqueue(identifier);
						}
						else
						{
							BD_INFO(BD_LOG_LEVEL, "Request timed out. Retrying. (stage 3)");
							bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
							bdCommonAddrInfo::getBriefInfo(data.m_remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
							BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

							sendStage3(data);
						}
					}
					break;
					default:
					{
						BD_ASSERT(false, "Invalid state.");
					}
				}
			}
			m_callbacks.next(it);
		}
		m_callbacks.releaseIterator(it);

		while(!toRemove.isEmpty())
		{
			m_callbacks.remove(toRemove.peek());

			toRemove.dequeue();
		}
	}
	if(0 == m_callbacks.getSize())
	{
		// If there are no more outstanding requests we will revert to a
		// status of initialized
		m_status = BD_NAT_TRAV_INITIALIZED;
	}
}

bdBool bdNATTravClient::quit()
{
	bdBool result = false;
	if(BD_NAT_TRAV_INITIALIZED == m_status)
	{
		m_socket = BD_NULL;
		m_bandArb = BD_NULL;
		m_status = BD_NAT_TRAV_UNINITIALIZED;
		m_introducers.clear();
		result = true;
	}
	else if(BD_NAT_TRAV_RUNNING == m_status)
	{
		// we will stop all active NAT traversal requests
		bdHashMap<bdUInt32, bdNATTravClientData>::Iterator it = m_callbacks.getIterator();
		while(it)
		{
			bdNATTravClientData data = m_callbacks.getValue(it);

			const bdUInt tmpSize = BD_COMMON_ADDR_INFO_SIZE;
			bdNChar8 tmp[tmpSize];
			bdCommonAddrInfo::getBriefInfo(data.m_remote, tmp, tmpSize);

			BD_INFO(BD_LOG_LEVEL, "NAT traversal to %s failed.", tmp);

			m_callbacks.next(it);
		}
		m_callbacks.releaseIterator(it);
		m_introducers.clear();

		m_socket = BD_NULL;
		m_bandArb = BD_NULL;
		m_status = BD_NAT_TRAV_UNINITIALIZED;
		result = true;
	}
	return result;
}

bdBool bdNATTravClient::acceptPacket(bdDefaultSocket *, bdAddr addr, void *data, const bdUInt size, const bdUByte8 type)
{
	bdBool accepted = false;

	// do not accept packets when this class is uninitialized.
	if(BD_NAT_TRAV_UNINITIALIZED != m_status)
	{
		if(type >= BD_NAT_TRAV_TYPE_MIN && type <= BD_NAT_TRAV_TYPE_MAX)
		{
			bdNATTraversalPacket packet;
			bdUInt tmpInt;

			if (packet.deserialize(data, size, 0, tmpInt) &&
				packet.getType() >= BD_NAT_TRAV_TYPE_MIN &&
				packet.getType() <= BD_NAT_TRAV_TYPE_MAX)
			{
				receiveFrom(addr, packet);

				accepted = true;
			}
		}
	}

	return accepted;
}


// protected:
bdBool bdNATTravClient::sendKeepAlive()
{
	bdBool success = true;

	m_keepAliveTimer.start();

	if(!m_introducers.isEmpty())
	{
		BD_INFO(BD_LOG_LEVEL, "Sending keep alive to introducer(s).");

		bdNATTraversalPacket packet(BD_NAT_TRAV_KEEP_ALIVE, 0, bdAddr(), bdAddr());

		const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 buffer[bufferSize];
		bdUInt tmpInt;

		if (!packet.serialize(buffer, bufferSize, 0, tmpInt))
		{
			return false;
		}

		for(bdUInt i = 0; i < m_introducers.getSize(); i++)
		{
			if (m_socket->sendTo(m_introducers[i], buffer, tmpInt) != static_cast<bdInt>(tmpInt))
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to send. Invalid socket?");
				success = false;
			}
		}
	}

	return success;
}

bdBool bdNATTravClient::sendStage1(bdNATTravClientData &data)
{
	// Update the last sent timestamp, before checking the throttling
	// Otherwise the last sent timer may never get set, and the NAT trav
	// process will fail to retry.
	data.m_lastSent.start();

	if (data.m_throttled && !m_bandArb->allowedSend(BD_UDP_IP_OVERHEAD))
	{
		return false;
	}

	data.m_tries++;

	bdCommonAddrRef local = data.m_local;
	bdCommonAddrRef remote = data.m_remote;

	const bdUInt32 identifier = remote->getHash();
	const bdAddr src = local->getLocalAddrByIndex(0);

	bdArray<bdAddr> destinations(remote->getLocalAddrs());
	if(remote->getPublicAddr().getAddress().isValid())
	{
		destinations.pushBack(remote->getPublicAddr());
	}

	bdBool sent = true;

	for(bdUInt i = 0; i < destinations.getSize(); i++)
	{
		const bdAddr dst = destinations[i];

		bdNATTraversalPacket packet(BD_NAT_TRAV_INTRO, identifier, src, dst);

		// add a hmac of the contents
		bdUInt hmacLength = BD_NAT_TRAV_HMAC_SIZE;
		bdUByte8 hmacTemp[BD_NAT_TRAV_HMAC_SIZE];

		if(doHMac(identifier, packet.getAddrSrc(), packet.getAddrDest(), hmacTemp))
		{
			packet.setHMAC(hmacTemp, hmacLength);

			const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
			bdUByte8 buffer[bufferSize];
			bdUInt serializedSize = 0;

			BD_ASSERT(packet.serialize(buffer, bufferSize, 0, serializedSize), "Failed to serialize the packet.");

			if (data.m_throttled)
			{
				m_bandArb->send(packet.getSize() + BD_UDP_IP_OVERHEAD);
			}

			if (m_socket->sendTo(dst, buffer, serializedSize) != static_cast<bdInt>(serializedSize))
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to send. Invalid socket?");
				sent = false;
			}
			else
			{
				const bdUWord strSize = BD_ADDR_STRING_SIZE;
				bdNChar8 dstStr[strSize];
				dst.toString(dstStr, strSize);

				BD_INFO(BD_LOG_LEVEL, "sent INTRO REQ to %s", dstStr);
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to do hmac.");
		}
	}

	return sent;
}

bdBool bdNATTravClient::sendStage2(bdNATTravClientData &data)
{
	BD_ASSERT(!m_introducers.isEmpty(), "No introducers specified!");

	if (data.m_throttled && !m_bandArb->allowedSend(BD_UDP_IP_OVERHEAD))
	{
		return false;
	}

	data.m_lastSent.start();
	data.m_tries++;

	bdCommonAddrRef local = data.m_local;
	bdCommonAddrRef remote = data.m_remote;

	bdBool sent = true;

	const bdUInt32 identifier = remote->getHash();
	const bdAddr src = local->getPublicAddr();
	const bdAddr dst = remote->getPublicAddr();

	bdNATTraversalPacket packet(BD_NAT_TRAV_INTRO_REQ, identifier, src, dst);

	// add a hmac of the contents
	bdUInt hmacLength = BD_NAT_TRAV_HMAC_SIZE;
	bdUByte8 hmacTemp[BD_NAT_TRAV_HMAC_SIZE];
	if (!doHMac(identifier, packet.getAddrSrc(), packet.getAddrDest(), hmacTemp))
	{
		return false;
	}
	packet.setHMAC(hmacTemp, hmacLength);

	const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
	bdUByte8 buffer[bufferSize];
	bdUInt serializedSize = 0;

	BD_ASSERT(packet.serialize(buffer, bufferSize, 0, serializedSize), "Failed to serialize the packet.");

	if (data.m_throttled)
	{
		m_bandArb->send(packet.getSize() + BD_UDP_IP_OVERHEAD);
	}

	if (m_socket->sendTo(m_introducers[0], buffer, serializedSize) != static_cast<bdInt>(serializedSize))
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to send. Invalid socket?");
		sent = false;
	}
	else
	{
		const bdUWord strSize = BD_ADDR_STRING_SIZE;
		bdNChar8 intr[strSize];
		m_introducers[0].toString(intr, strSize);
		bdNChar8 dstString[strSize];
		packet.getAddrDest().toString(dstString, strSize);
		bdNChar8 srcString[strSize];
		packet.getAddrSrc().toString(srcString, strSize);

		BD_INFO(BD_LOG_LEVEL, "sent INTRO NAT req to %s for %s from %s", intr, dstString, srcString);
	}

	return sent;
}

bdBool bdNATTravClient::sendStage3(bdNATTravClientData &data)
{
	return sendStage1(data);
}

bdBool bdNATTravClient::doHMac(const bdUInt32 identifier,
							   const bdAddr &src,
							   const bdAddr &dest,
							   bdUByte8 data[BD_NAT_TRAV_HMAC_SIZE]) const
{
	bdUInt offset = 0;
	bdUByte8 addrBuffer[sizeof(bdAddr)];

	const bdInt identSize = sizeof(identifier);
	bdUByte8 ident[identSize];

	if (!bdBytePacker::appendBasicType(ident, identSize, 0, offset, identifier))
	{
		return false;
	}

	bdHMacSHA1 hmac(m_secretKey, sizeof(m_secretKey));
	if (!hmac.process(ident, identSize))
	{
		return false;
	}

	offset = 0;

	if (!src.serialize(addrBuffer, sizeof(addrBuffer), offset, offset))
	{
		return false;
	}
	if (!hmac.process(addrBuffer, offset))
	{
		return false;
	}
	offset = 0;

	if (!dest.serialize(addrBuffer, sizeof(addrBuffer), offset, offset))
	{
		return false;
	}
	if (!hmac.process(addrBuffer, offset))
	{
		return false;
	}

	bdUInt hmacLength = BD_NAT_TRAV_HMAC_SIZE;
	return hmac.getData(data, hmacLength);
}

bdBool bdNATTravClient::setupIntroducers(const bdArray<bdAddr> &introducers)
{
	bdBool result = false;
	if(BD_NAT_TRAV_UNINITIALIZED != m_status)
	{
		if(m_introducers.getSize() > 0)
		{
			BD_WARN(BD_LOG_LEVEL, "Overwriting existing introducer entries.");
		}

		m_introducers = introducers;
		result = true;
	}
	return result;
}

bdBool bdNATTravClient::updateLocalCommonAddr(const bdCommonAddrRef newLocalCommonAddr)
{
	bdBool result = false;
	if(BD_NAT_TRAV_UNINITIALIZED != m_status)
	{
		if(newLocalCommonAddr.isNull())
		{
			BD_ERR(BD_LOG_LEVEL, "Null common addr cannot be used to setup NAT trav client");
		}
		else
		{
			bdHashMap<bdUInt32, bdNATTravClientData>::Iterator it = m_callbacks.getIterator();
			while(it)
			{
				bdNATTravClientData data = m_callbacks.getValue(it);
				data.m_local = newLocalCommonAddr;
				m_callbacks.next(it);
			}
			m_localCommonAddr = newLocalCommonAddr;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Updating local common addr has no effect when class is uninitialized");
	}
	return result;
}

bdBool bdNATTravClient::isConnectInProgress(const bdCommonAddrRef remote) const
{
	const bdUInt32 identifier = remote->getHash();
	return m_callbacks.containsKey(identifier);
}
