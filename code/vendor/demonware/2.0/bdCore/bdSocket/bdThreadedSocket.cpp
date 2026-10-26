// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 OSX WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdThreadedSocket.h>
#include <bdCore/bdSocket/bdSendThread.h>
#include <bdCore/bdSocket/bdReceiveThread.h>
#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>

#ifdef BD_PS2_USE_LIBEENET
#include <libeenet.h>
#undef bind
#endif // BD_PS2_USE_LIBEENET

#include <bdCore/bdThread/bdThread.h>

#define BD_LOG_LEVEL "threadedsocket"

// TODO . Currently memory is allocated for every received packet...
// It should be possible to only use stack allocated memory,
// either always or as a configuration option ?
#define BD_DEFAULT_INCOMING_BUFFER_SIZE 65000

bdThreadedSocket::bdThreadedSocket()
: m_receiveThread(BD_NULL),
  m_receiver(BD_NULL),
  m_sendThread(BD_NULL),
  m_sender(BD_NULL),
  m_incomingBufferSize(BD_DEFAULT_INCOMING_BUFFER_SIZE)
{
}

bdThreadedSocket::~bdThreadedSocket()
{
	close();
}

void bdThreadedSocket::setIncomingBufferSize(const bdUInt size)
{
	m_incomingBufferSize = size;
}

bdBool bdThreadedSocket::create(const bdBool /*blocking*/,
								const bdBool broadcast)
{
	bdInt priority = BD_DEFAULT_THREAD_PRIORITY;
#if (defined BD_PLATFORM_PS2 ||  defined BD_PLATFORM_PSP ||defined BD_PLATFORM_WII)
	// try to pick a thread priority that is higher that 'this' thread
	priority = bdPlatformThread::getPriority();
	BD_ASSERT(priority > 1, "bdThreadedSocket::create, threaded socket threads "
		"must be of higher priority than the main thread.");
	priority -= 1;
#endif // BD_PLATFORM_PS2

	// Threaded sockets are always blocking, so blocking param is ignored!
	const bdBool realBlockingVal = true;
	const bdBool created = bdSocket::create(realBlockingVal, broadcast);

	if(created)
	{
		if (!m_receiver)
		{
			m_receiver = new bdReceiveThread(m_handle, m_incomingBufferSize);
		}
		if (!m_receiveThread)
		{
			m_receiveThread = new bdThread(m_receiver, priority);
		}
		if (!m_sender)
		{
			m_sender = new bdSendThread(m_handle);
		}
		m_sender->m_isBound = false;
		if (!m_sendThread)
		{
			m_sendThread = new bdThread(m_sender, priority);
		}
		// Start the send-thread. The receive thread is started when either
		// sendTo() or bind() is called
		m_sendThread->start(BD_NULL,0);
	}

	return created;
}


bdSocketStatusCode bdThreadedSocket::bind(const bdPort port)
{
	bdSocketStatusCode success = bdSocket::bind(port);
	if (success)
	{
		if(m_receiveThread)
		{
			const bdBool receiveThreadStartResult = m_receiveThread->start(BD_NULL,0);
			if(false == receiveThreadStartResult)
			{
				BD_ERR(BD_LOG_LEVEL , "Failed to start receive thread ");
				success = BD_NET_ERROR;
			}
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL , "receive thread not created");
			success = BD_NET_ERROR;
		}
	}
	return success;
}

bdSocketStatusCode bdThreadedSocket::bind(const bdAddr &addr)
{
	bdSocketStatusCode success = bdSocket::bind(addr);
	if (success)
	{
		if(m_receiveThread)
		{
			const bdBool receiveThreadStartResult = m_receiveThread->start(BD_NULL,0);
			if(false == receiveThreadStartResult)
			{
				BD_ERR(BD_LOG_LEVEL , "Failed to start receive thread ");
				success = BD_NET_ERROR;
			}
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL , "receive thread not created !");
			success = BD_NET_ERROR;
		}
	}
	return success;
}

bdInt bdThreadedSocket::sendTo(const bdAddr &addr,
							   const void *data,
							   const bdUInt length)
{

	bdInt status = BD_NET_ERROR;
	if(m_sender)
	{
		status = m_sender->sendTo(addr, data, length);
		if(m_receiveThread)
		{
			if (!m_receiveThread->isRunning())
			{
				// Wait until the socket is bound
				m_sender->waitForBind();
				const bdBool receiveThreadStartResult = m_receiveThread->start(BD_NULL,0);
				if(false == receiveThreadStartResult)
				{
					BD_ERR(BD_LOG_LEVEL , "Failed to start receive thread ");
					status = BD_NET_ERROR;
				}
			}
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL , "receive thread not created !");
			status = BD_NET_ERROR;
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL , "sender not created !");
		status = BD_NET_ERROR;
	}

	return status;
}


bdInt bdThreadedSocket::receiveFrom(bdAddr &addr,
									void *data,
									const bdUInt size)
{
	bdInt result = BD_NET_ERROR;
	if(m_receiver)
	{
		result = m_receiver->receiveFrom(addr, data, size);
	}
	return result;
}

bdBool bdThreadedSocket::close()
{
	if (m_handle == BD_INVALID_SOCKET_HANDLE)
	{
		return false;
	}
	if (m_receiveThread)
	{
		m_receiveThread->stop();

#ifdef BD_PS2_USE_LIBEENET
		sceEENetThreadAbort( m_receiveThread->getThreadHandle() );
#endif

	}
	const bdBool status = bdSocket::close();

	if (m_sendThread)
	{
		m_sendThread->stop();
		m_sendThread->join();
#if defined BD_PLATFORM_PS2
		bdPlatformSocket::unregisterThread(m_sendThread->getThreadHandle());
#endif
		m_sendThread->cleanup();
		m_sendThread = BD_NULL;
	}
	if (m_receiveThread)
	{
		m_receiveThread->join();
#if defined BD_PLATFORM_PS2
		bdPlatformSocket::unregisterThread(m_receiveThread->getThreadHandle());
#endif
		m_receiveThread->cleanup();
		m_receiveThread = BD_NULL;
	}

	if (m_receiver)
	{
		delete m_receiver;
		m_receiver = BD_NULL;
	}
	if(m_sender)
	{
		delete m_sender;
		m_sender = BD_NULL;
	}

	return status;
}

void bdThreadedSocket::flushBuffers()
{
	if (m_receiver)
	{
		m_receiver->flushBuffer();
	}
	if(m_sender)
	{
		m_sender->flushBuffer();
	}
}


