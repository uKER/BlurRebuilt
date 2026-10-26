// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Queue for sending unreliable mesages.

#ifndef BD_UNRELIABLE_RECEIVE_WINDOW_H
#define BD_UNRELIABLE_RECEIVE_WINDOW_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdDataChunk);

/// Implements a message queue for incoming unreliable messages.
class bdUnreliableReceiveWindow
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdUnreliableReceiveWindow();

		/// Destructor.
		virtual ~bdUnreliableReceiveWindow();

		/// Inserts a message into the queue.
		/// \param chunk[in] A reference to the message to be inserted.
		/// \return
		///  - True if the message could be inserted.
		///  - False otherwise.
		bdBool add(bdDataChunkRef chunk);

		/// Retrieves the next message from the queue.
		/// \return
		///  - The next message in the queue if one exists.
		///  - BD_NULL otherwise.
		bdDataChunkRef getNextToRead();

		void reset();

	protected:

		/// Next sequence number to be used.
		bdSequenceNumber m_seqNumber;

		/// The actual message frames.
		bdQueue<bdDataChunkRef> m_recvQueue;
};

#endif // BD_UNRELIABLE_RECEIVE_WINDOW_H
