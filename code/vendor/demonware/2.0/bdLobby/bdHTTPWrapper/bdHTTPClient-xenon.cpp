// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "bdHTTPClient-xenon.h"
#include <bdCore/bdCore.h>
#include <bdLobby/bdContentStreaming/bdUploadInterceptor.h>
#include <bdLobby/bdContentStreaming/bdDownloadInterceptor.h>

#define BD_LOG_LEVEL ("http")

bdHTTPClient::bdHTTPClient(DWORD lspServiceID, BOOL usingThread) :
m_usingThread(usingThread),
m_status(BD_HTTP_STATUS_READY),
m_pResponseContentData(0),
m_dwResponseContentDataLength(0),
m_httpResponseCode(0),
m_expectedContentSize(0),
m_dwSocketErrorCode(0),
m_dataRate(0.0f),
m_bytesTransfered(0),
m_abort(false),
m_lspServiceID(lspServiceID),
m_hThread(0)
{
	// Create an event handle and setup an overlapped structure.
	m_overlapped.hEvent = WSACreateEvent();

	if(m_overlapped.hEvent == NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "Error creating event handler for bdHTTPClient.");
		m_status = BD_HTTP_STATUS_ERROR;
	}
	else
	{
		m_status = BD_HTTP_STATUS_READY;
	}
}


bdHTTPClient::~bdHTTPClient()
{
	WSACloseEvent(m_overlapped.hEvent);
	if (m_hThread != 0)
	{
		WaitForSingleObject(m_hThread,INFINITE);
		CloseHandle(m_hThread);
	}
	
}

// Worker thread entrance 
DWORD WINAPI HttpSendCommand(LPVOID lpParameter);

DWORD WINAPI HttpSendCommand(LPVOID lpParameter)
{
	BD_INFO(BD_LOG_LEVEL, "Starting HTTP operation");

    bdHTTPClient* httpClient = (bdHTTPClient*)lpParameter;

    if(!httpClient || httpClient->isAborted())
	{
		BD_ERR(BD_LOG_LEVEL,"Not executing HTTP operation");
        return S_FALSE;
	}

    bdHTTPBuffer& httpBuffer = httpClient->getInternalBuffer();
	
    int errorCode;

    // Create TCP/IP socket
    SOCKET hSocket;
    hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if((hSocket == SOCKET_ERROR) || (hSocket == INVALID_SOCKET))
    {
        errorCode = WSAGetLastError();
        httpClient->setSocketErrorCode(errorCode);
		httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);            
        return errorCode;
    }

	sockaddr_in httpServerAddr;
    httpServerAddr.sin_family = AF_INET;

	// check if we were given an IP address 
	httpServerAddr.sin_addr.s_addr = inet_addr(httpClient->m_serverName);
    
	// if not, lookup IP 
	XNDNS* pxndns = NULL;
	if(httpServerAddr.sin_addr.s_addr == INADDR_NONE)
    {
        //dns LOOKUP
        XNetDnsLookup(httpClient->m_serverName, httpClient->m_overlapped.hEvent, &pxndns);
        WaitForSingleObject(httpClient->m_overlapped.hEvent, INFINITE);
        WSAResetEvent(httpClient->m_overlapped.hEvent);
        if(pxndns->iStatus != 0)
        {
            //  An error occurred.  One of the following:
            //  pxndns->iStatus == WSAHOST_NOT_FOUND - No such host
            //  pxndns->iStatus == WSAETIMEDOUT - No response from DNS server(s)
            errorCode = pxndns->iStatus;
            XNetDnsRelease(pxndns);
            shutdown(hSocket, SD_BOTH);
            closesocket(hSocket);
            httpClient->setSocketErrorCode(errorCode);            
            httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);      
            return errorCode;
        }

		// select an address from those returned by DNS
        if (pxndns->cina == 1)
		{
            httpServerAddr.sin_addr = pxndns->aina[0];
        }
		else if (pxndns->cina > 1)
		{
			bdRandom rand;
			bdUInt32 r = rand.nextUInt() % pxndns->cina;
			BD_INFO(BD_LOG_LEVEL , "DNS returned multiple IPs - choosing at random.");
			httpServerAddr.sin_addr = pxndns->aina[r];
		}

        XNetDnsRelease(pxndns);
    }
    
	if (httpClient->isAborted())
	{
		BD_WARN(BD_LOG_LEVEL, "Aborting HTTP operation");
		httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return S_FALSE;
	}

	// connect
	{
		// translate from an unsecured IP to a secued IP
		IN_ADDR secureAddr;
		const INT convResult = XNetServerToInAddr(httpServerAddr.sin_addr, httpClient->getLspServiceID(), &secureAddr);
		if (0 == convResult )
		{
			BD_INFO(BD_LOG_LEVEL , "Converted unsercured web server addr to secure addr succeeded OK.");
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to convert IP addr to secure IP addr");
		}

		sockaddr_in secureSockAddr;
		secureSockAddr.sin_family = AF_INET;
		secureSockAddr.sin_addr = secureAddr;
		secureSockAddr.sin_port = htons(httpClient->m_port);

		// connect socket to IP/port
		if(connect(hSocket, (struct sockaddr*)&secureSockAddr, sizeof(secureSockAddr)) != 0)
		{
            errorCode = WSAGetLastError();
            shutdown(hSocket, SD_BOTH);
            closesocket(hSocket);
			BD_ERR(BD_LOG_LEVEL, "Failed to connect to connect for reason %d", errorCode);
            httpClient->setSocketErrorCode(errorCode);
            httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);   
            return errorCode;
        }
	}
	
	if (httpClient->isAborted())
	{
		BD_WARN(BD_LOG_LEVEL, "Aborting HTTP operation");
		httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return S_FALSE;
	}

	// configure the timers/metrics
	bdStopwatch httpTime;
	httpTime.start();
	bdFloat32 previousSampleTime(0.0f);
	httpClient->setDataRate(0.0f);
	httpClient->setBytesTransfered(0);

	// send the HTTP request (without the content data)
	if(SOCKET_ERROR != send(hSocket, static_cast<const char*>(httpBuffer.m_httpCommonBuffer), httpBuffer.m_httpCommonBufferSize, 0))
    {
		// send the content 
		bdInt contentSize = (bdInt)httpBuffer.m_contentSendBufferSize;
		bdInt sent(0);
		if (contentSize > 0)
		{
			bdInt res(0);
			while (sent < contentSize)
			{
				// abort?
				if (httpClient->isAborted())
				{
					BD_WARN(BD_LOG_LEVEL, "Aborting HTTP operation");
					httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
					return S_FALSE;
				}

				if (httpClient->m_uploadHandler != BD_NULL)
				{
					bdUInt32 toSend = httpClient->m_uploadHandler->handleUpload(httpBuffer.m_httpCommonBuffer, BD_XENON_MAX_PACKET_PAYLOAD, static_cast<bdUInt32>(sent));
					if (toSend > BD_XENON_MAX_PACKET_PAYLOAD)
					{
						BD_ERR(BD_LOG_LEVEL,"Client Error: bdUploadInterceptore::handleUpload must only send <= BD_XENON_MAX_PACKET_PAYLOAD (%s) bytes", BD_XENON_MAX_PACKET_PAYLOAD);
					}
					res = send(hSocket, static_cast<const char*>(httpBuffer.m_httpCommonBuffer), BD_MIN(toSend, BD_XENON_MAX_PACKET_PAYLOAD), 0);
				}
				else
				{
					// send
					res = send(hSocket, static_cast<const char*>(httpBuffer.m_contentSendBuffer) + sent, BD_MIN(contentSize - sent,BD_XENON_MAX_PACKET_PAYLOAD), 0);
				}
			
				// ok?
				if(res == SOCKET_ERROR)
				{
					bdInt32 err = WSAGetLastError(); 
					BD_ERR(BD_LOG_LEVEL,"Error %d occured when sending http payload", err);
					httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
					return S_FALSE;
				}
				else
				{
					sent += res;
				}

				// update the connection speed 
				{
					previousSampleTime = httpTime.getElapsedTimeInSeconds();
					httpClient->setDataRate(sent/previousSampleTime);
					httpClient->setBytesTransfered(sent);

					//BD_INFO(BD_LOG_LEVEL, "bytes sent %d @ %.2f bytes/sec", sent, sent/previousSampleTime);
				}
			}

			// update the connection speed 
			previousSampleTime = httpTime.getElapsedTimeInSeconds();
			httpClient->setDataRate(sent/previousSampleTime);
			httpClient->setBytesTransfered(sent);

			BD_INFO(BD_LOG_LEVEL, "total bytes sent %d @ %.2f bytes/sec", sent, sent/previousSampleTime);
		}

       	// recv the http response 
		bdInt read = recv(hSocket, httpBuffer.m_httpCommonBuffer, BD_XENON_MAX_PACKET_PAYLOAD, 0);

		// ok?
		if (read <= 0)
		{
			BD_ERR(BD_LOG_LEVEL, "Error occured when reading response from server.");
			httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
			return S_FALSE;
		}
		else
		{
			httpBuffer.m_httpCommonBufferSize = read;
		}

		// parse header and determine the start of the content data
		CHAR *data;
		HRESULT err = httpClient->parseIncomingHttpResponse(&data);

		if (err != ERROR_SUCCESS)
		{
			BD_ERR(BD_LOG_LEVEL, "Error occured when parsing initial data returned from server: %d", err);
			httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
			return S_FALSE;
		}

		// copy any content data received with the first recv into the content buffer
		bdUInt32 bytesDownloaded(0);
		CHAR *writePtr = httpBuffer.m_contentReceiveBuffer;
		
		bdUInt sizeToCopy = read - (data-httpBuffer.m_httpCommonBuffer);
		if (sizeToCopy > 0)
		{
			bytesDownloaded += sizeToCopy;

			if (httpClient->m_downloadHandler != BD_NULL)
			{
				while (sizeToCopy > 0)
				{
					bdUInt32 sizeCopied = BD_MIN(sizeToCopy,BD_XENON_MAX_PACKET_PAYLOAD);
					bdMemcpy(httpBuffer.m_httpCommonBuffer, data, sizeCopied);
					httpClient->m_downloadHandler->handleDownload(httpBuffer.m_httpCommonBuffer, sizeCopied);
					sizeToCopy -= sizeCopied;
				}
			}
			else
			{
				if (bytesDownloaded > httpBuffer.m_contentReceiveBufferSize)
				{
					BD_ERR(BD_LOG_LEVEL, "Error occured when reading data from socket: insufficent space to write %d bytes", sizeToCopy);
					httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
					return S_FALSE;
				}
				else
				{
					bdMemcpy(writePtr, data, sizeToCopy);
					writePtr += sizeToCopy;
				}
			}
		}

		// read content
		if (httpClient->getOperation() == bdHTTPClient::BD_HTTP_GET_OPERATION)
		{
			httpTime.start();
			previousSampleTime = 0.0f;

			// copy data from recieve in chunks
			CHAR* localBuffer = httpBuffer.m_httpCommonBuffer;
			bdBool done(false);
			while((bytesDownloaded < httpClient->getExpectedContentSize()) && !done)
			{
				if (httpClient->m_downloadHandler != BD_NULL)
				{
					// copy to local buffer
					read = recv(hSocket, localBuffer, BD_XENON_MAX_PACKET_PAYLOAD, 0);
					
					// call client callback
					read = httpClient->m_downloadHandler->handleDownload(localBuffer, read);

					if (read > 0)
					{
						bytesDownloaded += read;
					}
				}
				else
				{
					// copy to local buffer
					read = recv(hSocket, localBuffer, BD_XENON_MAX_PACKET_PAYLOAD, 0);

					// copy to client buffer
					if (read > 0)
					{
						bytesDownloaded += read;
						writePtr += read;
						
						if (bytesDownloaded <= httpBuffer.m_contentReceiveBufferSize)
						{
							bdMemcpy(writePtr, localBuffer, read);
						}
						else
						{
							BD_ERR(BD_LOG_LEVEL, "client specified buffer was not of sufficient size (allocated: %d required: > %d)", 
								httpBuffer.m_contentReceiveBufferSize,
								bytesDownloaded);

							httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
							return S_FALSE;
						}
					}
				}
				
				// done?
				if (read <= 0)
				{
					done = true;
				}

				// abort?
				if (httpClient->isAborted())
				{
					BD_WARN(BD_LOG_LEVEL, "Aborting HTTP operation");
					httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
					return S_FALSE;
				}

				// ok?
				if(read == SOCKET_ERROR)
				{
					bdInt32 err = WSAGetLastError(); 
					BD_ERR(BD_LOG_LEVEL,"Error %d occured when reading http payload", err);
					httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
					return S_FALSE;
				}

				// update the connection speed 
				{
					previousSampleTime = httpTime.getElapsedTimeInSeconds();
					httpClient->setDataRate(bytesDownloaded/previousSampleTime);
					httpClient->setBytesTransfered(bytesDownloaded);

					//BD_INFO(BD_LOG_LEVEL, "bytes recv %d @ %.2f bytes/sec", bytesDownloaded, bytesDownloaded/previousSampleTime);
				}
			}

			// check for socket error
			if (read == SOCKET_ERROR)
			{
				bdInt32 err = WSAGetLastError(); 
				BD_ERR(BD_LOG_LEVEL, "Error %d occured after reading %d bytes from socket", err, read);
				httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
				return S_FALSE;
			}
			else
			{
				// final update of the connection speed 
				previousSampleTime = httpTime.getElapsedTimeInSeconds();
				httpClient->setDataRate(bytesDownloaded/previousSampleTime);
				httpClient->setBytesTransfered(bytesDownloaded);

				BD_INFO(BD_LOG_LEVEL, "total bytes recv %d @ %.2f bytes/sec", bytesDownloaded, bytesDownloaded/previousSampleTime);
			}
		}

		// clean up
        shutdown(hSocket, SD_BOTH);
        closesocket(hSocket);
        httpClient->setSocketErrorCode(ERROR_SUCCESS);        
		
		// set error codes
		if(httpClient->getOperation() == bdHTTPClient::BD_HTTP_GET_OPERATION && bytesDownloaded < httpClient->getExpectedContentSize())
		{
			BD_ERR(BD_LOG_LEVEL, "Expected to receive %d bytes but instead received %d bytes", httpClient->getExpectedContentSize(), bytesDownloaded);

			httpClient->setResponseContentDataLength(0);
			httpClient->setResponseContentData(NULL);
			httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
			return S_FALSE;
		}
		else
		{
			httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_DONE);
			return ERROR_SUCCESS;
		}
    }
    else
    {
        shutdown(hSocket, SD_BOTH);
        closesocket(hSocket);
	    httpClient->setSocketErrorCode(WSAGetLastError());
		httpClient->setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return S_FALSE;
    }
}

HRESULT bdHTTPClient::parseIncomingHttpResponse(CHAR** contentDataPtr)
{
	UINT nDataLength = m_buffer.m_httpCommonBufferSize;

	// "HTTP/1.1 xxx" expected at a minimum from server
	if(nDataLength < 12)
	{
		BD_ERR(BD_LOG_LEVEL, "Error parsing HTTP response: server response too short to be valid");

		setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	CHAR* pData = m_buffer.m_httpCommonBuffer;

	// HTTP/1.1 expected
	if(bdMemcmp("HTTP", pData, 4) != 0)
	{
		BD_ERR(BD_LOG_LEVEL, "Error parsing HTTP response: HTTP protocol string not found.");

		setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	// find the response status (e.g. 200)
	while(!isspace(*pData) && nDataLength)
	{
		--nDataLength;
		++pData;
	}

	if(nDataLength == 0)
	{
		BD_ERR(BD_LOG_LEVEL, "Error parsing HTTP response: No space character found after HTTP protocol string.");

		setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	m_httpResponseCode = atoi((char*)pData);

	// OK 2xx expected
	if(!(m_httpResponseCode >= 200 && m_httpResponseCode < 300))
	{
		BD_ERR(BD_LOG_LEVEL, "Error reading HTTP response: error code is not in range 200 to 299. Error: %d", m_httpResponseCode);

		setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	if (m_operation == BD_HTTP_GET_OPERATION)
	{
		// find the start of the content data
		char* contentLen = bdStrstr(pData, "Content-Length: ");

		if (contentLen >= 0) 
		{
			contentLen += bdStrlen("Content-Length: ");
			m_expectedContentSize = atoi((char*)contentLen);
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Error reading HTTP response: no Content-Length header found.");
		}
	}

	// find the start of the content data
	// the format of the response is of form:
	// "HTTP/1.1 ... \r\nHost: %s:%d\r\nContent-Length: %d\r\n\r\n" serverName, port
	// followed by the data 

	*contentDataPtr = bdStrstr(pData, "\r\n\r\n");

	if (*contentDataPtr == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "Error searching for start of content data. Pattern \\r\\n\\r\\n not found in response");

		setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}
	else
	{
		*contentDataPtr = *contentDataPtr + bdStrlen("\r\n\r\n");
	}

	return ERROR_SUCCESS;
}

HRESULT bdHTTPClient::sendCommand()
{
    if(m_usingThread)
    {
        // start worker thread
        m_hThread = CreateThread(NULL, 0, HttpSendCommand, (VOID*)this, 0, NULL);
        if(m_hThread)
        {
			SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread(m_hThread);
            return E_PENDING;
        }
        else
        {
			setStatus(BD_HTTP_STATUS_ERROR);
			return E_FAIL;
        }
    }
    else
    {
        // Or process it directly, synchronized call. Don't do it in your main thread
        return HttpSendCommand(this);
    }
}

//--------------------------------------------------------------------------------------
// uri can't contain unsafe characters, for example, SPACE.
// Use URL encoded if nec.
//--------------------------------------------------------------------------------------
HRESULT bdHTTPClient::httpGet(const CHAR* serverName, const CHAR* uri, CHAR* content, DWORD contentLength, bdDownloadInterceptor* downloadHandler, DWORD port, bdUInt64 transactionID, bdUInt32 startByte, bdUInt32 endByte)
{
	m_operation = BD_HTTP_GET_OPERATION;
	
	// set the receive buffer
	m_buffer.init();
	m_buffer.setContentRecieveBuffer(content, contentLength);
	m_downloadHandler = downloadHandler;

	// set the request string
	bdInt written;
	if (endByte != 0)
	{
		written = bdSnprintf(m_buffer.m_httpCommonBuffer, BD_XENON_MAX_PACKET_PAYLOAD, 
			"GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%I32u-%I32u\r\nX-TransactionID: %I64u\r\n\r\n", uri, serverName, startByte, endByte, transactionID);
	}
	else if (startByte != 0)
	{
		written = bdSnprintf(m_buffer.m_httpCommonBuffer, BD_XENON_MAX_PACKET_PAYLOAD, 
			"GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%I32u-\r\nX-TransactionID: %I64u\r\n\r\n", uri, serverName, startByte, transactionID);
	}
	else
	{
		written = bdSnprintf(m_buffer.m_httpCommonBuffer, BD_XENON_MAX_PACKET_PAYLOAD, 
			"GET %s HTTP/1.1\r\nHost: %s\r\nX-TransactionID: %I64u\r\n\r\n", uri, serverName, transactionID);
	}
	
	m_buffer.m_httpCommonBufferSize = written;
	
	if (written <= 0)
	{
		BD_ERR(BD_LOG_LEVEL,"Error writing http request string - is request string greater than %d ?", BD_XENON_MAX_PACKET_PAYLOAD);
		setStatus(BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	return startHttpOperation(serverName, port);
}

//--------------------------------------------------------------------------------------
// uri can't contain unsafe characters, for example, SPACE.
// Use URL encoded if nec.
//--------------------------------------------------------------------------------------
HRESULT bdHTTPClient::httpPut(const CHAR* serverName, const CHAR* uri, CHAR* content, DWORD contentLength, bdUploadInterceptor* uploadHandler, DWORD port, bdUInt64 transactionID)
{
	m_operation = BD_HTTP_PUT_OPERATION;

	// set the send buffer
	m_buffer.init();
	m_buffer.setContentSendBuffer(content, contentLength);
	m_uploadHandler = uploadHandler;

	bdInt written = bdSnprintf(m_buffer.m_httpCommonBuffer, BD_XENON_MAX_PACKET_PAYLOAD,
		"PUT %s HTTP/1.1\r\nHost: %s:%d\r\nAccept: */*\r\nContent-Length: %d\r\nX-TransactionID: %I64u\r\n\r\n", 
		uri, serverName, port, contentLength, transactionID);

	if (written <= 0)
	{
		BD_ERR(BD_LOG_LEVEL,"Error writing http request string - is request string greater than %d ?", BD_XENON_MAX_PACKET_PAYLOAD);
		setStatus(BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	m_buffer.m_httpCommonBufferSize = written;

	return startHttpOperation(serverName, port);
}

//--------------------------------------------------------------------------------------
// uri can't contain unsafe characters, for example, SPACE.
// Use URL encoded if nec.
//--------------------------------------------------------------------------------------
HRESULT bdHTTPClient::httpDelete(const CHAR* serverName, const CHAR* uri, DWORD port, bdUInt64 transactionID)
{
	m_operation = BD_HTTP_DELETE_OPERATION;

	// set the send buffer
	m_buffer.init();

	// set the request string
	bdInt written = bdSnprintf(m_buffer.m_httpCommonBuffer, BD_XENON_MAX_PACKET_PAYLOAD,
		"DELETE %s HTTP/1.1\r\nHost: %s:%d\r\nX-TransactionID: %I64u\r\n\r\n", uri, serverName, port, transactionID);

	if (written <= 0)
	{
		BD_ERR(BD_LOG_LEVEL,"Error writing http request string");
		setStatus(BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	m_buffer.m_httpCommonBufferSize = written;

	return startHttpOperation(serverName, port);
}

//--------------------------------------------------------------------------------------
// uri can't contain unsafe characters, for example, SPACE.
// Use URL encoded if nec.
//--------------------------------------------------------------------------------------
HRESULT bdHTTPClient::httpPost(const CHAR* serverName, const CHAR* uri, CHAR* content, DWORD contentLength, bdUploadInterceptor* uploadHandler, DWORD port, bdUInt64 transactionID)
{
	m_operation = BD_HTTP_POST_OPERATION;

	// set the send buffer
	m_buffer.init();
	m_buffer.setContentSendBuffer(content, contentLength);
	m_uploadHandler = uploadHandler;

	// set the request string
	bdInt written = bdSnprintf(m_buffer.m_httpCommonBuffer, BD_XENON_MAX_PACKET_PAYLOAD,
		"POST %s HTTP/1.1\r\nHost: %s:%d\r\nAccept: */*\r\nContent-Length: %d\r\nX-TransactionID: %I64u\r\n\r\n", 
		uri, serverName, port, contentLength, transactionID);

	if (written <= 0)
	{
		BD_ERR(BD_LOG_LEVEL,"Error writing http request string - is request string greater than %d ?", BD_XENON_MAX_PACKET_PAYLOAD);
		setStatus(BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	m_buffer.m_httpCommonBufferSize = written;

	return startHttpOperation(serverName, port);
}

HRESULT bdHTTPClient::startHttpOperation(const CHAR* serverName, DWORD port)
{
	if(getStatus() == BD_HTTP_STATUS_BUSY)
	{
		BD_ERR(BD_LOG_LEVEL, "Only one http operation may be processed at a time.");
		setStatus(BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	m_dataRate = 0.0f;
	m_bytesTransfered = 0;
	m_abort = false;

	setStatus(bdHTTPClient::BD_HTTP_STATUS_BUSY);

	// set the server name
	bdInt written = bdSnprintf(m_serverName, BD_HTTP_HOST_IP_STRING_LENGTH, serverName);

	if (written <= 0)
	{
		BD_ERR(BD_LOG_LEVEL,"Error writing http server name - is server name length greater than %d ?", BD_HTTP_HOST_IP_STRING_LENGTH);
		setStatus(bdHTTPClient::BD_HTTP_STATUS_ERROR);
		return E_FAIL;
	}

	// set the server port
	m_port = port;

	return sendCommand();
}

void bdHTTPClient::finalize()
{
	if (m_hThread != 0)
	{
		WaitForSingleObject(m_hThread,INFINITE);
		CloseHandle(m_hThread);
		m_hThread = 0;
	}
}
