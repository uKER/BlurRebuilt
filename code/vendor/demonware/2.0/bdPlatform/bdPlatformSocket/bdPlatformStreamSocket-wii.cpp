// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <revolution/os.h>
#include <revolution/so/SOBasic.h>

#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL "bdPlatformStreamSocket"

bdInt BD_CALL bdPlatformStreamSocket::create(const bdBool blocking)
{
	// create the socket handle
	bdInt handle = SOSocket( SO_PF_INET, SO_SOCK_STREAM, 0 );

	if(handle < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "SOSocket failed with %i.", handle);
		handle = BD_INVALID_SOCKET_HANDLE;
	}

	else
	{
		bdBool ok = true;
		const bdInt val = SOFcntl( handle, SO_F_GETFL, 0 );
		if(!blocking)
		{
			// Change socket to nonblocking mode
			ok = (SOFcntl( handle, SO_F_SETFL, val | SO_O_NONBLOCK ) >= 0);
		}
		else
		{
			// Change socket to blocking mode
			ok = (SOFcntl( handle, SO_F_SETFL, val & ~SO_O_NONBLOCK ) >= 0);
		}

		if (false == ok)
		{
			handle = BD_INVALID_SOCKET_HANDLE;
		}
	}


	return handle;
}

bdSocketStatusCode BD_CALL bdPlatformStreamSocket::connect(const bdInt handle,
														   const bdInAddr addr,
														   const bdUInt16 port)
{
	bdSocketStatusCode error = BD_NET_ERROR;

	SOSockAddrIn sockAddr;
	bdMemset(&sockAddr, BD_NULL, sizeof(sockAddr));
	sockAddr.len = sizeof(sockAddr);
	sockAddr.family = SO_PF_INET;
	sockAddr.port = SOHtoNs(port);
	sockAddr.addr.addr = SOHtoNl(addr.m_inAddri);

	const bdInt retval = SOConnect(handle, &sockAddr);

	switch(retval)
	{
		case SO_SUCCESS:
		{
			error = BD_NET_SUCCESS;
		}
		break;
		case SO_EAFNOSUPPORT:
		{
			error = BD_NET_ADDRESS_INVALID;
		}
		break;
		case SO_EALREADY:
		{
			error = BD_NET_WOULD_BLOCK;
		}
		break;
		case SO_EBADF:
		{
			error = BD_NET_INVALID_HANDLE;
		}
		break;
		case SO_EBUSY:
		{
			error = BD_NET_WOULD_BLOCK;
		}
		break;
		case SO_ECONNREFUSED:
		{
			error = BD_NET_CONNECTION_RESET;
		}
		break;
		case SO_ECONNRESET:
		{
			error = BD_NET_CONNECTION_RESET;
		}
		break;
		case SO_EINPROGRESS:
		{
			error = BD_NET_WOULD_BLOCK;
		}
		break;
		case SO_EINVAL:
		{
			error = BD_NET_ADDRESS_INVALID;
		}
		break;
		case SO_EISCONN:
		{
			error = BD_NET_ERROR;
		}
		break;
		case SO_ENETRESET:
		{
			error = BD_NET_CONNECTION_RESET;
		}
		break;
		case SO_ENETUNREACH:
		{
			error = BD_NET_CONNECTION_RESET;
		}
		break;
		case SO_ENOBUFS:
		{
			error = BD_NET_ERROR;
		}
		break;
		case SO_ENOMEM:
		{
			error = BD_NET_ERROR;
		}
		break;
		case SO_ETIMEDOUT:
		{
			error = BD_NET_CONNECTION_RESET;
		}
		break;
		case SO_EFATAL:
		{
			error = BD_NET_ERROR;
		}
		break;
		default:
		{
			BD_WARN(BD_LOG_LEVEL, "SOConnect returned undocumented error code %i.", retval);
			error = BD_NET_ERROR;
		}
		break;
	}

	return error;
}

bdInt BD_CALL bdPlatformStreamSocket::send(const bdInt handle,
										   const void *data,
										   const bdUInt length)
{
	if(handle == BD_INVALID_SOCKET_HANDLE)
	{
		return BD_NET_INVALID_HANDLE;
	}

	bdInt sent = SOSend(handle, data, length, 0);

	if(sent < 0)
	{
		switch(sent)
		{
			case SO_EAFNOSUPPORT:
			{
				sent = BD_NET_ADDRESS_INVALID;
			}
			break;
			case SO_EAGAIN: // SO_EWOULDBLOCK:
			{
				sent = BD_NET_WOULD_BLOCK;
			}
			break;
			case SO_EBADF:
			{
				sent = BD_NET_INVALID_HANDLE;
			}
			break;
			case SO_EBUSY:
			{
				sent = BD_NET_WOULD_BLOCK;
			}
			break;
			case SO_ECONNRESET:
			{
				sent = BD_NET_CONNECTION_RESET;
			}
			break;
			case SO_EDESTADDRREQ:
			{
				sent = BD_NET_INVALID_HANDLE;
			}
			break;
			case SO_EINVAL:
			{
				sent = BD_NET_ADDRESS_INVALID;
			}
			break;
			case SO_EMSGSIZE:
			{
				sent = BD_NET_MSG_SIZE;
			}
			break;
			case SO_ENETRESET:
			{
				sent = BD_NET_CONNECTION_RESET;
			}
			break;
			case SO_ENETUNREACH:
			{
				sent = BD_NET_CONNECTION_RESET;
			}
			break;
			case SO_ENOBUFS:
			{
				sent = BD_NET_ERROR;
			}
			break;
			case SO_ENOMEM:
			{
				sent = BD_NET_ERROR;
			}
			break;
			case SO_ENOTCONN:
			{
				sent = BD_NET_NOT_CONNECTED;
			}
			break;
			case SO_EOPNOTSUPP:
			{
				sent = BD_NET_ERROR;
			}
			break;
			case SO_ETIMEDOUT:
			{
				sent = BD_NET_CONNECTION_RESET;
			}
			break;
			case SO_EFATAL:
			{
				sent = BD_NET_ERROR;
			}
			break;
			default:
			{
				BD_WARN(BD_LOG_LEVEL, "SOSend returned undocumented error code %i.", sent);
				sent = BD_NET_ERROR;
			}
			break;
		}
	}
	else
	{
		m_totalBytesSent += sent;
	}

	return sent;
}

bdInt BD_CALL bdPlatformStreamSocket::receive(const bdInt handle,
											  void *const data,
											  const bdUInt size)
{
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}

	bdInt received = SORecv(handle, data, size, 0);

	if(received < 0)
	{
		switch(received)
		{
			case SO_EAGAIN: //SO_EWOULDBLOCK:
			{
				received = BD_NET_WOULD_BLOCK;
			}
			break;
			case SO_EBADF:
			{
				received = BD_NET_INVALID_HANDLE;
			}
			break;
			case SO_EBUSY:
			{
				received = BD_NET_WOULD_BLOCK;
			}
			break;
			case SO_ECONNRESET:
			{
				received = BD_NET_CONNECTION_RESET;
			}
			break;
			case SO_EINVAL:
			{
				received = BD_NET_CONNECTION_RESET;
			}
			break;
			case SO_ENETRESET:
			{
				received = BD_NET_CONNECTION_RESET;
			}
			break;
			case SO_ENOMEM:
			{
				received = BD_NET_ERROR;
			}
			break;
			case SO_ENOTCONN:
			{
				received = BD_NET_NOT_CONNECTED;
			}
			break;
			case SO_EOPNOTSUPP:
			{
				received = BD_NET_ERROR;
			}
			break;
			case SO_ETIMEDOUT:
			{
				received = BD_NET_CONNECTION_RESET;
			}
			break;
			case SO_EFATAL:
			{
				received = BD_NET_ERROR;
			}
			break;
			default:
			{
				BD_WARN(BD_LOG_LEVEL, "SORecv returned undocumented error code %i.", received);
				received = BD_NET_ERROR;
			}
			break;
		}

	}
	else
	{
		m_totalBytesRecvd += received;
	}

	return received;
}

bdBool BD_CALL bdPlatformStreamSocket::close(const bdInt handle)
{
	bdBool ok = true;
	if (handle != BD_INVALID_SOCKET_HANDLE)
	{
		ok = (SOClose(handle) == 0);
	}
	return ok;
}

bdBool BD_CALL bdPlatformStreamSocket::isWritable(const bdInt handle)
{
	bdBool writable = false;

	// Check the status of the connection
	const bdUInt numFds = 1;
	SOPollFD fds [numFds];
	fds[0].fd = handle;
	OSTime timeout = 0; // Seems arbitrary..
	fds[0].events = SO_POLLOUT;
	fds[0].revents = 0;

	const bdInt pollRes = SOPoll(fds , numFds , timeout);
	if ((pollRes >= 0) &&
		(fds[0].revents == fds[0].events))
	{
		writable = true;
	}
	return writable;


}


