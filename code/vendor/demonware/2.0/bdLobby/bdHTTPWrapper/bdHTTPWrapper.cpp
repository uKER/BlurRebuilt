// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdHTTPWrapper/bdHTTPWrapper.h>
#include <bdLobby/bdContentStreaming/bdDownloadInterceptor.h>
#include <bdLobby/bdContentStreaming/bdUploadInterceptor.h>
#include <bdCore/bdMemory/bdMemory.h>

// Set to the location of demonware's cert pack if using CURLOPT_SSL_VERIFYPEER 
#if (defined BD_PLATFORM_WIN32) 
#define SSL_CERT_BUNDLE_FULL_PATH "C:\\sslcerts\\DemonwareSSL.pem"
#elif (defined BD_PLATFORM_UNIX)
#define SSL_CERT_BUNDLE_FULL_PATH "/tmp/DemonwareSSLCerts/DemonwareSSL.pem"
#endif

#if ((defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX))

#include <curl/easy.h>
//#define _CRT_SECURE_NO_WARNINGS
//#include <openssl/applink.c>
//#undef _CRT_SECURE_NO_WARNINGS

#define BD_HTTP_THREAD_STACKSIZE 1024 * 32

// To enable/disable SSL support, set BD_USE_SSL
// Make sure that libcurl is built with USE_SSL when using SSL
#define BD_USE_SSL

// To print out detailed SSL cert info for debugging, set BD_DEBUG_SSL_CHAIN
#define BD_DEBUG_SSL_CHAIN 

// To alter the size of the upload buffer edit CURL_MAX_WRITE_SIZE in curl.h

// timeout connection handshake after 60 seconds
#define BD_HTTP_CONNECTION_TIMEOUT 60

// max time to wait between pumps
#define BD_LIBCURL_MAX_PUMP_INTERVAL 40



size_t bdHTTPWrapper::s_bytesTransfered = 0;
size_t bdHTTPWrapper::s_downloadBufferSize = 0;
size_t bdHTTPWrapper::s_uploadSize = 0;
bool bdHTTPWrapper::s_abortOperation = false;

bdHTTPWrapper::bdHTTPWrapper() :
m_headerList(BD_NULL),
m_uploading(false),
m_status(BD_EMPTY),
m_operation(BD_IDLE),
m_httpErrorCode(0),
m_thread(BD_NULL),
m_verboseOutput(false),
m_progressMeter(false)
{
	init();
}

void bdHTTPWrapper::init()
{
	m_errorBuffer[0] = BD_NULL;

#ifdef BD_USE_SSL
	curl_global_init_mem(CURL_GLOBAL_SSL,
		bdMemory::getAllocateFunc(),
		bdMemory::getDeallocateFunc(),
		bdMemory::getReallocateFunc(),
		&bdStrdup,
		&bdCallocate);

	curl_version_info_data * vinfo = curl_version_info( CURLVERSION_NOW );
	BD_ASSERT( (vinfo->features & CURL_VERSION_SSL) != 0, "BD_USE_SSL is defined but SSL support is not enabled in libcurl" );
#else
	curl_global_init_mem(0,
		bdMemory::getAllocateFunc(),
		bdMemory::getDeallocateFunc(),
		bdMemory::getReallocateFunc(),
		&bdStrdup,
		&bdCallocate);
#endif

}

bdHTTPWrapper::~bdHTTPWrapper()
{
	finalize();
	curl_global_cleanup();
}

bdUInt bdHTTPWrapper::run(void*)
{
	s_abortOperation = false;
	CURLcode err = curl_easy_perform(m_curl);  
	if (err != CURLE_OK)
	{
		m_status = BD_FAILED;

		const char* errStr = curl_easy_strerror(err);
		BD_ERR("http","HTTP operation returned CURL error %d: %s", err, errStr);
	}
	else
	{	
		m_httpErrorCode = 0;
		bdInt64 errorCode = 0;
		curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &errorCode);
		m_httpErrorCode = (bdInt32)errorCode;
		if (m_httpErrorCode < 200 || m_httpErrorCode >= 300)
		{
			m_status = BD_FAILED;

			if (bdStrlen(m_errorBuffer) > 0)
			{
				BD_ERR("http","HTTP operation returned HTTP error %d: %s", m_httpErrorCode, m_errorBuffer);
				m_errorBuffer[0] = BD_NULL;
			}
			else
			{
				BD_ERR("http","HTTP operation returned error %d", m_httpErrorCode);
			}
		}
		else
		{
			m_status = BD_DONE;
			BD_INFO("http","Transfer complete: %d bytes @ %.0f bytes/sec", s_bytesTransfered, m_dataRate);
		}

#ifdef BD_DEBUG_SSL_CHAIN
		curl_certinfo *certInfo = BD_NULL;
		CURLcode resInfo = curl_easy_getinfo(m_curl, CURLINFO_CERTINFO, &certInfo);

		if(!resInfo && certInfo) 
		{
			int i;
			BD_INFO("ssl","%d certs acquired\n", certInfo->num_of_certs);
	
			for(i=0; i<certInfo->num_of_certs; i++) 
			{
				struct curl_slist *slist;
				for(slist = certInfo->certinfo[i]; slist; slist = slist->next)
				{
					BD_INFO("ssl","%s\n", slist->data);
				}
			}
		}
#endif
	}

	// cleanup
	if (m_headerList != NULL)
	{
		curl_slist_free_all(m_headerList);
		m_headerList=NULL;
	}

	curl_easy_cleanup(m_curl);  
	
	m_operation = BD_FINALIZE;
		
	return 0;
}

bdHTTPWrapper::bdStatus bdHTTPWrapper::_startDownload(const bdNChar8 *const downloadURL, void *const downloadBuffer, bdUInt32 bufferSize, bdDownloadInterceptor* downloadHandler, bdUInt64 transactionID, bdUInt32 startByte, bdUInt32 endByte)
{
	m_status = BD_FAILED_TO_START;
	m_curl = curl_easy_init();  
	int ok(CURLE_OK);

	if (m_headerList != NULL)
	{
		curl_slist_free_all(m_headerList);
		m_headerList = NULL;
	}

	ok = ok | curl_easy_setopt(m_curl, CURLOPT_VERBOSE, m_verboseOutput);

#ifdef BD_DEBUG_SSL_CHAIN
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_CERTINFO, 1);
#endif

	ok = ok | curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, m_errorBuffer);  
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_URL, downloadURL);  

	bdNChar8 transactionHeader[45]; // <25 chars + (20chars for the 64bit transactionID)
	bdSnprintf(transactionHeader, 45, "X-TransactionID: %I64u", transactionID);
	m_headerList = curl_slist_append(m_headerList, transactionHeader);
	
	if (startByte != 0 || endByte != 0)
	{
		bdNChar8 rangeHeader[40]; // <20 chars + (2x(10chars for 32bit decimal number))
		if (endByte != 0)
		{
			bdSnprintf(rangeHeader, 40, "Range: bytes=%I32u-%I32u", startByte, endByte);
		}
		else
		{
			bdSnprintf(rangeHeader, 40, "Range: bytes=%I32u-", startByte);
		}
		m_headerList = curl_slist_append(m_headerList, rangeHeader);
	}

	ok = ok | curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headerList);
	
	// support http redirection
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_FAILONERROR, 0);
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, !m_progressMeter);

	if (downloadHandler == BD_ARG_NOT_USED)
	{
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, defaultDownloadCallback);  
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, downloadBuffer); 
		s_downloadBufferSize = bufferSize;
	}
	else
	{
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, userDownloadCallback);  
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, downloadHandler);  
	}

#ifdef BD_USE_SSL		
	// Set to 1L to inspect the server's certificate against known certificates 
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
	// Set to 2L to ensure that the common name contained within the server's cert
	// matches the server we are attempting to communicate with
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
	// Set the format of the clien's certificate (if any)
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSLCERTTYPE,"PEM");
	// Set the location of the cert bundle to be used if CURLOPT_SSL_VERIFYPEER is set to 1L
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_CAINFO, SSL_CERT_BUNDLE_FULL_PATH); 
#endif

	if (ok == CURLE_OK)
	{
		m_status = BD_PENDING;
		resetTransferStats();
		m_operation = BD_DOWNLOAD;
		m_stopwatch.reset();
		m_stopwatch.start();
		m_thread = new bdThread(this, BD_DEFAULT_THREAD_PRIORITY, BD_HTTP_THREAD_STACKSIZE);
		ok = m_thread->start(BD_NULL, 0);
	}

	return m_status;
}

bdHTTPWrapper::bdStatus bdHTTPWrapper::_startUpload(const bdNChar8 *const uploadURL, 
													const void *const fileData, 
													bdUploadInterceptor* uploadHandler, 
													bdUInt32 uploadSize, 
													bdUInt64 transactionID)
{
	m_status = BD_FAILED_TO_START;
	
	m_curl = curl_easy_init();   

	if (m_curl)
	{
		int ok(CURLE_OK);

		if (m_headerList != NULL)
		{
			curl_slist_free_all(m_headerList);
			m_headerList = NULL;
		}

		ok = ok | curl_easy_setopt(m_curl, CURLOPT_VERBOSE, m_verboseOutput);

	#ifdef BD_DEBUG_SSL_CHAIN
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_CERTINFO, 1);
	#endif

		if (uploadHandler == BD_ARG_NOT_USED)
		{
			ok = ok | curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, defaultUploadCallback);
			ok = ok | curl_easy_setopt(m_curl, CURLOPT_READDATA, (void*)fileData);
		}
		else
		{
			ok = ok | curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, userUploadCallback);
			ok = ok | curl_easy_setopt(m_curl, CURLOPT_READDATA, (void*)uploadHandler);
		}

		ok = ok | curl_easy_setopt(m_curl, CURLOPT_FAILONERROR, 0);
		
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1) ;
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_PUT, 1);
		
		bdNChar8 transactionHeader[45]; // 20 chars for the 64bit transactionID
		bdSnprintf(transactionHeader, 45, "X-TransactionID: %I64u", transactionID);
		m_headerList = curl_slist_append(m_headerList, transactionHeader);
		
		// Using PUT with HTTP 1.1 implies the use of a "Expect: 100-continue" header. You can disable this header with CURLOPT_HTTPHEADER.
		m_headerList = curl_slist_append(m_headerList, "Expect:");
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headerList);
		
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_URL, uploadURL);
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)uploadSize);
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, !m_progressMeter);
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, BD_HTTP_CONNECTION_TIMEOUT);
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_DNS_CACHE_TIMEOUT, -1);

#ifdef BD_USE_SSL		
		// Set to 1L to inspect the server's certificate against known certificates 
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
		// Set to 2L to ensure that the common name contained within the server's cert
		// matches the server we are attempting to communicate with
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
		// Set the format of the clien's certificate (if any)
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSLCERTTYPE,"PEM");
		// Set the location of the cert bundle to be used if CURLOPT_SSL_VERIFYPEER is set to 1L
		ok = ok | curl_easy_setopt(m_curl, CURLOPT_CAINFO, SSL_CERT_BUNDLE_FULL_PATH); 
#endif

		if (ok == CURLE_OK)
		{
			
			m_status = BD_PENDING;
			s_uploadSize = uploadSize;
			resetTransferStats();
			m_stopwatch.reset();
			m_stopwatch.start();
			m_operation = BD_UPLOAD;
			m_thread = new bdThread(this, BD_DEFAULT_THREAD_PRIORITY, BD_HTTP_THREAD_STACKSIZE);
			ok = m_thread->start(BD_NULL, 0);
		}
	}

	return m_status;
}

bdHTTPWrapper::bdStatus bdHTTPWrapper::startDelete(const bdNChar8 *const deleteURL, bdUInt64 transactionID)
{
	m_status = BD_FAILED_TO_START;
	m_curl = curl_easy_init();  
	int ok(CURLE_OK);

	if (m_headerList != NULL)
	{
		curl_slist_free_all(m_headerList);
		m_headerList = NULL;
	}

	ok = ok | curl_easy_setopt(m_curl, CURLOPT_VERBOSE, m_verboseOutput);

#ifdef BD_DEBUG_SSL_CHAIN
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_CERTINFO, 1);
#endif

	bdNChar8 transactionHeader[45]; // 20 chars for the 64bit transactionID
	bdSnprintf(transactionHeader, 45, "X-TransactionID: %I64u", transactionID);
	m_headerList = curl_slist_append(m_headerList, transactionHeader);
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headerList);

	// Now set up all of the m_curl options  
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, m_errorBuffer);  
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_URL, deleteURL);  
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_HEADER, 0);  
	// support http redirection
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_FAILONERROR, 0);
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, !m_progressMeter);

	ok = ok | curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1);
	bdNChar8 deleteStr[7];
	bdSnprintf(deleteStr,7,"DELETE");
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, deleteStr);

#ifdef BD_USE_SSL		
	// Set to 1L to inspect the server's certificate against known certificates 
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
	// Set to 2L to ensure that the common name contained within the server's cert
	// matches the server we are attempting to communicate with
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
	// Set the format of the clien's certificate (if any)
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_SSLCERTTYPE,"PEM");
	// Set the location of the cert bundle to be used if CURLOPT_SSL_VERIFYPEER is set to 1L
	ok = ok | curl_easy_setopt(m_curl, CURLOPT_CAINFO, SSL_CERT_BUNDLE_FULL_PATH); 
#endif

	if (ok == CURLE_OK)
	{
		m_status = BD_PENDING;
		m_operation = BD_DELETE;
		m_thread = new bdThread(this, BD_DEFAULT_THREAD_PRIORITY, BD_HTTP_THREAD_STACKSIZE);
		ok = m_thread->start(BD_NULL, 0);
	}

	return m_status;
}


bdHTTPWrapper::bdStatus  bdHTTPWrapper::getStatus()
{
	if (m_operation == BD_FINALIZE)
	{
		finalize();
	}

	return m_status;
}

void bdHTTPWrapper::resetTransferStats()
{
	m_dataRate = 0.0f;
	s_bytesTransfered = 0;
}

bdUInt32 bdHTTPWrapper::getTransferProgress() const
{
	bdUInt32 bytesTransfered = static_cast<bdUInt32>(s_bytesTransfered);
	
	return bytesTransfered;
}

bdFloat32 bdHTTPWrapper::getTransferSpeed()
{
	if (m_status == BD_PENDING)
	{
		if (m_stopwatch.getElapsedTimeInSeconds() < 0.1f)
		{
			m_dataRate = 0.0f;
		}
		else
		{
			m_dataRate = static_cast<bdFloat32>(s_bytesTransfered) / m_stopwatch.getElapsedTimeInSeconds();
		}
	}

	return m_dataRate;
}

bdInt32 bdHTTPWrapper::getLastHTTPError() const
{
	if (m_status == BD_PENDING)
	{
		return 0;
	}
	
	return m_httpErrorCode;
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
}	

// curlBuffer: the buffer to read from during downloads
// size * nItems: the size of the curlBuffer
// userBuffer: the buffer to write to while downloading
int bdHTTPWrapper::defaultDownloadCallback(char *curlBuffer, size_t size, size_t nItems, void* userBuffer)  
{ 
	if (s_abortOperation)
	{
		return 0;
	}

	bdUWord dataSize = static_cast<bdUWord>(size * nItems);
	if (s_downloadBufferSize >= s_bytesTransfered + dataSize)
	{
		bdMemcpy(reinterpret_cast<bdUByte8*>(userBuffer) + s_bytesTransfered, 
			curlBuffer, 
			dataSize);
		
		s_bytesTransfered += dataSize;
	}
	else
	{
		BD_WARN("http","Not enough space in user supplied userBuffer to copy incoming data");
		dataSize = 0;
	}

	return static_cast<int>(dataSize);
} 

// curlBuffer: the buffer to fill for upload
// size * nItems: the size of the curlBuffer (CURL_MAX_WRITE_SIZE)
// userBuffer: the buffer to read from while uploading
int bdHTTPWrapper::defaultUploadCallback(char *curlBuffer, size_t size, size_t nItems, void* userBuffer)  
{
	if (s_abortOperation)
	{
		return 0;
	}

	bdUWord dataSize(0);

	if (s_bytesTransfered < s_uploadSize)
	{
		bdUByte8* bufferUByte = reinterpret_cast<bdUByte8*>(userBuffer);
		dataSize = BD_MIN(s_uploadSize - s_bytesTransfered, size * nItems);

		bdMemcpy(curlBuffer, 
			reinterpret_cast<void*>(bufferUByte + s_bytesTransfered), 
			dataSize);

		s_bytesTransfered += dataSize;
	}

	// note: returning 0 signals that uploading complete
	return static_cast<int>(dataSize);
}

int bdHTTPWrapper::userDownloadCallback(char *data, size_t size, size_t nItems, void* downloadInterceptor)
{
	bdDownloadInterceptor* handler = reinterpret_cast<bdDownloadInterceptor*>(downloadInterceptor);
	bdUInt32 dataSize = static_cast<bdUInt32>(size) * static_cast<bdUInt32>(nItems);

	bdInt32 res = handler->handleDownload(reinterpret_cast<void*>(data), dataSize);

	// update stats
	s_bytesTransfered += dataSize;
	
	return res;
}

int bdHTTPWrapper::userUploadCallback(char *data, size_t size, size_t nItems, void* uploadInterceptor)
{
	bdUploadInterceptor* handler = reinterpret_cast<bdUploadInterceptor*>(uploadInterceptor);
	bdUInt32 dataSize = static_cast<bdUInt32>(size) * static_cast<bdUInt32>(nItems);

	bdInt32 res = handler->handleUpload(reinterpret_cast<void*>(data), dataSize, static_cast<bdUInt32>(s_bytesTransfered));

	// update stats
	s_bytesTransfered += res;

	return res;
}

void bdHTTPWrapper::abortOperation()
{ 
	s_abortOperation = true; 
	m_status = BD_CANCELLED;
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

void bdHTTPWrapper::enableVerboseOutput(bdBool enable)
{
	m_verboseOutput = enable;
}

void bdHTTPWrapper::enableProgressMeter(bdBool enable)
{
	m_progressMeter = enable;
}

#endif
