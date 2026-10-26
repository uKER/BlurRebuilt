// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdWindow/bdUnreliableReceiveWindow.h>

#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdPacket/bdDataChunk.h>

bdUnreliableReceiveWindow::bdUnreliableReceiveWindow()
: m_seqNumber(-1)
{
}

bdUnreliableReceiveWindow::~bdUnreliableReceiveWindow()
{
}

bdBool bdUnreliableReceiveWindow::add(bdDataChunkRef chunk)
{
	bdSequenceNumber newSeqNum(m_seqNumber, chunk->getSequenceNumber(), 16);

	if(newSeqNum > m_seqNumber)
	{
		m_recvQueue.enqueue(chunk);
		m_seqNumber = newSeqNum;
	}

	return true;
}


bdDataChunkRef bdUnreliableReceiveWindow::getNextToRead()
{
	bdDataChunkRef chunk;

	if (!m_recvQueue.isEmpty())
	{
		chunk = m_recvQueue.peek();

		m_recvQueue.dequeue();
	}

	return chunk;
}

void bdUnreliableReceiveWindow::reset()
{
	m_seqNumber = -1;
	
	while(!m_recvQueue.isEmpty())
	{
		m_recvQueue.dequeue();
	}
}
