// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdDummySocket.h>

bdDummySocket::bdDummySocket()
{
}

bdDummySocket::~bdDummySocket()
{
	close();
}

bdSocketStatusCode bdDummySocket::bind(const bdPort )
{
	return BD_NET_SUCCESS;
}

bdSocketStatusCode bdDummySocket::bind(const bdAddr &)
{
	return BD_NET_SUCCESS;
}

bdInt bdDummySocket::sendTo(const bdAddr &, 
							 const void *, 
							 const bdUInt length)
{
	return length;
}


bdInt	bdDummySocket::receiveFrom(bdAddr &, 
								  void *,
								  const bdUInt )
{
	return BD_NET_WOULD_BLOCK;
}

bdBool bdDummySocket::close()
{
	return true;
}