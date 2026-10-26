// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 OSX WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdReceiveThread.h>

#define BD_LOG_LEVEL "receivethread"

bdReceiveThread::bdReceiveThread(const bdInt handle,
								 const bdUInt maxQueueSize)
: m_recvQueueSize(0),
  m_maxRecvQueueSize(maxQueueSize),
  m_handle(handle),
  m_status(BD_NET_WOULD_BLOCK)
{
}

bdReceiveThread::~bdReceiveThread()
{
}

bdInt bdReceiveThread::receiveFrom(bdAddr &addr,
								   void *data,
								   const bdUInt size)
{
	bdInt status = BD_NET_WOULD_BLOCK;
	bdByteBufferRef packetData = BD_NULL;

	m_lock.lock();
	if (m_received.isEmpty())
	{
		status = m_status;
	}
	else
	{
		bdReceiveData packet = m_received.peek();
		m_received.dequeue();
		m_recvQueueSize -= sizeof(bdReceiveData) + BD_MAX(0, packet.m_status);

		status = packet.m_status;
		packetData = packet.m_data;
		addr = packet.m_addr;
	}
	m_lock.unlock();

	if (packetData.notNull())
	{
		if (packetData->getSize() <= size)
		{
			bdMemcpy(data, packetData->getData(), packetData->getSize());
		}
		else
		{
			status = BD_NET_MSG_SIZE;
		}
	}

	return status;
}

bdUInt bdReceiveThread::run(void *)
{
	bdUByte8 buffer[BD_MAX_DATAGRAM_SIZE];

	while(!g_assertFalse)
	{
		// If receiveFrom returns an error, it is likely that it will continue to return errors.  If we keep trying,
		// we don't want to queue up a bunch of errors.  So this is probably the best approach:
		//   1. This class has an error status associated with it.
		//   2. When someone calls receiveFrom on us we do the following:
		//     a) If there is a packet in the buffer, return it!
		//     b) If there is no packet in the buffer then
		//       * return our error status if it is negative, or
		//       * return BD_NET_WOULD_BLOCK it if is non-negative.

		// TODO: It would be good to add back-off for cases when calling receiveFrom returns consecutive errors.
		//   X = 1ms
		//   MAX = 100ms
		//   receiveFrom -> ERROR
		//   receiveFrom -> ERROR
		//   wait X
		//   receiveFrom -> ERROR
		//   wait 2X
		//   receiveFrom -> ERROR
		//   wait 4X
		//    .
		//    .
		//    .
		//   receiveFrom -> ERROR
		//   wait MAX

		bdInAddr inAddr;
		bdPort port;
		bdInt status = bdPlatformSocket::receiveFrom(m_handle, inAddr, port, buffer, BD_MAX_DATAGRAM_SIZE);
		if (m_stop)
		{
			return 0;
		}

		// Turn the inAddr and port into something useful.
		bdAddr addr(bdInetAddr(inAddr), port);

		if (status < 0 && !addr.getAddress().isValid())
		{
			m_lock.lock();
			m_status = status;
			m_lock.unlock();
		}
		else if (m_recvQueueSize + sizeof(bdReceiveData) + BD_MAX(0, status) < m_maxRecvQueueSize)
		{
			bdByteBufferRef packetData = BD_NULL;

			if (status > 0)
			{
				packetData = new bdByteBuffer(static_cast<bdUInt>(status));
				bdMemcpy(packetData->getData(), buffer, static_cast<bdUInt>(status));
			}

			m_lock.lock();
			m_received.enqueue(bdReceiveData(status, addr, packetData));
			m_recvQueueSize += sizeof(bdReceiveData) + BD_MAX(0, status);
			m_status = BD_NET_WOULD_BLOCK;
			m_lock.unlock();
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "bdReceiveThread::run: buffer too small, packet discarded");
		}
	}

	return 0;
}

void bdReceiveThread::flushBuffer()
{
	m_lock.lock();
	while(!m_received.isEmpty())
	{
		m_received.dequeue();
	}
	m_recvQueueSize = 0;
	m_lock.unlock();
}

