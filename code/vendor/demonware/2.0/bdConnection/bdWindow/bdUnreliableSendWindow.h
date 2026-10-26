// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Queue for sending unreliable messages.

#ifndef BD_UNRELIABLE_SEND_WINDOW_H
#define BD_UNRELIABLE_SEND_WINDOW_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdPacket.h>

BD_REFERENCE(bdDataChunk);

/// Implements a packet queue for outgoing, unreliable packets.
class bdUnreliableSendWindow
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

	    /// Default constructor.
		bdUnreliableSendWindow();

		/// Destructor.
		virtual ~bdUnreliableSendWindow();

		/// Insert a message into the queue.
		/// \param chunk[in] A reference to the message to be inserted.
		void add(bdDataChunkRef chunk);

		void getDataToSend(bdPacket &packet);

		void reset();

	protected:

		/// Next sequence number to be used.
		bdSeqNumber m_seqNumber;

		/// The actual message frames.
		bdQueue<bdDataChunkRef> m_sendQueue;
};

#endif // BD_UNRELIABLE_SEND_WINDOW_H
