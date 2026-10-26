
#include "stdafx.h"
#include "UILobby.h"

#include "ui/Game/UIDataFinder.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "Ui/Graphics/SimpleTimer.h"

// Network headers.
#include <Network/Misc/NetDefines.h>
#include <Network/Misc/NetRaceSettings.h>
#include <Network/Core/NetServices.h>
#include <Network/Core/NetRace.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetplayerConnection.h>
#include <Network/Player/NetPlayerManager.h>

#include "ui/Game/Profiles/ProfileInterface.h"
#include "ui/Game/Profiles/GamerPictureManager.h"
#include "ui/LobbyDataFinder.h"
#include "ui/UiInterface.h"
#include "ui/SoundEffects.h"

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

// XT Definition
XT_TYPE( UI::UILobby, "UILobby" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_PartyLobby, "party_lobby" )
XT_END_TYPE


namespace UI
{

const S32 kLobbyPlayerIndex	= 0;
const F32 kLobbyTeamRefreshRate = 0.25f;

UILobby::UILobby()
{
	C_LobbyDataFinder::Instance().SetLobbyPtr( this );
}

UILobby::~UILobby()
{
	C_LobbyDataFinder::Instance().SetLobbyPtr( NULL );
}

void UILobby::Init()
{
	C_GadgetBase::Init();

	m_ShouldRefresh = false;
	m_PartyLobby = false;
	m_TeamLobby = false;
	m_TeamDataValid = false;
	m_TeamUpdateTimer = kLobbyTeamRefreshRate;

	AddSubscriber( E_UiMsg_FrameUpdate );
}

void UILobby::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	I_GamerPictureManager* pGamerPictureManager = C_ProfileInterface::Instance().GamerPictureManager();
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

	// Build a job list of all the current players in the lobby.
	if ( services.IsInitialised() && pGamerPictureManager )
	{
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

		// Session isnt created
		if ( mgr.TotalSize() <= 0 )
			return;

		pGamerPictureManager->Flush();

		/*
		// Clean up any pictures that are still active incase players have left outside the lobby
		for ( S32 i = 0; i < pGamerPictureManager->TotalSize(); i++ )
		{
			bool found = false;
			for ( S32 j = 0; j < mgr.TotalSize(); j++ )
			{
				if ( pGamerPictureManager->RemoteSlot( i )->UserData() == mgr.Index( j )->m_User )
				{
					found = true;
					break;
				}
			}

			// does it exist?
			if ( !found )
				pGamerPictureManager->ClearRemotePictureByIndex( i );
		}
		*/

		// Create slot data for players who are in the session
		for ( S32 i = 0; i < mgr.TotalSize(); i++ )
		{
			RacerNet::C_NetPlayer* pPlayer = mgr.Index(i);

			// Ignore players who are joining
			if ( !pPlayer || ( ( m_PartyLobby ) ? pPlayer->m_PartyConnection.m_PlayerState == RacerNet::eNetPlayerStateJoining : pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateJoining ) )
				continue;

			// ignore players who arnt in the party. This will stop race players poping into the party lobby when connecting to a game.
			if ( m_PartyLobby && !pPlayer->m_PartyConnection.IsActive() )
				continue;

			// Store slot information
			m_Slots[i].empty	= false;
			m_Slots[i].id		= pPlayer->m_ConsoleId;
			m_Slots[i].ref		= ( m_PartyLobby ) ? pPlayer->m_PartyConnection.m_JoinId : pPlayer->m_RaceConnection.m_JoinId;
			m_Slots[i].racing	= pPlayer->IsInRace();

			// Request gamerpic
			if ( !pPlayer->LocalPlayer() ) 
				pGamerPictureManager->RequestRemotePicture( SysProfile::C_Manager::Instance().GetPrimaryPadNo(), pPlayer->m_User, m_Slots[i].ref );
		}
	}

	// Is it a team game
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	if ( pLayer->GetGameModePtr()->GetID() == Game::E_GameMode_Online || pLayer->GetGameModePtr()->GetID() == Game::E_GameMode_SystemLink )
		m_TeamLobby = ((Game::C_GameModeNetwork*)pLayer->GetGameModePtr())->IsTeamGame();
}


void UILobby::FrameUpdate()
{
	if ( !m_Activated )
		return;

	C_GadgetBase::FrameUpdate();

	// Only on live
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

	// Session isnt created
	if ( mgr.TotalSize() <= 0 )
		return;

	I_GamerPictureManager* pGamerPictureManager = C_ProfileInterface::Instance().GamerPictureManager();

	// Has anybody left
	for ( S32 i = 0; i < m_Slots.Size(); i++ )
	{
		bool found = false;
		for ( S32 j = 0; j < mgr.TotalSize(); j++ )
		{
			if ( m_Slots[i].empty )
				continue;

			if ( m_Slots[i].id == mgr.Index(j)->m_ConsoleId )
			{
				found = true;
				break;
			}
		}
		// Flag slot as empty
		if ( !found && !m_Slots[i].empty )
		{
			m_ShouldRefresh = true; // flag ui to update

			// Empty gamerpic slot
			pGamerPictureManager->ClearRemotePicture( m_Slots[i].ref );

			m_Slots[i].empty = true;
			m_Slots[i].id = 0;
			m_Slots[i].ref = 0;
			m_Slots[i].racing = false;

			// Play leave sound
			UI::UiInterface()->PlaySound( E_UiSoundEffect_LobbyLeaveActionIn );
		}
	}

	// Has anybody joined
	for ( S32 i = 0; i < mgr.TotalSize(); i++ )
	{
		RacerNet::C_NetPlayer* pPlayer = mgr.Index(i);

		// Ignore players who are joining
		if ( !pPlayer || ( ( m_PartyLobby ) ? pPlayer->m_PartyConnection.m_PlayerState == RacerNet::eNetPlayerStateJoining : pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateJoining ) )
			continue;

		// ignore players who arnt in the party. This will stop race players poping into the party lobby when connecting to a game.
		if ( m_PartyLobby && !pPlayer->m_PartyConnection.IsActive() )
			continue;

		bool found = false;
		S32 index = -1;
		for ( S32 j = 0; j < m_Slots.Size(); j++ )
		{
			if ( m_Slots[j].empty )
			{
				if ( index == -1 )
					index = j; // store first availible slot
				continue;
			}

			if ( m_Slots[j].id == pPlayer->m_ConsoleId )
			{
				found = true;

				const bool inRace = ( pPlayer->m_RaceConnection.m_PlayerState >= RacerNet::eNetPlayerStateLoadingRace );

				// need to check this every frame incase we enter the lobby while a player is on the loading screen
				if ( inRace != m_Slots[j].racing )
				{
					m_Slots[j].racing = inRace;
					m_ShouldRefresh = true;
				}
				break;
			}
		}

		// Flag slot as taken
		if ( !found && index != -1 )
		{
			m_ShouldRefresh = true; // flag ui to update

			m_Slots[index].empty = false;
			m_Slots[index].id = pPlayer->m_ConsoleId;
			m_Slots[index].ref = ( m_PartyLobby ) ? pPlayer->m_PartyConnection.m_JoinId : pPlayer->m_RaceConnection.m_JoinId;
			m_Slots[index].racing = ( pPlayer->m_RaceConnection.m_PlayerState >= RacerNet::eNetPlayerStateLoadingRace );

			// Request gamerpic
			if ( !pPlayer->LocalPlayer() ) 
				pGamerPictureManager->RequestRemotePicture( SysProfile::C_Manager::Instance().GetPrimaryPadNo(), pPlayer->m_User, m_Slots[index].ref );

			// Play leave sound
			UI::UiInterface()->PlaySound( E_UiSoundEffect_LobbyJoinActionIn );
		}
	}

	// Is it a team game
	if ( m_TeamLobby )
	{
		m_TeamUpdateTimer -= theFETimer.FrameTime();
		if ( m_TeamUpdateTimer <= 0.0f )
		{
			m_TeamUpdateTimer = kLobbyTeamRefreshRate;
			m_ShouldRefresh = true;
		
			BuildTeamView();
		}
	}
}

void UILobby::BuildTeamView()
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetTeamBalancer& teams = services.Race().GetTeamBalancer();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeNetwork* pGameMode = ((Game::C_GameModeNetwork*)pLayer->GetGameModePtr());

	// Clear data
	for ( S32 i = 0; i < m_TeamSlots.Size(); i++ )
	{
		m_TeamSlots[i].empty = true;
		m_TeamSlots[i].id = 0;
		m_TeamSlots[i].ref = 0;
		m_TeamSlots[i].racing = false;
	}

	m_TeamDataValid = false; // invalidate this

	// Race is in progress
	if ( !services.Race().PlayersInLobby() )
		return;

	// Build team slots
	S8 playerTeam = pGameMode->GetLocalPlayerTeam();

	S8 indexA = ( playerTeam == RacerNet::NET_TEAM_A ) ? 0 : 10;
	S8 indexB = ( playerTeam == RacerNet::NET_TEAM_A ) ? 10 : 0;
	for ( S32 i = 0; i < services.PlayerManager().TotalSize(); i++ )
	{
		RacerNet::C_NetPlayer* pPlayer = services.PlayerManager().Index( (U8)i );
		BIZ_ASSERT_SPEED( pPlayer );

		if ( pPlayer->m_RaceData.m_Team == -1 ) // havent recieved any team data about this player
			continue;

		S8& index = ( pPlayer->m_RaceData.m_Team == RacerNet::NET_TEAM_A ) ? indexA : indexB;

		m_TeamSlots[index].empty = false;
		m_TeamSlots[index].id = pPlayer->m_ConsoleId;
		m_TeamSlots[index].ref = pPlayer->m_RaceConnection.m_JoinId;
		m_TeamSlots[index].racing = ( pPlayer->m_RaceConnection.m_PlayerState >= RacerNet::eNetPlayerStateLoadingRace );

		m_TeamDataValid = true;
		index++;
	}
}

void UILobby::SendToLua( lua_State* pState )
{
	lua_newtable( pState );

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeNetwork* pGameMode = ((Game::C_GameModeNetwork*)pLayer->GetGameModePtr());

	RacerNet::NetRaceSettings netSettings;
	pGameMode->GetNetRaceSettings( netSettings );

	const S32 maxSize = ( m_PartyLobby ) ? RacerNet::NET_MAX_PARTY_MEMBERS : netSettings.m_MaxDrivers;

	for ( S32 i = 0; i < maxSize; i++ )
	{
		// If we are in team view then render the team slots.
		const Slot* pSlot = ( m_TeamLobby && m_TeamDataValid ) ? &m_TeamSlots[i] : &m_Slots[i];

		RacerNet::C_NetPlayer* pPlayer = services.PlayerManager().FindFromJoinId( pSlot->ref, m_PartyLobby );
		
		U8 rank = 0, legend = 0, team = 0;
		bool player = false, party = false, host = false;
		if ( pPlayer )
		{
			player	= pPlayer->LocalPlayer();
			rank	= pPlayer->m_ProfileData.m_Game.m_Rank;
			legend	= pPlayer->m_ProfileData.m_Game.m_Legend;
			host	= pPlayer->m_PartyConnection.m_Host;
			party	= pPlayer->m_PartyConnection.IsActive();
			team	= pPlayer->m_RaceData.m_Team;
		}

		lua_newtable( pState );
			Lua::AddDataMember( pState, "empty",			pSlot->empty );
			Lua::AddDataMember( pState, "ref",				pSlot->ref );
			Lua::AddDataMember( pState, "racing",			pSlot->racing );
			Lua::AddDataMember( pState, "local_player",		player );
			Lua::AddDataMember( pState, "rank",				rank );
			Lua::AddDataMember( pState, "legend",			legend );
			Lua::AddDataMember( pState, "party_host",		host );
			Lua::AddDataMember( pState, "party_member", 	party );
			Lua::AddDataMember( pState, "team", 			team );
		lua_rawseti( pState, -2, i+1 );
	}

	m_ShouldRefresh = false; // the lobby should have called this after ShouldRefresh()
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", on )
#endif
