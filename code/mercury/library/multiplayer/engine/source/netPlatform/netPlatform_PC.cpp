
#include "stdafx.h"
#include "./netPlatform_Messages.h"
#include "./netPlatform_PC.h"
#include "netDebug/netDebug.h"


#include <bdLobby/bdLobbyConnection.h>
#include <bdLobby/bdLobbyService.h>
#include <bdNet/bdNet.h>
#include <bdNet/bdNetStartParams.h>
#include <Network/Core.h>
#include <SystemProfile/SystemProfileManager.h>
#include <misc_shared/Messaging/MessageSubscriberSystem.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif


#define DW_TITLE_ID						(0x1439)
#define DW_AUTH_ADDRESS					"jb09-pc.auth.v200.demonware.net" 
#define DW_AUTH_PORT					(3074)
#define DW_LSG_ADDRESS_DEV				"jb09-pc.lobby.v200.demonware.net"
#define DW_LSG_ADDRESS_CERT				"jb09-pc.lobby.v200.demonware.net"
#define DW_LSG_ADDRESS_LIVE				"jb09-pc.lobby.v200.demonware.net"
#define DW_LSG_PORT						(3074)
//#define DW_STREAMING_SERVICE_ADDRESS	"amax.binario.mmp3.demonware.net"	// not used since its the same address as the lsg
#define DW_STREAMING_SERVICE_PORT		(3075)

namespace netPlatform
{
	PC::PC()
	{}

	PC::~PC()
	{}

	int	PC::platformNATtype()
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
			LOGWARN(netDebug::g_Log, "<netPlatform::PC> NAT type not handled correctly.");
			return netPlatform::NAT_TYPE_NOT_SET;
		}
		return netPlatform::NAT_TYPE_NOT_SET;
	}
	
	// platform functions for state machine
	void PC::platformInitialise()
	{
		char name[64];
		getConsoleName(name, sizeof(name));
		SysProfile::C_Manager::Instance().SetDemonwareAuthenticatedPlayerData(name);
	}

	void PC::platformShutdown()
	{
	}

	void PC::platformUpdate()
	{
	}

	bool PC::platformRequiresSignIn()
	{
		return false;
	}

	bool PC::platformRequiresAuthentication()
	{
		return false;
	}

	bool PC::platformRequiresTicket()
	{
		return false;
	}

	bool PC::platformRequiresLobby()
	{
		return false;
	}

	int PC::platformTitleId()
	{
		return DW_TITLE_ID;
	}
	
	const char* PC::platformAuthAddress()
	{
		return "NO AUTH ADDRESS";
	}

	int	PC::platformAuthPort()
	{
		return 0;
	}

	const char* PC::platformLsgAddress()
	{
		return "NO LSG ADDRESS";
	}

	int PC::platformLsgPort()
	{
		return 0;
	}

	bool PC::platformSignIn()
	{
		return false;
	}

	bool PC::platformNetParams(bdNetStartParams& params)
	{
		params.m_onlineGame = (m_onlineMode == MODE_ONLINE)? true : false;

		if (!NetPlay::C_Core::Instance().CableConnected() && !NetPlay::C_Core::Instance().ValidLocalAddress())
		{
			return false;
		}
		return true;
	}

	bool PC::platformRequestTicket()
	{
		return false;
	}

	bool PC::platformAuthentication()
	{
		return false;
	}

	bool PC::platformCleanup()
	{
		return false;
	}

	bool PC::platformHasOnlineAccount()
	{
		SysProfile::FullProfile outProfileData;
		SysProfile::C_Manager::Instance().GetProfile(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), outProfileData);
		return (outProfileData.m_State == SysProfile::eProfileStateOnline);
	}

	int	PC::platformStartGetHostByName(const bdNChar8 *const domainNameStr, bdGetHostByName** getHostByName, const bdInetAddr resolvedAddr)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::PC> implement platformStartGetHostByName()");
		return (int) 0;
	}

	int	PC::platformPumpGetHostByName(bdGetHostByName **getHostByName, bdInetAddr &resolvedAddr)
	{
		LOGWARN(netDebug::g_Log, "<netPlatform::PC> implement platformPumpGetHostByName()");
		return (int) 0;
	}	

	bool PC::profileIdentifiable(const SysProfile::FullProfile& profile)
	{
		if(	profile.m_State == SysProfile::eProfileStateNotActive ||
			profile.m_State == SysProfile::eProfileStateBlagged ||
			profile.m_IsGuest)
		{
			return false;
		}
		return true;
	}

	bdOnlineUserID PC::getUserId(const SysProfile::UserIdentifier &user)
	{
		bdOnlineUserID id;

		if(user.m_Uid == 0)
		{
		#if defined( CFG_DEBUG )
			LOGFATAL(netDebug::g_Log, "<netPlatform::PC::getUserId()> 'null' user identifier.");
		#endif
			id = 0;
		}
		else
		{
			id = user.m_Uid;
		}
		return id;
	}

	bool PC::sameUser(const SysProfile::UserIdentifier &a, const SysProfile::UserIdentifier &b)
	{
		return (a == b);
	}

	int	PC::getHandleStatus(const bdAddrHandleRef& addrHandle)
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

