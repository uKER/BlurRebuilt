
#include "stdafx.h"
#include "./netPlatform_PS3.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Messages.h"
#include "netPlatform/netPlatform_Core.h"

#include <bdLobby/bdLobbyConnection.h>
#include <bdLobby/bdLobbyService.h>
#include <bdNet/bdNet.h>
#include <bdNet/bdNetStartParams.h>
#include <Network/Core.h>
#include <SystemProfile/SystemProfileManager.h>
#include <misc_shared/Messaging/MessageSubscriberSystem.h>

#include <np.h>
#include <netex/libnetctl.h>

//----------------------------------------------------------------
// AMAX
//----------------------------------------------------------------
/*
#define ISSUER_ID_SP_INT				1
#define ISSUER_ID_PROD_QA				2 
#define ISSUER_ID_NP					256
#define NP_SERVICE_ID					"EP0002-NPXX00471_00"
#define DW_TITLE_ID						(0x141F)
#define DW_AUTH_ADDRESS					"amax-ps3.auth.mmp3.demonware.net" 
#define DW_AUTH_PORT					(3074)
#define DW_LSG_ADDRESS_DEV				"amax.binario.mmp3.demonware.net"
#define DW_LSG_ADDRESS_CERT				"amax.binario.mmp3.demonware.net"
#define DW_LSG_ADDRESS_LIVE				"amax.binario.mmp3.demonware.net"
#define DW_LSG_PORT						(3074)
//#define DW_STREAMING_SERVICE_ADDRESS	"amax.binario.mmp3.demonware.net"	// not used since its the same address as the lsg
#define DW_STREAMING_SERVICE_PORT		(3075)
*/

//----------------------------------------------------------------
// NIGHTWING
//----------------------------------------------------------------
#define ISSUER_ID_SP_INT				1
#define ISSUER_ID_PROD_QA				2 
#define ISSUER_ID_NP					256
#define NP_SERVICE_ID					"EP0002-NPXX00531_00"
#define DW_TITLE_ID						(0x1438)
#define DW_AUTH_ADDRESS					"jb09-ps3.auth.v200.demonware.net" 
#define DW_AUTH_PORT					(3074)
#define DW_LSG_ADDRESS_DEV				"jb09-ps3-dev.lobby.v200.demonware.net"
#define DW_LSG_ADDRESS_CERT				"jb09-ps3-cert.lobby.v200.demonware.net"
#define DW_LSG_ADDRESS_LIVE				"jb09-ps3-live.lobby.v200.demonware.net"
#define DW_LSG_PORT						(3074)
//#define DW_STREAMING_SERVICE_ADDRESS	"sbox200-lsg.mmp3.demonware.net"	// not used since its the same address as the lsg
#define DW_STREAMING_SERVICE_PORT		(3075)

// NPWR00661_00 (https://ps3.scedev.net/support/issue/39936#n492997)
SceNpCommunicationId nightwing_network_platform_communication_id = 
{
	{'N','P','W','R','0','0','6','6','1'},
	'\0',
	0,
	0
};

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netPlatform
{
	PS3* PS3::s_instance=NULL;
	
	PS3* PS3::instance()
	{
		return s_instance;
	}
		
	PS3::PS3()
	{
		assert(s_instance == NULL);
		s_instance = this;

		m_sysutilCallbackSlot	= 3;
		m_Ps3Cookie				= NULL;
		m_Ps3CookieSize			= 0;	
	}

	PS3::~PS3()
	{
		assert(s_instance != NULL);
		s_instance = NULL;
	}

	int	PS3::platformNATtype()
	{
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		
		if (!lobbyService)
			return netPlatform::NAT_TYPE_NOT_SET;

		if(bdLobbyConnection::BD_CONNECTED != lobbyService->getStatus())
			return netPlatform::NAT_TYPE_NOT_SET;

		bdNetImpl *const net = bdNet::getInstance();

		bdCommonAddrRef addr = net->getLocalCommonAddr();

		if (addr.isNull())
			return netPlatform::NAT_TYPE_NOT_SET;

		switch (addr->getNATType())
		{
		case BD_NAT_OPEN:
			return netPlatform::NAT_TYPE_OPEN;
		case BD_NAT_MODERATE:
			return netPlatform::NAT_TYPE_MODERATE;
		case BD_NAT_STRICT:
			return netPlatform::NAT_TYPE_STRICT;
		default:
			LOGWARN(netDebug::g_Log, "<netPlatform::PS3> NAT type not handled correctly.");
			return netPlatform::NAT_TYPE_NOT_SET;
		}
		return netPlatform::NAT_TYPE_NOT_SET;
	}
	
	void PS3::platformInitialise()
	{
		initialiseCallbacks();
	}

	void PS3::platformShutdown()
	{
		shutdownCallbacks();
	}

	void PS3::platformUpdate()
	{
		//PS3
		updateCallbacks();
	}

	void PS3::initialiseCallbacks()
	{
		int slot = netPlatform::PS3::instance()->m_sysutilCallbackSlot;

		S32 sysutil = cellSysutilRegisterCallback(slot, sysutilCallback, NULL);

		if (0 != sysutil)
		{
			LOGFATAL(netDebug::g_Log, "<netPlatform::PS3> Sysutil callback registration failed on slot %d.", slot);
		}

		// connection changes etc callback
		S32 manager = sceNpManagerRegisterCallback(npManagerCallback, NULL); 

		if (0 != manager)
		{
			LOGFATAL(netDebug::g_Log, "<netPlatform::PS3> NP Manager callback registration failed.");
		}

		// friends management callback.
		S32 basic = sceNpBasicRegisterContextSensitiveHandler(&nightwing_network_platform_communication_id, npBasicEventHandler, NULL); 

		if (0 != basic)
		{
			LOGFATAL(netDebug::g_Log, "<netPlatform::PS3> NP Basic callback registration failed.");
		}
	}

	void PS3::shutdownCallbacks()
	{
		S32 basic = sceNpBasicUnregisterHandler();

		if (0 != basic)
		{
			LOGERROR(netDebug::g_Log, "<netPlatform::PS3> NP Basic callback registration removal failed.");
		}

		S32 manager = sceNpManagerUnregisterCallback();
		if (0 != manager)
		{
			LOGERROR(netDebug::g_Log, "<netPlatform::PS3> NP Manager callback registration removal failed.");
		}

		int slot = netPlatform::PS3::instance()->m_sysutilCallbackSlot;

		S32 sysutil = cellSysutilUnregisterCallback(slot);

		if (0 != sysutil)
		{
			LOGERROR(netDebug::g_Log, "<netPlatform::PS3> Sysutil callback registration removal failed.");
		}
	}

	void PS3::updateCallbacks()
	{
		cellSysutilCheckCallback();
	}

	// by default use slot 3 just incase any other game system is using slots 0,1,2
	void PS3::sysutilCallback(U64 status, U64 param, void *userdata)
	{
		switch (status)
		{
		case CELL_SYSUTIL_REQUEST_EXITGAME: // exit the game asap..
			{
				netPlatform::C_Msg_ExitGame msg;
				netPlatform::Core::instance()->GetSubscriberSystem().BroadcastMessage( &msg );
				break;
			}
		case CELL_SYSUTIL_NET_CTL_NETSTART_LOADED:
			{
				break;
			}
		case CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED:
			{
				assert(netPlatform::PS3::instance()->m_signInState == SIGNIN_DIALOG_STARTED);

				CellNetCtlNetStartDialogResult net_start_gui_result;
				memset(&net_start_gui_result, 0, sizeof(net_start_gui_result));
				net_start_gui_result.size = sizeof(net_start_gui_result);

				S32 ret = cellNetCtlNetStartDialogUnloadAsync(&net_start_gui_result);
				
				if (ret < 0)
				{
					LOGERROR(netDebug::g_Log, "<netPlatform::PS3> cellNetCtlNetStartDialogUnloadAsync() - failed");
					
					netPlatform::C_Msg_SignInFailed fmsg;
					netPlatform::C_Msg_ExitGame emsg;
					netPlatform::PS3::instance()->m_signInState = SIGNIN_DIALOG_FAILED;
					netPlatform::PS3::instance()->GetSubscriberSystem().BroadcastMessage( &fmsg );
					netPlatform::PS3::instance()->GetSubscriberSystem().BroadcastMessage( &emsg );
					return;
				}

				LOGINFO(netDebug::g_Log, "<netPlatform::PS3> cellNetCtlNetStartDialogUnloadAsync() - finished (result = %d)", net_start_gui_result.result);
					
				switch (net_start_gui_result.result)
				{
				case 0: // net start dialog finished successfully.
					{
						netPlatform::PS3::instance()->m_signInState = SIGNIN_DIALOG_SUCCESS;
						break;
					}
				default: // net start gui failed or canceled.
					{
						netPlatform::PS3::instance()->m_signInState = SIGNIN_DIALOG_FAILED;
						break;
					}
				}
			}
			break;

		case CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED:
			{
				// success notification
				if(netPlatform::PS3::instance()->m_signInState == SIGNIN_DIALOG_SUCCESS)
				{
					LOGINFO(netDebug::g_Log, "<netPlatform::PS3> sign in success.");
					netPlatform::C_Msg_SignInSuccess msg;
					netPlatform::PS3::instance()->GetSubscriberSystem().BroadcastMessage( &msg );
				}

				// failure notification
				if(netPlatform::PS3::instance()->m_signInState == SIGNIN_DIALOG_FAILED)
				{
					LOGWARN(netDebug::g_Log, "<netPlatform::PS3> sign in failed.");
					netPlatform::C_Msg_SignInFailed msg;
					netPlatform::PS3::instance()->m_signInState = SIGNIN_DIALOG_FAILED;
					netPlatform::PS3::instance()->GetSubscriberSystem().BroadcastMessage( &msg );

				}
				return;
			}
		default:
			{
				return;
			}
		}
	}

	void PS3::npManagerCallback(int cbevent, int result, void *arg)
	{
		// np online / offline notifications
		SysProfile::C_Manager::Instance().NpManagerCallback(cbevent, result, arg);

		switch(cbevent)
		{
		default:
			{
				return;
			}
		case SCE_NP_MANAGER_EVENT_GOT_TICKET:
			{
				if (result > 0)
				{
					netPlatform::PS3::instance()->m_ticketState = TICKET_SUCCESS;
				}
				else
				{
					netPlatform::PS3::instance()->m_ticketState = TICKET_ERROR;
				}
				
				netPlatform::C_Msg_GotTicket msg((result > 0)? true : false);
				netPlatform::PS3::instance()->GetSubscriberSystem().BroadcastMessage( &msg );
				return;
			}
		case SCE_NP_MANAGER_STATUS_OFFLINE:
			{
				LOGINFO(netDebug::g_Log, "<netPlatform::PS3::npManagerCallback()> OFFLINE");
				break;
			}		
		case SCE_NP_MANAGER_STATUS_ONLINE:
			{	
				LOGINFO(netDebug::g_Log, "<netPlatform::PS3::npManagerCallback()> ONLINE");
				break;
			}
		}
	}

	int PS3::npBasicEventHandler(int cbevent, int retCode, U32 reqId, void *arg)
	{
		SysProfile::C_Manager::Instance().NpBasicEventHandler(cbevent, retCode, reqId, arg);
		return 0;
	}

	bool PS3::platformHasOnlineAccount()
	{
		return SysProfile::C_Manager::Instance().NetworkPlatformAccountAvailable();
	}

	bool PS3::platformRequiresSignIn()
	{
		return platformHasOnlineAccount();
	}

	bool PS3::platformRequiresAuthentication()
	{
		return true;
	}

	bool PS3::platformRequiresTicket()
	{
		return true;
	}

	bool PS3::platformRequiresLobby()
	{
		return true;
	}
	
	bool PS3::platformCleanup()
	{
		m_Ps3Cookie		= NULL;
		m_Ps3CookieSize	= 0;
		return true;
	}		

	int	PS3::platformTitleId()
	{
		return DW_TITLE_ID;
	}
	
	const char* PS3::platformAuthAddress()
	{
		return DW_AUTH_ADDRESS;
	}

	int	PS3::platformAuthPort()
	{
		return DW_AUTH_PORT;
	}

	const char* PS3::platformLsgAddress()
	{
		union SceNpTicketParam outParam;

		bool ret = sceNpManagerGetTicketParam(SCE_NP_TICKET_PARAM_ISSUER_ID, &outParam);

		if (0 != ret)
			return NULL;

		switch (outParam.u32)
		{
		case ISSUER_ID_SP_INT:
			return DW_LSG_ADDRESS_DEV;
			break;
		case ISSUER_ID_PROD_QA:
			return DW_LSG_ADDRESS_CERT;
			break;
		case ISSUER_ID_NP:
			return DW_LSG_ADDRESS_LIVE;
			break;
		default:
			return DW_LSG_ADDRESS_CERT;
			break;
		}
	}

	int	PS3::platformLsgPort()
	{
		return DW_LSG_PORT;
	}

	bool PS3::platformSignIn()
	{
		CellNetCtlNetStartDialogParam npParams;
		memset(&npParams, 0, sizeof(npParams));
		npParams.size = sizeof(npParams);
		npParams.type = (platformHasOnlineAccount())? CELL_NET_CTL_NETSTART_TYPE_NP : CELL_NET_CTL_NETSTART_TYPE_NET;

		S32 res = cellNetCtlNetStartDialogLoadAsync(&npParams);

		if(res == 0)
		{
			LOGINFO(netDebug::g_Log, "<netPlatform::PS3> sign in started.");
			netPlatform::C_Msg_SignInStarted msg;
			netPlatform::PS3::instance()->m_signInState = SIGNIN_DIALOG_STARTED;
			netPlatform::PS3::instance()->GetSubscriberSystem().BroadcastMessage( &msg );
			return true;
		}
		else
		{
			LOGWARN(netDebug::g_Log, "<netPlatform::PS3> sign in failed (0x%08x).", res);
			netPlatform::C_Msg_SignInFailed msg;
			netPlatform::PS3::instance()->m_signInState = SIGNIN_DIALOG_FAILED;
			netPlatform::PS3::instance()->GetSubscriberSystem().BroadcastMessage( &msg );
			return false;
		}
	}

	bool PS3::platformNetParams(bdNetStartParams& params)
	{
		CellNetCtlInfo ctlInfo;
		
		if (!NetPlay::C_Core::Instance().CableConnected() || cellNetCtlGetInfo(CELL_NET_CTL_INFO_IP_ADDRESS, &ctlInfo) != 0)
		{
			return false;
		}

		// store the address we will use to bind the router to.
		params.m_localAddresses.pushBack(bdInetAddr(ctlInfo.ip_address));
		params.m_onlineGame = (m_onlineMode == MODE_ONLINE)? true : false;
		return true;
	}

	bool PS3::platformRequestTicket()
	{
		m_Ps3Cookie = (char*)m_pAuthService->getPS3Cookie(m_Ps3CookieSize);

		LOGINFO(netDebug::g_Log, "<netPlatform::PS3> ps3 cookie created.");

		SceNpId npId;
		S32 ret = sceNpManagerGetNpId(&npId);

		if(ret != 0)
		{
			LOGWARN(netDebug::g_Log, "<netPlatform::PS3> looks like local player is not connected to Network Platform.");
			return false;
		}

		SceNpTicketVersion ver;
		ver.major = 2;
		ver.minor = 1;
		ret = sceNpManagerRequestTicket2( &npId, &ver, NP_SERVICE_ID, m_Ps3Cookie, m_Ps3CookieSize, NULL, 0);

		if (0 != ret)
		{
			LOGERROR(netDebug::g_Log, "<netPlatform::PS3> failed to request ticket from Network Platform.");
			return false;
		}
		return true;
	}

	bool PS3::platformAuthentication()
	{
		size_t m_TicketSize = SCE_NP_TICKET_MAX_SIZE;
		U8* m_pTicket = new U8[m_TicketSize];
		memset(m_pTicket, 0, m_TicketSize);

		// request ticket data and get actual ticket size.
		const S32 res = sceNpManagerGetTicket(m_pTicket, &m_TicketSize);

		if(res != 0)
		{
			LOGERROR(netDebug::g_Log, "<netPlatform::PS3> faile to get ticket.");
			return false;
		}
		LOGINFO(netDebug::g_Log, "<netPlatform::PS3> ticket request started.");
		
		const bdPS3AuthInfoRef ps3AuthInfo = new bdPS3AuthInfo();
		assert(ps3AuthInfo.notNull());

		if (!m_pAuthService->authorizePS3Ticket(m_pTicket, m_TicketSize, ps3AuthInfo))
		{
			delete[] m_pTicket;
			LOGERROR(netDebug::g_Log, "<netPlatform::PS3> failed to start authorise ps3 ticket.");
			return false;
		}

		delete[] m_pTicket;
		return true;
	}

	bool PS3::profileIdentifiable(const SysProfile::FullProfile& profile)
	{
		if(	profile.m_State == SysProfile::eProfileStateNotActive || profile.m_IsGuest)
		{
			return false;
		}
		return true;
	}
	
	bdOnlineUserID PS3::getUserId(const SysProfile::UserIdentifier &user)
	{
		bdOnlineUserID id;
		if(user.m_Uid.handle.data[0] == 0)
		{
		#if defined( CFG_DEBUG )
			char bytes[32];
			netDebug::getByteInfo(user.m_Uid.handle.data, SCE_NET_NP_ONLINEID_MAX_LENGTH, bytes, sizeof(bytes));
			LOGFATAL(netDebug::g_Log, "<netPlatform::PS3::getUserId()> 'null' user identifier [%s].", bytes);
		#endif
			id = 0;
		}
		else
		{
			id = bdAuthUtility::getUserID(user.m_Uid.handle.data);
		}
		return id;
	}

	bool PS3::sameUser(const SysProfile::UserIdentifier &a, const SysProfile::UserIdentifier &b)
	{
		return (memcmp(a.m_Uid.handle.data, b.m_Uid.handle.data, SCE_NET_NP_ONLINEID_MAX_LENGTH) == 0);
	}

	const char* PS3::getAddrInfo(const bdCommonAddrRef& addr, char* str, int len)
	{
		if(addr.isNull())
		{
			sprintf_s(str, len, "NULL");
			return str;
		}

		bdAddr publicAddr = addr->getPublicAddr();
		if(addr->getLocalAddrs().isEmpty())
		{
			publicAddr.toString(str, len);
		}
		else
		{
			bdAddr localAddr = addr->getLocalAddrByIndex(0);
			localAddr.toString(str, len);
		}
		return str;
	}
	const char* PS3::getConsoleName(char* name, int len)
	{
		if(getAddrInfo(myCommonAddr(), name, len))
			return name;
		
		sprintf_s(name, len, "PS3");
		return name;
	}

	const char* PS3::getHandleInfo(const bdAddrHandleRef &handle, char* str, int len)
	{
		if(handle.isNull())
		{
			sprintf_s(str, len, "NULL");
			return str;
		}

		bdInAddr addr = (handle->getRealAddr().getAddress().getInAddr());
		int port = handle->getRealAddr().getPort();
		int addrLen = addr.toString(str, len);
		
		// append port
		sprintf_s((str + addrLen), (len - addrLen), ":%d", port);
		return str;
	}

	int	PS3::getHandleStatus(const bdAddrHandleRef& addrHandle)
	{
		bdAddrHandle::bdAddrHandleStatus status = addrHandle.isNull()? bdAddrHandle::BD_ADDR_NOT_CONSTRUCTED : addrHandle->getStatus();
		switch(status)
		{
		default:
		case bdAddrHandle::BD_ADDR_ERROR:				return ADDRESS_STATUS_LOST;
		case bdAddrHandle::BD_ADDR_NOT_CONSTRUCTED:		return ADDRESS_STATUS_PENDING;
		case bdAddrHandle::BD_ADDR_NOT_RESOLVED:		return ADDRESS_STATUS_PENDING;
		case bdAddrHandle::BD_ADDR_RESOLVED:			return ADDRESS_STATUS_CONNECTED;
		case bdAddrHandle::BD_ADDR_UNRESOLVED:			return ADDRESS_STATUS_LOST;
		}
		return status;
	}
};

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
