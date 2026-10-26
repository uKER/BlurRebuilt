
#include "stdafx.h"
#include "netTransport_LANDiscovery.h"
#include <bdNet/bdNet.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	LANDiscoveryServer::LANDiscoveryServer()
	{
	}

	LANDiscoveryServer::~LANDiscoveryServer()
	{
	}

	bdGameInfoRef LANDiscoveryServer::createGame(U32 titleId)
	{
		bdNetImpl *const net = bdNet::getInstance();
		const bdCommonAddrRef localAddr = net->getLocalCommonAddr();
		bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();

		// create and register a security key
		bdSecurityID secID;
		bdSecurityKey secKey;
		keyMap->initKey(secID, secKey);

		// set up a found game packet send to discovery requests
		bdGameInfoRef game = new bdGameInfo(titleId, secID, secKey, localAddr);
		return game;
	}
	
	LANDiscoveryClient::LANDiscoveryClient()
	{
		registerListener(this);
	}

	LANDiscoveryClient::~LANDiscoveryClient()
	{
		unregisterListener(this);
	}

	void LANDiscoveryClient::onDiscovery(bdGameInfoRef gameInfo)
	{
		m_results.addTail(gameInfo);
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

