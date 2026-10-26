// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdSession/bdSessionHandler.h>
		
bdSessionHandler::bdSessionHandler()
{
}

bdSessionHandler::~bdSessionHandler()
{
}

bdBool bdSessionHandler::onSessionConnectRequest(bdBitBufferRef,
												 bdBitBuffer *const)
{
	return true;
}

