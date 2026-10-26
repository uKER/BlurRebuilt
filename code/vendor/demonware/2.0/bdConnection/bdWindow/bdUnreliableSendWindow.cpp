// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdWindow/bdUnreliableSendWindow.h>
#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdPacket/bdDataChunk.h>

bdUnreliableSendWindow::bdUnreliableSendWindow()
: m_seqNumber(BD_INITIAL_SEQ_NUMBER)
{
} 

bdUnreliableSendWindow::~bdUnreliableSendWindow()
{
}


void bdUnreliableSendWindow::add(bdDataChunkRef chunk)
{
	BD_ASSERT(chunk.notNull(), "Chunk should never be null.");
	m_sendQueue.enqueue(chunk);
}

void bdUnreliableSendWindow::getDataToSend(bdPacket &packet)
{
	bdBool addedChunk = true;
	bdDataChunkRef chunk;

	while(!m_sendQueue.isEmpty() && addedChunk)
	{
		chunk = m_sendQueue.peek();
		BD_ASSERT(chunk.notNull(), "Chunk should never be null.");

		chunk->setSequenceNumber(m_seqNumber);
		addedChunk = packet.addChunk(chunk);
		if(addedChunk)
		{
			m_seqNumber++;
			m_sendQueue.dequeue();
		}
	}
}

void bdUnreliableSendWindow::reset()
{
	m_seqNumber = BD_INITIAL_SEQ_NUMBER;

	while(!m_sendQueue.isEmpty())
	{
		m_sendQueue.dequeue();
	}
}

