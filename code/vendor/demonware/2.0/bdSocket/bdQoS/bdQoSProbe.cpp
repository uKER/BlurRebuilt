// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdQoS/bdQoSProbe.h>

#include <bdSocket/bdQoS/bdQoS.h>
#include <bdSocket/bdQoS/bdQoSProbeListener.h>
#include <bdSocket/bdQoS/bdPacket/bdQoSRequestPacket.h>
#include <bdSocket/bdServiceBandwidthArbitrator.h>
#include <bdSocket/bdNAT/bdNATTravClient.h>
#include <bdCore/bdSocket/bdCommonAddrInfo.h>

#define BD_QOS_SEND_INTERVAL (0.2f)

#define BD_QOS_PROBE_TIMEOUT (0.9f)
#define BD_QOS_PROBE_RETRIES (4u)

#define BD_LOG_LEVEL "bdSocket/qos"

bdQoSProbe::bdQoSProbe()
: m_status(BD_QOS_PROBE_UNINITIALIZED)
{
}

bdQoSProbe::bdQoSProbe(bdDefaultSocket *socket, bdNATTravClient *natTrav, bdServiceBandwidthArbitrator *bandArb)
: m_maxBandwidth(BD_QOS_MAX_BANDWIDTH_DEFAULT),
  m_socket(socket),
  m_natTrav(natTrav),
  m_bandArb(bandArb),
  m_lastProbeId(0),
  m_secid(0),
  m_listenState(BD_QOS_OFF),
  m_status(BD_QOS_PROBE_INITIALIZED)
{
	m_probingTimer.start();
	m_bandArb->reset();
	// Setup an initial slice quota..
	m_bandArb->addSliceQuota(static_cast<bdUInt>((m_maxBandwidth/8) * BD_QOS_SEND_INTERVAL));

}

bdQoSProbe::~bdQoSProbe()
{
	m_bandArb = BD_NULL;
	m_natTrav = BD_NULL;
	m_socket = BD_NULL;
}

bdBool bdQoSProbe::init(bdDefaultSocket *socket, bdNATTravClient *natTrav, bdServiceBandwidthArbitrator *bandArb)
{
	bdBool result = true;

	if(BD_QOS_PROBE_UNINITIALIZED != m_status)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "Cannot call init multiple times.");
	}
	else if(BD_NULL == socket)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid socket pointer is required by this class");
	}
	else if(BD_NULL == natTrav)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid NAT trav pointer is required by this class");
	}
	else if (BD_NULL == bandArb)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid service bandwidth arbitrator pointer is required by this class");
	}
	else
	{

		m_maxBandwidth = BD_QOS_MAX_BANDWIDTH_DEFAULT;
		m_socket = socket;
		m_natTrav = natTrav;
		m_bandArb = bandArb;
		m_lastProbeId = 0;
		m_secid = 0;
		m_listenState = BD_QOS_OFF;

		m_probingTimer.start();
		m_bandArb->reset();
		// Setup an initial slice quota..
		m_bandArb->addSliceQuota(static_cast<bdUInt>((m_maxBandwidth/8) * BD_QOS_SEND_INTERVAL));

		m_status = BD_QOS_PROBE_INITIALIZED;
	}

	return result;
}

bdBool bdQoSProbe::quit()
{
	bdBool result = false;
	if(BD_QOS_PROBE_UNINITIALIZED != m_status)
	{
		cancelProbes();
		m_status = BD_QOS_PROBE_UNINITIALIZED;
		m_socket = BD_NULL;
		m_natTrav = BD_NULL;
		m_bandArb = BD_NULL;

		result = true;
	}
	return result;
}

void bdQoSProbe::setMaxBandwidth(const bdUInt maxBandwidth)
{
	if(BD_QOS_PROBE_UNINITIALIZED == m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Max bandwidth will be reset when this class is initialized.");
	}
	m_maxBandwidth = maxBandwidth;
}

bdBool bdQoSProbe::probe(bdQoSRemoteAddr &addr, bdQoSProbeListener *listener)
{
	return probe(addr.m_addr, addr.m_id, addr.m_key, listener);
}

bdBool bdQoSProbe::probe(const bdArray<bdQoSRemoteAddr> &addrs, bdQoSProbeListener *listener)
{

	bdBool result = true;
	for (bdUInt i=0; result && i < addrs.getSize(); i++)
	{
		result = result && probe(addrs[i].m_addr, addrs[i].m_id, addrs[i].m_key, listener);
	}
	return result;
}

bdBool bdQoSProbe::probe(bdCommonAddrRef addr,
						 const bdSecurityID &id,
						 const bdSecurityKey &key,
						 bdQoSProbeListener *listener)
{

	bdBool result = false;
	if(BD_QOS_PROBE_UNINITIALIZED != m_status)
	{
		bdQoSProbeEntryWrapper entry;
		entry.m_addr = addr;
		entry.m_id = id;
		entry.m_key = key;
		entry.m_listener = listener;
		if (!m_probesResolving.containsKey(addr))
		{
			m_probesResolving.put(addr, bdArray<bdQoSProbeEntryWrapper>(1, entry));
		}
		else
		{
			BD_INFO(BD_LOG_LEVEL, "Got multiple probes for the same address.\n");
			bdHashMap<bdCommonAddrRef, bdArray<bdQoSProbeEntryWrapper> >::Iterator it = m_probesResolving.getIterator(addr);
			m_probesResolving.getValue(it).pushBack(&entry, 1);
			m_probesResolving.releaseIterator(it);
		}

		result = m_natTrav->connect(addr, this, true);
	}
	return result;

}

void bdQoSProbe::onNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr &realAddr)
{
	if (m_probesResolving.containsKey(remote))
	{
		bdArray<bdQoSProbeEntryWrapper> entries;
		m_probesResolving.get(remote, entries);

		for (bdUInt i=0; i < entries.getSize(); i++)
		{
			entries[i].m_realAddr = realAddr;
			m_probesProbbing.put(m_lastProbeId++, entries[i]);
		}

		m_probesResolving.remove(remote);
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "Received NAT Traversal success for unrecognized address.");
	}
}

void bdQoSProbe::onNATAddrDiscoveryFailed(bdCommonAddrRef remote)
{
	if (m_probesResolving.containsKey(remote))
	{
		bdNChar8 addrStr[BD_COMMON_ADDR_INFO_SIZE];
		bdCommonAddrInfo::getBriefInfo(remote, addrStr, sizeof(addrStr));

		BD_INFO(BD_LOG_LEVEL, " NAT addr discovery failed to %s . QoS has therefore failed" ,addrStr);
		bdArray<bdQoSProbeEntryWrapper> entries;
		m_probesResolving.get(remote, entries);

		for (bdUInt i=0; i < entries.getSize(); i++)
		{
			entries[i].m_listener->onQoSProbeFail(entries[i].m_addr);
		}
		m_probesResolving.remove(remote);
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "Received NAT Traversal failure for unrecognized address.");
	}
}

bdBool bdQoSProbe::acceptPacket(bdDefaultSocket *, bdAddr addr, void *data, const bdUInt size, const bdUByte8 type)
{
	bdBool result = false;

	if ((BD_QOS_PROBE_INITIALIZED == m_status) &&
		(type >= BD_QOS_TYPE_MIN) &&
		(type <= BD_QOS_TYPE_MAX))
	{
		const bdUWord srcSize = BD_ADDR_STRING_SIZE;
		bdNChar8 src[srcSize];
		addr.toString(src, srcSize);

		bdUInt32 typeAsInt = type;
		BD_INFO(BD_LOG_LEVEL, "Received QoS packet from %s with type %u" , src , typeAsInt);
		switch (type)
		{
			case BD_QOS_REQUEST:
			{
				bdQoSRequestPacket packet;
				bdUInt deserializedSize;
				if (packet.deserialize(data, size, 0, deserializedSize)
					&& deserializedSize == size)
				{
					result = handleRequest(packet, addr);
				}
			}
			break;
			case BD_QOS_REPLY:
			{
				bdQoSReplyPacket packet;
				bdUInt deserializedSize;
				if (packet.deserialize(data, size, 0, deserializedSize)
					&& deserializedSize == size)
				{
					result = handleReply(packet, addr);
				}
			}
			break;
			default:
			{
				BD_WARN(BD_LOG_LEVEL, "Got packet with invalid type.");
			}
		}
	}

	return result;
}

void bdQoSProbe::pump()
{
	if (BD_QOS_PROBE_INITIALIZED == m_status)
	{
		if (m_probingTimer.getElapsedTimeInSeconds() > BD_QOS_SEND_INTERVAL)
		{
			// Start new time slice and reset bandwidth counter.
			m_bandArb->addSliceQuota(static_cast<bdUInt>((m_maxBandwidth/8) * BD_QOS_SEND_INTERVAL));
			m_probingTimer.start();
		}

		if (m_probesProbbing.getSize() > 0 && m_bandArb->allowedSend(BD_UDP_IP_OVERHEAD))
		{
			bdQueue<bdUInt32> toRemove;
			bdBool stopLoop = false;

			bdHashMap<bdUInt32, bdQoSProbeEntryWrapper>::Iterator it = m_probesProbbing.getIterator();
			while (it && m_bandArb->allowedSend(BD_UDP_IP_OVERHEAD) && !stopLoop)
			{
				bdUInt32 id = m_probesProbbing.getKey(it);
				bdQoSProbeEntryWrapper &entry = m_probesProbbing.getValue(it);

				if (entry.m_retries == 0 || entry.m_lastTry.getElapsedTimeInSeconds() > BD_QOS_PROBE_TIMEOUT)
				{
					if (entry.m_retries >= BD_QOS_PROBE_RETRIES)
					{
						bdNChar8 addrStr[BD_COMMON_ADDR_INFO_SIZE];
						bdCommonAddrInfo::getBriefInfo(entry.m_addr, addrStr, sizeof(addrStr));

						BD_WARN(BD_LOG_LEVEL, " QOS retries exceeded: QoS failed to %s " ,addrStr);

						toRemove.enqueue(id);
					}
					else
					{
						const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
						bdUByte8 buffer[bufferSize];
						bdUInt packetSize = 0;

						const bdUInt32 secID = shrinkSecId(entry.m_id);
						bdQoSRequestPacket packet(secID, id);


						const bdBool serializeOk = packet.serialize(buffer, bufferSize, packetSize, packetSize);
						if(false == serializeOk)
						{
							BD_ASSERT(false, "Failed to put the probe packet into a buffer.");
							// We will stop looping
							stopLoop = true;
						}
						else if (m_bandArb->allowedSend(packetSize + BD_UDP_IP_OVERHEAD))
						{
							bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
							entry.m_realAddr.toString(addrStr, BD_ADDR_STRING_SIZE);

							BD_INFO(BD_LOG_LEVEL, " Sending QOS packet to %s " ,addrStr);

							// TODO: ignore errors for now. Can do better.
							const bdInt sendResult = m_socket->sendTo(entry.m_realAddr, buffer, packetSize);
							if(sendResult < 0)
							{
								BD_WARN(BD_LOG_LEVEL, " Failed to send QOS packet" );
							}
							entry.m_retries++;
							entry.m_lastTry.start();
							m_bandArb->send(packetSize + BD_UDP_IP_OVERHEAD);
						}
						else
						{
							// All probe packets are of the same size, so we won't be able to send anymore.
							stopLoop = true;
							BD_INFO(BD_LOG_LEVEL, " bandwidth arbitration prevents the sending of anymore data right now" );
						}
					}
				}
				m_probesProbbing.next(it);
			}
			m_probesProbbing.releaseIterator(it);

			while (!toRemove.isEmpty())
			{
				bdQoSProbeEntryWrapper entry;
				m_probesProbbing.get(toRemove.peek(), entry);
				entry.m_listener->onQoSProbeFail(entry.m_addr);

				m_probesProbbing.remove(toRemove.peek());
				toRemove.dequeue();
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot pump class before it has been initialized.");
	}
}


bdBool bdQoSProbe::handleRequest(bdQoSRequestPacket &packet, bdAddr &addr)
{
	bdBool result = false;
	if (m_listenState == BD_QOS_OFF)
	{
		// Simply drop the packet and prevent further processing.
		result = true;
	}
	else if (BD_QOS_PROBE_UNINITIALIZED == m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot handle request when in uninitialized state");
		result = false;
	}
	else if((m_bandArb->allowedSend(BD_UDP_IP_OVERHEAD)) &&
			(packet.getSecId() == m_secid) &&
			(m_bandArb->allowedSend(m_replyData.getSerializedSize() + BD_UDP_IP_OVERHEAD)))
	{
		m_replyData.setId(packet.getId());
		m_replyData.setTimestamp(packet.getTimestamp());

		const bdUInt bufferSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 buffer[bufferSize];
		bdUInt dataSize = 0;

		const bdBool serializeResult = m_replyData.serialize(buffer, bufferSize, 0, dataSize);
		if(false == serializeResult)
		{
			BD_ASSERT(false , "Failed to put the reply packet into a buffer.");
		}
		else if (m_bandArb->allowedSend(dataSize + BD_UDP_IP_OVERHEAD))
		{
			// TODO: ignore errors for now. Can do better.
			const bdInt socketSendResult = m_socket->sendTo(addr, buffer, dataSize);
			if(socketSendResult > 0)
			{
				// record the sent data.
				m_bandArb->send(dataSize + BD_UDP_IP_OVERHEAD);
				result = true;
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to send reply.");
			}
		}
	}
	else if(packet.getSecId() != m_secid)
	{
		// the only reason to warn is where a secID mismatch occurs,
		// bandwidth throttling is normal.
		BD_WARN(BD_LOG_LEVEL, "Received a packet with invalid security ID. Dropping.");
	}

	return result;
}

bdBool bdQoSProbe::handleReply(bdQoSReplyPacket &packet, bdAddr &addr)
{
	bdBool result = false;
	if (BD_QOS_PROBE_UNINITIALIZED == m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot handle request when in uninitialized state");
		result = false;
	}
	else if (m_probesProbbing.containsKey(packet.getId()))
	{
		bdQoSProbeEntryWrapper entry;
		m_probesProbbing.get(packet.getId(), entry);

		if (entry.m_realAddr == addr)
		{
			// At this point assume it's a valid probe reply.

			bdQoSProbeInfo info;

			m_probesProbbing.remove(packet.getId());

			info.m_realAddr = entry.m_realAddr;
			info.m_latency = packet.getElapsedTime();
			info.m_addr = entry.m_addr;
			info.m_data = packet.getData();
			info.m_dataSize = packet.getDataSize();

			info.m_disabled = !packet.getEnabled();

			entry.m_listener->onQoSProbeSuccess(info);
			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Received probe reply with spoofed id?");
			result = false;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Received probe reply with invalid id.");
		result = false;
	}

	return result;
}


bdBool bdQoSProbe::listen(const bdSecurityID &secid, bdUByte8 *data, bdUInt dataSize)
{
	if (setData(data, dataSize))
	{
		m_secid = shrinkSecId(secid);
		m_listenState = BD_QOS_ENABLED;
		m_replyData.setEnabled(true);

		BD_INFO(BD_LOG_LEVEL, "Enabled QoS listener.");

		return true;
	}

	return false;
}

bdBool bdQoSProbe::setData(bdUByte8 *data, bdUInt dataSize)
{
	return m_replyData.setData(data, dataSize);
}

void bdQoSProbe::disableListener()
{
	m_listenState = BD_QOS_DISABLED;
	m_replyData.setEnabled(false);
}

void bdQoSProbe::enableListener()
{
	m_listenState = BD_QOS_ENABLED;
	m_replyData.setEnabled(true);
}

void bdQoSProbe::shutdownListener()
{
	m_listenState = BD_QOS_OFF;
}

bdBool bdQoSProbe::isProbing() const
{
	const bdBool probing = (m_probesProbbing.getSize() > 0);
	const bdBool resolving =  (m_probesResolving.getSize() > 0);
	return (probing || resolving);
}

bdUInt bdQoSProbe::shrinkSecId(const bdSecurityID &id) const
{
	bdUInt tmp = 0;
	bdUInt result = 0;

	bdBytePacker::removeBasicType(id.ab, BD_SECURITY_ID_LENGTH, 0, tmp, result);

	return result;
}


void bdQoSProbe::cancelProbes()
{
	if (BD_QOS_PROBE_INITIALIZED == m_status)
	{
		bdArray<bdCommonAddrRef> probesToCancel;

		bdHashMap<bdCommonAddrRef, bdArray<bdQoSProbeEntryWrapper> >::Iterator resit = m_probesResolving.getIterator();
		while (resit)
		{
			probesToCancel.pushBack(m_probesResolving.getKey(resit));
			m_probesResolving.next(resit);
		}
	
		for( unsigned int i = 0; i < probesToCancel.getSize(); ++i )
		{
			m_natTrav->cancelConnect(probesToCancel[i]);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot cancel probes when class is uninitialized.");
	}

	m_probesProbbing.clear();
	m_probesResolving.clear();
}
