#include "Stdafx.h"

#include "MatchingDataFinder.h"
#include <Network/SessionEnumerator.h>
#include <SystemProfile/SystemProfile.h>
#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Game/UILUA.h>
#include <ui/Game/StringStore.h>
#include <ui/Game/UIVariableStore.h>
#include <ui/Game/UILocaliser.h>
#include <Network/Core/NetServices.h>
#include <Network/Core/NetRace.h>
#include <Network/Core/Netparty.h>
#include <Network/Misc/NetQoS.h>


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

void C_MatchingDataFinder::Init()
{
	//
}

const WCHAR* C_MatchingDataFinder::GetVariableText( const char* pVariableName ) 
{
	m_VariableText.Clear();

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if ( !services.IsInitialised() )
		return L"<NET SERVICES NOT INTIALISED>";

	if ( strncmp(pVariableName, "JOIN_ERROR_RACE", 15 ) == 0 ) 
	{
		// race session knows the name of the peer who we failed to connect to so build a generic msg that shows this players name.
		m_VariableText.Format(C_TextStringID( "MP_JOIN_ERROR_SPECIFIC_PLAYER" ).String(), services.Race().JoinHelper().PeerJoinFailureData().Name() );
		return m_VariableText;
	}
	else if ( strncmp(pVariableName, "JOIN_ERROR_PARTY", 16 ) == 0 ) 
	{
		// party session knows the name of the peer who we failed to connect to so build a generic msg that shows this players name.
		m_VariableText.Format(C_TextStringID( "MP_JOIN_ERROR_SPECIFIC_PLAYER" ).String(), services.Party().JoinHelper().PeerJoinFailureData().Name() );
		return m_VariableText;
	}

	RacerNet::C_NetServices::NetMatchingRemapList& remapList = services.MatchingRemapList();
	const S32 sIndex = (S32)ParseIndex( pVariableName, "", true );

	if ( sIndex < 0 && sIndex >= remapList.Size() )
		return L"<INVALID INDEX>";


	NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
	NetPlay::C_SessionEnumerator::SessionQoSData qos;

	if ( !enumerator.GetQoSDataFromEnumerated( remapList[sIndex], qos ) || qos.m_pQoSData == NULL)
		return L"<INVALID>";


	if ( strncmp(pVariableName, "HOST", 4 ) == 0 ) 
	{
		const RacerNet::NetQoSRace* pQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
		BIZ_ASSERT(pQos->m_Type == RacerNet::NetQoSBase::QoSTypeRace);

		m_VariableText.Format( L"%s", pQos->m_Host.Name() );
	}
	else
	if ( strncmp(pVariableName, "PLAYERS", 7 ) == 0 )
	{
		const RacerNet::NetQoSRace* pQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
		BIZ_ASSERT(pQos->m_Type == RacerNet::NetQoSBase::QoSTypeRace);

		m_VariableText.Format( C_TextStringID( "MP_PLAYER_COUNT" ).String(), pQos->m_MaxPlayers - pQos->m_NumSLotsStillOpen, pQos->m_MaxPlayers );
	}
	else
	if ( strncmp(pVariableName, "STATUS", 6 ) == 0 ) 
	{
		const RacerNet::NetQoSRace* pQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
		BIZ_ASSERT(pQos->m_Type == RacerNet::NetQoSBase::QoSTypeRace);

		if (RacerNet::NetQoSBase::QoSStateInRace == pQos->m_SessionState)
			m_VariableText.Format( C_TextStringID( "MP_RACE_IN_PROGRESS" ).String() );
		else
			m_VariableText.Format( C_TextStringID( "MP_LOBBY" ).String() );
	}
	else
	if ( strncmp(pVariableName, "PING", 4 ) == 0 ) 
	{
		const RacerNet::NetQoSRace* pQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
		BIZ_ASSERT(pQos->m_Type == RacerNet::NetQoSBase::QoSTypeRace);

		float ping = (float)qos.m_Ping;

	#if defined ( PLATFORM_PS3 ) || defined ( PLATFORM_PC )
		// when using demonware its ping is related to the frame update time, so its out by this time.
		// we get around that by subtracting the normal frame update time from the calculated ping
		ping -= ((1.0f/30.0f)*1000);
		if (ping < 0.0f)
			ping = 0.0f;
	#endif
		m_VariableText.Format(L"%dms", (U32)ping );
	}
	
	return m_VariableText;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
