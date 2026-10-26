// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef bdHTTPClient_H
#define bdHTTPClient_H

#include <xtl.h>
#include <winsockx.h>
#include <bdCore/bdCore.h>
#include "bdHTTPBuffer-xenon.h"

#define BD_HTTP_HOST_IP_STRING_LENGTH	128

class bdDownloadInterceptor;
class bdUploadInterceptor;

class bdHTTPClient
{
public:

	enum BD_HTTP_OPERATION
	{
		BD_HTTP_GET_OPERATION,
		BD_HTTP_PUT_OPERATION,
		BD_HTTP_POST_OPERATION,
		BD_HTTP_DELETE_OPERATION
	};

    enum BD_HTTP_STATUS
    {
        BD_HTTP_STATUS_READY,
        BD_HTTP_STATUS_BUSY,
        BD_HTTP_STATUS_DONE,
        BD_HTTP_STATUS_ERROR
    };

	/// create an instance of the bdHTTPClient class indicating whether or not to use
	/// a thread for each HTTP operation (without threads the operations will block execution
	/// until complete). Default is threaded.
	bdHTTPClient(DWORD lspServiceID, 
				 BOOL usingThread = TRUE);

	~bdHTTPClient();

	/// initiates a HTTP GET operation on the URL given by: serverName:port/uri
	/// \param content[in/out] the area of data to store the content received upon success
	/// \param contentLenght[in] the size of the allocated data 'content' and thus the max data to receive
    HRESULT httpGet(const CHAR* serverName, 
					const CHAR* uri, 
					CHAR* content, 
					DWORD contentLength,
					bdDownloadInterceptor* downloadHandler,
					DWORD port = 80, 
					bdUInt64 transactionID = 0,
					bdUInt32 startBytes = 0,
					bdUInt32 endBytes = 0);
	
	/// initiates a HTTP PUT operation on the URL given by: serverName:port/uri
	/// \param content[in] the content to send up with the PUT request 
	/// \param contentLenght[in] the size of the allocated data 'content' and thus the max data to receive
	HRESULT httpPut(const CHAR* serverName, 
					const CHAR* uri, 
					CHAR* content, 
					DWORD contentLength, 
					bdUploadInterceptor* uploadHandler,
					DWORD port = 80, 
					bdUInt64 transactionID = 0);
    
	/// initiates a HTTP POST operation on the URL given by: serverName:port/uri
	/// \param content[in] the content to send up with the POST request 
	/// \param contentLenght[in] the size of the allocated data 'content' and thus the max data to receive
	HRESULT httpPost(const CHAR* serverName, 
					 const CHAR* uri, 
					 CHAR* content, 
					 DWORD contentLength, 
					 bdUploadInterceptor* uploadHandler,
					 DWORD port = 80, 
					 bdUInt64 transactionID = 0);
		
	/// initiates a HTTP DELETE operation on the URL given by: serverName:port/uri
	HRESULT httpDelete(const CHAR* serverName, 
					   const CHAR* uri, 
					   DWORD port = 80, 
					   bdUInt64 transactionID = 0);

	/// extract the HTTP response code, content length from Http response header
	/// and return the content data start point
	/// \param contentDataPtr[in/out]
	HRESULT parseIncomingHttpResponse(CHAR** contentDataPtr);

	/// get the Content Data (excluding the HTTP header)
    BYTE* getResponseContentData() const;
    
    /// returns the size of the Content Data (excluding the HTTP header)
	DWORD getResponseContentDataLength() const;
    
	VOID setResponseContentData(BYTE* p);
	
	VOID setResponseContentDataLength(DWORD dwLen);
	
    /// return the status of the current HTTP operation
    BD_HTTP_STATUS getStatus() const; 
    
    /// set the status of the current HTTP operation
	VOID setStatus(BD_HTTP_STATUS status);
    
	/// set the average data rate over the course of the current HTTP operation (in bytes per sec)
	FLOAT getDataRate() const; 
	
	/// set the data rate of the current HTTP operation (in bytes per sec)
	VOID setDataRate(FLOAT dataRate);
	
    /// return the last error code: e.g. HTTP 200, 500 etc
    DWORD getResponseCode() const; 
    
    /// return the last socket level error
    DWORD getSocketErrorCode() const;
    
	/// set the socket level error
    VOID setSocketErrorCode(DWORD dwCode);
    
    /// return the internal http buffer container
    bdHTTPBuffer& getInternalBuffer();
        
	/// get HTTP operation VERB (GET/PUT/POST...)
	BD_HTTP_OPERATION getOperation() const;
	
	DWORD getExpectedContentSize() const;
		
	void setBytesTransfered(DWORD bytesTransfered);
	
	DWORD getBytesTransfered() const;

	void setLspServiceID(DWORD lspServiceID);

	DWORD getLspServiceID() const;
	
	void abortOperation();
	
	BOOL isAborted() const;

	void finalize();
	
	/// enables syncronisation
	WSAOVERLAPPED m_overlapped;

	/// http server name		
	CHAR m_serverName[BD_HTTP_HOST_IP_STRING_LENGTH];

	/// http server port
	DWORD m_port;

	bdDownloadInterceptor* m_downloadHandler;
	bdUploadInterceptor* m_uploadHandler;

protected:

	HRESULT startHttpOperation(const CHAR* serverName, 
							   DWORD port);

    HRESULT sendCommand();


    // flag for whether to start worker thread
    BOOL m_usingThread;

	/// current operation
	BD_HTTP_OPERATION m_operation;

	/// current client status
    BD_HTTP_STATUS m_status;

    /// HTTP buffer, include outgoing and incoming buffer
    bdHTTPBuffer m_buffer;
	
    BYTE* m_pResponseContentData;
    DWORD m_dwResponseContentDataLength;
    DWORD m_httpResponseCode;
	DWORD m_expectedContentSize;
    DWORD m_dwSocketErrorCode;
	FLOAT m_dataRate;
	UINT m_bytesTransfered;

	BOOL m_abort;
	DWORD m_lspServiceID;
	
	HANDLE m_hThread;
};


#include <bdLobby/bdHTTPWrapper/bdHTTPClient-xenon.inl>

#endif
