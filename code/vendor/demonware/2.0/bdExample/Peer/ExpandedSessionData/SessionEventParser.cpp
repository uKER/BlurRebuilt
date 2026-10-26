// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "SessionEventParser.h"
#include "SessionData.h"

#include <bdCore/bdSocket/bdCommonAddrInfo.h>

#define MAX_SESSION_DATA_DESCRIPTION_SIZE 2000

SessionEventParser::SessionEventParser()
{
}

void SessionEventParser::init(bdPeer *peer)
{
	m_peer = peer;

	peer->getSession()->registerListener(this);

	// the remote log subscriber can only be initialised after bdNet has started.
	bdAddr remoteLogger ("127.0.0.1:3900");
	if(m_remoteLogSubscriber.init(RemoteLogSubscriber::BD_UDP_SOCKET  , remoteLogger))
	{
		m_remoteLogSubscriber.addChannel("info/session data");
	}

	bdNetImpl *const net = bdNet::getInstance();
	bdNChar8 buffer [BD_COMMON_ADDR_INFO_SIZE];
	bdCommonAddrInfo::getBriefInfo(net->getLocalCommonAddr(), buffer , BD_COMMON_ADDR_INFO_SIZE);
	BD_INFO("session data" , "Join Event Peer 0 Address %s" , buffer);	

	sendSessionDataUpdate();

}

void SessionEventParser::quit()
{
	BD_INFO("session data" , "Host Quit:"  );
}

void SessionEventParser::onSessionStatusChange(	const bdSession::bdSessionStatus previous,
									   const bdSession::bdSessionStatus current)
{
	const bdNChar8 *const unknown = "**UNKNOWN**";
	const bdNChar8 *const sessionStates[] = { "BD_SESSION_NOT_CONNECTED",
		"BD_SESSION_CONNECTING_TO_HOST",
		"BD_SESSION_CONNECTED_TO_HOST",
		"BD_SESSION_CONNECTING_TO_PEERS",
		"BD_SESSION_NOT_READY",
		"BD_SESSION_READY",
		"BD_SESSION_CONNECTING_TO_HOST_FAILED",
		"BD_SESSION_CONNECTING_TO_PEERS_FAILED"};	


	const bdNChar8* prevSessionState = unknown;
	bdUInt sessionStatusIndex = previous;
	if(sessionStatusIndex < (sizeof(sessionStates) / sizeof(sessionStates[0])))
	{
		prevSessionState = sessionStates[sessionStatusIndex];
	}

	const bdNChar8* currentSessionState = unknown;
	sessionStatusIndex = current;
	if(sessionStatusIndex < (sizeof(sessionStates) / sizeof(sessionStates[0])))
	{
		currentSessionState = sessionStates[sessionStatusIndex];
	}

	BD_INFO("session data" , "Session Status changed from %s to %s ."  , prevSessionState , currentSessionState);
	sendSessionDataUpdate();
}

void SessionEventParser::sendSessionDataUpdate()
{
	bdNChar8 buffer [MAX_SESSION_DATA_DESCRIPTION_SIZE];
	const bdInt numBytesWritten = getSessionData(buffer , MAX_SESSION_DATA_DESCRIPTION_SIZE);
	buffer[numBytesWritten] = '\0';
	BD_INFO("session data" , buffer);	
}

void SessionEventParser::onSessionConnect(const bdConnectionRef connection)
{
	bdUInt index = 0;
	if(m_peer->getSession()->getPeerIndex(connection , index))
	{
		bdNChar8 buffer [BD_COMMON_ADDR_INFO_SIZE];
		bdCommonAddrInfo::getBriefInfo(connection->getAddress(), buffer , BD_COMMON_ADDR_INFO_SIZE);
		BD_INFO("session data" , "Join Event Peer %u Address %s" , index , buffer);	

		sendSessionDataUpdate();
	}
}

bdUInt SessionEventParser::getSessionData(bdNChar8 *buf,
								  const bdUInt length) const
{
	// iterate all peers and print their session data details to the string
	bdNChar8 *const end = buf + length;
	bdSession *const session = m_peer->getSession();

	buf += bdSnprintf(buf , 5 , "\n" );

	for(bdUInt i = 0; i < session->getSessionData().getSize(); i++)
	{
		const SessionDataRef data = reference_cast<SessionData>(session->getSessionData()[i]);
		const bdInt written = bdSnprintf(buf , 20 , "Data for Peer %u \n" , i);
		if(written > 0)
		{
			buf += written;
		}
		for(bdUInt j = 0 ; j < data->m_connectedPeerAddrs.getSize() ; j++)
		{
			const bdInt written = bdCommonAddrInfo::getBriefInfo(data->m_connectedPeerAddrs[j], buf , (end - buf));
			if(written > 0)
			{
				buf += written;
			}
			else
			{
				buf = end - 1;
			}
		}
	}

	if(length > 0)
	{
		*buf = '\0';
	}

	const bdUInt bytesWritten = length - (end - buf);
	return bytesWritten;
}
