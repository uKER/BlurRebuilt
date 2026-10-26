#include "Stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include "LobbyDataFinder.h"
#include <GameApp.h>
#include <GameLogic/GameData/GameData.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic\GameModes\GameModeNetwork.h>

#include <SystemProfile/SystemProfile.h>
#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Game/UILUA.h>
#include <ui/Game/StringStore.h>
#include <ui/Game/UIVariableStore.h>
#include <ui/Game/UILocaliser.h>
#include "Ui/Game/Multiplayer/MultiplayerCache.h"
#include <Network/Misc/NetDefines.h>
#include <Network/Misc/NetRaceSettings.h>
#include <Network/Core/NetServices.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetplayerConnection.h>
#include <Network/Player/NetPlayerManager.h>

#include "ui/Game/Gadgets/UILobby.h"

#if !defined CFG_GOLD
Debug::Bool g_NetLobby_ShowJoinReferences("Network.Lobby.ShowJoinReferences", false);
#endif


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

void C_LobbyDataFinder::Init()
{
	//
	m_pLobby = NULL;
}

const WCHAR* C_LobbyDataFinder::GetVariableText( const char* pVariableName ) 
{
	m_VariableText.Clear();

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

	// [WillB] #37077 - Made this safe as there could be a few frames where we are no longer in gamemode mp and pure virtual calls arnt good.
	if ( !services.IsInitialised() || !C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork() )
		return L"";

	Game::C_GameModeNetwork* pGameMode = ((Game::C_GameModeNetwork*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr());

	if ( 0 == strncmp(pVariableName, "NUM_PLAYERS", 11)) 
	{
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		RacerNet::NetRaceSettings raceSettings;
		pGameMode->GetNetRaceSettings( raceSettings );

		U8 count = 0;
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			RacerNet::C_NetPlayer* p = mgr.Index(i);
			if ( p && p->m_RaceConnection.IsActive() && p->m_RaceConnection.m_PlayerState >= RacerNet::eNetPlayerStateIdle )
				count++;
		}
		m_VariableText.Format( UI::C_TextStringID( "MP_LOBBY_PLAYERS" ).String(), count, raceSettings.m_MaxDrivers );
	}
	else
	if ( 0 == strncmp(pVariableName, "PLAYER_VIEWING", 14 ) )
	{
		pVariableName += 14;	

		const U8 uRef = (U8)ParseIndex( pVariableName, "" );

		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		RacerNet::C_NetPlayer* pPlayer = mgr.FindFromJoinId( uRef, false );

		if ( pPlayer )
		{
			m_VariableText.Format( pPlayer->m_User.Name() );
		}
		else
		{
			// [WillB] #37771 - Incase we try to view a disconnected player. His data will no longer exist in the player manager
			Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			for ( S8 i = 0; i < levelState.mCars.Size(); i++ )
			{
				if ( levelState.mCars[i].mNetJoinId == uRef )
				{
					m_VariableText.Format( levelState.mCars[i].mNameAboveCar );
					break;
				}
			}
		}
	}
	else // [WillB] #37077 - Added this to make the managing of rendering timers on the lobby screens easier. This allows it to not render when the timer isnt active.
	if ( 0 == strncmp(pVariableName, "LOBBY_STATE", 11 ) )
	{
		pVariableName += 11;

		// Display different text based on state
		switch( C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyState )
		{ 
			// All these states should render the timer, otherwise dont render anything
			case E_LobbyState_Countdown	:
			case E_LobbyState_TeamCountdown :
			case E_LobbyState_Ready	: 
				{
					m_VariableText.Format( C_TextStringID( "MP_TIMER_COUNTDOWN" ).String(), (S32)C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_LobbyTimer );
					break;
				}
		}
	}
	else
	if ( 0 == strncmp(pVariableName, "PARTY_PLAYER_VIEWING", 20 ) )
	{
		pVariableName += 20;	

		const U8 uRef = (U8)ParseIndex( pVariableName, "" );

		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		RacerNet::C_NetPlayer* pPlayer = mgr.FindFromJoinId( uRef, true );

		if ( pPlayer )
			m_VariableText.Format( pPlayer->m_User.Name() );
	}
	else
	if ( strncmp(pVariableName, "INVITE_HELPTEXT", 15 ) == 0 )
	{
		if ( SysProfile::C_Manager::Instance().IsInExternalParty(1) )
			m_VariableText.Format( C_TextStringID( "MP_INVITE_PARTY_HELPTEXT" ).String() );
		else
			m_VariableText.Format( C_TextStringID( "MP_INVITE_FRIENDS_HELPTEXT" ).String() );
	}
	else
	if ( strncmp(pVariableName, "INVITE_BUTTON", 13 ) == 0 )
	{
		if ( SysProfile::C_Manager::Instance().IsInExternalParty(1) )
			m_VariableText.Format( C_TextStringID( "INFO_INVITE_PARTY" ).String() );
		else
			m_VariableText.Format( C_TextStringID( "INFO_INVITE_FRIENDS" ).String() );
	}
	else
	if ( strncmp(pVariableName, "PLAYER_CAR", 10 ) == 0 )
	{
		RacerNet::NetRaceSettings raceSettings;
		pGameMode->GetNetRaceSettings( raceSettings );

		// Find the local player
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		for (U8 i=0; i<mgr.TotalSize(); i++)
		{
			RacerNet::C_NetPlayer* p = mgr.Index(i);

			if ( p->PrimaryPlayer() )
			{
				const AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)p->m_RaceData.m_VehicleData.m_xtVehicleId];

				if ( pVehicle ) 
				{
					m_VariableText.Format(UI::C_TextStringID(pVehicle->loc_string.id).String());
				}
				else
				{
					// must be dlc vehicle as we dont have it in our content list.
					m_VariableText.Format(UI::C_TextStringID( "CMN_DOWNLOADABLE_CONTENT" ).String());
				}
				break;
			}
		}
	}
	/*---------------------------
		Session Data
	----------------------------*/
	else
	if ( strncmp(pVariableName, "PLAYER", 6 ) == 0 ) 
	{
		pVariableName += 6;	

		const U8 uRef = (U8)ParseIndex( pVariableName, "", true );

		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		RacerNet::C_NetPlayer* pPlayer = mgr.FindFromJoinId( uRef, false );

		RacerNet::NetRaceSettings raceSettings;
		pGameMode->GetNetRaceSettings( raceSettings );

		// Only proceed if the player exists.
		if ( !pPlayer )
			return L"";

		if ( strncmp(pVariableName, "_NAME", 5 ) == 0 )
		{
			if ( pGameMode->IsRanked() )
			{
				m_VariableText.Format(L"%s", pPlayer->m_User.Name() );
			}
			else
				m_VariableText.Format(L"%s %s", pPlayer->m_User.Name(), pPlayer->m_RaceConnection.m_Host ? UI::C_TextStringID("MP_LOBBY_HOST").String() : L""); 

#ifndef CFG_GOLD
			if (g_NetLobby_ShowJoinReferences)
			{
				m_VariableText.Format(L"<%d> %s %s", pPlayer->m_RaceConnection.m_JoinId, pPlayer->m_User.Name(), pPlayer->m_RaceConnection.m_Host ? UI::C_TextStringID("MP_LOBBY_HOST").String() : L""); 
			}
#endif // CFG_GOLD
		}

		if ( strncmp(pVariableName, "_RANK", 5 ) == 0 ) 
		{
			m_VariableText.Format( L"%i", pPlayer->m_ProfileData.m_Game.m_Rank + 1 );
		}
		else
		if ( strncmp(pVariableName, "_RATING", 7 ) == 0 ) 
		{
			m_VariableText.Format( L"%i", pPlayer->m_ProfileData.m_Game.m_RacePointsThisSession );
		}
		else
		if ( strncmp(pVariableName, "_INFO", 5 ) == 0 ) 
		{
			// Find the player in the manager from the lobby slot.
			bool all_ready = true;
			U8 activePlayers = 0;

			for (U8 i=0; i<mgr.TotalSize(); i++)
			{
				RacerNet::C_NetPlayer* p = mgr.Index(i);

				if ( !p->m_RaceConnection.IsActive() || p->m_RaceConnection.m_PlayerState < RacerNet::eNetPlayerStateIdle )
					continue; // ignore network players who dont have a race connection, could have a party connection only.

				switch ( p->m_RaceConnection.m_PlayerState )
				{
				case RacerNet::eNetPlayerStateRacing:
				case RacerNet::eNetPlayerStateLoadingRace:
				case RacerNet::eNetPlayerStateLoadingUi:
				case RacerNet::eNetPlayerStateViewingResults:
				case RacerNet::eNetPlayerStateViewingResultsIdle:
					all_ready = false;
					break;
				}
				activePlayers++;
			}

			#ifndef CFG_GOLD
			if ( g_AllowSingleRacer )
				activePlayers = raceSettings.m_MinDrivers;
			#endif // CFG_GOLD

			// random cars?
			const RacerNet::NetPlayerVehicle& netVehicle = pPlayer->m_RaceData.m_VehicleData; 

			if ( pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateLoadingRace ||
				 pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateWaitingForOthers )
			{
				m_VariableText.Format(UI::C_TextStringID( "MP_LOADING_RACE" ).String());
			}
			else
			if ( pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateRacing )
			{
				m_VariableText.Format(UI::C_TextStringID( "MP_IN_RACE" ).String());
			}
			else
			if ( pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateFinished )
			{
				m_VariableText.Format(UI::C_TextStringID( "MP_FINISHED" ).String());
			}
			else
			if ( pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateViewingResults )
			{
				m_VariableText.Format(UI::C_TextStringID( "MP_VIEWING_RESULTS" ).String());
			}
			else
			if ( pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateViewingResultsIdle )
			{
				m_VariableText.Format(UI::C_TextStringID( "MP_VIEWING_AWARDS" ).String());
			}
			else
			if ( all_ready && activePlayers >= raceSettings.m_MinDrivers && netVehicle.m_xtVehicleId != 0 ) // 0 means we have yet to receive this data from the peer.
			{
				const AmaxVehicle *xtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)netVehicle.m_xtVehicleId];

				if ( xtVehicle ) 
				{
					if(pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateChangingCar)
						m_VariableText.Format(UI::C_TextStringID( "MP_CHOOSING_CAR" ).String());
					else
						m_VariableText.Format(UI::C_TextStringID(xtVehicle->loc_string.id).String());
				}
				else
				{
					// must be dlc vehicle as we dont have it in our content list.
					m_VariableText.Format(UI::C_TextStringID( "CMN_DOWNLOADABLE_CONTENT" ).String());
				}
			}
			else
				m_VariableText.Format(UI::C_TextStringID( "CMN_WAITING" ).String());
		}
	}
	/*---------------------------
		Party Data
	----------------------------*/
	else
	if (0 == strncmp(pVariableName, "PARTY_SIZE", 10 )) 
	{
		m_VariableText.Format(L"%i/%i", services.PlayerManager().Size(true), RacerNet::NET_MAX_PARTY_MEMBERS );
	}
	else
	if (0 == strncmp(pVariableName, "PARTY_LEADER", 12 )) 
	{
		RacerNet::C_NetPlayer* pPlayer = services.PlayerManager().HostPlayer( true );

		if ( pPlayer )
			m_VariableText.Format( pPlayer->m_User.Name() );
	}
	else
	if ( strncmp(pVariableName, "PARTY_PLAYER", 12 ) == 0 ) 
	{
		// [WillB] #37775 - The lobby could shut down before the network layer so this can be null.
		if ( !m_pLobby )
			return L"";

		pVariableName += 12;	

		const S32 sIndex = (S32)ParseIndex( pVariableName, "", true );

		if ( sIndex < 0 && sIndex > RacerNet::NET_MAX_VEHICLES )
			return L"";

		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		RacerNet::C_NetPlayer* pPlayer = NULL;

		// Find the player in the manager from the lobby slot.
		bool found = false;
		for (U8 i=0; i<mgr.TotalSize() && !found; i++)
		{
			pPlayer = mgr.Index(i);
			if ( pPlayer->m_PartyConnection.IsActive() && pPlayer->m_ConsoleId == m_pLobby->GetSlot( sIndex ).id )
				found = true;
		}

		// Only proceed if the player exists.
		if ( !found || !pPlayer )
			return L"";

		if ( strncmp(pVariableName, "_NAME", 5 ) == 0 )
		{
			m_VariableText.Format(L"%s", pPlayer->m_User.Name() ); 

#ifndef CFG_GOLD
			if (g_NetLobby_ShowJoinReferences)
			{
				m_VariableText.Format(L"<%d> %s %s", pPlayer->m_RaceConnection.m_JoinId, pPlayer->m_User.Name(), pPlayer->m_RaceConnection.m_Host ? UI::C_TextStringID("MP_LOBBY_HOST").String() : L""); 
			}
#endif // CFG_GOLD
		}

		if ( strncmp(pVariableName, "_RANK", 5 ) == 0 ) 
		{
			m_VariableText.Format( L"%i", pPlayer->m_ProfileData.m_Game.m_Rank + 1 );
		}
		else
		if ( strncmp(pVariableName, "_INFO", 5 ) == 0 ) 
		{
			const RacerNet::NetPlayerVehicle& netVehicle = pPlayer->m_RaceData.m_VehicleData; 
			if (netVehicle.m_xtVehicleId != 0) // 0 means we have yet to receive this data from the peer.
			{
				const AmaxVehicle *xtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)netVehicle.m_xtVehicleId];
				if ( pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateRacing || pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateLoadingRace )
				{
					m_VariableText.Format(UI::C_TextStringID( "MP_IN_RACE" ).String());
				}
				else
				if ( pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateViewingResults || pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateViewingResultsIdle )
				{
					m_VariableText.Format(UI::C_TextStringID( "MP_FINISHED" ).String());
				}
				else
				if (xtVehicle) 
				{
					RacerNet::NetRaceSettings raceSettings;
					pGameMode->GetNetRaceSettings( raceSettings );
					bool random_cars = ( raceSettings.m_Misc & ( 1 << UI::E_MiscSettings_RandomCars ) ) ? true : false;
						
					// Random cars in world tour
					if ( pGameMode->GetID() == Game::E_GameMode_Online ) // TODO_MP_REFACTOR
					{
						if (random_cars)
							m_VariableText.Format(UI::C_TextStringID( "CMN_RANDOM" ).String());
						else
							m_VariableText.Format(UI::C_TextStringID( "CMN_WAITING" ).String());
					}
					else
					{
						if (random_cars)
							m_VariableText.Format(UI::C_TextStringID( "CMN_RANDOM" ).String());
						else
							m_VariableText.Format(UI::C_TextStringID(xtVehicle->loc_string.id).String());
					}
				}
				else
				{
					// must be dlc vehicle as we dont have it in our content list.
					m_VariableText.Format(UI::C_TextStringID( "CMN_DOWNLOADABLE_CONTENT" ).String());
				}
			}
		}
	}
	/*---------------------------
		Race Preview Data
	----------------------------*/
	else
	if ( strncmp(pVariableName, "PREVIEW", 7 ) == 0 ) 
	{
		pVariableName += 7;

		const S8 index = (S8)ParseIndex( pVariableName, "", true ) - 1;
		const Game::LobbyWaitingRaceInfo& lobbyInfo = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo();

		// Get game mode pointer
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if ( !pLayer )
			return L"";

		Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());
		if ( !pGameMode )
			return L"";

		const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
		if (!pEvent && pGameMode->GetPlayMode() == Game::E_PlayMode_Playlist)			
			return L"";

		if ( strncmp(pVariableName, "_NAME", 5 ) == 0 ) 
		{
			if ( index >= 0 && index < lobbyInfo.m_Vehicles.Size() )
			{
				// Its a player whos in the race or hes disconnected and has an invalid id
				if( lobbyInfo.m_Vehicles[index].m_JoinId < RacerNet::NET_AI_STARTING_JOIN_ID || lobbyInfo.m_Vehicles[index].m_JoinId == 0xff )
				{
					RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
					RacerNet::C_NetPlayer* pPlayer = mgr.FindFromJoinId( lobbyInfo.m_Vehicles[index].m_JoinId, false );

					m_VariableText.Format( L"%s", ( pPlayer ) ? pPlayer->m_User.m_Name : C_TextStringID( "MP_DISCONNECTED" ).String() );
				}
				else // Is an AI player
				{
					if ( C_MultiplayerData::Instance().m_pConfig && C_MultiplayerData::Instance().m_pConfig->m_AiCharacters.count )
					{
						const U8 aiIndex = Maths::Clamp( (U8)(lobbyInfo.m_Vehicles[index].m_JoinId - RacerNet::NET_AI_STARTING_JOIN_ID), (U8)0, (U8)RacerNet::NET_MAX_VEHICLES );
						m_VariableText.Format( UI::C_TextStringID( C_MultiplayerData::Instance().m_pConfig->m_AiCharacters[aiIndex]->loc_string.id ).String() );
					}
				}
			}
		}
		else
		if ( strncmp(pVariableName, "_STATE", 6 ) == 0 ) 
		{
			if ( index >= 0 && index < lobbyInfo.m_Vehicles.Size() )
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				RacerNet::C_NetPlayer* pPlayer = mgr.FindFromJoinId( lobbyInfo.m_Vehicles[index].m_JoinId, false );

				if ( !pPlayer )
					return L"";

				switch ( pPlayer->m_RaceConnection.m_PlayerState )
				{
					case RacerNet::eNetPlayerStateFinished				: m_VariableText.Format( L"%s", C_TextStringID( "MP_FINISHED" ).String() ); break;
					case RacerNet::eNetPlayerStateViewingResults		: m_VariableText.Format( L"%s", C_TextStringID( "MP_VIEWING_RESULTS" ).String() ); break;
					case RacerNet::eNetPlayerStateViewingResultsIdle	: m_VariableText.Format( L"%s", C_TextStringID( "MP_VIEWING_AWARDS" ).String() ); break;

					default:
					{
						if ( pGameMode->GetPlayModeBase().GetRaceMode() == UI::E_RaceType_Destruction )
							m_VariableText.Format( L"%d", (S32)lobbyInfo.m_Vehicles[index].m_StatData );
					}
				}
			}
		}
		else
		if ( strncmp(pVariableName, "_LAPS", 5 ) == 0 )
		{
			if ( pGameMode->GetPlayModeBase().GetRaceMode() == UI::E_RaceType_Racing )
			{
				if ( lobbyInfo.m_Vehicles.Size() )
				{
					// Vehicles are sorted so check the leader
					const S8 lap = Maths::Clamp( (S8)lobbyInfo.m_Vehicles[0].m_StatData, (S8)1, pGameMode->GetPlayModeBase().GetLapLimit() );

					m_VariableText.Format( C_TextStringID( "MP_PREVIEW_LAP_COUNT" ).String(), lap, pGameMode->GetPlayModeBase().GetLapLimit() );
				}
				else
				{
					// If its loading then we are most likely to be on lap 1
					m_VariableText.Format( C_TextStringID( "MP_PREVIEW_LAP_COUNT" ).String(), 1, pGameMode->GetPlayModeBase().GetLapLimit() );
				}
			}
		}
		else
		if ( strncmp(pVariableName, "_TIME", 5 ) == 0 )
		{
			// -1.0f means the timer isnt active
			if ( lobbyInfo.m_Timeleft >= 0.0f )
			{
				m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTime( lobbyInfo.m_Timeleft ) );
			}
			else
			if ( pGameMode->GetPlayModeBase().GetRaceMode() == UI::E_RaceType_Destruction && lobbyInfo.m_Vehicles.Size() == 0 )
				m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTime( pGameMode->GetPlayModeBase().GetTimeLimit() ) );
		}
	}
	return m_VariableText;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
