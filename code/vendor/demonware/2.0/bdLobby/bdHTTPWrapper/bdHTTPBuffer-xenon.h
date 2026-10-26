// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef bdHTTPBuffer_H
#define bdHTTPBuffer_H

#include <bdCore/bdCore.h>

#define BD_XENON_MAX_PACKET_PAYLOAD	(1264)

struct bdHTTPBuffer
{
	/// reset all cached data in the buffers
	void init();

	/// cache pointers to an allocated area of memory which contains 
	/// data to be sent as content with the http message
	void setContentSendBuffer(CHAR* content, 
							  DWORD contentLength);

	/// cache pointers to an allocated area of memory which can be used
	/// to receive data
	void setContentRecieveBuffer(CHAR* content, 
		                         DWORD contentLength);

	/// content buffer (for GET)
	CHAR* m_contentReceiveBuffer;

	/// content buffer size (for GET)
	DWORD m_contentReceiveBufferSize;

	/// content buffer (for PUT/POST)
	CHAR* m_contentSendBuffer;

	/// content buffer size (for PUT/POST)
	DWORD m_contentSendBufferSize;

	/// http request and respond buffer
	CHAR m_httpCommonBuffer[BD_XENON_MAX_PACKET_PAYLOAD];

	/// size of m_httpCommonBuffer
	DWORD m_httpCommonBufferSize;

};

#include <bdLobby/bdHTTPWrapper/bdHTTPBuffer-xenon.inl>

#endif
