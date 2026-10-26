// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 OSX WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A wrapper around a socket to make it threaded.

#ifndef BD_THREADED_SOCKET_H
#define BD_THREADED_SOCKET_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdSocket/bdSocket.h>

class bdThread;
class bdSendThread;
class bdReceiveThread;

class bdThreadedSocket : public bdSocket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdThreadedSocket();

		virtual ~bdThreadedSocket();

		void setIncomingBufferSize(const bdUInt size);

		//
		virtual bdBool create(	const bdBool blocking,
								const bdBool broadcast = true);

		virtual bdSocketStatusCode bind(const bdPort port);

		virtual bdSocketStatusCode bind(const bdAddr &addr);
		//
		virtual bdInt sendTo(const bdAddr &addr,
							 const void *data,
							 const bdUInt length);

		//
		virtual bdInt receiveFrom(bdAddr &addr,
								  void *data,
								  const bdUInt size);

		// See base class comment.
		virtual bdBool close();

		void flushBuffers();

	protected:

		/// Separate receiving thread
		bdThread *m_receiveThread;

		bdReceiveThread *m_receiver;

		/// Separate sending thread
		bdThread *m_sendThread;
		bdSendThread *m_sender;

		bdUInt m_incomingBufferSize;


};

#endif // BD_THREADED_SOCKET_H
