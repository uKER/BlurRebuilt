
#include "stdafx.h"
#include "netGameWorldEmptyApp.h"
#include "netCore\netStats\netLog.h"
#include "netServices\netServices.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netGame
{
	worldEmptyApp::worldEmptyApp()
	{
		AddSubscriber(netGame::E_Msg_CommandLoad);
		AddSubscriber(netGame::E_Msg_CommandUnload);
		AddSubscriber(netGame::E_Msg_CommandClearActors);
	}

	bool worldEmptyApp::MessageHandler(const Messaging::C_MessageBase* msg)
	{
		switch(msg->Type())
		{
		default:
			{
				return worldInterface::MessageHandler(msg);
			}
		case netGame::E_Msg_CommandLoad:
			{
				netLogs::instance()->log(NET_LOG_PARTY, netLog::green, "<netGame::worldEmptyApp> world LOADED.");
				
				netGame::C_Msg_NotificationLoaded loaded;
				netServices::instance()->BroadcastMessage(&loaded);
				return true;
			}
		case netGame::E_Msg_CommandClearActors:
			{
				netLogs::instance()->log(NET_LOG_PARTY, netLog::green, "<netGame::worldEmptyApp> world ACTORS CLEARED.");
				
				netGame::C_Msg_NotificationActorsCleared cleared;
				netServices::instance()->BroadcastMessage(&cleared);
				return true;
			}
		case netGame::E_Msg_CommandUnload:
			{
				netLogs::instance()->log(NET_LOG_PARTY, netLog::green, "<netGame::worldEmptyApp> world UNLOADED.");
				
				netGame::C_Msg_NotificationUnloaded unloaded;
				netServices::instance()->BroadcastMessage(&unloaded);
				return true;
			}
		}
	}
};

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

