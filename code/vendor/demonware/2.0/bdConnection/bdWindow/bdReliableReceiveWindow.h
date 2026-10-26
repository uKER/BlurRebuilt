// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Queue for receiving reliable mesages.

#ifndef BD_RELIABLE_RECEIVE_WINDOW_H
#define BD_RELIABLE_RECEIVE_WINDOW_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdPacket.h>
#include <bdConnection/bdConnectionConfig.h>

BD_REFERENCE(bdDataChunk);
BD_REFERENCE(bdSAckChunk);

/// Implements a message queue for incoming reliable messages.
class bdReliableReceiveWindow
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdReliableReceiveWindow();

		/// Destructor.
		~bdReliableReceiveWindow();

		/// Inserts a message into the queue.
		/// \param chunk[in] A reference to the message to be inserted.
		/// \return
		///  - True if the message needs to be acknowledged.
		///  - False otherwise.
		bdBool add(bdDataChunkRef chunk);

		void getDataToSend(bdPacket &packet);

		/// Retrieves the next message from the queue.
		/// \return
		///  - The next message in the queue if one exists.
		///  - BD_NULL otherwise.
		bdDataChunkRef getNextToRead();

	protected:

		void calculateAck();

	protected:

		bdSequenceNumber m_newest;

		bdSequenceNumber m_lastCumulative;

		bdSequenceNumber m_lastDispatched;

		/// The actual messages.
		/// \todo Change to use bdPriorityHeap
		bdDataChunkRef m_frame[BD_MAX_WINDOW_SIZE];

		bdBool m_shouldAck;

		/// This value represents the dedicated buffer space, in number of
		/// bytes, the sender of the INIT has reserved in association with
		/// this window.  During the life of the association this buffer space
		/// SHOULD not be lessened (i.e. dedicated buffers taken away from
		/// this association); however, an endpoint MAY change the value of
		/// a_rwnd it sends in SACK chunks.
		bdWord m_recvWindowCredit;

		bdWord m_recvWindowUsedCredit;

		bdSAckChunkRef m_sack;
};

#endif // BD_RELIABLE_RECEIVE_WINDOW_H
