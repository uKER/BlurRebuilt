// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP_ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdConnectionStore.h>
#include <bdSocket/bdSocketRouter.h>


#if defined BD_SOCKET_DO_BROADCAST

void bdConnectionStore::flushSendBuffer()
{
	const bdInt status = m_socket->flushBuffer();
	if(status < 0)
	{
		switch(status)
		{
		case BD_NET_SUBSYTEM_ERROR:
			// something very wrong.
			BD_WARN(BD_LOG_LEVEL, "net subsystem error!");
			break;
		case BD_NET_ADDRESS_INVALID:
			BD_INFO(BD_LOG_LEVEL, "Invalid address. Closing connection.");
			break;
		case BD_NET_ERROR:
			// unknown error..
			BD_WARN(BD_LOG_LEVEL, "unknown error.");
			break;
		case BD_NET_WOULD_BLOCK:
			// doesn't happen.
			BD_WARN(BD_LOG_LEVEL, "would block.");
			break;
		case BD_NET_ADDRESS_IN_USE:
			// can't happen.. not binding
			break;
		case BD_NET_CONNECTION_RESET:
		case BD_NET_MSG_SIZE:
			// can't happen.. not receiving
			break;
		default:
			BD_WARN(BD_LOG_LEVEL, "unknown error.");
			break;
		}
	}
}

#endif

