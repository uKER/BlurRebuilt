#ifndef NETTRANSPORT_LANDISCOVERY_H
#define NETTRANSPORT_LANDISCOVERY_H

#ifdef _MSC_VER
#pragma once
#endif


#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>
#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryClient.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryListener.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryServer.h>

namespace netTransport
{
	BD_REFERENCE(LANDiscoveryServer);
	class LANDiscoveryServer: public bdLANDiscoveryServer, public bdReferencable
	{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		LANDiscoveryServer();
		~LANDiscoveryServer();

		static bdGameInfoRef createGame(U32 titleId);
	};


	BD_REFERENCE(LANDiscoveryClient);
	class LANDiscoveryClient: public bdLANDiscoveryListener, public bdLANDiscoveryClient, public bdReferencable
	{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		LANDiscoveryClient();
		~LANDiscoveryClient();

		void onDiscovery(bdGameInfoRef gameInfo);

		bdLANDiscoveryClient m_client;
	
		bdLinkedList<bdGameInfoRef> m_results;
	};
}

#endif //NETTRANSPORT_LANDISCOVERY_H