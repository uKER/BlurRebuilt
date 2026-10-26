// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdSession/bdSessionInfo.h>

#include <bdConnection/bdConnection.h>
#include <bdConnection/bdUnicastConnection.h>

#include <bdPeer/bdSession/bdSession.h>

#include <bdNet/bdNet.h>

#include <bdSocket/bdAddressMap.h>

bdUInt BD_CALL bdSessionInfo::getInfo(const bdSession *const session,
									  bdNChar8 *buf,
									  const bdUInt length)
{
	bdNChar8 *const end = buf + length;

	const bdNChar8 *const host = "BD_SESSION_HOST";
	const bdNChar8 *const peer = "BD_SESSION_PEER";

	const bdNChar8 *role = peer;	
	if(session->getRole() == bdSession::BD_SESSION_HOST)
	{
		role = host;
	}

	bdInt written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "Role: %s\n", role);
	if(written > 0 && written <= static_cast<bdInt>(end - buf))
	{
		buf += written;
	}
	else
	{
		buf = end - 1;
		if(length > 0)
		{
			*buf = '\0';
		}			
	}

	const bdNChar8 *const unknown = "**UNKNOWN**";
	const bdNChar8 *const sessionStates[] = { "BD_SESSION_NOT_CONNECTED",
											 "BD_SESSION_CONNECTING_TO_HOST",
											 "BD_SESSION_CONNECTED_TO_HOST",
											 "BD_SESSION_CONNECTING_TO_PEERS",
											 "BD_SESSION_NOT_READY",
											 "BD_SESSION_READY",
											 "BD_SESSION_CONNECTING_TO_HOST_FAILED",
											 "BD_SESSION_CONNECTING_TO_PEERS_FAILED"};	


	const bdNChar8* sessionState = unknown;
	const bdUInt sessionStatusIndex = session->getStatus();
	if(sessionStatusIndex < (sizeof(sessionStates) / sizeof(sessionStates[0])))
	{
		sessionState = sessionStates[sessionStatusIndex];
	}

	written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "Status: %s\n", sessionState);
	if(written > 0 && written <= static_cast<bdInt>(end - buf))
	{
		buf += written;
	}
	else
	{
		buf = end - 1;
		if(length > 0)
		{
			*buf = '\0';
		}			
	}

	const bdUInt32 numPeers = session->getNumPeers();
	const bdConnectionRef hostConnection = session->getHost();

	for(bdUInt32 i = 0; i < numPeers; i++)
	{
		bdConnectionRef connection = session->getConnection(i);

		const bdNChar8 *post = "\n";
		/// todo bdReference == operator review..
		if(connection == hostConnection)
		{
			post = "(host)\n";
		}

		const bdNChar8 *const connectionStates[] = {"BD_NOT_CONNECTED",
													"BD_CONNECTING",
													"BD_CONNECTED",
													"BD_DISCONNECTING",
													"BD_DISCONNECTED" };

		const bdNChar8* connectionState = unknown;
		const bdUInt connectionStatusIndex = connection->getStatus();
		if(connectionStatusIndex < (sizeof(connectionStates) / sizeof(connectionStates[0])))
		{
			connectionState = connectionStates[connectionStatusIndex];
		}

		bdUInt rtt = 0;
		if(!connection->getAddress()->isLoopback())
		{
			const bdUnicastConnectionRef unicastConnection = reference_cast<bdUnicastConnection>(connection);
			rtt = static_cast<bdUInt>(unicastConnection->getStats().getAvgRTT()*1000);
		}

		bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
		bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, BD_ADDR_STRING_SIZE);		

		const bdInt numWritten = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "%u - Peer: %s, %u ms RTT, %s, %s", i, addrStr, rtt, connectionState, post);
		if(numWritten > 0 && numWritten <= static_cast<bdInt>(end - buf))
		{
			buf += numWritten;
		}
		else
		{
			buf = end - 1;
			if(length > 0)
			{
				*buf = '\0';
			}			
		}
	}

	// return num bytes written (not including the null terminator)
	const bdUInt bytesWritten = length - (end - buf);
	return bytesWritten;
}


// TODO: refactor - tidy getPeerInfo then make getInfo call getPeerInfo
bdUInt BD_CALL bdSessionInfo::getPeerInfo(const bdSession *const session,
										  bdNChar8 *buf,
										  const bdUInt length,
										  const bdUInt peerIndex)
{
	bdNChar8 *const end = buf + length;

	const bdUInt32 numPeers = session->getNumPeers();
	const bdConnectionRef hostConnection = session->getHost();

	if ( peerIndex >= numPeers )
		return 0;

	bdConnectionRef connection = session->getConnection(peerIndex);

	const bdNChar8 *const host = "(host)";
	const bdNChar8 *const peer = "";

	const bdNChar8 *const connectionStates[] = {	"BD_NOT_CONNECTED",
		"BD_CONNECTING",
		"BD_CONNECTED",
		"BD_DISCONNECTING",
		"BD_DISCONNECTED" };

	const bdNChar8 *post = peer;
	/// todo bdReference == operator review..
	if(connection == hostConnection)
	{
		post = host;
	}

	const bdNChar8 *const unknown = "**UNKNOWN**";
	const bdNChar8* connectionState = unknown;
	const bdUInt connectionStatusIndex = connection->getStatus();
	if(connectionStatusIndex < (sizeof(connectionStates) / sizeof(connectionStates[0])))
	{
		connectionState = connectionStates[connectionStatusIndex];
	}

	bdUInt rtt = 0;
	if(!connection->getAddress()->isLoopback())
	{
		const bdUnicastConnectionRef unicastConnection = reference_cast<bdUnicastConnection>(connection);
		rtt = static_cast<bdUInt>(unicastConnection->getStats().getAvgRTT()*1000);
	}

	bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
	bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, BD_ADDR_STRING_SIZE);

	const bdInt written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "Peer %u - IP: %s, %u ms RTT, %s, %s", peerIndex, addrStr, rtt, connectionState, post);
	if(written > 0 && written <= static_cast<bdInt>(end - buf))
	{
		buf += written;
	}
	else
	{
		buf = end - 1;
		if(length > 0)
		{
			*buf = '\0';
		}			
	}

	// return num bytes written (not including the null terminator)
	const bdUInt bytesWritten = length - (end - buf);
	return bytesWritten;
}

bdSessionInfo::bdSessionInfo()
{
}
