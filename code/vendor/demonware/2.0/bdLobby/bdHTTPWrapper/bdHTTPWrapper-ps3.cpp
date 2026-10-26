// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdHTTPWrapper/bdHTTPWrapper.h>
#include <bdLobby/bdContentStreaming/bdUploadInterceptor.h>
#include <bdLobby/bdContentStreaming/bdDownloadInterceptor.h>
#include <bdCore/bdMemory/bdMemory.h>

#define BD_LOG_LEVEL "bdHTTP"
#define BD_HTTP_THREAD_STACKSIZE 1024 * 32

static char cacert_pem[] __attribute__((aligned(64))) =
"-----BEGIN CERTIFICATE-----\n"
"MIIDmTCCAwKgAwIBAgIJAMORRmKSfWy0MA0GCSqGSIb3DQEBBQUAMIGQMQswCQYD\n"
"VQQGEwJVUzELMAkGA1UECBMCTlkxCzAJBgNVBAcTAk5ZMRMwEQYDVQQKEwpBY3Rp\n"
"dmlzaW9uMRIwEAYDVQQLEwlEZW1vbndhcmUxHDAaBgNVBAMTE2RnMTgyLmRlbW9u\n"
"d2FyZS5uZXQxIDAeBgkqhkiG9w0BCQEWEXN5c0BkZW1vbndhcmUubmV0MB4XDTA5\n"
"MDQwODE1MzUyNFoXDTEwMDQwODE1MzUyNFowgZAxCzAJBgNVBAYTAlVTMQswCQYD\n"
"VQQIEwJOWTELMAkGA1UEBxMCTlkxEzARBgNVBAoTCkFjdGl2aXNpb24xEjAQBgNV\n"
"BAsTCURlbW9ud2FyZTEcMBoGA1UEAxMTZGcxODIuZGVtb253YXJlLm5ldDEgMB4G\n"
"CSqGSIb3DQEJARYRc3lzQGRlbW9ud2FyZS5uZXQwgZ8wDQYJKoZIhvcNAQEBBQAD\n"
"gY0AMIGJAoGBALIFfXS7UEEyMeYrq+HRueaViJoeeUB7Tk0TDNAF8p0A0dCv+N1v\n"
"39OX/iLn1xL8U3OBZsXGVqtLuPrxhLTcx8yQoCUQDKA/LVgDSo5MRIlrp/LNu419\n"
"YS2QjmhicwgbhcsZCKl83ImBsFivB90X+7ukSlRbU5Vj5bfcqD4jmMm9AgMBAAGj\n"
"gfgwgfUwHQYDVR0OBBYEFNC5kI5MCyHRXGuUcjKQEzEhgj3NMIHFBgNVHSMEgb0w\n"
"gbqAFNC5kI5MCyHRXGuUcjKQEzEhgj3NoYGWpIGTMIGQMQswCQYDVQQGEwJVUzEL\n"
"MAkGA1UECBMCTlkxCzAJBgNVBAcTAk5ZMRMwEQYDVQQKEwpBY3RpdmlzaW9uMRIw\n"
"EAYDVQQLEwlEZW1vbndhcmUxHDAaBgNVBAMTE2RnMTgyLmRlbW9ud2FyZS5uZXQx\n"
"IDAeBgkqhkiG9w0BCQEWEXN5c0BkZW1vbndhcmUubmV0ggkAw5FGYpJ9bLQwDAYD\n"
"VR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOBgQB3COvzfhQsqS9SJ9vRcQmE4an3\n"
"jyAneRt2Ls5EVfTBc5XoGpOTbFY7xPIKkC3YQzIXrX5+aFc6xSYiU1vh5ou3SW+z\n"
"P97DRE7vr5wykSMZXCtPBTzWFyN6goR+pLExaGBWmZZNQUm1L+m5jADt3m7y2vUS\n"
"XyutD+nvX0+5xzSSlg==\n"
"-----END CERTIFICATE-----\n";

static int32_t ssl_cb(uint32_t verifyErr, CellSslCert const sslCert[], int certNum, const char *hostname, CellHttpSslId id, void *arg);

bdHTTPWrapper::bdHTTPWrapper() :
m_abortOperation(false),
m_operationBuffer(BD_NULL),
m_operationBufferSize(0),
m_transfered(0),
m_transferSpeed(0.0f),
m_downloadHandler(BD_NULL),
m_uploadHandler(BD_NULL),
m_operation(BD_IDLE),
m_thread(BD_NULL)
{
	m_errorBuffer[0] = BD_NULL;
	loadCertificate();
	init();
	
}

void bdHTTPWrapper::init()
{
	bdInt errorCode;
	errorCode = cellHttpCreateClient(&m_clientID);
	if (errorCode < 0 )
	{ 
		m_status = BD_FAILED;
		BD_ERR(BD_LOG_LEVEL, "Failed to create client, error code %x\n", errorCode);
	}
	errorCode = cellHttpClientSetSslCallback(m_clientID, ssl_cb, NULL);
	if (errorCode < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "unable to set ssl callback... (0x%x)\n", errorCode);
	}
}

bdHTTPWrapper::~bdHTTPWrapper()
{
	if (m_thread != BD_NULL)
	{
		abortOperation();
	}

	finalize();

	bdInt errorCode;
	errorCode = cellHttpDestroyClient(m_clientID);
	if (errorCode < 0 )
	{ 
		m_status = BD_FAILED;
		BD_ERR(BD_LOG_LEVEL, "Failed fo destroy client, error code %x\n", errorCode);
	}
	cellHttpsEnd();
}

void bdHTTPWrapper::abortOperation()
{
	m_abortOperation = true;
}

bdHTTPWrapper::bdStatus bdHTTPWrapper::_startDownload(const bdNChar8* downloadURL, void* downloadBuffer, bdUInt32 bufferSize, bdDownloadInterceptor* downloadHandler, bdUInt64 transactionID, bdUInt32 startRange, bdUInt32 endRange)
{
	m_status = BD_FAILED_TO_START;
	m_operationBufferSize = bufferSize;
	m_operationBuffer = (bdUByte8*)(downloadBuffer);
	m_downloadHandler = downloadHandler;
	m_httpErrorCode = 0;
	m_abortOperation = false;

	size_t URIPoolSize;
	bdBool ok = true;
	bdInt errorCode;

	errorCode = cellHttpUtilParseUri(NULL, downloadURL, NULL, 0, &URIPoolSize);
	if (errorCode < 0)
	{ 
		m_status = BD_FAILED;
		BD_ERR(BD_LOG_LEVEL, "Failed fo parse URL %s, erro code %x\n", downloadURL, errorCode);
		ok = false;
	}

	void* URIPool = NULL;

	if (ok)
	{
		URIPool = bdMemory::allocate(URIPoolSize);

		// parse URI twice to actually retrieve it
		errorCode = cellHttpUtilParseUri(&m_getURI, downloadURL, URIPool, URIPoolSize, NULL);
		if (errorCode < 0 )
		{ 
			BD_ERR(BD_LOG_LEVEL, "Failed fo parse URL %s, error code %x\n", downloadURL, errorCode);
			ok = false;
		}
	}

	if (ok)
	{ 
		// add the (optional) demonware transaction ID as a HTTP header 
		bdNChar8 xtransactionID[25];
		bdSnprintf(xtransactionID, 25, "%I64u", transactionID);

		CellHttpHeader header;
		header.name = "X-TransactionID";
		header.value = xtransactionID;

		errorCode = cellHttpClientSetHeader(m_clientID, &header);
		if (errorCode < 0 )
		{ 
			BD_ERR(BD_LOG_LEVEL, "Failed to set header X-TransactionID to %I64u, error code %x\n", transactionID, errorCode);
		}
	}

	if (ok && (startRange != 0 || endRange !=0))
	{ 
		// add the (optional) range HTTP header 
		bdNChar8 range[25];
		if (endRange !=0)
		{
			bdSnprintf(range, 25, "%I32u-%I32u", startRange, endRange);
		}
		else
		{
			bdSnprintf(range, 25, "%I32u-", startRange);
		}
		
		CellHttpHeader header;
		header.name = "range";
		header.value = range;

		errorCode = cellHttpClientSetHeader(m_clientID, &header);
		if (errorCode < 0 )
		{ 
			BD_ERR(BD_LOG_LEVEL, "Failed to set header 'range' to %I32u-%I32u, error code %x\n", startRange, endRange, errorCode);
		}
	}
		
	if (ok)
	{ 
		errorCode = cellHttpCreateTransaction(&m_transaction, m_clientID, CELL_HTTP_METHOD_GET, &m_getURI);
		if (errorCode < 0)
		{
			m_status = BD_FAILED;
			// couldn't create transaction
			BD_ERR(BD_LOG_LEVEL, "Failed to create transaction, errorCode = %x \n", errorCode);
			ok = false;
		}
	}

	if (ok)
	{
		m_operation = BD_DOWNLOAD;
		m_status = BD_PENDING;
		m_thread = new bdThread(this, BD_DEFAULT_THREAD_PRIORITY, BD_HTTP_THREAD_STACKSIZE);
		ok = m_thread->start(BD_NULL, 0);
	}

	if (URIPool)
	{
		bdMemory::deallocate(URIPool);
	}
	return m_status;
}


bdHTTPWrapper::bdStatus bdHTTPWrapper::getStatus()
{
	if (m_operation == BD_FINALIZE)
	{
		finalize();
	}
	return m_status;
}


bdHTTPWrapper::bdStatus bdHTTPWrapper::_startUpload(const bdNChar8* uploadURL, const void* fileData, bdUploadInterceptor* uploadHandler, bdUInt32 bufferSize, bdUInt64 transactionID)
{
	m_operationBuffer = (bdUByte8*)(fileData);
	m_operationBufferSize = bufferSize;
	m_uploadHandler = uploadHandler;
	m_httpErrorCode = 0;
	m_abortOperation = false;

	size_t URIPoolSize;
	bdBool ok = true;
	bdInt errorCode;

	m_status = BD_FAILED_TO_START;
	errorCode = cellHttpUtilParseUri(NULL, uploadURL, NULL, 0, &URIPoolSize);
	if (errorCode < 0)
	{ 
		m_status = BD_FAILED;
		BD_ERR(BD_LOG_LEVEL, "Failed fo parse URL %s, error code %x\n", uploadURL, errorCode);
		ok = false;
	}

	void* URIPool = NULL;
	
	if (ok)
	{
		URIPool = bdMemory::allocate(URIPoolSize);

		// parse URI twice to actually retrieve it
		errorCode = cellHttpUtilParseUri(&m_getURI, uploadURL, URIPool, URIPoolSize, NULL);
		if (errorCode < 0 )
		{ 
			BD_ERR(BD_LOG_LEVEL, "Failed fo parse URL %s, error code %x\n", uploadURL, errorCode);
			ok = false;
		}
	}

	if (ok)
	{ 
		// add the (optional) demonware transaction ID as a HTTP header 
		bdNChar8 xtransactionID[25];
		bdSnprintf(xtransactionID, 25, "%I64u", transactionID);

		CellHttpHeader header;
		header.name = "X-TransactionID";
		header.value = xtransactionID;

		errorCode = cellHttpClientSetHeader(m_clientID, &header);
		if (errorCode < 0 )
		{ 
			BD_ERR(BD_LOG_LEVEL, "Failed to set header X-TransactionID to %I64u, error code %x\n", transactionID, errorCode);
		}
	}

	if (ok)
	{ 
		errorCode = cellHttpCreateTransaction(&m_transaction, m_clientID, CELL_HTTP_METHOD_PUT, &m_getURI);
		if (errorCode < 0)
		{
			m_status = BD_FAILED;
			// couldn't create transaction
			BD_ERR(BD_LOG_LEVEL, "Failed to create transaction, errorCode = %x \n", errorCode);
			ok = false;
		}
	}

	if (ok)
	{
		errorCode = cellHttpRequestSetContentLength(m_transaction, bufferSize);

		if (errorCode < 0)
		{
			m_status = BD_FAILED;
			BD_ERR(BD_LOG_LEVEL, "Failed to set content length, errorCode = %x \n", errorCode);
			ok = false;
		}
	}

	if (ok)
	{
		m_operation = BD_UPLOAD;
		m_status = BD_PENDING;
		m_thread = new bdThread(this, BD_DEFAULT_THREAD_PRIORITY, BD_HTTP_THREAD_STACKSIZE);
		ok = m_thread->start(BD_NULL, 0);
		m_status = BD_PENDING;
	}
	
	if (URIPool)
	{
		bdMemory::deallocate(URIPool);
	}
	return m_status;
}

bdHTTPWrapper::bdStatus bdHTTPWrapper::startDelete(const bdNChar8* deleteURL, bdUInt64 transactionID)
{
	m_status = BD_FAILED_TO_START;
	m_httpErrorCode = 0;
	m_abortOperation = false;

	size_t URIPoolSize;
	bdBool ok = true;
	bdInt errorCode;

	m_status = BD_FAILED_TO_START;
	errorCode = cellHttpUtilParseUri(NULL, deleteURL, NULL, 0, &URIPoolSize);
	if (errorCode < 0)
	{ 
		m_status = BD_FAILED;
		BD_ERR(BD_LOG_LEVEL, "Failed fo parse URL %s, erro code %x\n", deleteURL, errorCode);
		ok = false;
	}

	void* URIPool = NULL;
	
	if (ok)
	{
		URIPool = bdMemory::allocate(URIPoolSize);

		// parse URI twice to actually retrieve it
		errorCode = cellHttpUtilParseUri(&m_getURI, deleteURL, URIPool, URIPoolSize, NULL);
		if (errorCode < 0 )
		{ 
			BD_ERR(BD_LOG_LEVEL, "Failed fo parse URL %s, error code %x\n", deleteURL, errorCode);
			ok = false;
		}
	}

	if (ok)
	{ 
		// add the (optional) demonware transaction ID as a HTTP header 
		bdNChar8 xtransactionID[25];
		bdSnprintf(xtransactionID, 25, "%I64u", transactionID);

		CellHttpHeader header;
		header.name = "X-TransactionID";
		header.value = xtransactionID;

		errorCode = cellHttpClientSetHeader(m_clientID, &header);
		if (errorCode < 0 )
		{ 
			BD_ERR(BD_LOG_LEVEL, "Failed to set header X-TransactionID to %I64u, error code %x\n", transactionID, errorCode);
		}
	}

	if (ok)
	{ 
		errorCode = cellHttpCreateTransaction(&m_transaction, m_clientID, CELL_HTTP_METHOD_DELETE, &m_getURI);
		if (errorCode < 0)
		{
			m_status = BD_FAILED;
			// couldn't create transaction
			BD_ERR(BD_LOG_LEVEL, "Failed to create transaction, errorCode = %x \n", errorCode);
			ok = false;
		}
	}

	if (ok)
	{
		m_operation = BD_DELETE;
		m_status = BD_PENDING;
		m_thread = new bdThread(this, BD_DEFAULT_THREAD_PRIORITY, BD_HTTP_THREAD_STACKSIZE);
		ok = m_thread->start(BD_NULL, 0);
		m_status = BD_PENDING;
	}
	
	if (URIPool)
	{
		bdMemory::deallocate(URIPool);
	}
	return m_status;
}

bdUInt32 bdHTTPWrapper::getTransferProgress()const
{
	return m_transfered;
}

bdFloat32 bdHTTPWrapper::getTransferSpeed()
{
	return m_transferSpeed;
}

void bdHTTPWrapper::resetTransferStats()
{
	// there's nothing to do on PS3
}


bdInt32 bdHTTPWrapper::getLastHTTPError() const
{
	if (m_status == BD_PENDING)
	{
		return 0;
	}

	return m_httpErrorCode;
}

 bdUInt bdHTTPWrapper::run(void* args)
 {
	bdBool ok = true;
	bdInt errorCode = 0;

	bdStopwatch stopwatch;
	stopwatch.reset();
	stopwatch.start();
	m_transfered = 0;
	m_transferSpeed = 0.0f;

    switch (m_operation)
	{
        case BD_UPLOAD:
		{
			bdBool done = false;
		    bdUInt bytesSent = 0;
            bdUInt bytesToSend = m_operationBufferSize;
			while(ok && !done)
			{
				if (m_uploadHandler != BD_NULL)
				{
					// query client code for data chunk
					bdUInt bytesToSendNow = m_uploadHandler->handleUpload(m_httpCommonBuffer, BD_HTTP_COMMON_BUFFER_SIZE, m_transfered);
					
					if (bytesToSendNow > 0)
					{
						errorCode  = cellHttpSendRequest(m_transaction, m_httpCommonBuffer, bytesToSendNow, &bytesSent);
					}
					else
					{
						BD_INFO(BD_LOG_LEVEL,"Upload complete\n");
						done = true;
					}
				}
				else
				{
					// copy data chunk from client buffer
					m_operationBuffer += bytesSent;
					bytesToSend -= bytesSent;
					errorCode  = cellHttpSendRequest(m_transaction, m_operationBuffer, bytesToSend, &bytesSent);

					if (bytesSent >= bytesToSend)
					{
						BD_INFO(BD_LOG_LEVEL,"Upload complete\n");
						done = true;
					}
				}

				if (errorCode < 0)
				{
					m_status = BD_FAILED;
					BD_ERR(BD_LOG_LEVEL, "Failed to send Http request: %x \n", errorCode);
					ok = false;
				}
				else
				{
					m_transfered += bytesSent;
				}

				if (stopwatch.getElapsedTimeInSeconds() > 1.0f)
				{
					m_transferSpeed = static_cast<bdFloat32>(bytesSent) / stopwatch.getElapsedTimeInSeconds();
					BD_INFO(BD_LOG_LEVEL,"Sent %i bytes (%.2f)\n", bytesSent, m_transferSpeed);
				}

				if (m_abortOperation)
				{
					m_status = BD_CANCELLED;
					ok = false;
				}

				BD_INFO(BD_LOG_LEVEL, "Uploaded %u bytes", bytesSent);

			}
			if (ok)
			{
				errorCode = cellHttpResponseGetStatusCode(m_transaction, &m_httpErrorCode);
				if (m_httpErrorCode < 0)
				{
					BD_ERR(BD_LOG_LEVEL, "Failed to get status code%x \n", errorCode);
					m_status = BD_FAILED;
					ok = false;
				}
				switch (m_httpErrorCode)
				{
					case 200:
					case 201:
					{
						BD_INFO(BD_LOG_LEVEL, "Finished uploading file");
						m_transferSpeed = m_transfered / stopwatch.getElapsedTimeInSeconds();
						m_status = BD_DONE;
					}
					break;
					default:
					{
						BD_INFO(BD_LOG_LEVEL, "Status code %i", m_httpErrorCode);
						m_status = BD_FAILED;
					}
				}
			}
			if (ok)
			{
				bdUInt responseSize;
				errorCode = cellHttpRecvResponse(m_transaction, m_errorBuffer, BD_HTTP_RESPONSE_BUF_SIZE, &responseSize);

				if (errorCode < 0)
				{
					BD_ERR(BD_LOG_LEVEL, "Failed to get HTTP response%x \n", errorCode);
					m_status = BD_FAILED;
					ok = false;
				}
				if (responseSize > 0)
				{
					BD_INFO(BD_LOG_LEVEL, "%s", m_errorBuffer);
				}
			}
	
		}
		break;
		case BD_DOWNLOAD:
		{
			if(ok)
			{
				errorCode  = cellHttpSendRequest(m_transaction, NULL, 0, NULL);
				if (errorCode < 0)
				{
					m_status = BD_FAILED;
					BD_ERR(BD_LOG_LEVEL, "Failed to send Http request: %x \n", errorCode);
					ok = false;
				}
			}

			if (ok)
			{
				errorCode = cellHttpResponseGetStatusCode(m_transaction, &m_httpErrorCode);
				if (errorCode < 0)
				{
					m_status = BD_FAILED;
					BD_ERR(BD_LOG_LEVEL, "Failed to get Http response: %x \n", errorCode);
					ok = false;
				}

				if (m_httpErrorCode != 200)
				{
					m_status = BD_FAILED;
					// file was not found
					BD_ERR(BD_LOG_LEVEL, "File not found: %i \n", m_httpErrorCode);
					ok = false;
				}
			}

			// get content lenght
			bdUInt64 contentLength = 0;
			if (ok)
			{
				int contentLenRetval = cellHttpResponseGetContentLength(m_transaction, &contentLength);

				if (contentLenRetval < 0)
				{
					m_status = BD_FAILED;
					ok = false;
				}
				if (contentLenRetval == CELL_HTTP_ERROR_NO_CONTENT_LENGTH)
				{
					contentLength = 0;
				}

				if (m_downloadHandler == BD_NULL && contentLength > m_operationBufferSize)
				{
					m_status = BD_FAILED;
					BD_ERR(BD_LOG_LEVEL, "Not enough buffer space to download content\n");
					ok = false;
				}
			}

			if (ok && contentLength > 0)
			{
				m_transfered = 0;
				while (ok && m_transfered < contentLength)
				{
					size_t receivedThisIter = 0;
					bdInt errorCode;

					if (m_downloadHandler != BD_NULL)
					{
						// download to local buffer and pass data to client 
						errorCode = cellHttpRecvResponse(m_transaction, m_httpCommonBuffer, BD_HTTP_COMMON_BUFFER_SIZE, &receivedThisIter); 
						if (m_downloadHandler->handleDownload(m_httpCommonBuffer, static_cast<bdUInt32>(receivedThisIter)) < 0)
						{
							m_abortOperation = true;
						}					
					}
					else
					{
						errorCode = cellHttpRecvResponse(m_transaction, m_operationBuffer + m_transfered, contentLength - m_transfered, &receivedThisIter); 
					}

					if ( errorCode < 0)
					{
						m_status = BD_FAILED;
						ok = false;
					}
					else
					{
						m_transfered += receivedThisIter;
						if (stopwatch.getElapsedTimeInSeconds() > 0.1f)
						{
							m_transferSpeed = m_transfered / stopwatch.getElapsedTimeInSeconds();
						}
					}
					if (m_abortOperation)
					{
						m_status = BD_CANCELLED;
						ok = false;
					}
				}

				m_transferSpeed = m_transfered / stopwatch.getElapsedTimeInSeconds();
				m_status = BD_DONE;
			}
		}
		break;
		case BD_DELETE:
		{
			if(ok)
			{
				errorCode  = cellHttpSendRequest(m_transaction, NULL, 0, NULL);
				if (errorCode < 0)
				{
					m_status = BD_FAILED;
					BD_ERR(BD_LOG_LEVEL, "Failed to send Http request: %x \n", errorCode);
					ok = false;
				}
				else
				{
					m_status = BD_PENDING;
				}
			}

			if (ok)
			{
				errorCode = cellHttpResponseGetStatusCode(m_transaction, &m_httpErrorCode);
				if (errorCode < 0)
				{
					m_status = BD_FAILED;
					BD_ERR(BD_LOG_LEVEL, "Failed to get Http response: %i \n", errorCode);
					ok = false;
				}

				if (m_httpErrorCode != 204)
				{
					m_status = BD_FAILED;
					// file was not found
					BD_ERR(BD_LOG_LEVEL, "File not found: %i \n", m_httpErrorCode);
					ok = false;
				}
				else
				{
					m_status = BD_DONE;
				}
			}
		}
		break;
		default:
		{
			BD_WARN("warn", "Illegal operation");
			break;
		}

	}
	if (m_transaction)
	{
		cellHttpDestroyTransaction(m_transaction);
		m_transaction = 0;
	}
	m_operation = BD_FINALIZE;
	return 0;
 }


void bdHTTPWrapper::finalize()
{
	if(m_thread != BD_NULL)
	{
		m_thread->join();
		m_thread->cleanup();
		m_thread = BD_NULL;
	}
	m_operation = BD_IDLE;
	m_operationBufferSize = 0;
	m_operationBuffer = BD_NULL;
 }

bdInt bdHTTPWrapper::loadCertificate()
{
	bdInt ret = 0;

	m_httpsData[0].ptr = cacert_pem;
	m_httpsData[0].size = sizeof(cacert_pem);

	ret = cellHttpsInit(1, m_httpsData);
	if (ret < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "unable to start https... (0x%x)\n", ret);
	}

	return ret;

}

bdBool bdHTTPWrapper::isOkayToStart()
{ 
	// call finalize if nec:
	getStatus();

	if (m_operation == BD_IDLE)
	{
		return true;
	}
	else
	{
		return false;
	}
}
static int32_t ssl_cb(uint32_t verifyErr, CellSslCert const sslCert[], int certNum, const char *hostname, CellHttpSslId id, void *arg)
{

	// #if SSL_CERT_IGNORE_DATE_CHECK
	verifyErr &= ~CELL_HTTPS_VERIFY_ERROR_EXPIRED;
	verifyErr &= ~CELL_HTTPS_VERIFY_ERROR_NOT_YET_VALID;

	// Ignore name check
	verifyErr &= ~CELL_HTTPS_VERIFY_ERROR_COMMON_NAME;

	BD_INFO(BD_LOG_LEVEL, "Verified hostname: %s", hostname);
	if (verifyErr != 0)
	{
		BD_WARN(BD_LOG_LEVEL, "Certificate verification error=0x%x", verifyErr);
	}
	return(verifyErr);
}

