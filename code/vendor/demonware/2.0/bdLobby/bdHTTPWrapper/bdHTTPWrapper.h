// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: A bespoke HTTP wrapper exposing functionality to upload and download via HTTP
// Currently it wraps libcurl on win32, libhttp on ps3 and a bespoke solution on xenon
// Future versions will wrap nhttp on wii

#ifndef BD_HTTP_WRAPPER_H
#define BD_HTTP_WRAPPER_H

#include <bdCore/bdCore.h>
#include <bdCore/bdThread/bdRunnable.h>
#include <bdCore/bdThread/bdThread.h>

#if ((defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX))

// Demonware supply a version of libcurl and openssl in the external directory of the release
// Add the include directories to your solution or project settings
#include <curl/curl.h>
#elif defined BD_PLATFORM_PS3

#include <cell/http.h>
#elif defined BD_PLATFORM_XENON

#include "bdHTTPClient-xenon.h"

#endif

#define BD_HTTP_RESPONSE_BUF_SIZE (1024)
#define BD_HTTP_COMMON_BUFFER_SIZE (8 * 1024) // PS3

class bdDownloadInterceptor;
class bdUploadInterceptor;

class bdHTTPWrapper : public bdRunnable
{

public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Status of the current HTTP operation, if any
	enum bdStatus
	{
		BD_EMPTY,
		BD_PENDING,
		BD_DONE,
		BD_FAILED,
		BD_TIMED_OUT,
		BD_CANCELLED,
		BD_FAILED_TO_START,
		BD_MAX_STATUS
	};

	/// The current operation being handled by the wrapper, if any
	enum bdOperation
	{
		BD_IDLE,
		BD_UPLOAD,
		BD_DOWNLOAD,
		BD_FINALIZE,
		BD_DELETE
	};

	bdHTTPWrapper();

	~bdHTTPWrapper();

#ifdef BD_PLATFORM_XENON

	/// init the HTTP wrapper
	/// An lsp service ID is required by the xenon version of the wrapper to enable the wrapper translate from
	/// unsecured HTTP address to their secured equivalents. If init is not called, HTTP operations will not start
	/// and return a fail status
	/// \param translateServiceNameToIP When set all http methods expect a valid lsp service description string in place of 
	/// an IP addresss. e.g. the URL would contain  'AMAX LSP - NY:1017' as oppose to the IP '207.1.2.334:1017'. Internally
	/// a translation is performed to replace 'AMAX LSP - NY' with the correct IP. 
	/// \note init calls XTitleServerCreateEnumerator and creates a mapping from service string to IP addresses
	void init(DWORD lspServiceID, bdBool translateServiceNameToIP = false);
#endif

	/// Download a resource into preallocated buffer using HTTP GET 
	/// This method returns after starting the a thread to perform the download
	/// getTransferProgress can be called to ascertain progress
	bdHTTPWrapper::bdStatus startDownload(const bdNChar8 *const downloadURL, void *const downloadBuffer, bdUInt32 bufferSize, bdUInt64 transactionID = 0, bdUInt32 startByte = 0, bdUInt32 endByte = 0);

	/// Download a resource to user defined handler using HTTP GET 
	/// This method returns after starting the a thread to perform the download
	/// downloadHandler is called when a chunk of data has been received on the underlying connection
	bdHTTPWrapper::bdStatus startDownload(const bdNChar8 *const downloadURL, bdDownloadInterceptor* downloadHandler, bdUInt64 transactionID = 0, bdUInt32 startByte = 0, bdUInt32 endByte = 0);

	/// Upload a resource from preallocated buffer using HTTP PUT 
	/// This method returns after starting the a thread to perform the upload
	/// getTransferProgress can be called to ascertain progress
	bdHTTPWrapper::bdStatus startUpload(const bdNChar8 *const uploadURL, const void *const fileData, bdUInt32 uploadSize, bdUInt64 transactionID = 0);

	/// Upload a resource from user defined handler using HTTP PUT 
	/// This method returns after starting the a thread to perform the upload
	/// uploadHandler will be called when the connection is ready to recieve more data
	bdHTTPWrapper::bdStatus startUpload(const bdNChar8 *const uploadURL, bdUploadInterceptor* uploadHandler, bdUInt32 uploadSize, bdUInt64 transactionID = 0);

	/// Delete a resource using HTTP DELETE idiom
	/// This method returns after starting the a thread to perform the delete
	bdHTTPWrapper::bdStatus startDelete(const bdNChar8 *const deleteURL, bdUInt64 transactionID = 0);

	/// This method returns the status of the current operation
	bdHTTPWrapper::bdStatus getStatus();

	/// Returns true if the wrapper can accept a call to startDownload, startUpload or startDelete
	bdBool isOkayToStart();

	/// Cancel a currently running http operation
	void abortOperation();

	/// Get the number of bytes transfered so far for the running operation
	/// On: PS3 - only valid after an operation is complete
	bdUInt32 getTransferProgress() const;

	/// Get the transfer speed in bytes per second so far for this operation
	/// \note this will not be accurate for v. quick transfers 
	/// On: PS3 - only valid after an operation is complete
	bdFloat32 getTransferSpeed();

	/// Resets the transfer speed and progress counters
	/// On: PS3 - irrelevant
	void resetTransferStats();

	/// Get the error code associated with the most recent completed http operation.
	/// \note this is only valid after an operation is complete. While an operation is
	/// pending this method will return 0.
	/// \note errors in range 2xx indicates success
	/// errors in range 3xx indicate a non fatal failure
	/// errors above 400 indicate a failure
	bdInt32 getLastHTTPError() const;

	/// Inherited from bdRunnable - the code the thread executes
	/// Required by PS3 version
	virtual bdUInt run(void* args);

#if (defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX)

	/// Enables verbuse debug text from LibCurl. Debug text is not output by default.
	/// \param enable[in] True, to output debug text
	void enableVerboseOutput(bdBool enable);

	/// Toggles whether to disable progress statistics within LibCurl. Progress statistics
	/// are disabled by default.
	/// \param enable[in] True, to enable progress statistics
	void enableProgressMeter(bdBool enable);

#endif

protected:

	bdHTTPWrapper::bdStatus _startDownload(const bdNChar8 *const downloadURL, void *const downloadBuffer, bdUInt32 bufferSize, bdDownloadInterceptor* downloadHandler, bdUInt64 transactionID, bdUInt32 startRange = 0, bdUInt32 endRange = 0);

	bdHTTPWrapper::bdStatus _startUpload(const bdNChar8 *const uploadURL, const void *const fileData, bdUploadInterceptor* uploadHandler, bdUInt32 uploadSize, bdUInt64 transactionID);

	/// extracts the serverName, fileName and port from the supplied URL
	/// The URL must be of form proto://serverName:port/fileName
	/// where proto contains substring 'http'.
	/// The port is optional and if not found defaults to 80
	bdBool parseURL(const bdNChar8 *const URL,
					bdNChar8 *serverName,
					bdNChar8 **startFileName,
					bdUInt32 &port);

#ifndef BD_PLATFORM_XENON

	/// called by ctor on xenon and win32
	void init();

	/// Finalizes a HTTP request and cleans up threads.
	void finalize();

#endif

#if ((defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX))

	/// download libcurl callback
	static int defaultDownloadCallback(char *data, size_t size, size_t nItems, void* buffer);

	/// upload libcurl callback
	static int defaultUploadCallback(char *writeBuffer, size_t size, size_t nItems, void* readBuffer);

	/// libcurl callback
	static int userDownloadCallback(char *data, size_t size, size_t nItems, void* downloadInterceptor);

	/// libcurl callback
	static int userUploadCallback(char *data, size_t size, size_t nItems, void* uploadInterceptor);

#elif defined BD_PLATFORM_PS3

	/// load ssl cert
	bdInt loadCertificate();

#endif


#if ((defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX))

	CURL *m_curl;
	struct curl_slist *m_headerList;
	bdBool m_uploading;
	bdBool m_downloading;

	// variables accessed inside the callbacks
	static bdUWord s_bytesTransfered;
	static bdUWord s_downloadBufferSize;
	static bdUWord s_uploadSize;

#elif defined BD_PLATFORM_PS3

	CellHttpClientId m_clientID;
	CellHttpTransId m_transaction;
	CellHttpUri m_getURI;
	CellHttpsData m_httpsData[1];
	
	bdBool m_abortOperation;

	bdUByte8 *m_operationBuffer; 	 
	bdUInt m_operationBufferSize;
	bdUInt32 m_transfered;
	bdFloat32 m_transferSpeed;

	/// http request and respond buffer
	bdUByte8 m_httpCommonBuffer[BD_HTTP_COMMON_BUFFER_SIZE];
	bdDownloadInterceptor* m_downloadHandler;
	bdUploadInterceptor* m_uploadHandler;

#elif defined BD_PLATFORM_XENON

	bdHTTPClient* m_httpClient;
	bdBool m_translateServiceNameToIP;

	// A hash map of 'site description' -> SG IP addresses
	// bdHashMap<bdString(LSP_SITE_DESCRIPTON), bdArray<bdString(IP_ADDR_SG_1, IP_ADDR_SG_n)>> m_lspDescToIP
	bdHashMap<bdString, bdArray<bdString>> m_lspDescToIP;

#endif

	static bdBool s_abortOperation;
	
	bdBool m_customVerification;
	bdNChar8 m_errorBuffer[BD_HTTP_RESPONSE_BUF_SIZE];
	bdStatus m_status;
	bdOperation m_operation;

	bdInt m_httpErrorCode;
	bdThread *m_thread;
	bdFloat32 m_dataRate;
	bdStopwatch m_stopwatch;

#ifndef BD_PLATFORM_XENON

	bdBool m_verboseOutput;
	bdBool m_progressMeter;

#endif

};

char* bdStrdup(const char *str);
void* bdCallocate(size_t nmemb, size_t size);

#include <bdLobby/bdHTTPWrapper/bdHTTPWrapper.inl>

#endif // BD_HTTP_WRAPPER_H
