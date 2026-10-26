// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 OSX WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A wrapper around a socket to make it threaded.

#ifndef BD_SEND_THREAD_H
#define BD_SEND_THREAD_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdByteBuffer.h>
#include <bdCore/bdContainers/bdQueue.h>
#include <bdCore/bdThread/bdRunnable.h>
#include <bdCore/bdThread/bdMutex.h>
#include <bdCore/bdThread/bdSignal.h>
#include <bdCore/bdSocket/bdSocket.h>

BD_REFERENCE(bdByteBuffer);

class bdSendThread : public bdRunnable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdSendThread(const bdInt handle);

		virtual ~bdSendThread();

		/// Version of sendTo() that makes use of a separate thread
		/// \param addr[in] The destination address
		/// \param data[in] A pointer to the data to be sent
		/// \param length[in] Number of bytes to be sent
		virtual bdInt sendTo(const bdAddr &addr,
							 const void *data,
							 const bdUInt length);

		void flushBuffer();

	protected:

		/// Inherited from bdRunnable
		virtual bdUInt run(void *args);

		/// Called to indicate the thread to stop
		virtual void stop();

		void waitForBind();

	protected:

		class bdSendData
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				inline bdSendData(const bdAddr& addr,
								  bdByteBufferRef data)
				: m_addr(addr),
				  m_data(data)
				{
				}

				bdAddr m_addr;
				bdByteBufferRef m_data;
		};

	protected:

		/// A signal for synchronizing access to the buffer.
		bdSignal m_signal;

		bdMutex m_lock;

		bdQueue<bdSendData> m_toSend;

		bdInt m_handle;

		// Flag to indicate whether the socket has been bound to a port
		// Needed to safely start the receive thread
		friend class bdThreadedSocket;
		bdBool m_isBound;
		bdSignal m_bindSignal;

};



#endif // BD_SEND_THREAD_H
