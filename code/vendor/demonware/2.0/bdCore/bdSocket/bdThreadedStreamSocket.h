// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A wrapper around a socket to make it threaded.

#ifndef BD_THREADED_STREAM_SOCKET_H
#define BD_THREADED_STREAM_SOCKET_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdByteBuffer.h>
#include <bdCore/bdContainers/bdQueue.h>
#include <bdCore/bdSocket/bdStreamSocket.h>
#include <bdCore/bdThread/bdRunnable.h>
#include <bdCore/bdThread/bdMutex.h>
#include <bdCore/bdThread/bdSignal.h>

BD_REFERENCE(bdByteBuffer);

class bdThread;
class bdSignal;
class bdSendStreamThread;
class bdReceiveStreamThread;

class bdThreadedStreamSocket : public bdStreamSocket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdThreadedStreamSocket();

		~bdThreadedStreamSocket();

		bdBool create(const bdBool blocking);

		bdSocketStatusCode bind(const bdPort port);

		bdSocketStatusCode bind(const bdAddr addr);

		bdSocketStatusCode connect(const bdAddr addr) const;

		bdInt send(const void *data, const bdUInt length);

		bdInt recv(void *data, const bdUInt size);

		void close();

	protected:

		/// Separate receiving thread
		bdThread *m_receiveThread;
		bdReceiveStreamThread *m_receiver;

		/// Separate sending thread
		bdThread *m_sendThread;
		bdSendStreamThread *m_sender;

		bdUInt m_incomingBufferSize;


};

class bdSendStreamThread : public bdRunnable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdSendStreamThread(const bdInt handle);

		virtual ~bdSendStreamThread();

		/// Version of sendTo() that makes use of a separate thread
		/// \param addr[in] The destination address
		/// \param data[in] A pointer to the data to be sent
		/// \param length[in] Number of bytes to be sent
		virtual bdInt sendTo(const void *data,
							 const bdUInt length);

	protected:

		/// Inherited from bdRunnable
		virtual bdUInt run(void *args);

		/// Called to indicate the thread to stop
		virtual void stop();

		void waitForBind();

	protected:

		class bdSendStreamData
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				inline bdSendStreamData(bdByteBufferRef data)
				: m_data(data)
				{
				}

				bdByteBufferRef m_data;
		};

	protected:

		/// A signal for synchronizing access to the buffer.
		bdSignal m_signal;

		bdMutex m_lock;

		bdQueue<bdSendStreamData> m_toSend;

		bdInt m_handle;

		// Flag to indicate whether the socket has been bound to a port
		// Needed to safely start the receive thread
		friend class bdThreadedStreamSocket;
		bdBool m_isBound;
		bdSignal m_bindSignal;

};


class bdReceiveStreamThread : public bdRunnable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdReceiveStreamThread(const bdInt handle);

		virtual ~bdReceiveStreamThread();

		/// Non-blocking version of recvfrom() that makes use of a separate thread
		/// \param buf[in] A pointer to a buffer in which the incoming data is stored
		/// \param len[in] The maximum length of the receiving buffer
		/// \param flags[in]
		/// \param from[in] Pointer to an address structure to store the source address
		/// \param fromlen[in] Pointer to a variable that holds the size for the source address
		/// \param addr[in]
		/// \param data[in]
		/// \param size[in]
		virtual bdInt receiveFrom(void *data,
								  const bdUInt size);

	protected:

		/// Inherited from bdRunnable
		virtual bdUInt run(void *args);

	protected:

		class bdReceiveStreamData
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				inline bdReceiveStreamData(const bdInt status,
									 bdByteBufferRef data)
				: m_status(status),
				  m_data(data)
				{
				}

				bdInt m_status;
				bdByteBufferRef m_data;
		};

	protected:
		bdUByte8 m_bufferData[0xffffu];

		bdMutex m_lock;

		bdQueue<bdReceiveStreamData> m_received;

		bdUInt m_recvQueueSize;

		// A handle to the socket
		bdInt m_handle;
};

#endif // BD_THREADED_STREAM_SOCKET_H
