#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include <SystemProfile/SystemProfileManager.h>
#include <cell/sysmodule.h>
#include <np.h>
#include <netex/libnetctl.h>
#include "Platformcallbacks.h"
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>
#include "../GameApp.h"
#include <sysutil/sysutil_gamecontent.h>
#include <PS3Installer/PS3InstallData.h>

namespace SonyNetworkPlatform
{
	// allocated by sony for the Amax project.
	SceNpCommunicationId amax_network_platform_communication_id = 
	{
		{'N','P','W','R','0','0','5','6','7'},
		'\0',
		0,
		0
	};

	// this is the development title ID ! not the final one
	const char* amax_network_platform_title_id = "NPXX00471_00";

	// allocated by sony for the amax project.
	// service id is used to authenticate to demonware and also for dlc processing.
	// amax_network_platform_service_id_prefix + amax_network_platform_title_id =  service id
	const char* amax_network_platform_service_id_prefix  = "EP0002-";
	const char* amax_network_platform_service_id = "EP0002-BLES00759_00";

	const char* amax_network_platform_service_id_prefix_us  = "UP0002-";
	const char* amax_network_platform_service_id_us = "UP0002-BLUS30295_00";

	const char* amax_network_platform_service_id_prefix_jp  = "JP0082-";
	const char* amax_network_platform_service_id_jp = "JP0082-BLJM60244_00";

	const char* get_amax_network_platform_service_id_prefix()
	{
		if (memcmp(PlatformPS3::g_ps3_titleId,"BLUS",4) == 0)
		{
			return amax_network_platform_service_id_prefix_us;
		}
		else if (memcmp(PlatformPS3::g_ps3_titleId,"BLJM", 4) == 0)
		{
			return amax_network_platform_service_id_prefix_jp;
		}
		else
		{
			return amax_network_platform_service_id_prefix;
		}
	}
	
	const char* get_amax_network_platform_service_id()
	{
		if (memcmp(PlatformPS3::g_ps3_titleId,"BLUS",4) == 0)
		{
			return amax_network_platform_service_id_us;
		}
		else if (memcmp(PlatformPS3::g_ps3_titleId,"BLJM", 4) == 0)
		{
			return amax_network_platform_service_id_jp;
		}
		else
		{
			return amax_network_platform_service_id;
		}
	}

}

namespace PlatformCallbacks
{	
static void ForceDisconnect(void *);
static void ForceConnect(void *);
}

Debug::Function g_ForceDisconnectPS3("Network.PS3.ForceDisconnect", PlatformCallbacks::ForceDisconnect, 0);
Debug::Function g_ForceConnectPS3("Network.PS3.ForceConnect", PlatformCallbacks::ForceConnect, 0);

namespace PlatformCallbacks
{	
	Debug::Logger &g_CallbacksLogger = Debug::GetLogger("racer.platform.callbacks");

	MT::Mutex g_pollMutex;
	bool SystemMenuOpen = false;

	// by default use slot 3 just incase any other game system is using slots 0,1,2
	static const S32 sysutil_callback_slot = 3;

	void SysutilCallback(uint64_t status, uint64_t param, void *userdata)
	{
		switch (status)
		{
		case CELL_SYSUTIL_REQUEST_EXITGAME: // exit the game asap..
			((C_GameApp*)Framework::C_Application::GetInstance())->Shutdown(true);
			break;

		case CELL_SYSUTIL_NET_CTL_NETSTART_LOADED:
			break;

		case CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED:
			if (RacerNet::C_NetServices::Instance().IsInitialised())
				RacerNet::C_NetServices::Instance().Router().NetStartDialogFinished();
			break;

		case CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED:
			if (RacerNet::C_NetServices::Instance().IsInitialised())
				RacerNet::C_NetServices::Instance().Router().NetStartDialogUnloaded();
			break;

		case CELL_SYSUTIL_SYSTEM_MENU_OPEN:  /* System menu opened */
			SystemMenuOpen = true;
			C_PS3InstallData::Instance().SetThrottle( 200 );
			C_PS3InstallData::Instance().SetXMBActiveState( true );
			((C_GameApp*)Framework::C_Application::GetInstance())->SystemMenuOpen();
			break;
				
		case CELL_SYSUTIL_SYSTEM_MENU_CLOSE: /* System menu closed */
			SystemMenuOpen = false;
			C_PS3InstallData::Instance().SetThrottle( 0 );
			C_PS3InstallData::Instance().SetXMBActiveState( false );
			((C_GameApp*)Framework::C_Application::GetInstance())->SystemMenuClose();
			break;

		default:
			break;
		}
	}

	static void NpManagerCallback(int event, int result, void *arg)
	{
		// np online / offline notifications
		SysProfile::C_Manager::Instance().NpManagerCallback(event, result, arg);

		// ticketing info for network router : demonware authentication.
		if (RacerNet::C_NetServices::Instance().IsInitialised())
			RacerNet::C_NetServices::Instance().Router().PlatformManagerCallback(event, result, arg);
	}

	static void ForceDisconnect(void *)
	{
		NpManagerCallback(SCE_NP_MANAGER_STATUS_OFFLINE,0,0);
	}

	static void ForceConnect(void *)
	{
		NpManagerCallback(SCE_NP_MANAGER_STATUS_ONLINE,0,0);
	}


	static int NpBasicEventHandler(int event, int retCode, uint32_t reqId, void *arg)
	{
		SysProfile::C_Manager::Instance().NpBasicEventHandler(event, retCode, reqId, arg);

		switch (event)
		{
		case SCE_NP_BASIC_EVENT_ADD_PLAYERS_HISTORY_RESULT:
			RacerNet::C_NetServices::Instance().FinishedAddingRecentPlayers();
			break;
		}

		return 0;
	}
	
//	void DiscChangeCallbackEject( void )
//	{
//		((C_GameApp*)Framework::C_Application::GetInstance())->m_DiscEjected = true;
//	}


	void Register(void)
	{
		S32 sysutil = cellSysutilRegisterCallback(sysutil_callback_slot, SysutilCallback, NULL);
		if (0 != sysutil)
		{
			LOGWARN(g_CallbacksLogger, "PlatformCallbacks::Register() - Sysutil callback registration failed on slot %d.", sysutil_callback_slot);
		}

		// connection changes etc callback
		S32 manager = sceNpManagerRegisterCallback(NpManagerCallback, NULL); 
		if (0 != manager)
		{
			LOGWARN(g_CallbacksLogger, "PlatformCallbacks::Register() - NP Manager callback registration failed.");
		}

		// friends management callback.
		S32 basic = sceNpBasicRegisterContextSensitiveHandler(&SonyNetworkPlatform::amax_network_platform_communication_id, NpBasicEventHandler, NULL); 
		if (0 != basic)
		{
			LOGWARN(g_CallbacksLogger, "PlatformCallbacks::Register() - NP Basic callback registration failed.");
		}
		
//		S32 disc = cellGameRegisterDiscChangeCallback( DiscChangeCallbackEject, NULL );
//		if (0 != disc)
//		{
//			LOGWARN(g_CallbacksLogger, "PlatformCallbacks::Register() - Disc Eject callback registration failed.");
//		}
	}

	void UnRegister(void)
	{
		S32 basic = sceNpBasicUnregisterHandler();
		if (0 != basic)
		{
			LOGWARN(g_CallbacksLogger, "PlatformCallbacks::UnRegister() - NP Basic callback registration removal failed.");
		}

		S32 manager = sceNpManagerUnregisterCallback();
		if (0 != manager)
		{
			LOGWARN(g_CallbacksLogger, "PlatformCallbacks::UnRegister() - NP Manager callback registration removal failed.");
		}

		S32 sysutil = cellSysutilUnregisterCallback(sysutil_callback_slot);
		if (0 != sysutil)
		{
			LOGWARN(g_CallbacksLogger, "PlatformCallbacks::UnRegister() - Sysutil callback registration removal failed.");
		}
	}

	void Poll(void)
	{
		g_pollMutex.Wait();
		cellSysutilCheckCallback();
		g_pollMutex.Release();
	}
} // end namespace PlatformCallbacks
