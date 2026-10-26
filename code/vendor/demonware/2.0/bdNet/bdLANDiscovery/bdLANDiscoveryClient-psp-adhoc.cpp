// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdNet.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryClient.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryListener.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>

#include <pspnet_adhoc_matching.h>

#define MATCHING_POOLSIZE	32*1024
#define MATCHING_MAXIMUM	16
#define MATCHING_PORT		50000
#define MATCHING_RXBUFLEN	0x2000
#define HELLO_INTERVAL		5*1000*1000
#define KEEP_ALIVE_INTERVAL	10*1000*1000
#define KEEP_ALIVE_COUNT	3
#define REXMT_INTERVAL		500*1000

#define MATCHING_EVENT_TPL			0x10
#define MATCHING_EVENT_STACKSIZE	0x8000
#define MATCHING_INPUT_TPL			0x10
#define MATCHING_INPUT_STACKSIZE	0x8000


bdLANDiscoveryClient *g_discoveryClient = BD_NULL;

void bdLANDiscoveryClientHandler(bdInt id, bdInt event, struct SceNetEtherAddr *peer,
									 bdInt optLen, void *optArg)
{
	bdNChar8 addressStr[18];

	sceNetEtherNtostr(peer, addressStr);
	switch (event)
	{
		case SCE_NET_ADHOC_MATCHING_EVENT_ACCEPT:
		{
			bdGameInfoRef gameInfo;
			bdInetAddr gameAddr(peer->data);
			BD_INFO("info","Received Accept event:%s\n" , addressStr);
		}
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_HELLO:
		{

			BD_INFO("info","Received Hello event:%s, id: %i\n", addressStr, id);

			bdGameInfoRef gameInfo = bdGameInfoFactory::getInstance()->create();

			bdInetAddr gameAddr(peer->data);
			bdAddr addr(gameAddr,0);
			bdBitBuffer buffer((bdUByte8*)optArg, optLen << 3);
			bdCommonAddrRef me = bdNet::getInstance()->getLocalCommonAddr();
			gameInfo->deserialize(me , buffer);

			if(g_discoveryClient)
			{
				g_discoveryClient->fireOnDiscoveryListeners(gameInfo);
			}
			/*
			int res = sceNetAdhocMatchingSelectTarget(id, peer, 0,0);
			if (res < 0 )
			{
				BD_ERR("err","error calling sceNetAdhocMatchingSelectTarget: %x\n");
			}
			*/
		}
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_LEAVE:
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_ESTABLISHED:
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_TIMEOUT:
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_ERROR:
		break;
		case SCE_NET_ADHOC_MATCHING_EVENT_BYE:
		break;
	}

}

bdLANDiscoveryClient::bdLANDiscoveryClient()
: m_contextID(0)
{
	BD_INFO("","bdLANDiscoveryClient \n");
	bdInt ret = sceNetAdhocMatchingInit(MATCHING_POOLSIZE);
	if (ret < 0)
	{
			BD_WARN("warn", "sceNetAdhocMatchingInit failed %x\n", ret);
	}
	m_status = BD_IDLE;
	m_timer.reset();
	if(g_discoveryClient != BD_NULL)
	{
		BD_WARN("warn","Another discovery client instance is in use.");
	}
	g_discoveryClient = this;
}


bdLANDiscoveryClient::~bdLANDiscoveryClient()
{

	bdInt ret = sceNetAdhocMatchingStop(m_contextID);
	if (ret < 0)
	{
		  BD_ERR("err", "sceNetAdhocMatchingStop failed %x\n",ret);
	}
	ret = sceNetAdhocMatchingDelete(m_contextID);
	if (ret < 0)
	{
		  BD_ERR("err", "sceNetAdhocMatchingDelete failed %x\n",ret);
	}
	ret = sceNetAdhocMatchingTerm();
	if (ret < 0)
	{
		BD_ERR("err", "sceNetAdhocMatchingTerm failed %x\n",ret);
	}

	g_discoveryClient = BD_NULL;

}

bdBool bdLANDiscoveryClient::discover(const bdUInt titleID,
								   const bdFloat32 timeout,
								   const bdInetAddr&,
								   const bdPort)
{

	BD_INFO("info","Starting discovery client\n");
	if (m_status == BD_PENDING)
	{
		return false;
	}

	bdInt ret;
	if (m_contextID == 0)
	{
		ret = sceNetAdhocMatchingCreate( SCE_NET_ADHOC_MATCHING_MODE_CHILD, MATCHING_MAXIMUM,
			MATCHING_PORT, MATCHING_RXBUFLEN, HELLO_INTERVAL, KEEP_ALIVE_INTERVAL,
			KEEP_ALIVE_COUNT, REXMT_INTERVAL, bdLANDiscoveryClientHandler);
		if (ret < 0)
		{
			BD_ERR("err", "sceNetAdhocMAtchingCreate failed %x\n", ret);
			return false;
		}
		m_contextID = ret;

		ret = sceNetAdhocMatchingStart(m_contextID,
					MATCHING_EVENT_TPL, MATCHING_EVENT_STACKSIZE,
					MATCHING_INPUT_TPL, MATCHING_INPUT_STACKSIZE,
					0,
					BD_NULL);
		if (ret < 0)
		{
			BD_ERR("err", "sceNetAdhocMatchingStart failed %x \n", ret);
			return false;
		}
	}
	m_timeout = timeout;
	m_timer.start();
	m_status = BD_PENDING;
	return true;

}

void bdLANDiscoveryClient::update()
{
	if (m_status == BD_PENDING)
	{
		bdFloat32 time = m_timer.getElapsedTimeInSeconds() ;
		bdPlatformTiming::sleep(100);
		if (time > m_timeout )
		{
			m_status = BD_IDLE;
		}
	}
}

bdLANDiscoveryClient::bdStatus bdLANDiscoveryClient::getStatus() const
{
	return m_status;
}


void bdLANDiscoveryClient::registerListener(bdLANDiscoveryListener *listener)
{
	m_listeners.pushBack(listener);
}

void bdLANDiscoveryClient::unregisterListener(bdLANDiscoveryListener *listener)
{
	m_listeners.removeAll(listener);
}

void bdLANDiscoveryClient::fireOnDiscoveryListeners(bdGameInfoRef gameInfo) const
{
	for (bdUInt i = 0; i < m_listeners.getSize(); i++)
	{
		m_listeners[i]->onDiscovery(gameInfo);
	}
}

