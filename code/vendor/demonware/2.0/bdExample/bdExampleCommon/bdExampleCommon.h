// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_EXAMPLE_COMMON_H
#define BD_EXAMPLE_COMMON_H

#include <bdCore/bdCore.h>

#include <bdGraphics/bdVector4.h>

#include <bdExample/bdExampleCommon/Sphere.h>
#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>

#include <bdNet/bdNet.h>

#include <bdGraphics/bdDemo.h>

#include <bdConnection/bdConnectionListener.h>
#include <bdConnection/bdConnection.h>
#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdMessage/bdMessageTypes.h>

#include <bdNet/bdLANDiscovery/bdLANDiscoveryServer.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryClient.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryListener.h>
#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>
#include <bdNet/bdGetHostByName/bdGetHostByName.h>

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

class DiscoveryManager : public bdLANDiscoveryListener
{

	public:

		virtual ~DiscoveryManager();

		bdGameInfoRef discoverLanGames(const bdUInt32 titleID);

		bdGameInfoRef createGame(const bdUInt32 titleID);

		bdGameInfoRef directConnect(const bdUInt32 titleID,
									const bdNChar8 *const publicAddr);

		virtual void onDiscovery(bdGameInfoRef gameInfo);

		bdLinkedList<bdGameInfoRef>& getFoundGames();

	protected:

		/// A linked list of games we found
		bdLinkedList<bdGameInfoRef> m_gameList;

};



#endif // BD_EXAMPLE_COMMON_H
