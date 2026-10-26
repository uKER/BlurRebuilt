// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdLoopbackConnection.h>
#include <bdConnection/bdConnectionListener.h>
#include <bdConnection/bdMessage/bdMessage.h>

bdBool bdLoopbackConnection::receive(const bdUByte8 *, 
									 const bdUInt32)
{
	return true;
}

bdBool bdLoopbackConnection::send(const bdMessageRef message,
								  const bdBool /*reliable*/)
{
	if(message->getPayload())
	{
		message->getPayload()->resetReadPosition();
	}

	m_messages.enqueue(message);

	return true;
}


bdBool bdLoopbackConnection::getMessageToDispatch(bdMessageRef &message)
{
	bdBool result = false;
	if (!m_messages.isEmpty())
	{
		message = m_messages.peek();
		m_messages.dequeue();
		result = true;
	}
	return result;
}

bdUInt bdLoopbackConnection::getDataToSend(bdUByte8 *const,
										   const bdUInt32)
{
	return 0;
}

void bdLoopbackConnection::updateStatus()
{
	switch(m_status)
	{
		case BD_NOT_CONNECTED:
		break;

		case BD_CONNECTING:
		{
			for(bdUInt32 i = 0; i < m_listeners.getSize(); i++)
			{
				m_listeners[i]->onConnect(this);
			}
			m_status = BD_CONNECTED;
		}
		break;

		case BD_CONNECTED:
		break;

		case BD_DISCONNECTING:
		{
			for(bdUInt32 i = 0; i < m_listeners.getSize(); i++)
			{
				m_listeners[i]->onDisconnect(this);
			}
			m_status = BD_DISCONNECTED;
		}
		break;

		case BD_DISCONNECTED:
		break;
	}
}


bdLoopbackConnection::bdLoopbackConnection(bdCommonAddrRef addr)
: bdConnection(addr)
{
}

bdLoopbackConnection::~bdLoopbackConnection()
{
}
