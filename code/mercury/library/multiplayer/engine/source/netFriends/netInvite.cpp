#include "stdafx.h"

#include "./netInvite.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"
#include <bdNet/bdLANDiscovery/bdGameInfo.h>
#include "ui/util/WideString.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

bdGameInfoRef netInvite::getGameInfo()
{
	MEMORY_TRACKER_PUSH("network.multiplayer");
	bdGameInfoRef gameInfo = new bdGameInfo;
	gameInfo->setSecurityID(m_Id);
	gameInfo->setSecurityKey(m_Key);
	gameInfo->setTitleID(netPlatform::Core::instance()->titleId());
	MEMORY_TRACKER_POP("network.multiplayer");
	return gameInfo;
}

int netInvite::debugstring(char* buff, int len)
{
	char str_sessionId[128];
	char str_sessionKey[128];

	netDebug::getByteInfo(&m_Id , sizeof(m_Id) ,	str_sessionId,	sizeof(str_sessionId));
	netDebug::getByteInfo(&m_Key, sizeof(m_Key),	str_sessionKey,	sizeof(str_sessionKey));

	int count = sprintf_s(	buff, len,
							"active(%s)\nsessionId(%s)\nsessionKey(%s)\n", 
							m_Active?"true":"false", 
							str_sessionId[128],
							str_sessionKey[128]);

	return count;
}

netInvite::netInvite(const SysProfile::GameInvite& inviteData)
: SysProfile::GameInvite(inviteData)
{
	m_Active = true;
}
	
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

