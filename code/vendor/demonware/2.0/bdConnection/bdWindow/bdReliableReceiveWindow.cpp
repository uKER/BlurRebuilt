// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdWindow/bdReliableReceiveWindow.h>

#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdPacket/bdDefaultPacket.h>
#include <bdConnection/bdPacket/bdDataChunk.h>
#include <bdConnection/bdPacket/bdSAckChunk.h>

#define BD_LOG_LEVEL	"bdConnection/windows"

bdReliableReceiveWindow::bdReliableReceiveWindow()
: m_shouldAck(false),
  m_recvWindowCredit(BD_DEFAULT_RECEIVE_WINDOW_CREDIT),
  m_recvWindowUsedCredit(0)
{
}

bdReliableReceiveWindow::~bdReliableReceiveWindow()
{
}

bdBool bdReliableReceiveWindow::add(bdDataChunkRef chunk)
{
	// result indicates whether or not the chunk was added to the window
	bdBool result = true;

	BD_ASSERT(chunk.notNull(), "bdReliableReceiveWindow::add, null chunk.");

	bdSequenceNumber seqNumber(m_newest, chunk->getSequenceNumber(), sizeof(bdSeqNumber) << 3);

	const bdInt serializedSize = static_cast<bdInt>(chunk->getSerializedSize());

	// if the chunk is newer than all the other chunks in the window then
	// check to see if we can fit the window (according to available window credit)
	if(seqNumber < m_newest || serializedSize + m_recvWindowUsedCredit <= m_recvWindowCredit)
	{
		//BD_INFO(BD_LOG_LEVEL, "recvd %d", seqNumber.getValue());

		if(seqNumber > m_lastDispatched + BD_MAX_WINDOW_SIZE)
		{
			BD_INFO(BD_LOG_LEVEL, "Window overflow (a) - info only");
			result = false;
		}
		else if(seqNumber > m_lastDispatched)
		{
			const bdUInt index = static_cast<bdUInt>(seqNumber.getValue()) % BD_MAX_WINDOW_SIZE;

			if(m_frame[index].isNull())
			{
				m_recvWindowUsedCredit += chunk->getSerializedSize();

				m_frame[index] = chunk;

				if(seqNumber > m_newest)
				{
					m_newest = seqNumber;
				}
			}
			else if(m_frame[index]->getSequenceNumber() != chunk->getSequenceNumber())
			{
				BD_ERR(BD_LOG_LEVEL, "Window overflow (b) - error");
				result = false;
			}
		}
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "Not enough recv window credit.");
		result = false;
	}

	// always send an ack on receiving. this will not ack chunks that could
	// not fit into the window.
	calculateAck();

	return result;
}

void bdReliableReceiveWindow::getDataToSend(bdPacket &packet)
{
	if(m_sack)
	{
		if(packet.addChunk(m_sack))
		{
			//BD_INFO(BD_LOG_LEVEL, "acked %d, cred: %d", m_sack->getCumulativeAck(), m_sack->getWindowCredit());

			m_sack = BD_NULL;
		}
		else
		{
			BD_INFO(BD_LOG_LEVEL, "SACK chunk didn't fit in packet");
		}
	
	}
}

bdDataChunkRef bdReliableReceiveWindow::getNextToRead()
{
	bdDataChunkRef chunk;

	const bdUInt32 index = static_cast<bdUInt>((m_lastDispatched.getValue()) + 1) % BD_MAX_WINDOW_SIZE;	
	const bdSequenceNumber toDispatch = m_lastDispatched + 1;

	if(m_frame[index])
	{
		const bdSequenceNumber frameSeqNumber(m_lastDispatched, m_frame[index]->getSequenceNumber(), sizeof(bdSeqNumber) << 3);
		
		if(toDispatch == frameSeqNumber)
		{
			chunk = m_frame[index];
			m_frame[index] = BD_NULL;
			m_lastDispatched = frameSeqNumber;

			m_recvWindowUsedCredit -= chunk->getSerializedSize();

			if(m_sack)
			{
				m_sack->setWindowCredit(m_recvWindowCredit - m_recvWindowUsedCredit);
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "No SACK available");
			}
		}
		else
		{
			BD_INFO(BD_LOG_LEVEL, "unexpected seq number. (%d != %d)", toDispatch.getValue(), frameSeqNumber.getValue());
		}
	}

	return chunk;
}

void bdReliableReceiveWindow::calculateAck()
{	
	

	bdSequenceNumber i = m_lastCumulative > m_lastDispatched ? 
									m_lastCumulative + 1 : m_lastDispatched + 1;

	for(; i <= m_newest; i++)
	{
		const bdUInt index = static_cast<bdUInt>(i.getValue()) % BD_MAX_WINDOW_SIZE;

		if(m_frame[index])
		{
			m_lastCumulative = i;
		}
		else
		{
			break;
		}
	}

	// check to make sure we have a cumulative ack. we can't ack anything until
	// we have packet 0.
	if(m_lastCumulative != bdSequenceNumber())
	{
		m_sack = new bdSAckChunk(m_recvWindowCredit - m_recvWindowUsedCredit);

		bdSAckChunk::bdGapAckBlock block;
		for(i = m_lastCumulative + 1; i <= m_newest; i++)
		{
			const bdUInt index = static_cast<bdUInt>(i.getValue()) % BD_MAX_WINDOW_SIZE;

			if(m_frame[index])
			{
				if(!block.m_start)
				{
					block.m_start = static_cast<bdUInt>((i - m_lastCumulative).getValue());
				}
			}
			else
			{
				if(block.m_start)
				{
					block.m_end = static_cast<bdUInt>(((i - 1) - m_lastCumulative).getValue());

					m_sack->addGap(block);

#if defined BD_DEBUG
					// sanity check.
					// In debug mode checks to make sure the gap ack blocks
					// refer to valid blocks.
					bdSequenceNumber dstart = (m_lastCumulative + block.m_start).getValue();
					bdSequenceNumber dend = (m_lastCumulative + block.m_end).getValue();

					for(bdSequenceNumber jj = dstart; jj < dend; jj++)
					{
						const bdUInt index = jj.getValue() % BD_MAX_WINDOW_SIZE;
						BD_ASSERT(m_frame[index].notNull(), "Invalid gap ack.");
						BD_ASSERT(m_frame[index]->getSequenceNumber() == 
										static_cast<bdUInt16>(jj.getValue()), "Invalid seq num.");
					}
#endif

					block.m_start = 0;
					block.m_end = 0;
				}
			}
		}

		if(block.m_start)
		{
			block.m_end = static_cast<bdUInt>(((i - 1) - m_lastCumulative).getValue());
			m_sack->addGap(block);
		}

		m_sack->setCumulativeAck(static_cast<bdSeqNumber>(m_lastCumulative.getValue()));
	}
}
