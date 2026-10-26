// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdSocket.h>

#include <bdCore/bdSocket/bdCommonAddr.h>
#include <bdCore/bdContainers/bdByteBuffer.h>

#define BD_LOG_LEVEL "socket"

bdSocket::bdSocket()
: m_handle(BD_INVALID_SOCKET_HANDLE)
{
	
}

bdSocket::~bdSocket()
{
	close();
}

bdBool bdSocket::create(const bdBool blocking ,
						const bdBool broadcast)
{
	bdBool success = false;
	if (m_handle == BD_INVALID_SOCKET_HANDLE)
	{
		const bdInt createRes = bdPlatformSocket::create(blocking,broadcast);
		if (createRes < 0)
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to create socket. Err: %i" , createRes);
		}
		else
		{
			m_handle = createRes;
			success = true;
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "bdSocket::create(), already created.");
	}
	return success;
}

bdSocketStatusCode bdSocket::bind(const bdPort port)
{
	bdAddr address(bdInetAddr::Any(), port);
	return bdSocket::bind(address);
}

bdSocketStatusCode bdSocket::bind(const bdAddr &addr)
{
	return bdPlatformSocket::bind(m_handle, 
								  addr.getAddress().getInAddr(),
								  addr.getPort());
}


bdInt	bdSocket::sendTo(const bdAddr &addr, 
						 const void *data, 
						 const bdUInt length)
{
	return bdPlatformSocket::sendTo(m_handle, 
									addr.getAddress().getInAddr(), 
									addr.getPort(),
									data,
									length);
}

bdInt bdSocket::receiveFrom(bdAddr &addr, 
							void *data,
							const bdUInt size)
{
	bdInAddr inaddr;
	bdPort port;

	bdInt status = bdPlatformSocket::receiveFrom(m_handle, inaddr, port, data, size);

	if(status >= 0 || status == BD_NET_CONNECTION_RESET)
	{
		addr.set(bdInetAddr(inaddr), port);
	}

	return status;
}

bdBool bdSocket::close()
{
	return bdPlatformSocket::close(m_handle);
}

bdInt bdSocket::getHandle() const
{
	if(m_handle == BD_INVALID_SOCKET_HANDLE)
	{
		BD_WARN(BD_LOG_LEVEL,"Socket not yet created.");
	}
#ifdef BD_MULTITHREADED_SOCKET
	const bdInt realHandle = m_handle & 0x7FFFFFFF;
#else //  BD_MULTITHREADED_SOCKET
	const bdInt realHandle = m_handle;
#endif // BD_MULTITHREADED_SOCKET
	return realHandle;
}

void bdSocket::setExclusiveAccess()
{
#ifdef BD_PLATFORM_WIN32
	bdPlatformSocket::setExclusiveAccess(m_handle);
#endif
}
