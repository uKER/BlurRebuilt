// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>

#if !defined BD_PLATFORM_PSP_ADHOC
#error "BD_PLATFORM_PSP_ADHOC must be defined"
#endif

#define BD_LOG_LEVEL "bdExampleCommon/init-psp/adhoc"

#include <bdNet/bdNet.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

///// #includes

#include <kernel.h>
#include <pspnet.h>


#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>

#include <utility/utility_netconf.h>
#include <utility/utility_module.h>

#define BD_MAIN_THREAD_PRIO		0x40

#define PSPNET_MEMPOOLSIZE		0x80000
#define CALLOUT_THREADPRIO		0x35

#define CALLOUT_STACKSIZE		0x4000
#define NETINTR_THREADPRIO		0x35

#define NETINTR_STACKSIZE		0x4000

#define PSP_ADHOC_MEMPOOLSIZE	0x2000
#define PSP_ADHOC_THREADPRIO	0x30


SceUID ifhandle_modid, memab_modid, pspnet_modid;

SceUID pspnet_adhoc_auth_modid, pspnet_adhoc_modid, pspnet_adhocctl_modid,	pspnet_adhoc_matching_modid;

struct SceNetAdhocctlAdhocId adhocID =
{
	SCE_NET_ADHOCCTL_ADHOCTYPE_RESERVED,
	{'S','C','E','S','M','P','L','0','5'}
};


static SceUtilityNetconfParam netconfParam;
static SceUtilityNetconfAdhocParam netconfAdhocParam;

static int evf = 0;
static int error_code = 0;
static int handlerId = 0;

#define EVF_ERROR		0x01
#define EVF_CONNECT		0x02
#define EVF_DISCONNECT	0x04
#define EVF_SCAN		0x08

void AdhocEventHandler (int event, int error, void *arg);
int utility_init_netconf(SceUtilityNetconfType type, struct SceNetAdhocctlGroupName *group_name);
int wait_connect(void) ;

void printInfo()
{
	struct SceNetAdhocctlParameter param;
	int ret = sceNetAdhocctlGetParameter(&param);
	if (ret < 0)
	{
		BD_INFO(BD_LOG_LEVEL, "sceNetAdhocctlScan() failed. ret = 0x%x\n", ret);

	}
	BD_INFO(BD_LOG_LEVEL, "Channel:%i\n", param.channel);
	BD_INFO(BD_LOG_LEVEL, "BSSID: %x:%x:%x:%x:%x:%x\n", param.bssid.data[0],
										   param.bssid.data[1],
										   param.bssid.data[2],
										   param.bssid.data[3],
										   param.bssid.data[4],
										   param.bssid.data[5]);
}


int loadAllModules(const char *modulePath)
{
	int ret = 0;

	ret = sceUtilityLoadModule(SCE_UTILITY_MODULE_NET_COMMON);
	if(ret < 0){
		bdPrintf("sceUtilityLoadNetModule(SCE_UTILITY_NET_MODULE_COMMON) failed. ret = 0x%x\n", ret);
		goto error;
	}

	ret = sceUtilityLoadModule(SCE_UTILITY_MODULE_NET_ADHOC);
	if(ret < 0){
		bdPrintf("sceUtilityLoadNetModule(SCE_UTILITY_NET_MODULE_ADHOC) failed. ret = 0x%x\n", ret);
		goto error;
	}

error:
	return ret;
}

int unloadAllModules()
{
	int ret = 0;

	ret = sceUtilityUnloadModule(SCE_UTILITY_MODULE_NET_COMMON);
	if (ret < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "unloadModule() failed. ret = 0x%x\n", ret);
	}

	ret = sceUtilityUnloadModule(SCE_UTILITY_MODULE_NET_ADHOC);
	if (ret < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "unloadModule() failed. ret = 0x%x\n", ret);
	}
	return ret;
}

bdPlatformParams::bdPlatformParams()
{
}

bdInt BD_CALL bdExamplePlatformInit::init(bdNetStartParams &params,
										  bdPlatformParams *platformParams)
{
	// set the main thread priority to a relatively low value
	bdThreadHandle threadhandle = bdPlatformThread::getThreadHandle();
	bdPlatformThread::setPriority(threadhandle, BD_MAIN_THREAD_PRIO);

	int ret;
	if (platformParams && platformParams->m_modulePath.getLength() > 1)
	{
		BD_INFO(BD_LOG_LEVEL, "Loading modules from path %s", platformParams->m_modulePath.getBuffer());
		ret = loadAllModules(platformParams->m_modulePath.getBuffer());
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "No module path specified, using default");
		ret = loadAllModules("host0:/usr/local/sce/psp/devkit/");
	}

	if (ret < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "load modules failed\n");
		return ret;
	}

	ret = sceNetInit(PSPNET_MEMPOOLSIZE, CALLOUT_THREADPRIO,
						 CALLOUT_STACKSIZE, NETINTR_THREADPRIO,
						 NETINTR_STACKSIZE);
	if (ret < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "sceNetInit failed %i\n", ret);
		return ret;
	}

	ret = sceNetAdhocInit();
	if (ret < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "sceNetAdhocInit failed %i\n", ret);
		return ret;
	}


	// set the mac address
	SceNetEtherAddr macAddr;

	ret = sceNetAdhocctlInit(PSP_ADHOC_MEMPOOLSIZE, PSP_ADHOC_THREADPRIO, &adhocID);
	if (ret != 0)
	{
		BD_ERR(BD_LOG_LEVEL, "error calling sceNetAdhocctlInit: %x\n", ret);
	}
	ret = sceNetAdhocctlAddHandler(AdhocEventHandler, BD_NULL);
	if (ret != 0)
	{
		BD_ERR(BD_LOG_LEVEL, "error calling sceNetAdhocctlAddHandler: %x\n", ret);
	}
	if (ret >= 0)
	{
		handlerId = ret;
	}
	struct SceNetAdhocctlGroupName group_name;
	memset(&group_name, 0, sizeof(struct SceNetAdhocctlGroupName));
	ret = sceNetAdhocctlConnect(&group_name);
	if (ret < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "sceNetAdhocctlConnect() failed. ret = 0x%x\n", ret);
		return ret;
	}
	/*
	ret = utility_init_netconf(SCE_UTILITY_NETCONF_TYPE_CONNECT_ADHOC,
	    					NULL);
	if (ret < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "utility_init_netconf failed: 0x%x", ret);
		return ret;
	}*/
	wait_connect();

	bdInt result = sceNetGetLocalEtherAddr(&macAddr);

	// set the IP address -- same as mac address for psp ad hoc...
	params.m_localAddresses.pushBack(bdInetAddr(macAddr.data));
	printInfo();
	return ret;
}


int utility_init_netconf(SceUtilityNetconfType type, struct SceNetAdhocctlGroupName *group_name)
{
	memset(&netconfParam, 0x00, sizeof(SceUtilityNetconfParam));
	netconfParam.base.size = sizeof(SceUtilityNetconfParam);
	netconfParam.base.ctrl_assign = SCE_UTILITY_CTRL_ASSIGN_CIRCLE_IS_ENTER;
	netconfParam.base.main_thread_priority = SCE_KERNEL_USER_HIGHEST_PRIORITY + 1;
	netconfParam.base.sub_thread_priority = SCE_KERNEL_USER_HIGHEST_PRIORITY + 3;
	netconfParam.base.font_thread_priority = SCE_KERNEL_USER_HIGHEST_PRIORITY + 2;
	netconfParam.base.sound_thread_priority = SCE_KERNEL_USER_HIGHEST_PRIORITY;
	netconfParam.type = type;

	if(group_name == NULL)
		memset(&netconfAdhocParam, 0, sizeof(netconfAdhocParam));
	else
		memcpy(&netconfAdhocParam, group_name, sizeof(netconfAdhocParam));
	netconfParam.adhoc_param = &netconfAdhocParam;

	int ret = sceUtilityNetconfInitStart(&netconfParam);
	if(ret)
	{
		BD_ERR(BD_LOG_LEVEL,  "sceUtilityNetconfInitStart failed - 0x%x\n", ret );
	}

	return ret;
}


int wait_connect(void)
{
	while(1)
	{
		if(evf & EVF_CONNECT)
			break;
		if(evf & EVF_ERROR)
		{
			evf &= ~EVF_ERROR;
			break;
		}
		sceKernelDelayThread(1000 * 1000);
	}
	evf &= ~EVF_CONNECT;

	return error_code;
}



void AdhocEventHandler (int event, int error, void *arg)
{
	if (event == SCE_NET_ADHOCCTL_EVENT_ERROR)
	{
		error_code = error;
		evf |= EVF_ERROR;
	}
	else if (event == SCE_NET_ADHOCCTL_EVENT_CONNECT)
	{
		error_code = 0;
		evf |= EVF_CONNECT;
	}
	else if (event == SCE_NET_ADHOCCTL_EVENT_DISCONNECT)
	{
		error_code = 0;
		evf |= EVF_DISCONNECT;
	}
	else if (event == SCE_NET_ADHOCCTL_EVENT_SCAN)
	{
		error_code = 0;
		evf |= EVF_SCAN;
	}

}

void BD_CALL bdExamplePlatformInit::quit()
{

	sceNetAdhocctlDisconnect();
	if (handlerId > 0)
	{
		sceNetAdhocctlDelHandler(handlerId);
	}
	sceNetAdhocctlTerm();

	sceNetAdhocTerm();
	sceNetTerm();
	unloadAllModules();
}

