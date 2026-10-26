// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline BYTE* bdHTTPClient::getResponseContentData() const
{
	return m_pResponseContentData;
}

/// returns the size of the Content Data (excluding the HTTP header)
inline DWORD bdHTTPClient::getResponseContentDataLength() const
{
	return m_dwResponseContentDataLength;
}

inline VOID bdHTTPClient::setResponseContentData( BYTE* p )
{
	m_pResponseContentData = p;
}

inline VOID bdHTTPClient::setResponseContentDataLength( DWORD dwLen )
{
	m_dwResponseContentDataLength = dwLen;
}

/// return the status of the current HTTP operation
inline bdHTTPClient::BD_HTTP_STATUS bdHTTPClient::getStatus() const 
{
	return m_status;
}

/// set the status of the current HTTP operation
inline VOID bdHTTPClient::setStatus( BD_HTTP_STATUS status )
{
	m_status = status;
}

/// set the average data rate over the course of the current HTTP operation (in bytes per sec)
inline FLOAT bdHTTPClient::getDataRate() const 
{
	return m_dataRate;
}

/// set the data rate of the current HTTP operation (in bytes per sec)
inline VOID bdHTTPClient::setDataRate( FLOAT dataRate )
{
	m_dataRate = dataRate;
}

/// return the last error code: e.g. HTTP 200, 500 etc
inline DWORD bdHTTPClient::getResponseCode() const 
{
	return m_httpResponseCode;
}

/// return the last socket level error
inline DWORD bdHTTPClient::getSocketErrorCode() const
{
	return m_dwSocketErrorCode;
}

/// set the socket level error
inline VOID bdHTTPClient::setSocketErrorCode( DWORD dwCode )
{
	m_dwSocketErrorCode = dwCode;
}

/// return the internal http buffer container
inline bdHTTPBuffer& bdHTTPClient::getInternalBuffer()
{
	return m_buffer;
}

/// get HTTP operation VERB (GET/PUT/POST...)
inline bdHTTPClient::BD_HTTP_OPERATION bdHTTPClient::getOperation() const
{
	return m_operation;
}

inline DWORD bdHTTPClient::getExpectedContentSize() const
{
	return m_expectedContentSize;
}


inline void bdHTTPClient::setBytesTransfered(DWORD bytesTransfered)
{
	m_bytesTransfered = bytesTransfered;
}

inline DWORD bdHTTPClient::getBytesTransfered() const
{
	return m_bytesTransfered;
}

inline void bdHTTPClient::setLspServiceID(DWORD lspServiceID)
{
	m_lspServiceID = lspServiceID;
}

inline DWORD bdHTTPClient::getLspServiceID() const
{
	return m_lspServiceID;
}

inline void bdHTTPClient::abortOperation()
{
	m_abort = true;
}

inline BOOL bdHTTPClient::isAborted() const
{
	return m_abort;
}