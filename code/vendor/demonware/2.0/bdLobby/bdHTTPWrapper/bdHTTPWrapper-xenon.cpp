// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdHTTPWrapper/bdHTTPWrapper.h>
#include <bdCore/bdMemory/bdMemory.h>
#include <Xonline.h>


#ifdef BD_PLATFORM_XENON

#define BD_LOG_LEVEL "bdHttpWrapper-xenon"

#define BD_MAX_SERVER_NAME_SIZE 256
#define MAX_LSP_DESC_TO_RETRIEVE 10u

// timeout connection handshake after 60 seconds
#define BD_HTTP_CONNECTION_TIMEOUT 60

bool bdHTTPWrapper::s_abortOperation = false;

bdHTTPWrapper::bdHTTPWrapper() :
m_status(BD_EMPTY), 
m_operation(BD_IDLE), 
m_httpErrorCode(0), 
m_dataRate(0.0f),
m_httpClient(BD_NULL)
{
	m_errorBuffer[0] = BD_NULL;
}

void bdHTTPWrapper::init(DWORD lspServiceID, bdBool translateServiceNameToIP)
{
	if (m_httpClient == BD_NULL)
	{
		m_httpClient = new bdHTTPClient(lspServiceID, TRUE);
	}
	else
	{
		m_httpClient->setLspServiceID(lspServiceID);
	}

	m_translateServiceNameToIP = translateServiceNameToIP;

	if (m_translateServiceNameToIP)
	{
		XTITLE_SERVER_INFO resultsa[MAX_LSP_DESC_TO_RETRIEVE];
		HANDLE serverEnumHandle = BD_NULL;
		DWORD serverCount = 0;
		DWORD itemBufferRequiredSize = 0;
		const DWORD createResult = XTitleServerCreateEnumerator("", MAX_LSP_DESC_TO_RETRIEVE, &itemBufferRequiredSize, &serverEnumHandle);
		
		if(ERROR_SUCCESS == createResult)
		{
			BD_INFO(BD_LOG_LEVEL , "Title Server enumerator successfully created.");
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to enumerate LSP servers, HTTP access will not be available. 0x%0x.", createResult);
			CloseHandle(serverEnumHandle);
			return;
		}

		if(sizeof(resultsa) >= itemBufferRequiredSize)
		{
			// Enumerate the title servers
			const DWORD enumerateResult = XEnumerate(serverEnumHandle, resultsa, sizeof(resultsa), &serverCount, NULL);
			if(ERROR_SUCCESS == enumerateResult)
			{
				BD_INFO(BD_LOG_LEVEL , "Title Servers successfully enumerated.");
				// We should find at least one server
				if (serverCount < 1)
				{
					BD_ERR(BD_LOG_LEVEL, "Got no results for LSP servers query, HTTP access will not be available.");
				}
			}
			else if (ERROR_NO_MORE_FILES == enumerateResult)
			{
				BD_ERR(BD_LOG_LEVEL, "No more files ???");
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to enumerate servers, HTTP access will not be available. 0x%0x.");
			}
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Insufficient memory allocated, cannot safely call XEnumerate:  Mem req: %u Bytes, allocated %u Bytes", itemBufferRequiredSize , sizeof(resultsa));
		}

		for (bdUInt i(0); i<serverCount; i++)
		{
			bdString lspDesc(resultsa[i].szServerInfo);
			IN_ADDR ip = resultsa[i].inaServer;
			
			bdNChar8 lspIP[17];
			bdSnprintf(lspIP, 17, "%d.%d.%d.%d\0", ip.S_un.S_un_b.s_b1, ip.S_un.S_un_b.s_b2, ip.S_un.S_un_b.s_b3, ip.S_un.S_un_b.s_b4);
			
			bdArray<bdString> ipList;
			if (!m_lspDescToIP.remove(lspDesc, ipList))
			{
				ipList.pushBack(bdString(lspIP));
				BD_INFO(BD_LOG_LEVEL, "Adding mapping from %s to %s", resultsa[i].szServerInfo, lspIP);

				if (!m_lspDescToIP.put(lspDesc, ipList))
				{
					BD_ERR(BD_LOG_LEVEL, "Failed to placing mapping from LSP description to IP into map, HTTP access may not be available.");
				}
			}
			else
			{
				// add an element to the arrary
				ipList.pushBack(bdString(lspIP));
				BD_INFO(BD_LOG_LEVEL, "Adding mapping from %s to %s", resultsa[i].szServerInfo, lspIP);

				if (!m_lspDescToIP.put(lspDesc, ipList))
				{
					BD_ERR(BD_LOG_LEVEL, "Failed to placing mapping from LSP description to IP into map, HTTP access may not be available.");
				}
			}
		}

		CloseHandle(serverEnumHandle);
	}
}

bdHTTPWrapper::~bdHTTPWrapper()
{
	if (m_httpClient != BD_NULL)
	{
		delete m_httpClient;
	}
}

bdUInt bdHTTPWrapper::run(void*)
{
	m_status = BD_DONE;	
	return 0;
}

void bdHTTPWrapper::resetTransferStats()
{
	if (m_httpClient)
	{
		m_httpClient->setBytesTransfered(0);
		m_httpClient->setDataRate(0.0f);
	}
}

bdHTTPWrapper::bdStatus bdHTTPWrapper::_startDownload(const bdNChar8 *const downloadURL, void *const downloadBuffer, bdUInt32 bufferSize, bdDownloadInterceptor* downloadHandler, bdUInt64 transactionID, bdUInt32 startByte, bdUInt32 endByte)
{
	if (m_httpClient == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "bdHTTPWrapper::init must be called before before calling a HTTP operation");
		return BD_FAILED;
	}

	bdNChar8 serverName[BD_MAX_SERVER_NAME_SIZE];
	bdNChar8 *startFileName = BD_NULL;
	bdUInt32 port;
	
	if (!parseURL(downloadURL, reinterpret_cast<bdNChar8*>(serverName), &startFileName, port))
	{
		return BD_FAILED;
	}

	m_httpClient->httpGet(serverName, startFileName, static_cast<CHAR*>(downloadBuffer), bufferSize, downloadHandler, port, transactionID, startByte, endByte);	

	return BD_PENDING;
}

bdHTTPWrapper::bdStatus bdHTTPWrapper::_startUpload(const bdNChar8 *const uploadURL, const void *const fileData, bdUploadInterceptor* uploadHandler, bdUInt32 bufferSize, bdUInt64 transactionID)
{
	if (m_httpClient == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "bdHTTPWrapper::init must be called before before calling a HTTP operation");
		return BD_FAILED;
	}

	bdNChar8 serverName[BD_MAX_SERVER_NAME_SIZE];
	bdNChar8 *startFileName = BD_NULL;
	bdUInt32 port;
	
	if (!parseURL(uploadURL, reinterpret_cast<bdNChar8*>(serverName), &startFileName, port))
	{
		return BD_FAILED;
	}

	// httpPut can't maintain the const-ness as it caches a ptr to the data
	void* fileData2 = const_cast<void*>(fileData);
	m_httpClient->httpPut(serverName, startFileName, static_cast<CHAR*>(fileData2), bufferSize, uploadHandler, port, transactionID);	

	return BD_PENDING;
}

bdHTTPWrapper::bdStatus bdHTTPWrapper::startDelete(const bdNChar8 *const deleteURL, bdUInt64 transactionID)
{
	if (m_httpClient == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "bdHTTPWrapper::init must be called before before calling a HTTP operation");
		return BD_FAILED;
	}

	bdNChar8 serverName[BD_MAX_SERVER_NAME_SIZE];
	bdNChar8 *startFileName = BD_NULL;
	bdUInt32 port;
	
	if (!parseURL(deleteURL, reinterpret_cast<bdNChar8*>(serverName), &startFileName, port))
	{
		return BD_FAILED;
	}

	m_httpClient->httpDelete(serverName, startFileName, port, transactionID);	

	return BD_PENDING;
}

bdBool bdHTTPWrapper::parseURL(const bdNChar8 *const URL, 
							   bdNChar8 *serverName, 
							   bdNChar8 **startFileName, 
							   bdUInt32 &port)
{
	// the URL is of form 'proto://serverName:port/fileName'
	// we need to extract the serverName, port and fileName components
	bdNChar8 *startServerName;
	bdNChar8 *endServerName;
	const bdNChar8 *notFound = reinterpret_cast<bdNChar8 *>(-1);

	if (!m_translateServiceNameToIP)
	{
		// check 'http' substring is present
		if (bdStrncmp(URL,"http",4) != 0)
		{
			BD_ERR(BD_LOG_LEVEL, "URL was not of expected form. expected: http(s)://serverName:port/resource\nactual: %s", URL);
			return false;
		}

		// find the server name by searching for the first ':'
		startServerName = const_cast<bdNChar8 *>(bdStrchr(URL, ':')) + 3;
		if (startServerName == notFound)
		{
			BD_ERR(BD_LOG_LEVEL, "URL was not of expected form. expected: proto://serverName:port/resource\nactual: %s", URL);
			return false;
		}
	}
	else
	{
		startServerName = const_cast<bdNChar8 *>(URL);
	}

	// find the resource name by searching for the '/' after the server name
	*startFileName = bdStrchr(startServerName, '/');
	if (*startFileName == notFound)
	{
		BD_ERR(BD_LOG_LEVEL, "URL was not of expected form. expected: proto://serverName:port/resource\nactual: %s", URL);
		return false;
	}

	// find the (optional) port by searching for ':' after the server name 
	endServerName = bdStrchr(startServerName, ':') - 1;
	if (endServerName == notFound)
	{
		// use the default
		port = 80;
		endServerName = *startFileName-1;
	}
	else
	{
		// check for bad parsings
		if (startServerName > endServerName || endServerName > *startFileName)
		{
			BD_ERR(BD_LOG_LEVEL, "URL was not of expected form. expected: proto://serverName:port/resource\nactual: %s", URL);
			return false;
		}

		port = atoi(endServerName+2);

		if (port <= 0)
		{
			BD_ERR(BD_LOG_LEVEL, "URL was not of expected form. expected: proto://serverName:port/resource\nactual: %s", URL);
			return false;
		}
	}

	if (!m_translateServiceNameToIP)
	{
		// extract server name
		bdStrlcpy(serverName, startServerName, endServerName - startServerName + 2);
	}
	else
	{
		// get server desc
		bdNChar8 serverDescBuf[128];
		bdUWord len = endServerName - startServerName + 2;
		bdStrlcpy(serverDescBuf, startServerName, len);
		serverDescBuf[len] = 0;
		bdString serverDesc(serverDescBuf);

		// get the equivalent IP for the server desc
		bdArray<bdString> serverIPs;
		m_lspDescToIP.get(serverDesc, serverIPs);

		// choose an IP at random from the returned array
		bdUInt numLSPs = serverIPs.getSize();
		if (numLSPs > 1)
		{
			bdUInt choice = bdRandom().nextUInt() % numLSPs;
			const bdNChar8* addr = serverIPs[choice].getBuffer();
			bdStrlcpy(serverName, addr, BD_MAX_SERVER_NAME_SIZE);
		}
		else if (numLSPs == 1)
		{
			const bdNChar8* addr = serverIPs[0].getBuffer();
			bdStrlcpy(serverName, addr, BD_MAX_SERVER_NAME_SIZE);
		}
		else
		{
			bdStrlcpy(serverName, "", 1);
			BD_ERR(BD_LOG_LEVEL, "error when accessing lsp desc to ip hash map");
			return false;
		}
	}

	return true;
}


bdHTTPWrapper::bdStatus  bdHTTPWrapper::getStatus()
{
	if (m_httpClient)
	{
		switch (m_httpClient->getStatus())
		{
			case bdHTTPClient::BD_HTTP_STATUS_BUSY:
				return BD_PENDING;
				break;
			case bdHTTPClient::BD_HTTP_STATUS_DONE:
			{
				m_httpClient->finalize();
				return BD_DONE;
				break;
			}
			case bdHTTPClient::BD_HTTP_STATUS_ERROR:
			{
				m_httpClient->finalize();
				return BD_FAILED;
				break;
			}
			default:
				BD_ERR(BD_LOG_LEVEL, "Unexpeceted BD_HTTP_STATUS state returned");
				break;
		}
	}
	// default
	return BD_EMPTY;
}

bdUInt32 bdHTTPWrapper::getTransferProgress() const
{
	if (m_httpClient)
	{
		return m_httpClient->getBytesTransfered();
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "No active HTTP task");
		return 0;
	}
}

bdFloat32 bdHTTPWrapper::getTransferSpeed()
{
	if (m_httpClient)
	{
		return m_httpClient->getDataRate();
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "No active HTTP task");
		return 0;
	}
}

bdInt32 bdHTTPWrapper::getLastHTTPError() const	
{
	if (m_httpClient)
	{
		return m_httpClient->getResponseCode();
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"No active HTTP task");
		return 0;
	}
}

void bdHTTPWrapper::abortOperation()
{ 
	if (m_httpClient)
	{
		m_httpClient->abortOperation();
		s_abortOperation = true; 
		m_status = BD_CANCELLED;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "No active HTTP task");
	}
}

bdBool bdHTTPWrapper::isOkayToStart()
{ 
	if (m_httpClient == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "bdHTTPWrapper::init must be called before before calling a HTTP operation");
		return false;
	}

	return true;
}

#endif
