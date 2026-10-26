// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 OSX WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A wrapper around a socket to make it threaded.

#ifndef BD_RECEIVE_THREAD_H
#define BD_RECEIVE_THREAD_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdByteBuffer.h>
#include <bdCore/bdContainers/bdQueue.h>
#include <bdCore/bdSocket/bdSocket.h>
#include <bdCore/bdThread/bdRunnable.h>
#include <bdCore/bdThread/bdMutex.h>

BD_REFERENCE(bdByteBuffer);

class bdReceiveThread : public bdRunnable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdReceiveThread(const bdInt handle,
						const bdUInt maxQueueSize);

		virtual ~bdReceiveThread();

		/// Non-blocking version of recvfrom() that makes use of a separate thread
		/// \param buf[in] A pointer to a buffer in which the incoming data is stored
		/// \param len[in] The maximum length of the receiving buffer
		/// \param flags[in]
		/// \param from[in] Pointer to an address structure to store the source address
		/// \param fromlen[in] Pointer to a variable that holds the size fo the source address
		/// \param addr[in]
		/// \param data[in]
		/// \param size[in]
		virtual bdInt receiveFrom(bdAddr &addr,
								  void *data,
								  const bdUInt size);

		void flushBuffer();

	protected:

		/// Inherited from bdRunnable
		virtual bdUInt run(void *args);

	protected:

		class bdReceiveData
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				inline bdReceiveData(const bdInt status,
									 const bdAddr& addr,
									 bdByteBufferRef data)
				: m_status(status),
				  m_addr(addr),
				  m_data(data)
				{
				}

				bdInt m_status;
				bdAddr m_addr;
				bdByteBufferRef m_data;
		};

	protected:

		bdMutex m_lock;

		bdQueue<bdReceiveData> m_received;

		bdUInt m_recvQueueSize;

		bdUInt m_maxRecvQueueSize;

		// A handle to the socket
		bdInt m_handle;

		// Last known status of the socket after calling bdPlatformSocket::receiveFrom
		bdInt m_status;
};

#endif // BD_RECEIVE_THREAD_H
