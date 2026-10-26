
#include "stdafx.h"
#include "MultiplayerCache.h"

#include "GameApp.h"
#include "Game/Game.h"

#include "GameLogic/Layers/RaceBookLayer.h"
#include "GameLogic/GameData/MultiplayerData.h"

#include "Ui/Game/Multiplayer/LspData.h"

#include <Network/SessionEnumerator.h>
#include <Network/Core/NetServices.h>
#include <Network/Core/NetRace.h>
#include <Network/Misc/NetQoS.h>
#include <Network/Core/NetParty.h>
#include <Network/Misc/NetJoinRaceFromPartyData.h>

#include <GameLogic/ContentServer/ContentServerManager.h>

#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Profile/GameProfile.h>

namespace UI
{

C_MultiplayerCache C_MultiplayerCache::m_sInstance;

void C_MultiplayerCache::ClearEventCache()
{
	m_Events.Clear();
	m_CurrentEventIndex = -1;
}

void C_MultiplayerCache::CachePlaylists( const Game::E_GameMode eMode )
{
	ClearEventCache();

	const XtArray< XtHandle<MultiplayerEvent > >* pPlaylist = NULL;
	switch( eMode )
	{
		case Game::E_GameMode_Online :
		{
			pPlaylist = &C_MultiplayerData::Instance().m_pConfig->m_OnlineEvents;
			break;
		}
		case Game::E_GameMode_SplitScreen :
		{
			pPlaylist = &C_MultiplayerData::Instance().m_pConfig->m_SplitscreenEvents;
			break;
		}
		case Game::E_GameMode_SystemLink :
		{
			pPlaylist = &C_MultiplayerData::Instance().m_pConfig->m_SystemLinkEvents;
			break;
		}
		default :
		{
			BIZ_ASSERT_FAIL( "Unsupported game mode" );
			return;
		}
	}

	// Cache all playlists for the current mode
	for ( S32 i = 0; i < pPlaylist->count; i++ )
	{
		const XtHandle< MultiplayerEvent > pEvent = pPlaylist->elements[i];
		BIZ_ASSERT_MSG_SPEED( pEvent, "NULL event in multiplayer config event(%i)", i );

		if ( pEvent )
		{
// Check event data is valid
#ifndef CFG_GOLD
			// We need races otherwise the event is broke
			BIZ_ASSERT_MSG( pEvent->races.count, "No races in event %S", C_TextStringID( pEvent->name.id ).String() );
			if ( pEvent->races.count <= 0 )
				continue;

			bool valid = true;
			for ( S32 j = 0; j < pEvent->races.count; j++ )
			{
				if ( !pEvent->races[j] )
				{
					BIZ_ASSERT_FAIL_SPEED( "NULL race %i, %S event", j, C_TextStringID( pEvent->name.id ).String() );
					valid = false;
					break;
				}

				if ( !pEvent->races[j]->route )
				{
					BIZ_ASSERT_FAIL_SPEED( "NULL route in race %i, %S event", j, C_TextStringID( pEvent->name.id ).String() );
					valid = false;
					break;
				}

				// Check race limits are set up correctly
				if ( pEvent->races[j]->min_players < pEvent->min_players )
				{
					BIZ_ASSERT_FAIL_SPEED( "The min players for race %i does not meet event(%S) minimum", j, C_TextStringID( pEvent->name.id ).String() );
					valid = false;
					break;
				}

				if ( pEvent->races[j]->max_players < pEvent->races[j]->min_players )
				{
					BIZ_ASSERT_FAIL_SPEED( "The max players for race %i is less than the min, %s event", j, C_TextStringID( pEvent->name.id ).String());
					valid = false;
					break;
				}
			}
			if ( !valid )
				continue;
#endif // CFG_GOLD

			m_Events.PushBack( pEvent );
		}

		// Stop the cache from overflowing
		if ( m_Events.Size() >= MAX_EVENT_CACHE )
			break;
	}
}

void C_MultiplayerCache::CacheNetRaceSettings()
{
	ClearEventCache();
}

const MultiplayerEvent*	C_MultiplayerCache::GetEventByID( const U32 uID ) const
{
	for ( S32 i = 0; i < m_Events.Size(); i++ )
	{
		if ( m_Events[i]->ID == uID )
		{
			return m_Events[i];
		}
	}
	return NULL;
}

bool C_MultiplayerCache::CachePlaylistByID( const U32 uEventID )
{
	for ( S32 i = 0; i < m_Events.Size(); i++ )
	{
		if ( m_Events[i]->ID == uEventID )
		{
			m_CurrentEventIndex = i;
			return true;
		}
	}
	return false;
}

bool C_MultiplayerCache::CacheInviteSettings( const bool bParty )
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	U32 event_id = 0;

	if ( bParty )
	{
		RacerNet::C_NetParty& party = services.Party();

		RacerNet::NetJoinRaceFromPartyData partyData;
		if (party.IsRunning())
		{
			if (party.GetJoinRaceFromPartyData(partyData))
				event_id = partyData.m_EventID;
		}
	}
	else
	{
		NetPlay::C_SessionEnumerator& enumerator = services.MatchmakingSessionEnumerator();
		NetPlay::C_SessionEnumerator::SessionQoSData qos;
		if ( !enumerator.GetQoSDataFromEnumerated( 0, qos ) )
			return false;

		if ( qos.m_pQoSData && qos.m_QoSDataSize >= sizeof( RacerNet::NetQoSBase ) )
		{
			const RacerNet::NetQoSBase* pBaseQos = reinterpret_cast<RacerNet::NetQoSBase*> (qos.m_pQoSData);
			if ( RacerNet::NetQoSBase::QoSTypeRace == pBaseQos->m_Type )
			{
				const RacerNet::NetQoSRace* pRaceQos = reinterpret_cast<RacerNet::NetQoSRace*> (qos.m_pQoSData);
				event_id = (U32)pRaceQos->m_Race.m_EventID;
			}
		}
	}

	BIZ_ASSERT_MSG( event_id, "Failed to find event from sorcery" );

	for ( S32 i = 0; i < m_Events.Size(); i++ )
	{
		if ( m_Events[i]->ID == event_id )
		{
			m_CurrentEventIndex = i;
			return true;
		}
	}
	return false;
}

bool C_MultiplayerCache::IsCurrentEventValid() const
{
	return ( m_CurrentEventIndex >= 0 && m_CurrentEventIndex < m_Events.Size());
}
}; // namespace UI
