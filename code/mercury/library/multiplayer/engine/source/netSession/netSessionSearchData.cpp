
#include "stdafx.h"
#include "netSessionSearchData.h"
#include "netEnumeratedSession.h"
#include "netFriends/netFriends.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

// data to start a search (either by invites, sessionInfo or filters).
netSessionSearchData::netSessionSearchData()
{
	m_pad					= -1;
	m_flags					= -1;
	m_qos					= false;
	m_online				= true;
	m_titleId				= -1;
	m_lanSearchTime			= 3.0f;
	m_queryId				= -1;
	m_maxResults			= 0;
}

netSessionSearchData::~netSessionSearchData()
{
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

