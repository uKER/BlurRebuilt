// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>
#include <bdNet/bdNet.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <bdGraphics/bdGfx.h>

#include <netex/net.h>
#include <netex/libnetctl.h>
#include <sysutil/sysutil_common.h>
#include <cell/sysmodule.h>
#include <cell/http.h>


#define BD_LOG_LEVEL "bdExampleCommon/init-ps3"
#define MAX_NETCTL_WAIT 5.0f

bdBool g_dialogIsUp = false;
bdInt g_netStartDialogResult = 0;

#define HTTP_POOL_SIZE (1024*64)
bdByte8 g_libHttpMemPool[HTTP_POOL_SIZE];

#define SSL_POOL_SIZE  (256 * 1024)
bdByte8 g_libSSLMemPool[SSL_POOL_SIZE];


bdPlatformParams::bdPlatformParams()
: m_NPSignIn(false)
{
}

void BD_CALL CellSysUtilCallback(uint64_t status,
								 uint64_t param,
								 void * userdata)
{
	switch(status)
	{
		case CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED:
		{
			CellNetCtlNetStartDialogResult netstartResult;
			bdMemset(&netstartResult, BD_NULL, sizeof(netstartResult));
			netstartResult.size = sizeof(netstartResult);

			const bdInt ret = cellNetCtlNetStartDialogUnloadAsync(&netstartResult);
			if(ret == 0)
			{
				g_netStartDialogResult = netstartResult.result;
			}
			else
			{
				// Return value stored in netstart_result.type
				BD_ERR(BD_LOG_LEVEL, "cellNetCtlNetStartDialogUnloadAsync(..) failed with 0x%x.", ret);			
			}			
		}
		break;
		case CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED:
		{
			// Termination of network start dialog utility complete
			g_dialogIsUp = false;
		}
		break;
	}
}

bdInt BD_CALL bdExamplePlatformInit::init(bdNetStartParams &params,
										  bdPlatformParams *platformParams)
{
	bdInt ret = 0;
	
	// load the networking modules (libnet + libnetctl)
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
	if(ret != CELL_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "cellSysmoduleLoadModule(CELL_SYSMODULE_NET) failed with 0x%x.", ret);
	}
	else
	{
		// just incase CELL_OK is not 0
		ret = 0;
	}
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_HTTP);
	if (ret != CELL_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "cellSysmoduleLoadModule(CELL_SYSMODULE_HTTP) failed with 0x%x.", ret);
		return 0;
	}
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_HTTPS);
	if (ret != CELL_OK) 
	{
		BD_ERR(BD_LOG_LEVEL, "cellSysmoduleLoadModule failed (0x%x)\n", ret);
		return 0;
	}

	// start libnet
	if(ret == 0)
	{
		// TODO: use sys_net_initialize_network_ex? Docs are unclear on if they require the
		// memory pointed to by sys_net_initialize_parameter_t to remain after the call.
		ret = sys_net_initialize_network();
		if(ret != 0)
		{
			BD_ERR(BD_LOG_LEVEL, "sys_net_initialize_network() failed with 0x%x.", ret);
		}
	}
	ret = cellHttpInit(g_libHttpMemPool, HTTP_POOL_SIZE);
	if (ret <0)
	{
		BD_ERR(BD_LOG_LEVEL, "cellHttpInit failed failed with 0x%x.", ret);
		return 0;
	}
	
	ret = cellSslInit(g_libSSLMemPool, SSL_POOL_SIZE);
	if (ret < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "unable to start libssl... (0x%x)\n", ret);
		return 0;
	}

	// start libnetctl
	if(ret == 0)
	{
		ret = cellNetCtlInit();
		if(ret != 0)
		{
			BD_ERR(BD_LOG_LEVEL, "cellNetCtlInit() failed with 0x%x.", ret);
		}
	}

	// register a callback
	if(ret == 0)
	{
		ret = cellSysutilRegisterCallback(0, CellSysUtilCallback, BD_NULL);
		if(ret != 0)
		{
			BD_ERR(BD_LOG_LEVEL, "cellSysutilRegisterCallback(..) failed with 0x%x.", ret);
		}
	}

	// Wait till we have obtained an IP, or we time out 
	if( ret == 0 )
	{
		bdInt connectionStatus;
		bdFloat32 delay = 0.0f;
		do
		{
			ret = cellNetCtlGetState( &connectionStatus );
			if( ret!= 0 )
			{
				BD_ERR(BD_LOG_LEVEL, "cellNetCtlGetState(..) failed with 0x%x", ret);				
			}
			else
			{
				if( CELL_NET_CTL_STATE_IPObtained == connectionStatus )
				{
					break;
				}
			}

			if( MAX_NETCTL_WAIT < delay )
			{
				BD_WARN(BD_LOG_LEVEL, "Timeout while trying to obtain IP");				
				break;
			}

			sys_timer_usleep( 100000 );
			delay += 0.1f;
		}
		while( true );
	}

	
	// open the net start dialog
	if(ret == 0)
	{
		CellNetCtlNetStartDialogParam netstartParam;
		bdMemset(&netstartParam, BD_NULL, sizeof(netstartParam));

		netstartParam.size = sizeof(netstartParam);
		netstartParam.type = CELL_NET_CTL_NETSTART_TYPE_NP;
		if(platformParams->m_NPSignIn)
		{
			netstartParam.type = CELL_NET_CTL_NETSTART_TYPE_NP;
		}

		ret = cellNetCtlNetStartDialogLoadAsync(&netstartParam);
		if(ret != 0)
		{
			BD_ERR(BD_LOG_LEVEL, "cellNetCtlNetStartDialogLoadAsync(..) failed with 0x%x.", ret);
		}
		else
		{
			g_dialogIsUp = true;
		}
	}

	// wait till the dialog closes
	while(g_dialogIsUp && (ret == 0))
	{
		ret = cellSysutilCheckCallback();
		bdPlatformTiming::sleep(10);
		bdGfx::getInstance()->runFrame();

		if(ret != CELL_OK)
		{
			BD_ERR(BD_LOG_LEVEL, "cellSysutilCheckCallback() failed with 0x%x.", ret);
		}
		else
		{
			// just incase CELL_OK is not 0
			ret = 0;
		}
	}
	

	// check the net start dialog result if no errors so far
	if(ret == 0)
	{
		ret = g_netStartDialogResult;
	}

	// get the ip address
	if(ret == 0)
	{
		CellNetCtlInfo info;
		bdMemset(&info, BD_NULL, sizeof(info));

		ret = cellNetCtlGetInfo(CELL_NET_CTL_INFO_IP_ADDRESS, &info);

		if(ret == 0)
		{
			params.m_localAddresses.pushBack(bdInetAddr(info.ip_address));			
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "cellNetCtlGetInfo() failed with 0x%x.", ret);
		}				
	}

	return ret;
}

void BD_CALL bdExamplePlatformInit::quit()
{
	// Important: this frees up any thread contexts that were implicitly
	// allocated by libnet socket functions
	bdInt ret = sys_net_free_thread_context(0, SYS_NET_THREAD_ALL);
	if(ret != 0)
	{
		BD_ERR(BD_LOG_LEVEL, "sys_net_free_thread_context(..) failed with 0x%x.", ret);
	}
	
	ret = sys_net_finalize_network();
	if(ret != 0)
	{
		BD_ERR(BD_LOG_LEVEL, "sys_net_finalize_network() failed with 0x%x.", ret);
	}

	cellSslEnd();
	cellHttpEnd();


	cellNetCtlTerm();
}

