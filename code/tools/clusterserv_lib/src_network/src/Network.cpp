#include "Network/StdAfx.h"
#include "Network/Network.h"
#include "Network/SerializeBin.h"
#include "Network/Connection.h"

Debug::Logger &g_NetLog = Debug::GetLogger("tools.clusterlib");

// destroy all services
C_Network::~C_Network()
{
	for (int i=0; i<m_Services.Size(); i++)
		delete m_Services[i];
	m_Services.Destroy();
}

// get a socket service with an empty slot
ost::SocketService *C_Network::GetService()
{
	// find one with a free slot
	for (int i=0; i<m_Services.Size(); i++)
	{
		if (m_Services[i]->getCount() < 60)
			return m_Services[i];
	}

	// create a new service
	ost::SocketService *pService = new ost::SocketService();
	m_Services.PushBack(pService);
	return pService;
}
