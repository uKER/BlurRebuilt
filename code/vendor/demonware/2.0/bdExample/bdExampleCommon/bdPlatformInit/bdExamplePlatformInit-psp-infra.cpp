// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>

#if !defined BD_PLATFORM_PSP_INFRA
#error "BD_PLATFORM_PSP_INFRA must be defined"
#endif

#include <bdNet/bdNet.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>


///// #includes

#include <kernel.h>
#include <pspnet.h>
#include <wlan.h>


#include <pspnet_error.h>
#include <pspnet_inet.h>
#include <pspnet_resolver.h>
#include <pspnet_apctl.h>
#include <pspnet_ap_dialog_dummy.h>
#include <wlan.h>
#include <utility/utility_module.h>

#define SCE_APCTL_STACKSIZE 0x8000
static union SceNetApctlInfo apctl_info;
static int apctl_handler_id;

#define BD_MAIN_THREAD_PRIO		0x40
#define SCE_APCTL_PRIO			0x35

#define PSPNET_MEMPOOLSIZE		0x80000
#define CALLOUT_THREADPRIO		0x30
#define CALLOUT_STACKSIZE		0x4000
#define NETINTR_THREADPRIO		0x30
#define NETINTR_STACKSIZE		0x4000


static struct SceNetApDialogDummyParam ap_dialog_dummy_param;
static struct SceNetApDialogDummyStateInfo ap_dialog_dummy_state;

#define AP_DIALOG_DUMMY_WAIT_TIME (1000 * 1000)


SceUID ifhandle_modid, memab_modid, pspnet_modid;

SceUID pspnet_inet_modid, pspnet_apctl_modid, pspnet_ap_dialog_dummy_modid, resolver_modid;


void apctl_handler(	int prev_state, int new_state, int event, int error_code, void *arg )
{

	if(new_state == SCE_NET_APCTL_STATE_Disconnected)
	{
	    if(event == SCE_NET_APCTL_EVENT_DISCONNECT_REQ)
		{
			BD_WARN("err", "Disconnect event occured\n");
		}
		if(prev_state == SCE_NET_APCTL_STATE_IPObtained &&
		    event == SCE_NET_APCTL_EVENT_ERROR)
		{
			BD_ERR("err", "Apctl error happened, error = 0x%x\n", error_code);
		}
	}
}


static SceUID loadModule( const char *path, SceUID mpid )
{
	SceKernelLMOption option;
	SceUID modid = 0;
	int ret = 0, mresult;

	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);
	option.mpidtext = mpid;
	option.mpiddata = mpid;
	option.position = SCE_KERNEL_LMWO_POS_Low;
	option.access = SCE_KERNEL_LMWO_ACCESS_Noseek;

	ret = sceKernelLoadModule(path, 0, &option);
	if(ret < 0)
	{
		bdPrintf("sceKernelLoadModule() failed. ret = 0x%x\n", ret);
		goto error;
	}
	modid = ret;

	ret = sceKernelStartModule(modid, 0, 0, &mresult, NULL);
	if(ret < 0)
	{
		BD_ERR("err", "sceKernelStartModule() failed. ret = 0x%x\n", ret);
		goto error;
	}
	ret = modid;

	return ret;

error:
	if (modid > 0)
	{
		ret = sceKernelUnloadModule(modid);
		if (ret < 0)
		{
			BD_ERR("err", "sceKernelUnloadModule() failed. ret = 0x%x\n", ret);
		}
	}
	return ret;
}

static int unloadModule(SceUID modid)
{
	int ret = 0;

	ret = sceKernelStopModule(modid, 0, NULL, NULL, NULL);
	if (ret < 0)
	{
		BD_ERR("err", "sceKernelStopModule() failed. ret = 0x%x\n", ret);
		return ret;
	}

	ret = sceKernelUnloadModule(modid);
	if (ret < 0)
	{
		BD_ERR("err", "sceKernelUnloadModule() failed. ret = 0x%x\n", ret);
		return ret;
	}

	return ret;
}

int loadAllModules(const char *modulePath)
{
	const bdUInt maxPathLength = 500;
	bdNChar8 fullModulePath[maxPathLength];
	int ret = 0;
	ret = sceUtilityLoadModule(SCE_UTILITY_MODULE_NET_COMMON);
	if(ret < 0 && ret != 0x80110802){
		bdPrintf("sceUtilityLoadNetModule(SCE_UTILITY_NET_MODULE_COMMON) failed. ret = 0x%x\n", ret);
		goto error;
	}

	ret = sceUtilityLoadModule(SCE_UTILITY_MODULE_NET_INET);
	if(ret < 0 && ret!= 0x80110802){
		bdPrintf("sceUtilityLoadNetModule(SCE_UTILITY_NET_MODULE_INET) failed. ret = 0x%x\n", ret);
		goto error;
	}

	bdSnprintf(fullModulePath, maxPathLength, "%s" "module/pspnet_ap_dialog_dummy.prx" , modulePath );
	fullModulePath[maxPathLength - 1] = '\0';
	ret = loadModule(fullModulePath, SCE_KERNEL_PRIMARY_USER_PARTITION);
	if(ret < 0 && ret!= 0x80110802)
	{
		BD_ERR("err", "loadModule %s failed. ret = 0x%x\n",  fullModulePath, ret);
		goto error;
	}
	pspnet_ap_dialog_dummy_modid = ret;

error:
	return ret;
}

bdPlatformParams::bdPlatformParams()
{
}

bdInt BD_CALL bdExamplePlatformInit::init(bdNetStartParams &params,
										  bdPlatformParams *platformParams)
{
	int ret;
	bdInt priority = BD_MAIN_THREAD_PRIO;
	bdThreadHandle threadHandle = bdPlatformThread::getThreadHandle();
	bdPlatformThread::setPriority(threadHandle, priority);
	if (platformParams && platformParams->m_modulePath.getLength() > 1)
	{
		BD_INFO("info", "Loading modules from path %s", platformParams->m_modulePath.getBuffer());
		ret = loadAllModules(platformParams->m_modulePath.getBuffer());
	}
	else
	{
		BD_INFO("info", "No module path specified, using default");
		ret = loadAllModules("host0:/usr/local/psp/devkit/");
	}

	if (ret < 0)
	{
		BD_ERR("err", "load modules failed\n");
		return ret;
	}

	ret = sceNetInit(PSPNET_MEMPOOLSIZE, CALLOUT_THREADPRIO,
						 CALLOUT_STACKSIZE, NETINTR_THREADPRIO,
						 NETINTR_STACKSIZE);
	if (ret < 0)
	{
		BD_ERR("err", "sceNetInit failed 0x%x\n", ret);
		return ret;
	}

	// Initialize pspnet_inet module
	ret = sceNetInetInit();
	if(ret < 0)
	{
		BD_ERR("err", "sceNetInetInit() failed. ret = 0x%x\n", ret);
		return ret;
	}

	ret = sceNetApctlInit(SCE_APCTL_STACKSIZE, SCE_APCTL_PRIO);
	if(ret < 0)
	{
		BD_ERR("err", "sceNetApctlInit() failed. ret = 0x%x\n", ret);
		return ret;
	}

	ret = sceNetApctlAddHandler(apctl_handler, NULL);
	if(ret < 0)
	{
		BD_ERR("err", "sceNetApctlAddHandler() failed. ret = 0x%x\n", ret);
		return ret;
	}
	apctl_handler_id = ret;
	ret = sceNetApDialogDummyInit();
	if(ret < 0)
	{
		BD_ERR("err", "sceNetApDialogDummyInit() failed. ret = 0x%x\n", ret);
		return ret;
	}

	// check Wireless LAN switch
	ret = sceWlanGetSwitchState();
	if(ret == SCE_WLAN_SWITCH_STATE_OFF)
	{
		BD_ERR("err", "Wireless LAN switch has been turned off.\n");
		return ret;
	}

	memset(&ap_dialog_dummy_param, 0, sizeof(ap_dialog_dummy_param));
	//

	bdInt latestID = 0;
	ret = sceUtilityGetNetParamLatestID(&latestID);
	if(ret < 0)
	{
		BD_ERR("err", "sceUtilityGetNetParamLatestId() failed. ret = 0x%x\n", ret);
		return ret;
	}
	union SceUtilityNetParamEntry entry;
	ret = sceUtilityGetNetParam(latestID, SCE_UTILITY_NET_PARAM_CODE_SSID, &entry);
	if(ret < 0)
	{
		BD_ERR("err", "sceUtilityGetNetParam() failed. ret = 0x%x\n", ret);
		return ret;
	}
	BD_INFO("info", "Connecting to SSID %s\n", entry.ssid);
	strcpy(ap_dialog_dummy_param.ssid, entry.ssid);
	ret = sceUtilityGetNetParam(latestID, SCE_UTILITY_NET_PARAM_CODE_WEP_KEY, &entry);
	if(ret < 0)
	{
		BD_ERR("err", "sceUtilityGetNetParam() failed. ret = 0x%x\n", ret);
		return ret;
	}
	memcpy(ap_dialog_dummy_param.wep_key, entry.auth_key, sizeof(entry.auth_key));

	ret = sceUtilityGetNetParam(latestID, SCE_UTILITY_NET_PARAM_CODE_WPA_KEY, &entry);
	if(ret < 0)
	{
		BD_ERR("err", "sceUtilityGetNetParam() failed. ret = 0x%x\n", ret);
		return ret;
	}
	memcpy(ap_dialog_dummy_param.wpa_key, entry.wpa_key, SCE_UTILITY_NET_PARAM_WPA_KEY_LEN);

	ret = sceUtilityGetNetParam(latestID, SCE_UTILITY_NET_PARAM_CODE_WPA_KEY_TYPE, &entry);
	if(ret < 0)
	{
		BD_ERR("err", "sceUtilityGetNetParam() failed. ret = 0x%x\n", ret);
		return ret;
	}
	ap_dialog_dummy_param.wpa_key_type = entry.wpa_key_type;

	ret = sceUtilityGetNetParam(latestID, SCE_UTILITY_NET_PARAM_CODE_AUTH_PROTO, &entry);
	if(ret < 0)
	{
		BD_ERR("err", "sceUtilityGetNetParam() failed. ret = 0x%x\n", ret);
		return ret;
	}
	ap_dialog_dummy_param.auth_proto = entry.auth_proto;
	ret = sceNetApDialogDummyConnect(&ap_dialog_dummy_param);
	if(ret < 0)
	{
		if (ret == (int)SCE_ERROR_NET_WLAN_POWER_OFF)
		{
			BD_ERR("err", "Wireless LAN switch has been turned off.\n");
		}
		if(ret == 2148017950)
		{
			// equivalent to 0x8008271e
			// As far as we can tell this currently indicates a registry corruption in the psp :)

			// to fix you need to boot the psp to the system software mode [OSD]
			// then go to System Settings
			// select restore Factory Default Settings.

			// problem solved...

			BD_ERR("err", "Internal Error, please see code for explanation.\n");

			BD_ASSERT(false, "psp needs to be rest...");

		}
		BD_ERR("err", "sceNetApDialogDummyConnect() failed. ret = 0x%x\n", ret);
		return ret;
	}

	// it seems lke threads are getting starved so we will sleep lots...
	sceKernelDelayThread(1000 * 1000);


	while(1)
	{
		ret = sceNetApDialogDummyGetState(&ap_dialog_dummy_state);
		if(ret == 0)
		{
			if(ap_dialog_dummy_state.state == SceNetApDialogDummyState_Connected ||
		    	ap_dialog_dummy_state.state == SceNetApDialogDummyState_Disconnected)
				break;
		}
		sceKernelDelayThread(AP_DIALOG_DUMMY_WAIT_TIME);
	}
	if(ap_dialog_dummy_state.state == SceNetApDialogDummyState_Disconnected)
	{
		BD_ERR("err", "failed to Join or Get IP addr. error = 0x%x\n",
		    ap_dialog_dummy_state.error_code);
		return ret;
	}

	// it seems lke threads are getting starved so we will sleep lots...
	sceKernelDelayThread(1000 * 1000);

	ret = sceNetApctlGetInfo(SCE_NET_APCTL_INFO_IP_ADDRESS, &apctl_info);
	if(ret < 0)
	{
		BD_ERR("err", "sceNetApctlGetInfo() failed. ret = 0x%x\n", ret);
		return ret;
	}

	// set the IP address
	ret = sceNetApctlGetInfo(SCE_NET_APCTL_INFO_IP_ADDRESS, &apctl_info);
	if(ret == 0)
	{
		bdInetAddr ipAddress(apctl_info.ip_address);
		params.m_localAddresses.pushBack(ipAddress);
	}
	else if(ret < 0)
	{
		if(SCE_ERROR_NET_APCTL_INVALID_CODE == ret)
		{
			BD_ERR("err", "IP Address detection failed -- Invalid Code.");
		}
		else if(SCE_ERROR_NET_APCTL_INVALID_ADDR == ret)
		{
			BD_ERR("err", "IP Address detection failed -- Invalid Address." );
		}
		else if(SCE_ERROR_NET_APCTL_NOT_IN_BSS == ret)
		{
			BD_ERR("err", "IP Address detection failed -- Not In BSS" );
		}
	}

	ret = sceNetResolverInit();
	return ret;
}


void BD_CALL bdExamplePlatformInit::quit()
{
	int ret ;

	ret = sceNetApctlDelHandler(apctl_handler_id);
	if(ret< 0)
	{
		BD_ERR("err", "sceNetApctlDelHandler() failed. ret = 0x%x\n", ret);
	}

	ret = sceNetApctlTerm();
	if(ret< 0) 
	{
		BD_ERR("err", "sceNetApctlTerm() failed. ret = 0x%x\n", ret);
	}

	ret = sceNetResolverTerm();
	if (ret < 0)
	{
		BD_ERR("err", "sceNetResolverTerm() failed. ret = 0x%x\n", ret);
	}

	ret = sceNetInetTerm();
	if (ret != 0)
	{
		BD_ERR("err", "sceNetInetTerm() failed. ret = 0x%x\n", ret);

	}

	ret = sceNetTerm();
	if(ret) 
	{
		BD_ERR("err", "sceNetTerm() failed. ret = 0x%x\n", ret);
		return;
	}
	

	ret = unloadModule(pspnet_ap_dialog_dummy_modid);
	if (ret < 0)
	{
		BD_ERR("err", "unloadModule() failed. ret = 0x%x\n", ret);
	}
	pspnet_ap_dialog_dummy_modid = 0;


	ret = sceUtilityUnloadModule(SCE_UTILITY_MODULE_NET_INET);
	if (ret < 0) 
	{
		BD_ERR("err", "sceUtilityUnloadNetModule() failed. ret = 0x%x\n", ret);
	}

	ret = sceUtilityUnloadModule(SCE_UTILITY_MODULE_NET_COMMON);
	if (ret < 0)
	{
		BD_ERR("err", "sceUtilityUnloadNetModule() failed. ret = 0x%x\n", ret);
	}

}

