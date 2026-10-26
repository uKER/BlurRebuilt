// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 OSX WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdSendThread.h>

#define BD_LOG_LEVEL "sendthread"

bdSendThread::bdSendThread(const bdInt handle)
: m_handle(handle),
  m_isBound(false)
{
}

bdSendThread::~bdSendThread()
{
}

bdInt bdSendThread::sendTo(const bdAddr &addr,
						   const void *data,
						   const bdUInt length)
{
	bdByteBufferRef buffer = new bdByteBuffer(length);
	bdMemcpy(buffer->getData(), data, length);

	m_lock.lock();
	m_toSend.enqueue(bdSendData(addr, buffer));
	m_lock.unlock();

	// data available
	m_signal.signal();
	return length;
}

bdUInt bdSendThread::run(void *)
{

	while(!m_stop)
	{
		m_signal.wait();
		while(!m_toSend.isEmpty())
		{
			m_lock.lock();
			const bdSendData sendData = m_toSend.peek();
			m_toSend.dequeue();
			m_lock.unlock();

			bdPlatformSocket::sendTo(m_handle,
									 sendData.m_addr.getAddress().getInAddr(),
									 sendData.m_addr.getPort(),
									 sendData.m_data->getData(),
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

void bdSendThread::stop()
{
	if(!m_stop)
	{
		bdRunnable::stop();
		m_signal.signal();
	}
}

void bdSendThread::waitForBind()
{
	if (!m_isBound)
	{
		m_bindSignal.wait();
	}
}

void bdSendThread::flushBuffer()
{
	m_lock.lock();
	while(!m_toSend.isEmpty())
	{
		m_toSend.dequeue();
	}
	m_lock.unlock();

}

