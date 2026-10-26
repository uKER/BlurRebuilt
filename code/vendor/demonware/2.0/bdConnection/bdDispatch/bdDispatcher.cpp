// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdDispatch/bdDispatcher.h>


#include <bdConnection/bdDispatch/bdDispatchInterceptor.h>
#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdConnection.h>
#include <bdConnection/bdMessage/bdReceivedMessage.h>

#define BD_LOG_LEVEL "dispatcher"

bdDispatcher::bdDispatcher()
{
}

void bdDispatcher::process(bdConnectionRef connection)
{
	bdMessageRef message;

	while(connection->getMessageToDispatch(message))
	{
		bdReceivedMessage rmessage(message, connection);

		bdBool accepted = false;
		for(bdUInt i = 0; !accepted && (i < m_interceptors.getSize()); i++)
		{
			accepted = m_interceptors[i]->accept(rmessage);
		}
	}
}

void bdDispatcher::registerInterceptor(bdDispatchInterceptor *const interceptor)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(m_interceptors.findFirst(interceptor, index))
	{
		BD_WARN(BD_LOG_LEVEL,"Same listener registered multiple times.");
	}
#endif // BD_DEBUG

	m_interceptors.pushBack(interceptor);
}

void bdDispatcher::unregisterInterceptor(bdDispatchInterceptor *const interceptor)
{
#ifdef BD_DEBUG
	bdUInt32 index;
	if(false == m_interceptors.findFirst(interceptor, index))
	{
		BD_WARN(BD_LOG_LEVEL,"Attempt to unregister unknown listener.");
	}
#endif // BD_DEBUG
	m_interceptors.removeAllKeepOrder(interceptor);
}

void bdDispatcher::reset()
{
	m_interceptors.clear();
}
