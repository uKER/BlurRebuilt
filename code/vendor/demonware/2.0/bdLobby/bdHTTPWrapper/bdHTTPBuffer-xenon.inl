// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline void bdHTTPBuffer::init()
{
	m_httpCommonBufferSize = 0;
	setContentSendBuffer(BD_NULL, 0);
	setContentRecieveBuffer(BD_NULL, 0);
}

inline void bdHTTPBuffer::setContentSendBuffer(CHAR* content, DWORD contentLength)
{
	m_contentSendBuffer = content;
	m_contentSendBufferSize = contentLength;
}

inline void bdHTTPBuffer::setContentRecieveBuffer(CHAR* content, DWORD contentLength)
{
	m_contentReceiveBuffer = content;
	m_contentReceiveBufferSize = contentLength;
}