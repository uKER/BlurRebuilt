// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdThreadedStreamSocket.h>

#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#ifdef BD_PS2_USE_LIBEENET
#include <libeenet.h>
#undef bind
#endif // BD_PS2_USE_LIBEENET

#include <bdCore/bdThread/bdThread.h>
#include <bdCore/bdThread/bdSemaphore.h>

#define BD_LOG_LEVEL "threadedstreamsocket"

bdThreadedStreamSocket::bdThreadedStreamSocket()
: m_receiveThread(BD_NULL),
  m_receiver(BD_NULL),
  m_sendThread(BD_NULL),
  m_sender(BD_NULL)
{
}

bdThreadedStreamSocket::~bdThreadedStreamSocket()
{
	close();
}

bdBool bdThreadedStreamSocket::create(const bdBool /*blocking*/)
{
	bdInt priority = BD_DEFAULT_THREAD_PRIORITY;
#if (defined BD_PLATFORM_PS2 ||  defined BD_PLATFORM_PSP ||defined BD_PLATFORM_WII)
	// try to pick a thread priority that is higher that 'this' thread
	priority = bdPlatformThread::getPriority();
	BD_ASSERT(priority > 1, "bdThreadedSocket::create, threaded socket threads "
		"must be of higher priority than the main thread.");
	priority -= 1;
#endif // BD_PLATFORM_PS2

	// The blocking param is ignored..
	const bdBool realBlocking = true;
	const bdBool created = bdStreamSocket::create(realBlocking);
	if(created)
	{
		if (!m_receiver)
		{
			m_receiver = new bdReceiveStreamThread(m_handle);
		}
		if (!m_receiveThread)
		{
			m_receiveThread = new bdThread(m_receiver, priority);
		}
		if (!m_sender)
		{
			m_sender = new bdSendStreamThread(m_handle);
		}
		m_sender->m_isBound = false;
		if (!m_sendThread)
		{
			m_sendThread = new bdThread(m_sender, priority);
		}
		// Start the send-thread. The receive thread is started when either
		// sendTo() or bind() is called
		bdSemaphore* pSema = new bdSemaphore(0, 1);
		m_sendThread->start(&pSema, sizeof(pSema));
		pSema->wait();
		m_receiveThread->start(&pSema, sizeof(pSema));
		pSema->wait();
		pSema->destroy();
	}


	return created;
}


bdSocketStatusCode bdThreadedStreamSocket::bind(const bdPort port)
{
	bdSocketStatusCode success = bdStreamSocket::bind(port);
	return success;
}

bdSocketStatusCode bdThreadedStreamSocket::bind(const bdAddr addr)
{
	bdSocketStatusCode success = bdStreamSocket::bind(addr);
	return success;
}


bdSocketStatusCode bdThreadedStreamSocket::connect(const bdAddr addr) const
{
	return bdStreamSocket::connect(addr);
}


bdInt bdThreadedStreamSocket::send(const void *data,
							   const bdUInt length)
{
	const bdInt status = m_sender->sendTo(data, length);
	return status;
}


bdInt bdThreadedStreamSocket::recv(void *data,
									const bdUInt size)
{
	return m_receiver->receiveFrom(data, size);
}

void bdThreadedStreamSocket::close()
{
	if (m_handle == BD_INVALID_SOCKET_HANDLE)
	{
		return;
	}
	if (m_receiveThread)
	{
		m_receiveThread->stop();

#ifdef BD_PS2_USE_LIBEENET
		sceEENetThreadAbort( m_receiveThread->getThreadHandle() );
#endif

	}
	bdStreamSocket::close();

	if (m_sendThread)
	{
		m_sendThread->stop();
		m_sendThread->join();
#if defined BD_PLATFORM_PS2
		bdPlatformStreamSocket::unregisterThread(m_sendThread->getThreadHandle());
#endif
		m_sendThread->cleanup();
		m_sendThread = BD_NULL;
	}
	if (m_receiveThread)
	{
		m_receiveThread->join();
#if defined BD_PLATFORM_PS2
		bdPlatformStreamSocket::unregisterThread(m_receiveThread->getThreadHandle());
#endif
		m_receiveThread->cleanup();
		m_receiveThread = BD_NULL;
	}

	delete m_receiver;
	m_receiver = BD_NULL;
	delete m_sender;
	m_sender = BD_NULL;
	return;
}


// class bdSendStreamThread

bdSendStreamThread::bdSendStreamThread(const bdInt handle)
: m_handle(handle),
  m_isBound(false)
{
}

bdSendStreamThread::~bdSendStreamThread()
{
}

bdInt bdSendStreamThread::sendTo(const void *data,
						   const bdUInt length)
{
	bdByteBufferRef buffer = new bdByteBuffer(length);
	bdMemcpy(buffer->getData(), data, length);

	m_lock.lock();
	m_toSend.enqueue(bdSendStreamData(buffer));
	m_lock.unlock();

	// data available
	m_signal.signal();
	return length;
}

bdUInt bdSendStreamThread::run(void *_pArg)
{
	bdSemaphore** ppSema = (bdSemaphore**)_pArg;
	bdSemaphore* pSema = *ppSema;

	pSema->release();

	while(!m_stop)
	{
		m_signal.wait();

		for(;;)
		{
			m_lock.lock();
			if ( m_toSend.isEmpty() )
			{
				m_lock.unlock();
				break;
			}

			const bdSendStreamData sendData = m_toSend.peek();
			m_toSend.dequeue();
			m_lock.unlock();

			bdPlatformStreamSocket::send(m_handle, sendData.m_data->getData(),
									 sendData.m_data->getSize());

			// After the first sendTo() the socket will be bound. We keep track of it
			// here in order to safely start the receiving thread
			if (!m_isBound)
			{
				m_bindSignal.signal();
				m_isBound = true;
			}
		}
	}

	return 0;
}

void bdSendStreamThread::stop()
{
	if(!m_stop)
	{
		bdRunnable::stop();
		m_signal.signal();
	}
}

void bdSendStreamThread::waitForBind()
{
	if (!m_isBound)
	{
		m_bindSignal.wait();
	}
}

// class bdReceiveStreamThread

bdReceiveStreamThread::bdReceiveStreamThread(const bdInt handle)
: m_recvQueueSize(0),
  m_handle(handle)
{
}

bdReceiveStreamThread::~bdReceiveStreamThread()
{
}

bdInt bdReceiveStreamThread::receiveFrom(void *data,
								   const bdUInt size)
{
	bdInt status = BD_NET_WOULD_BLOCK;

	m_lock.lock();

	if(!m_received.isEmpty())
	{
		bdReceiveStreamData rdata = m_received.peek();
		m_received.dequeue();

		if(rdata.m_status >= 0)
		{
			if(rdata.m_data->getSize() <= size)
			{
				status = rdata.m_status;
				bdMemcpy(data, rdata.m_data->getData(), rdata.m_data->getSize());
			}
			else
			{
				status = BD_NET_MSG_SIZE;
			}

			m_recvQueueSize -= status;
		}
		else
		{
			status = rdata.m_status;
		}
	}

	m_lock.unlock();

	return status;
}

bdUInt bdReceiveStreamThread::run(void *_pArg)
{
	bdSemaphore** ppSema = (bdSemaphore**)_pArg;
	bdSemaphore* pSema = *ppSema;

	pSema->release();

	while(!g_assertFalse)
	{
		bdInt status = bdPlatformStreamSocket::receive(m_handle, m_bufferData, 0xffffu);
		if (m_stop)
		{
			return 0;
		}

		if (status > 0 && m_recvQueueSize + status < 0xffffu)
		{
			bdByteBufferRef buffer;

			buffer = new bdByteBuffer(static_cast<bdUInt>(status));
			bdMemcpy(buffer->getData(), m_bufferData, static_cast<bdUInt>(status));

			m_recvQueueSize += status;

			m_lock.lock();
			m_received.enqueue(bdReceiveStreamData(status, buffer));
			m_lock.unlock();
		}
		else
		{
			//BD_WARN(BD_LOG_LEVEL, "bdReceiveStreamThread::run, buffer too small. Packet discarded");
		}

		bdPlatformTiming::sleep(1);
	}

	return 0;
}


