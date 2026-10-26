#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include <SystemProfile/SystemProfileManager.h>
#include <cell/sysmodule.h>
#include <np.h>
#include <netex/libnetctl.h>
#include "Platformcallbacks.h"
#include "../Network/Core/NetServices.h"
#include "../Network/Router/NetRouterBase.h"

namespace PlatformCallbacks
{	
	Debug::Logger &g_CallbacksLogger = Debug::GetLogger("racer.platform.callbacks");

	// nicked this from the np sample code, 
	// we will need to request a correct communication id for the game
	SceNpCommunicationId amax_network_platform_communication_id = 
	{
		{'N','P','X','S','0','0','0','0','5'},
		'\0',
		0,
		0
	};

	// by default use slot 3 just incase any other game system is using slots 0,1,2
	static const S32 sysutil_callback_slot = 3;

	void SysutilCallback(uint64_t status, uint64_t param, void *userdata)
	{
		switch (status)
		{
		case CELL_SYSUTIL_REQUEST_EXITGAME: // exit the game asap..
			break;

		case CELL_SYSUTIL_NET_CTL_NETSTART_LOADED:
			break;

		case CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED:
			{
				CellNetCtlNetStartDialogResult net_start_gui_result;
				memset(&net_start_gui_result, 0, sizeof(net_start_gui_result));
				net_start_gui_result.size = sizeof(net_start_gui_result);

				S32 ret = cellNetCtlNetStartDialogUnloadAsync(&net_start_gui_result);
				if (ret < 0)
				{
					// TODO -> should we leave game application if we get this error ?
					LOGWARN(g_CallbacksLogger, "cellNetCtlNetStartDialogUnloadAsync() - failed");
				}

				switch (net_start_gui_result.result)
				{
				case 0: // net start dialog finished successfully.
					break;

				default: // net start gui failed in some way..
					break;
				}
			}
			break;

		case CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED:
			// continue game flow now that net start gui has terminated.
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

	static int NpBasicEventHandler(int event, int retCode, uint32_t reqId, void *arg)
	{
		SysProfile::C_Manager::Instance().NpBasicEventHandler(event, retCode, reqId, arg);
		return 0;
	}

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
		S32 basic = sceNpBasicRegisterContextSensitiveHandler(&amax_network_platform_communication_id, NpBasicEventHandler, NULL); 
		if (0 != basic)
		{
			LOGWARN(g_CallbacksLogger, "PlatformCallbacks::Register() - NP Basic callback registration failed.");
		}
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
		cellSysutilCheckCallback();
	}
} // end namespace PlatformCallbacks
