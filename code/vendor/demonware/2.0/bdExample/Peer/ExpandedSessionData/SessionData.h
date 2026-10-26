// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SESSION_DATA_H
#define SESSION_DATA_H

#include <bdExample/bdExampleCommonPeer/bdExampleCommonP2P.h>

BD_REFERENCE(SessionData);

class SessionData : public bdSessionData
{
	public:

		SessionData();

		void serialize(bdBitBufferRef buffer);
		bdBool deserialize(bdBitBufferRef buffer);

		bdBool equals(bdSessionDataRef other) const;

		void calculateLocalHash(bdArray<bdSessionDataRef> peers);

		bdArray<bdCommonAddrRef> m_connectedPeerAddrs;
};

#endif // SESSION_DATA_H
