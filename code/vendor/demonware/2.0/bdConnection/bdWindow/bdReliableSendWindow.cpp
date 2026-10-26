// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdWindow/bdReliableSendWindow.h>

#include <bdConnection/bdPacket/bdSAckChunk.h>

#define BD_LOG_LEVEL	"bdConnection/windows"

bdReliableSendWindow::bdReliableSendWindow()
: m_nextFree(BD_INITIAL_SEQ_NUMBER),
  m_remoteReceiveWindowCredit(BD_DEFAULT_RECEIVE_WINDOW_CREDIT),
  m_flightSize(0),
  m_partialBytesAcked(0),
  m_slowStartThresh(BD_DEFAULT_RECEIVE_WINDOW_CREDIT),
  m_congestionWindow(2 * BD_MAX_DATAGRAM_SIZE)
{
	m_timeoutPeriod = BD_RTT_START_VALUE;
	m_retransmitCountThreshold = BD_FAST_RETRANSMIT_THRESH;

	m_lastSent.start();
}

bdReliableSendWindow::~bdReliableSendWindow()
{
}

void bdReliableSendWindow::setTimeoutPeriod(const bdFloat32 secs)
{
	m_timeoutPeriod = secs;
}

bdFloat32 bdReliableSendWindow::getTimeoutPeriod() const
{
	return m_timeoutPeriod;
}

bdBool bdReliableSendWindow::add(bdDataChunkRef chunk)
{
	bdBool messageAdded = false;
	const bdUInt index = static_cast<bdUInt>(m_nextFree.getValue()) % BD_MAX_WINDOW_SIZE;

	bdMessageFrame &frameSlot = m_frame[index];

	if(!frameSlot.m_chunk)
	{
		const bdInt nextFreeSeqNum = m_nextFree.getValue();
		chunk->setSequenceNumber(static_cast<bdSeqNumber>(nextFreeSeqNum));
		frameSlot = bdMessageFrame(chunk);

		m_nextFree++;
		messageAdded = true;
	}
	else
	{
		// There is already a message in that slot, frame buffer is full.
		BD_WARN(BD_LOG_LEVEL, "reliable message window is full.");
	}

	return messageAdded;
}

void bdReliableSendWindow::getDataToSend(bdPacket &packet)
{
	const bdUWord pmtu = BD_MAX_DATAGRAM_SIZE;

	
	bdBool retransmitTimerExpired = false;
	bdBool neededFastRetransmit = false;
	bdBool haveDataToSend = false;

	// Check if any chunks need to be resent.
	for(bdSequenceNumber i = m_lastAcked + 1; i < m_nextFree; i++)
	{
		const bdUInt index = static_cast<bdUInt>(i.getValue()) % BD_MAX_WINDOW_SIZE;

		// we might not have anything at this index
		if(m_frame[index].m_chunk.notNull())
		{
			bdMessageFrame &frameSlot = m_frame[index];
			const bdSequenceNumber frameSeqNum = bdSequenceNumber(m_lastAcked, frameSlot.m_chunk->getSequenceNumber(), sizeof(bdSeqNumber) << 3);

			if(frameSeqNum.getValue() != i.getValue())
			{
				BD_ERR(BD_LOG_LEVEL, "Window error");
			}

			if(frameSlot.m_chunk)
			{
				const bdInt serializedSize = frameSlot.m_chunk->getSerializedSize();

				if(frameSlot.m_sendCount == 0)
				{
					// never sent this before.
					// there can't be any chunks that need to be resent after a chunk
					// that has never been sent.
					break;
				}
				else if(frameSlot.m_missingCount >= m_retransmitCountThreshold)
				{
					// fast retransmit
					neededFastRetransmit = true;
					m_flightSize -= serializedSize;
				}
				else if(frameSlot.m_timer.getElapsedTimeInSeconds() > m_timeoutPeriod)
				{
					// time out on packet. hasn't been acked in greater than RTO.
					retransmitTimerExpired = true;
					m_flightSize -= serializedSize;
				}
			}
		}
	}

	// Decrease congestion window because we needed to do fast retransmit.
	if(neededFastRetransmit)
	{
		decreaseCongestionWindow(BD_CWDR_PACKET_LOSS_DETECTED);
	}

	// Decrease congestion window because the resend timer expired.
	if(retransmitTimerExpired)
	{
		decreaseCongestionWindow(BD_CWDR_RESEND_TIMER_EXPIRED);

		//6.3.3 Handle T3-rtx Expiration
		//   E2) For the destination address for which the timer expires, set RTO
		//       <- RTO * 2 ("back off the timer").  The maximum value discussed
		//       in rule C7 above (RTO.max) may be used to provide an upper bound
		//       to this doubling operation.
		m_timeoutPeriod = 2 * m_timeoutPeriod;
		if(m_timeoutPeriod > BD_UC_RTO_MAX)
		{
			m_timeoutPeriod = BD_UC_RTO_MAX;
		}
	}

	// true if the packet is full or we have sent as much as we can (window size)
	bdBool full = false;

	for(bdSequenceNumber i = m_lastAcked + 1; i < m_nextFree && !full; i++)
	{
		//6.1  Transmission of DATA Chunks
		//A) At any given time, the data sender MUST NOT transmit new data to
		//   any destination transport address if its peer's rwnd indicates
		//   that the peer has no buffer space (i.e. rwnd is 0, see Section
		//   6.2.1).  However, regardless of the value of rwnd (including if it
		//   is 0), the data sender can always have one DATA chunk in flight to
		//   the receiver if allowed by cwnd (see rule B below).  This rule
		//   allows the sender to probe for a change in rwnd that the sender
		//   missed due to the SACK having been lost in transit from the data
		//   receiver to the data sender.
		//B) At any given time, the sender MUST NOT transmit new data to a
		//   given transport address if it has cwnd or more bytes of data
		//   outstanding to that transport address. The sender may exceed cwnd
		//   by up to (PMTU-1) bytes on a new transmission if the cwnd is not
		//   currently exceeded.
		//
		//   RFC 7.2.4 & the Implementers Guide 2.8.
		//
		//   3) Determine how many of the earliest (i.e., lowest TSN) DATA chunks
		//		marked for retransmission will fit into a single packet, subject
		//		to constraint of the path MTU of the destination transport address
		//		to which the packet is being sent. Call this value K. Retransmit
		//		those K DATA chunks in a single packet. When a Fast Retransmit is
		//		being performed the sender SHOULD ignore the value of cwnd and
		//		SHOULD NOT delay retransmission for this single packet.
		//
		//C) When the time comes for the sender to transmit, before sending new
		//	 DATA chunks, the sender MUST first transmit any outstanding DATA
		//	 chunks which are marked for retransmission (limited by the current
		//	 cwnd).
		//D) Then, the sender can send out as many new DATA chunks as Rule A
		//   and Rule B above allow.

		const bdUInt index = static_cast<bdUInt>(i.getValue()) % BD_MAX_WINDOW_SIZE;
		bdMessageFrame &frameSlot = m_frame[index];

		if(frameSlot.m_chunk)
		{
			bdDataChunkRef chunk = frameSlot.m_chunk;

			if(frameSlot.m_sendCount == 0)
			{
				// never sent this chunk before.

				const bdUInt serializedSize = chunk->getSerializedSize();

				const bdBool enoughWindowCredit = m_remoteReceiveWindowCredit - m_flightSize > static_cast<bdWord>(chunk->getSerializedSize());

				const bdBool fitsInPMTUAndCwmdNotExceeded = m_flightSize < m_congestionWindow ? (serializedSize < pmtu) : false;

				if(enoughWindowCredit)
				{
					if(packet.addChunk(chunk))
					{
						//BD_INFO(BD_LOG_LEVEL, "sent new packet %u", chunk->getSequenceNumber());

						frameSlot.m_sendCount++;
						frameSlot.m_timer.start();

						m_flightSize += serializedSize;
						haveDataToSend = true;
					}
					else
					{
						BD_INFO(BD_LOG_LEVEL, "packet full.");
						full = true;
					}
				}
				else if(fitsInPMTUAndCwmdNotExceeded)
				{
					// See rules A and B above:
					// ... can always have one DATA chunk in flight to
					//   the receiver if allowed by cwnd ...
					if(packet.addChunk(chunk))
					{
						BD_INFO(BD_LOG_LEVEL, "sent 1 new packet %u (rule b)", chunk->getSequenceNumber());

						frameSlot.m_sendCount++;
						frameSlot.m_timer.start();

						m_flightSize += serializedSize;
						haveDataToSend = true;
						full = true;
					}
					else
					{
						BD_INFO(BD_LOG_LEVEL, "packet full.");
						full = true;
					}
				}
				else
				{
					full = true;
				}
			}
			else if(frameSlot.m_missingCount >= m_retransmitCountThreshold)
			{
				// Fast Retransmit
				// missing count hits retransmit threshold.

				if(packet.addChunk(chunk))
				{
					BD_INFO(BD_LOG_LEVEL, "sent retransmit (fast retransmit) %u", chunk->getSequenceNumber());

					frameSlot.m_missingCount = 0;
					frameSlot.m_sendCount++;
					frameSlot.m_timer.start();

					m_flightSize += chunk->getSerializedSize();
					haveDataToSend = true;
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL, "packet full.");
					full = true;
				}
			}
			else if(frameSlot.m_timer.getElapsedTimeInSeconds() > m_timeoutPeriod)
			{
				// time out on packet. hasn't been acked in greater than RTO.

				const bdBool congestionWindowAllows = m_flightSize < m_congestionWindow;

				if(congestionWindowAllows)
				{
					if(packet.addChunk(chunk))
					{
						BD_INFO(BD_LOG_LEVEL, "sent retransmit (rto timeout) %u", chunk->getSequenceNumber());

						frameSlot.m_missingCount = 0;
						frameSlot.m_sendCount++;
						frameSlot.m_timer.start();

						m_flightSize += chunk->getSerializedSize();
						haveDataToSend = true;
					}
					else
					{
						BD_INFO(BD_LOG_LEVEL, "packet full.");
						full = true;
					}
				}
			}
		}
	}

	if(!haveDataToSend)
	{
		//When the endpoint does not transmit data on a given transport
		//address, the cwnd of the transport address should be adjusted to
		//max(cwnd/2, 2*MTU) per RTO.
		// note: the above statement assumes an RTO.MIN of 1 second. Because
		// our RTO.MIN is 10ms we quite often don't send
		// something for that time period, therefore we use 1.f below:
		if(m_lastSent.getElapsedTimeInSeconds() > 1.f)
		{
			decreaseCongestionWindow(BD_CWDR_INACTIVE);
			m_lastSent.start();
		}
	}
	else
	{
		m_lastSent.start();
	}

}


bdBool bdReliableSendWindow::handleAck(bdSAckChunkRef chunk, bdFloat32 &rtt)
{
	bdSequenceNumber ack(m_lastAcked, chunk->getCumulativeAck(), sizeof(bdSeqNumber) << 3);

	bdBool validAck = true;

	bdSequenceNumber lastSent = m_nextFree - 1;
	if(ack > lastSent)
	{
		BD_WARN(BD_LOG_LEVEL, "Acking unsent chunk.");
		validAck = false;
	}

	// if it's a new ack (and not an out of seq ack)
	if(validAck && ack.getValue() >= m_lastAcked.getValue())
	{
		const bdUInt index = ack.getValue() % BD_MAX_WINDOW_SIZE;

		//BD_INFO(BD_LOG_LEVEL, "received ack for %u", ack.getValue());

		// calculate RTT
		//Karn's algorithm: RTT measurements MUST NOT be made using packets
		//that were retransmitted (and thus for which it is ambiguous
		//whether the reply was for the first instance of the packet or a
		//later instance).
		if(m_frame[index].m_chunk && m_frame[index].m_sendCount == 1)
		{
			rtt = m_frame[index].m_timer.getElapsedTimeInSeconds();
		}
		else
		{
			rtt = 0.f;
		}

		// the remote side's receive window credit
		m_remoteReceiveWindowCredit = chunk->getWindowCredit();

		// count the amount of bytes in flight
		m_flightSize = 0;
		bdUInt count = 0;
		for(bdSequenceNumber i = ack + 1; count < BD_MAX_WINDOW_SIZE; i++)
		{
			const bdUInt index = static_cast<bdUInt>(i.getValue()) % BD_MAX_WINDOW_SIZE;

			bdMessageFrame &frameSlot = m_frame[index];
			
			if(frameSlot.m_chunk)
			{
				m_flightSize += frameSlot.m_chunk->getSerializedSize() * frameSlot.m_sendCount;
			}
			else
			{
				break;
			}

			count++;
		}

		//BD_INFO(BD_LOG_LEVEL, "remote recv credit: %d/%d", m_remoteReceiveWindowCredit, m_flightSize);

		// calculate the amount of bytes acked and
		// set the chunks in the window up to cumulative ack to NULL
		bdUInt bytesAcked = 0;
		for(bdSequenceNumber i = m_lastAcked + 1; i <= ack; i++)
		{
			const bdUInt index = static_cast<bdUInt>(i.getValue()) % BD_MAX_WINDOW_SIZE;

			if(m_frame[index].m_chunk.notNull())
			{
				bdMessageFrame &frameSlot = m_frame[index];

				bytesAcked += frameSlot.m_chunk->getSerializedSize();

				frameSlot.m_chunk = BD_NULL;
				frameSlot.m_timer.reset();
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Invalid ack.");
				validAck = false;
			}
		}

		bdLinkedList<bdSAckChunk::bdGapAckBlock> &blocks = chunk->getGapList();
		bdSequenceNumber lastSeq = ack + 1;
		while(validAck && !blocks.isEmpty())
		{
			bdSAckChunk::bdGapAckBlock &block = blocks.getHead();
			
			const bdSequenceNumber startSeq = ack + block.m_start;
			const bdSequenceNumber endSeq = ack + block.m_end;

			for(bdSequenceNumber i = lastSeq; i <= endSeq; i++)
			{
				const bdUInt index = static_cast<bdUInt>(i.getValue()) % BD_MAX_WINDOW_SIZE;
				bdMessageFrame &frameSlot = m_frame[index];

				if(frameSlot.m_chunk.isNull())
				{
					BD_WARN(BD_LOG_LEVEL, "Shouldn't be null!");
					validAck = false;
				}
				

				if(frameSlot.m_sendCount == 0)
				{
					BD_WARN(BD_LOG_LEVEL, "Send count should be > 0");
					validAck = false;
				}

				if(i < startSeq)
				{
					// not acked.. so missing
					frameSlot.m_missingCount++;
					//
					//   R4) Whenever a SACK is received missing a TSN that was previously
					//       acknowledged via a Gap Ack Block, start T3-rtx for the
					//       destination address to which the DATA chunk was originally
					//       transmitted if it is not already running.
					if(frameSlot.m_gapAcked)
					{
						frameSlot.m_gapAcked = false;
						if(frameSlot.m_timer.getElapsedTimeInSeconds() == 0)
						{
							frameSlot.m_timer.start();
						}
					}
				}
				else
				{
					frameSlot.m_gapAcked = true;
					frameSlot.m_timer.start();
					bytesAcked += frameSlot.m_chunk->getSerializedSize();
				}
			}

			lastSeq = endSeq + 1;
			blocks.removeHead();
		}

		increaseCongestionWindow(bytesAcked);

		// newest ack
		m_lastAcked = ack;
	}

	return validAck;
}

bdBool bdReliableSendWindow::isEmpty() const
{
	bdBool empty = true;

	for(bdSequenceNumber i = m_lastAcked; i < m_nextFree && empty; i++)
	{
		const bdUInt index = static_cast<bdUInt>(i.getValue()) % BD_MAX_WINDOW_SIZE;
		const bdMessageFrame &frameSlot = m_frame[index];

		if(frameSlot.m_chunk.notNull())
		{
			empty = false;
		}
	}

	return empty;
}

// protected:


void bdReliableSendWindow::increaseCongestionWindow(const bdUWord bytesAcked)
{
	const bdUWord pmtu = BD_MAX_DATAGRAM_SIZE;

	// only if the congestion window is fully utilized
	if(m_flightSize >= m_congestionWindow)
	{
		if (m_congestionWindow <= m_slowStartThresh)
		{
			// 7.2.1 Slow-Start (draft-ietf-tsvwg-sctpimpguide-13.txt)
			//When cwnd is less than or equal to ssthresh an SCTP endpoint MUST use
			//the slow start algorithm to increase cwnd only if the current
			//congestion window is being fully utilized, an incoming SACK advances
			//the Cumulative TSN Ack Point, and the data sender is not in Fast
			//Recovery. Only when these three conditions are met, can the cwnd be
			//increased; otherwise the cwnd MUST not be increased. If these conditions
			//are met then cwnd MUST be increased by at most the lesser of 
			// 1) the total size of the previously outstanding DATA chunk(s) acknowledged,
			//and 
			// 2) the destination's path MTU.
			if (bytesAcked > pmtu)
			{
				m_congestionWindow += pmtu;
			}
			else
			{
				m_congestionWindow += bytesAcked;
			}

			//BD_INFO(BD_LOG_LEVEL, "SS: ba: %u, cwnd: %d, ssthesh: %d, fs: %d",
			//						bytesAcked, 
			//						m_congestionWindow,
			//						m_slowStartThresh,
			//						m_flightSize);
		}
		else
		{
			// 7.2.2 Congestion Avoidance
			//Whenever cwnd is greater than ssthresh, upon each SACK arrival
			//that advances the Cumulative TSN Ack Point, increase
			//partial_bytes_acked by the total number of bytes of all new chunks
			//acknowledged in that SACK including chunks acknowledged by the new
			//Cumulative TSN Ack and by Gap Ack Blocks.
			//When partial_bytes_acked is equal to or greater than cwnd and
			//before the arrival of the SACK the sender had cwnd or more bytes
			//of data outstanding (i.e., before arrival of the SACK, flightsize
			//was greater than or equal to cwnd), increase cwnd by MTU, and
			//reset partial_bytes_acked to (partial_bytes_acked - cwnd).
			
			m_partialBytesAcked += bytesAcked;

			if(m_partialBytesAcked >= m_congestionWindow)
			{
				if(m_flightSize >= m_congestionWindow)
				{
					m_congestionWindow += pmtu;

					if(m_congestionWindow < m_partialBytesAcked)
					{
						m_partialBytesAcked = m_partialBytesAcked - m_congestionWindow;
					}
					else
					{
						m_partialBytesAcked = 0;
					}
				}
			}

			//BD_INFO(BD_LOG_LEVEL, "CA: ba: %u, cwnd: %d, ssthesh: %d, fs: %d",
			//						bytesAcked, 
			//						m_congestionWindow,
			//						m_slowStartThresh,
			//						m_flightSize);
		}
	}
}

void bdReliableSendWindow::decreaseCongestionWindow(const bdCongestionWindowDecreaseReason reason)
{
	const bdUWord pmtu = BD_MAX_DATAGRAM_SIZE;

	switch (reason) {
		case BD_CWDR_INACTIVE:
			//7.2.1 Slow-Start (draft-ietf-tsvwg-sctpimpguide-13.txt)
			//When the endpoint does not transmit data on a given transport
			//address, the cwnd of the transport address should be adjusted to
			//max(cwnd/2, 4*MTU) per RTO.
			m_congestionWindow = BD_MAX(m_congestionWindow / 2, static_cast<bdWord>(4 * pmtu));
			// fixme: the following line isn't in the RFC but makes sense.
			// if we've timed out then any data in flight that is unacked
			// is most likely lost.
			m_partialBytesAcked = 0;
			// fixme: this mightn't even make sense but there seems to be a bug with
			// flightsize and this fixes it.
			m_flightSize = 0;
			break;
		case BD_CWDR_PACKET_LOSS_DETECTED:
			//7.2.3 Congestion Control (draft-ietf-tsvwg-sctpimpguide-13.txt)
			//Upon detection of packet losses from SACK (see Section 7.2.4), an
			//	endpoint should do the following if not in Fast Recovery:

			//ssthresh = max(cwnd/2, 2*MTU)
			//	cwnd = ssthresh
			//	partial_bytes_acked = 0

			//	Basically, a packet loss causes cwnd to be cut in half.
			m_slowStartThresh = BD_MAX(m_congestionWindow / 2, static_cast<bdWord>(2 * pmtu));
			m_congestionWindow = m_slowStartThresh;
			m_partialBytesAcked = 0;
			break;
		case BD_CWDR_RESEND_TIMER_EXPIRED:
			//7.2.3 Congestion Control (draft-ietf-tsvwg-sctpimpguide-13.txt)
			//	When the T3-rtx timer expires on an address, SCTP should perform slow
			//	start by:

			//ssthresh = max(cwnd/2, 2*MTU)
			//	cwnd = 1*MTU
			//	partial_bytes_acked = 0
			m_slowStartThresh = BD_MAX(m_congestionWindow / 2, static_cast<bdWord>(2 * pmtu));
			m_congestionWindow = pmtu;
			m_partialBytesAcked = 0;
			break;
	};

	//BD_INFO(BD_LOG_LEVEL, "DCW: cwnd: %d, ssthesh: %d, fs: %d",
	//								m_congestionWindow,
	//								m_slowStartThresh,
	//								m_flightSize);
}
