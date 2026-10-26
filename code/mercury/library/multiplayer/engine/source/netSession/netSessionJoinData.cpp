

#include "stdafx.h"
#include "netSessionJoinData.h"
#include "netEnumeratedSession.h"
#include "netSessionState.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif


netSessionJoinData::netSessionJoinData()
{
	m_pad = -1;
	m_online = false;
	m_party = false;
	m_titleId = -1;
	m_flags = -1;
	m_enumeratedSession = NULL;
}

netSessionJoinData::~netSessionJoinData()
{}

netSessionStateRef netSessionJoinData::toSessionState()
{
	netSessionStateRef  state = new netSessionState();

	state->m_pad			= m_pad;
	state->m_online			= m_online;
	state->m_hosting		= false;
	state->m_parameters		= m_parameters;
	state->m_players		= m_players;
	state->m_slotConfig		= m_slotConfig;
	state->m_titleId		= m_titleId;
	state->m_flags			= m_flags;
	state->m_securityInfo	= m_enumeratedSession->security();	
	return state;
}
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

