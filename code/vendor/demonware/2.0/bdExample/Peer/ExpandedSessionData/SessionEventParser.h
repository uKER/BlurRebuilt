// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SESSION_EVENT_PARSER_H
#define SESSION_EVENT_PARSER_H

#include <bdExample/bdExampleCommonPeer/bdExampleCommonP2P.h>
#include <bdExample/bdExampleCommon/RemoteLogSubscriber.h>

class SessionEventParser : public bdSessionListener
{
	public:

		SessionEventParser();

		void init(bdPeer *peer);
		void quit();

	protected:
		void sendSessionDataUpdate();

		void onSessionStatusChange(	const bdSession::bdSessionStatus previous,
									const bdSession::bdSessionStatus current);

		void onSessionConnect(const bdConnectionRef connection);
	
		bdUInt getSessionData(bdNChar8 *buf, 
			const bdUInt length) const;

	protected:
		bdPeer *m_peer;

		RemoteLogSubscriber m_remoteLogSubscriber;


};

#endif // SESSION_EVENT_PARSER_H
