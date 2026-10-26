// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <revolution/os.h>
#include <revolution/soex.h>

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL "bdPlatformSocket"

bdInt BD_CALL bdPlatformSocket::create(const bdBool blocking,
									   const bdBool /*broadcast*/)
{

	// create the socket handle
	bdInt handle = SOSocket( SO_PF_INET, SO_SOCK_DGRAM, 0 );

	if(handle < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "SOSocket failed with %i.", handle);
		handle = BD_INVALID_SOCKET_HANDLE;
	}

	else
	{
		const bdInt val = SOFcntl( handle, SO_F_GETFL, 0 );

		if(!blocking)
		{
			// Change socket to nonblocking mode
			SOFcntl( handle, SO_F_SETFL, val | SO_O_NONBLOCK );
		}
		else
		{
			// Change socket to blocking mode
			SOFcntl( handle, SO_F_SETFL, val & ~SO_O_NONBLOCK );
		}
	}

	return handle;
}

bdSocketStatusCode BD_CALL bdPlatformSocket::bind(bdInt &handle,
												  const bdInAddr addr,
												  const bdUInt16 port)
{
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}
	
	bdSocketStatusCode error = BD_NET_ERROR;

	SOSockAddrIn sockAddr;
	bdMemset(&sockAddr, BD_NULL, sizeof(sockAddr));
	sockAddr.len = sizeof(sockAddr);
	sockAddr.family = SO_PF_INET;
	sockAddr.port = SOHtoNs(port);
	sockAddr.addr.addr = SOHtoNl(addr.m_inAddri);

	const bdInt retVal = SOBind(handle, &sockAddr);

	switch(retVal)
	{
		case SO_SUCCESS:
		{
			error = BD_NET_SUCCESS;
		}
		break;
		case SO_EADDRINUSE:
		{
			error = BD_NET_ADDRESS_IN_USE;
		}
		break;
		case SO_EAFNOSUPPORT:
		{
			error = BD_NET_ADDRESS_INVALID;
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
		case SO_EINVAL:
		{
			error = BD_NET_ADDRESS_INVALID;
		}
		break;
		case SO_ENETRESET:
		{
			error = BD_NET_ERROR;
		}
		break;
		case SO_ENOMEM:
		{
			error = BD_NET_ERROR;
		}
		break;
		case SO_EFATAL:
		{
			error = BD_NET_ERROR;
		}
		break;
		default:
		{
			BD_WARN(BD_LOG_LEVEL, "SOBind returned undocumented error code %i.", retVal);
			error = BD_NET_ERROR;
		}
		break;
	}
 
	return error;

}

bdInt BD_CALL bdPlatformSocket::sendTo(const bdInt handle,
									   const bdInAddr addr,
									   const bdUInt16 port,
									   const void *data,
									   const bdUInt length)
{
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}

	SOSockAddrIn sockAddr;
	bdMemset(&sockAddr, BD_NULL, sizeof(sockAddr));
	sockAddr.len = sizeof(sockAddr);
	sockAddr.family = SO_PF_INET;
	sockAddr.port = SOHtoNs(port);
	sockAddr.addr.addr = SOHtoNl(addr.m_inAddri);

	bdInt sent = SOSendTo(handle, data, length, 0, &sockAddr);

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
				BD_WARN(BD_LOG_LEVEL, "SOSendTo returned undocumented error code %i.", sent);
				sent = BD_NET_ERROR;
			}
			break;
		}
	}
	else
	{
		m_totalBytesSent += sent + BD_UDP_IP_OVERHEAD;
		m_totalPacketsSent++;
	}

	return sent;
}

bdInt BD_CALL bdPlatformSocket::receiveFrom(const bdInt handle,
											bdInAddr &addr,
											bdUInt16 &port,
											void *data,
											const bdUInt size)
{
	if(handle < 0)
	{
		return BD_NET_INVALID_HANDLE;
	}

	SOSockAddrIn sockAddr;
	bdMemset(&sockAddr, BD_NULL, sizeof(sockAddr));
	sockAddr.len = sizeof(sockAddr);

	bdInt received = SORecvFrom(handle, data, size, 0, &sockAddr);

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
				received = BD_NET_NOT_CONNECTED;
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
				BD_WARN(BD_LOG_LEVEL, "SORecvFrom returned undocumented error code %i.", received);
				received = BD_NET_ERROR;
			}
			break;
		}

	}
	else
	{
		addr.m_inAddri = SONtoHl(sockAddr.addr.addr);
		port = SONtoHs(sockAddr.port);
		m_totalBytesRecvd += received;
		
	}

	return received;

}

bdBool BD_CALL bdPlatformSocket::close(bdInt& handle)
{
	bdBool ok = true;
	if (handle != BD_INVALID_SOCKET_HANDLE)
	{
		ok = (SOClose(handle) == 0);			
		handle = BD_INVALID_SOCKET_HANDLE;
	}
	return ok;
}
/*
void BD_CALL bdPlatformSocket::unregisterThread()
{
}
*/

/*
bdDNSHandle BD_CALL bdPlatformSocket::getDNSHandle(void *, bdInt)
{

}
*/
bdUInt BD_CALL bdPlatformSocket::getHostByName(const bdNChar8* const name,
											   bdInAddr *adresses,
											   bdUInt numAdresses,
											   const bdDNSHandle )
{
	bdUInt numResults = 0;

	if(name && adresses && numAdresses > 0)
	{
		SOAddrInfo hints;
		SOAddrInfo* result = BD_NULL;

		// clear the hints
		bdMemset(&hints, BD_NULL, sizeof(hints));

		// set the hints

		// IPv4 only
		hints.family = SO_PF_INET; // PF_UNSPEC

		const bdInt socketError = SOGetAddrInfo(name, BD_NULL, &hints, &result);
		SOAddrInfo *const resultHead = result;
		
		if(socketError == 0)
		{
			while(result)
			{
				const SOSockAddrIn *const socketAddr = reinterpret_cast<const SOSockAddrIn *const>(result->addr);
				if(socketAddr)
				{
					// copy the sockaddr_in address into the dest array
					if(numResults < numAdresses)
					{
						// IPv4 only
						// convert the sockaddr to a sockaddr_in
						SOSockAddrIn inAddr;
						bdMemset(&inAddr, BD_NULL, sizeof(inAddr));
						bdMemcpy(&inAddr, socketAddr, BD_MIN(result->addrLen, sizeof(inAddr)));
						
						adresses[numResults].inUn.m_iaddr = SONtoHl(inAddr.addr.addr);
					}

					numResults++;
				}				
								
				result = result->next;
			}			

			if(resultHead)
			{
				SOFreeAddrInfo(resultHead);
			}
		}
		else
		{
			// check the error values
			switch(socketError)
			{
				case SO_EBUSY:
				{
				}
				break;
				case SO_EINVAL:
				{
				}
				break;
				case SO_ENETRESET:
				{
				}
				break;
				case SO_ENOMEM:
				{
				}
				break;
				case SO_EPROTONOSUPPORT:
				{
				}
				break;
				case SO_EAI_FAIL:
				{
				}
				break;
				case SO_EAI_MEMORY:
				{
				}
				break;
				case SO_EAI_NONAME:
				{
				}
				break;
				case SO_EFATAL:
				{
				}
				break;
				default:
				{
					BD_WARN(BD_LOG_LEVEL, "SOGetAddrInfo returned undocumented error code %i.", socketError);
				}
			}			
		}
	}

	return numResults;
}

void BD_CALL bdPlatformSocket::abortGetHostByName(bdDNSHandle)
{
	
}


