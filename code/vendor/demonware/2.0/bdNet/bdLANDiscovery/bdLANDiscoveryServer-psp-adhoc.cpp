// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdLANDiscovery/bdLANDiscoveryServer.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>

#include <pspnet_adhoc_matching.h>

#define MATCHING_POOLSIZE	128*1024
#define MATCHING_MAXIMUM	16
#define MATCHING_PORT		50000
#define MATCHING_RXBUFLEN	0x2000
#define HELLO_INTERVAL		1*1000*1000
#define KEEP_ALIVE_INTERVAL	20*1000*1000
#define KEEP_ALIVE_COUNT	5
#define REXMT_INTERVAL		500*1000

#define MATCHING_EVENT_TPL			0x10
#define MATCHING_EVENT_STACKSIZE	0x8000
#define MATCHING_INPUT_TPL			0x10
#define MATCHING_INPUT_STACKSIZE	0x8000

static void bdLANDiscoveryServerHandler(bdInt id, bdInt event, struct SceNetEtherAddr *peer,
									 bdInt optLen, void *optArg);

/// List of all registered listeners
bdFastArray<bdLANDiscoveryListener*> g_serverListeners;

bdLANDiscoveryServer::bdLANDiscoveryServer()
{


	bdInt ret = sceNetAdhocMatchingInit(MATCHING_POOLSIZE);
	if (ret < 0)
	{
		BD_ERR("err", "sceNetAdhocMatchingInit failed %x\n", ret);
	}

}

bdLANDiscoveryServer::~bdLANDiscoveryServer()
{
	stop();
	sceNetAdhocMatchingTerm();
}

bdBool bdLANDiscoveryServer::start(const bdGameInfoRef gameInfo,
								const bdInetAddr&,
								const bdPort)
{
	BD_INFO("info","Starting Discovery Server\n");
	bdInt ret;
	ret = sceNetAdhocMatchingCreate( SCE_NET_ADHOC_MATCHING_MODE_PARENT, MATCHING_MAXIMUM,
		MATCHING_PORT, MATCHING_RXBUFLEN, HELLO_INTERVAL, KEEP_ALIVE_INTERVAL,
		KEEP_ALIVE_COUNT, REXMT_INTERVAL, bdLANDiscoveryServerHandler);
	if (ret < 0)
	{
		BD_ERR("err", "sceNetAdhocMatchingCreate failed %x\n", ret);
		return false;
	}
	m_contextID = ret;
	bdBitBuffer buffer;
	m_gameInfo = gameInfo;
	m_gameInfo->serialize(buffer);
	ret = sceNetAdhocMatchingStart(m_contextID,
				MATCHING_EVENT_TPL, MATCHING_EVENT_STACKSIZE,
				MATCHING_INPUT_TPL, MATCHING_INPUT_STACKSIZE,
				buffer.getDataSize(),
				buffer.getData());
	if (ret < 0)
	{
	 	BD_ERR("err", "sceNetAdhocMatchingStart failed %x\n", ret);

		return false;
	}
	return true;

}

void bdLANDiscoveryServer::update()
{

}

void bdLANDiscoveryServer::stop()
{
	if (m_contextID > 0)
	{
		bdInt ret;
		ret = sceNetAdhocMatchingStop(m_contextID);
		if (ret < 0)
		{
			return;
		}
		ret = sceNetAdhocMatchingDelete(m_contextID);
		if (ret < 0)
		{
			return;
		}
		m_contextID = 0;
	}
}

void bdLANDiscoveryServer::registerListener(bdLANDiscoveryListener *listener)
{
	g_serverListeners.pushBack(listener);
}

void bdLANDiscoveryServer::unregisterListener(bdLANDiscoveryListener *listener)
{
	g_serverListeners.removeAll(listener);
}

bdLANDiscoveryServer::bdStatus bdLANDiscoveryServer::getStatus() const
{
	return m_status;
}

bdGameInfoRef bdLANDiscoveryServer::getGameInfo()
{
	return m_gameInfo;
}

static void bdLANDiscoveryServerHandler(bdInt id, bdInt event, struct SceNetEtherAddr *peer,
									 bdInt optLen, void *optArg)
{
	bdNChar8 addressStr[18];
	BD_INFO("info","Received Discovery event : %x %x\n", id, event);
	sceNetEtherNtostr(peer, addressStr);
	switch (event)
	{
		case SCE_NET_ADHOC_MATCHING_EVENT_REQUEST:
		{
			BD_INFO("info","Received request event from %s\n", addressStr);
			if (optLen > 0)
			{
				BD_INFO("info","Argument: %s\n ", optArg);
			}
			sceNetAdhocMatchingSelectTarget(id, peer, optLen , optArg);

		}
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_CANCEL:
		{
			BD_INFO("info","Received cancel event %s\n", addressStr);

		}
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_LEAVE:
		{
			BD_INFO("info","Received leave event %s\n", addressStr);
		}
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_ESTABLISHED:
		{
			BD_INFO("info","Received established event %s\n", addressStr);
		}
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_TIMEOUT:
		{
			BD_INFO("info","Received timeout event %s\n", addressStr);
		}
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_ERROR:
		{
			BD_INFO("info","Received error error %s\n", addressStr);
		}
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_BYE:
		{
			BD_INFO("info","Received bye event %s\n", addressStr);
		}
		break;
	}

}

