// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdExampleCommon.h>

#define MAX_NUM_RETRIES 3


DiscoveryManager::~DiscoveryManager()
{
}

bdGameInfoRef DiscoveryManager::discoverLanGames(const bdUInt32 titleID)
{
	bdBool ok = true;
	bdLANDiscoveryClient discoveryClient;
	discoveryClient.registerListener(this);
	bdUInt numRetries = 0;
	bdGameInfoRef gameInfo;
	while(ok && (gameInfo.isNull() && numRetries < MAX_NUM_RETRIES))
	{
	
		ok = discoveryClient.discover(titleID, 2.0f);
		while(ok && (discoveryClient.getStatus() == bdLANDiscoveryClient::BD_PENDING))
		{
			discoveryClient.update();

			// join the first game with matching name
			bdLinkedList<bdGameInfoRef>::Position pos = m_gameList.getHeadPosition();
			if (pos)
			{
				gameInfo = m_gameList.forward(pos);
			} 
		}
		numRetries++;
	}
	
	
	discoveryClient.unregisterListener(this);

	return gameInfo;
}

 bdGameInfoRef DiscoveryManager::createGame(const bdUInt32 titleID)
{
	bdNetImpl *const net = bdNet::getInstance();
	const bdCommonAddrRef localAddr = net->getLocalCommonAddr();

	// create and register a security key
	bdSecurityID secID;
	bdSecurityKey secKey;
	bdSecurityKeyMap::initKey(secID, secKey);

	// set up a found game packet send to discovery requests
	
	bdGameInfoRef gameInfo = bdGameInfoFactory::getInstance()->create();
	gameInfo->setSecurityID(secID);
	gameInfo->setSecurityKey(secKey);
	gameInfo->setTitleID(titleID);
	gameInfo->setHostAddr(localAddr);

	return gameInfo;
}

bdGameInfoRef DiscoveryManager::directConnect(const bdUInt32 titleID,
									   const bdNChar8 *const publicAddr)
{
	bdLANDiscoveryClient discoveryClient;
	discoveryClient.registerListener(this);
	// directly connect to the discovery server at publicAddr
	
	discoveryClient.discover(titleID, 2.0f, bdInetAddr(publicAddr));
	while(discoveryClient.getStatus() == bdLANDiscoveryClient::BD_PENDING)
	{
		discoveryClient.update();
	}

	// join the first game with matching name
	bdLinkedList<bdGameInfoRef>::Position pos = m_gameList.getHeadPosition();

	bdGameInfoRef game;
	if(pos)
	{
		game = m_gameList.forward(pos);
	}
	else
	{
		bdNChar8 buffer[100];

		bdInetAddr(publicAddr).toString(buffer, 100);
		bdPrintf("Unable to direct connect to %s.\n", buffer);
	}
	return game;
}

void DiscoveryManager::onDiscovery(bdGameInfoRef gameInfo)
{
	// Filter out duplicate game infos
	bdLinkedList<bdGameInfoRef>::Position pos = m_gameList.getHeadPosition();
	bdBool isUnique = true;
	while (pos)
	{
		bdGameInfoRef existingGame = m_gameList.forward(pos);
		if (*(existingGame->getHostAddr()) ==(*(gameInfo->getHostAddr())))
		{
			isUnique = false;
		}
	}
	if (isUnique)
	{
		m_gameList.addHead(gameInfo);
	}
}

bdLinkedList<bdGameInfoRef>& DiscoveryManager::getFoundGames()
{
	return m_gameList;
}

