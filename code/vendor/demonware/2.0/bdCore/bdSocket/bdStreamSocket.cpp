// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdStreamSocket.h>


#include <bdCore/bdContainers/bdByteBuffer.h>

#define BD_LOG_LEVEL "bdSocket"

bdStreamSocket::bdStreamSocket()
: m_handle(BD_INVALID_SOCKET_HANDLE)
{
}

bdStreamSocket::~bdStreamSocket()
{
}

bdBool bdStreamSocket::create(const bdBool blocking)
{
	BD_ASSERT(m_handle == BD_INVALID_SOCKET_HANDLE,
				"bdStreamSocket::create(), already created.");

	m_handle = bdPlatformStreamSocket::create(blocking);
	const bdBool ok = (m_handle != BD_INVALID_SOCKET_HANDLE);
	return ok;
}

bdSocketStatusCode bdStreamSocket::bind(const bdPort port) const
{
	const bdAddr address(bdInetAddr::Any(), port);
	const bdSocketStatusCode status =  bind(address);
	return status;
}

bdSocketStatusCode bdStreamSocket::bind(const bdAddr addr) const
{
	const bdSocketStatusCode status = bdPlatformStreamSocket::bind(m_handle, 
																   addr.getAddress().getInAddr(),
																   addr.getPort());
	return status;

}

bdSocketStatusCode bdStreamSocket::connect(const bdAddr addr) const
{
	const bdSocketStatusCode status = bdPlatformStreamSocket::connect(m_handle, 
																	  addr.getAddress().getInAddr(),
																	  addr.getPort());
	return status;
}
bdBool bdStreamSocket::isConnected() const
{
	return bdPlatformStreamSocket::isWritable(m_handle);
}

bdInt bdStreamSocket::send(const void *data,
						   const bdUInt length) const
{
	return bdPlatformStreamSocket::send(m_handle, data, length);
}

bdInt bdStreamSocket::recv(void *data,
						   const bdUInt size) const
{
	return bdPlatformStreamSocket::receive(m_handle, data, size);
}

void bdStreamSocket::close()
{
	const bdBool ok = bdPlatformStreamSocket::close(m_handle);
	m_handle = BD_INVALID_SOCKET_HANDLE;

	if(!ok)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to close stream socket, system resources may be leaked.");
	}
}

#ifdef BD_PLATFORM_WIN32
bdBool bdStreamSocket::getSocketAddr(bdInAddr& socketAddr) const
{
	return bdPlatformStreamSocket::getSocketAddr(m_handle, socketAddr);
}
#endif

