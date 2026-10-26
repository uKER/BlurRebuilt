#include "Stdafx.h"

#include "LanDataFinder.h"

#include <SystemProfile/SystemProfile.h>
#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Game/UILUA.h>
#include <ui/Game/StringStore.h>
#include <ui/Game/UIVariableStore.h>
#include <ui/Game/UILocaliser.h>
#include <Network/Core/NetServices.h>
#include <Network/Core/NetRace.h>
#include <Network/Lan/NetLanDiscoveryDefines.h>
#include <Network/Lan/NetLanDiscovery.h>

#include "UI/Game/Multiplayer/MultiplayerCache.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

void C_LanDataFinder::Init()
{
	//
}

const WCHAR* C_LanDataFinder::GetVariableText( const char* pVariableName ) 
{
	m_VariableText.Clear();

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if ( !services.IsInitialised() )
		return L"";

	RacerNet::C_LanDiscoveryManager& lanDiscovery = services.Race().LanDiscoveryManager();

	const U32 sIndex = (U32)ParseIndex( pVariableName, "", true );
	if ( sIndex >= lanDiscovery.NumEnumerated() )
		return L"";
	
	if ( strncmp(pVariableName, "HOST", 4) == 0 ) 
	{
		RacerNet::LanDiscoveryReply data;
		if (lanDiscovery.GetEnumeratedSesionData(sIndex, data))
			m_VariableText.Format(L"%s", data.m_HostProfile.Name() );
	}
	else
	if ( strncmp( pVariableName, "PLAYERS", 7 ) == 0 )
	{
		RacerNet::LanDiscoveryReply data;
		if ( lanDiscovery.GetEnumeratedSesionData( sIndex, data ) )
			m_VariableText.Format( L"%d/%d", data.m_SlotsTaken, data.m_MaxSlots);
	}
	else
	if ( strncmp( pVariableName, "MODE", 4 ) == 0 )
	{
		RacerNet::LanDiscoveryReply data;
		if ( lanDiscovery.GetEnumeratedSesionData( sIndex, data ) )
		{
			const MultiplayerEvent* pEvent = C_MultiplayerCache::Instance().GetEventByID( data.m_RaceQos.m_EventID );

			if ( pEvent )
			{
				m_VariableText.Format( UI::C_TextStringID( pEvent->name.id ).String() );
			}
			else
			if ( data.m_RaceQos.m_CustomRace )
			{
				m_VariableText.Format( UI::C_TextStringID( "MP_CUSTOM_GAME" ).String() );
			}
			else
				m_VariableText.Format( UI::C_TextStringID( "MP_UNKNOWN_PLAYLIST" ).String() );
		}
	}

	return m_VariableText;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
