#include "Stdafx.h"

#include <algorithm>

#include <ui/Game/UILUA.h>
#include <ui/DashboardInterface.h>

#include "GameLogic/GameData/MultiplayerData.h"
#include "UI/Game/Multiplayer/MultiplayerCache.h"
#include "UI/UiHelpers.h"
#include "UI/UiXtConfig.h"

#include "GameApp.h"
#include "Game/Game.h"
#include "GameLogic/Structures/LevelState.h"
#include "GameLogic/Structures/MpResults.h"
#include "GameLogic/Structures/MultiplayerAwards.h"
#include "GameLogic/Profile/MultiplayerProfile.h"
#include <GameLogic/ScoreboardManager.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"
#include "Game/GameActors/Components/GameComponents.h"
#include "Game/messaging/GameflowMessages.h"

#include "Network/Misc/NetRaceSettings.h"

#include "ui/Game/Profiles/ProfileInterface.h"
#include "ui/Game/Profiles/GamerPictureManager.h"

#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/UILobby.h"
#include "ui/Game/Gadgets/UIRacePreview.h"

#include <Network/Core/NetServices.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Player/NetPlayerConnection.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Core/NetParty.h>

#include "Automation\GameDataLogging.h"

#include <system\time\Time.h>
#include <GameLogic\GameModes\GameModeBase.h>
#include <GameLogic\GameModes\GameModeMultiplayer.h>

#include <Gamelogic\SocialNetworking\SocialNetworkManager.h>
#include <Game\dlc\DLC_Controller.h>

#include <GameLogic\StickerManager.h>
#include <GameLogic\ActorStudioActors\DynamicObjectMessages.h>
#include <GameLogic\ActorStudioActors\ActorUtils.h>

#include <Game\Achievements\AvatarAwardSystem.h>
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#ifndef CFG_GOLD
Debug::Bool g_DebugCheatShowAllGarage( "Cheats.ShowAllGarage", false );
Debug::Int g_DebugGarageMpLevelOverride( "RaceBook.DebugGarageMpLevelOverride", -1, Debug::Range( -1, 49 ) );
extern Debug::Int g_MaxLegendLevel;

// Debug test func for legend reset functionality
// Made to copy behaviour of static S32 LegendModeReset( lua_State* pState )
static void DebugAdvanceLegendLevel(void* par)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	if(pGameProfile)
	{
		Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
		if(pMultiplayerProfile)
		{
			pMultiplayerProfile->UnlockLegendMode();
		}
	}

}
Debug::Function g_MultiplayerProfileAdvanceLegendLevel("Multiplayer.PrimaryProfile.AdvanceLegendLevel", DebugAdvanceLegendLevel, 0);
#endif

namespace UI
{

Debug::Int g_FindPlayersTimer("Multiplayer.Ranked.FindPlayersTimer", -1, Debug::Range( 0, 60 ) );

bool NameAlphaSort( const WCHAR* pLhv, const WCHAR* pRhv )
{
	bool finished = false;
	while( !finished )
	{
		if ( *pLhv == '\0' || *pRhv == '\0' || *pLhv > *pRhv )
		{
			return false;
		}
		else
		if ( *pLhv < *pRhv )
		{
			finished = true;
		}
		pLhv++;
		pRhv++;
	}
	return true;
}

bool CitySort( const AmaxCity* pLhv, const AmaxCity* pRhv )
{
	return NameAlphaSort( C_TextStringID( pLhv->loc_string.id ).String(), C_TextStringID( pRhv->loc_string.id ).String() );
}

bool RouteSort( const AmaxRoute* pLhv, const AmaxRoute* pRhv )
{
	return NameAlphaSort( C_TextStringID( pLhv->loc_string.id ).String(), C_TextStringID( pRhv->loc_string.id ).String() );
}

static S32 GetPlaylist( lua_State* pState )
{
	lua_newtable(pState);

	const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
	BIZ_ASSERT( pEvent );

	// Push data
	Lua::AddDataMember( pState, "icon_name",	pEvent->icon_name );
	Lua::AddDataMember( pState, "image_name",	pEvent->image_name );

	return 1;
}

// [WillB] #35660 - Fixed up player counts for demonware groups
static S32 GetPlaylists( lua_State* pState )
{	
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "GetPlaylists( ranked:bool ) - incorrect arguments" );
		return 0;
	}

	const bool ranked = lua_toboolean( pState, 1 ) ? true : false;

	// Get profile data
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
	const Game::C_MultiplayerUnlockHistory& history=pProfile->GetRecentUnlockHistory();

	const S32 rank = pProfile->GetCurrentLevel() + 1; // playlist rank values are 1-50
	const S32 legend = ( ranked ) ? pProfile->GetLegendLevel() : 10; // Force driving school off in private games

	lua_newtable(pState);

	S32 index = 1;
	for ( S32 i = 0; i < UI::C_MultiplayerCache::Instance().NumEvents(); i++ )
	{
		const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().Event( i );

		const bool forceUnlock = (pEvent && pEvent->dlcUnlockId > 0 && DLCController::Instance().IdIsUnlocked(pEvent->dlcUnlockId));
		const bool isLocked = ( rank < pEvent->min_rank || rank > pEvent->max_rank ) && !forceUnlock;

		// Check rank cap
		if ( ranked && rank > pEvent->max_rank )
			continue;

		// Check legend cap
		if ( legend > pEvent->max_legend )
			continue;

		lua_newtable( pState );
			Lua::AddDataMember( pState, "playlist_index",	i );
			Lua::AddDataMember( pState, "name",				pEvent->name.id );
			Lua::AddDataMember( pState, "icon_name",		pEvent->icon_name );
			Lua::AddDataMember( pState, "image_name",		pEvent->image_name );
			Lua::AddDataMember( pState, "rank",				pEvent->min_rank );
			Lua::AddDataMember( pState, "not_available",	pEvent->not_available );

			// TODO : This should only pass data if its online ranked
			Lua::AddDataMember( pState, "locked",		isLocked );
			Lua::AddDataMember( pState, "hidden",		pProfile->GetCurrentLevel() > pEvent->max_rank );
			Lua::AddDataMember( pState, "new",			history.GetEventUnlocked(pEvent->ID));
		lua_rawseti( pState, -2, index++ );
	}	

	return 1;
}


static S32 GetPlaylistRace( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetPlaylistRace( index ) - incorrect arguments" );
		return 0;
	}

	const S16 raceIndex = (S16)lua_tonumber( pState, 1 );

	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	const MultiplayerEvent* pEvent = C_MultiplayerCache::Instance().CurrentEvent();
	const MultiplayerRace* pRace = pEvent->races[pGameMode->GetPlaylist().GetRaceDataIndex( raceIndex )];

	if ( !pRace )
		return 0;

	lua_newtable(pState);
	Lua::AddDataMember( pState, "city",				pRace->route->city->ID );
	Lua::AddDataMember( pState, "city_name",		pRace->route->city->loc_string.id );
	Lua::AddDataMember( pState, "route",			pRace->route->ID );
	Lua::AddDataMember( pState, "route_name",		pRace->route->loc_string.id );
	Lua::AddDataMember( pState, "type",				pRace->type );
	Lua::AddDataMember( pState, "class",			pGameMode->GetPlaylist().GetCarClass( raceIndex ) );

	Lua::AddDataMember( pState, "laps",				pRace->laps );
	Lua::AddDataMember( pState, "time",				pRace->time );
	Lua::AddDataMember( pState, "score_limit",		pRace->score_limit );

	Lua::AddDataMember( pState, "thumbnail",		pRace->route->tag );
	Lua::AddDataMember( pState, "city_icon",		pRace->route->city->icon_name );
		
	return 1;
}

static S32 GetCurrentRace( lua_State* pState )
{
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	if ( pGameMode->GetPlayMode() == Game::E_PlayMode_Playlist )
	{
		const MultiplayerEvent* pEvent = C_MultiplayerCache::Instance().CurrentEvent();
		const MultiplayerRace* pRace = pEvent->races[pGameMode->GetPlaylist().GetRaceDataIndex()];

		lua_newtable(pState);
		Lua::AddDataMember( pState, "city",				pRace->route->city->ID );
		Lua::AddDataMember( pState, "city_name",		pRace->route->city->loc_string.id );
		Lua::AddDataMember( pState, "route",			pRace->route->ID );
		Lua::AddDataMember( pState, "route_name",		pRace->route->loc_string.id );
		Lua::AddDataMember( pState, "type",				pRace->type );
		Lua::AddDataMember( pState, "class",			pGameMode->GetPlaylist().GetCarClass() );

		Lua::AddDataMember( pState, "laps",				pRace->laps );
		Lua::AddDataMember( pState, "time",				pRace->time );

		Lua::AddDataMember( pState, "thumbnail",		pRace->route->tag );
		Lua::AddDataMember( pState, "city_icon",		pRace->route->city->icon_name );
	}
	else
	if ( pGameMode->GetPlayMode() == Game::E_PlayMode_CustomRace )
	{
		const Game::C_CustomRace& customRace = pGameMode->GetCustomRace();

		const AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)customRace.GetCity() ];
		const AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)customRace.GetRoute() ];
		BIZ_ASSERT( pCity && pRoute );

		lua_newtable(pState);
		Lua::AddDataMember( pState, "city",				pCity->ID );
		Lua::AddDataMember( pState, "city_name",		pCity->loc_string.id );
		Lua::AddDataMember( pState, "route",			pRoute->ID );
		Lua::AddDataMember( pState, "route_name",		pRoute->loc_string.id );
		Lua::AddDataMember( pState, "type",				customRace.GetRaceMode() );
		Lua::AddDataMember( pState, "class",			customRace.GetCarClass() );

		Lua::AddDataMember( pState, "laps",				customRace.GetLapLimit() );
		Lua::AddDataMember( pState, "time",				customRace.GetTimeLimit() );

		Lua::AddDataMember( pState, "thumbnail",		pRoute->tag );
		Lua::AddDataMember( pState, "city_icon",		pCity->icon_name );
	}
	return 1;
}

static S32 GetFeatureUnlocked( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetFeatureUnlocked( FeatureId ) - incorrect arguments" );
		return 0;
	}

	const UI::E_MultiplayerFeature feature = (UI::E_MultiplayerFeature)(S32)lua_tonumber( pState, 1 );

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG( pGameProfile, "No game profile" );

	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
	BIZ_ASSERT_MSG( pProfile, "No multiplayer profile" );

	lua_pushboolean( pState, pProfile->GetFeatureUnlocked(feature));

	return 1;
}

static S32 GetFeatureUnlockedMap( lua_State* pState )
{
	lua_newtable( pState );

	TLevelData levels = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;

	for( S32 i = 0; i < levels.count; i++ )
	{
		XtArray<XtHandle<MultiplayerFeature > >& featureUnlocks = levels[i].mFeatureUnlocks;
		for( S32 j = 0; j < featureUnlocks.count; j++ )
		{
			lua_pushnumber( pState, i + 1 );
			lua_rawseti( pState, -2, featureUnlocks[j]->mFeatureId );
		}
	}

	return 1;
}

static S32 CachePlaylists( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "CachePlaylists( mode(gamemode) ) - incorrect arguments" );
		return 0;
	}

	// [WillB] #35660 - Fixed up player counts for demonware groups
	C_MultiplayerCache::Instance().CachePlaylists( (Game::E_GameMode)(S32)lua_tonumber( pState, 1 ) );

	return 0;
}


static S32 FindPlaylistFromID( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "FindPlaylistFromID( num ) - incorrect arguments" );
		return 0;
	}

	const U32 eventID = (U32)lua_tonumber( pState, 1 );
	const bool result = C_MultiplayerCache::Instance().CachePlaylistByID( eventID );

	lua_pushboolean( pState, result );
	return 1;
}


static S32 CacheNetRaceSettings( lua_State* pState )
{
	C_MultiplayerCache::Instance().CacheNetRaceSettings();

	return 0;
}


static S32 CacheInviteSettings( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "CacheInviteSettings( party ) - incorrect arguments" );
		return 0;
	}

	const bool party = lua_toboolean( pState, 1 ) ? true : false;
	lua_pushboolean( pState, C_MultiplayerCache::Instance().CacheInviteSettings( party ) );

	return 1;
}

static S32 SetCurrentEventIndex( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetCurrentEventIndex( index ) - incorrect arguments" );
		return 0;
	}

	S32 index=(S32)lua_tonumber( pState, 1 );
	C_MultiplayerCache::Instance().SetCurrentEventIndex( index );

	return 0;
}

static S32 GetCurrentEventIndex( lua_State* pState )
{
	lua_pushnumber( pState, C_MultiplayerCache::Instance().GetCurrentEventIndex() );

	return 1;
}

static S32 IsChampionshipMode( lua_State* pState )
{
	// Get the multiplayer mode
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	lua_pushboolean( pState, pGameMode->GetPlaylist().IsChampionship() );

	return 1;
}

static S32 ResetChampionship( lua_State* pState )
{
	Game::C_GameModeBase* pBaseMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	if ( pBaseMode->IsNetwork() )
		((Game::C_GameModeNetwork*)pBaseMode)->ResetChampionship();
	
	return 0;
}

static S32 GetCurrentRaceIndex( lua_State* pState )
{
	// Get the multiplayer mode
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	lua_pushnumber( pState, pGameMode->GetPlaylist().GetRaceIndex() );

	return 1;
}

static S32 GetChampionshipStandings( lua_State* pState )
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	Game::C_GameModeBase* pGameMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	BIZ_ASSERT(pGameMode);
	if (!pGameMode->IsOnline())
	{
		g_pUILua->StackDump( pState, "GetChampionshipStandings( ) - no online game in progress" );
		return 0;
	}

	Game::C_GameModeNetwork* networkGameMode = static_cast<Game::C_GameModeNetwork*>(pGameMode);
	const Game::C_GameModeNetwork::SortedChampionshipStandingContainer& standings=networkGameMode->GetSortedChampionshipStandings();

	lua_newtable(pState);

	for ( int i = 0; i < standings.Size(); i++ )
	{
		lua_newtable( pState );

		S8 localIndex=Game::GetLocalIndexFromJoinId(standings[i].mJoinId);

		if (localIndex>=0)
		{
			const Game::LevelStateCar& car=levelState.mCars[localIndex];

			Lua::AddDataMember( pState, "rank",			car.mMultiplayerRank );
			Lua::AddDataMember( pState, "legend",		car.mMultiplayerLegendLevel );
			Lua::AddDataMember( pState, "local_player", ( localIndex == 0 ) );
		}
		else
		{ // error handling
			Lua::AddDataMember( pState, "rank",1 );
			Lua::AddDataMember( pState, "legend",1 );
		}

		lua_rawseti( pState, -2, i+1 );
	}

	return 1;
}


static S32 GetRaceResults( lua_State* pState )
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();


	lua_newtable(pState);

	Lua::AddDataMember( pState, "num_players",	levelState.mNumCars );
	Lua::AddDataMember( pState, "player_index",	Game::C_MpResults::Instance().m_PlayerSortedIndices[SysProfile::C_Manager::Instance().GetPrimaryPadNo()] + 1);

	lua_pushstring( pState, "standings" );
	lua_newtable(pState);

	S32 playerIndex = 0;
	for ( S32 i = 0; i < (S32)levelState.mNumCars; i++ )
	{
		const Game::NetVehicleData& vehicleData = Game::C_MpResults::Instance().m_SortedVehicleResults[i];

		lua_newtable( pState );
		Lua::AddDataMember( pState, "racing",			( vehicleData.m_State == Game::E_VEHICLE_STATE_RACING ) ? true : false );
		Lua::AddDataMember( pState, "finished",			( vehicleData.m_State == Game::E_VEHICLE_STATE_FINISHED || vehicleData.m_State == Game::E_VEHICLE_STATE_ELIMINATED ) ? true : false );
		Lua::AddDataMember( pState, "timeout",			( vehicleData.m_State == Game::E_VEHICLE_STATE_TIMED_OUT ) ? true : false );
		Lua::AddDataMember( pState, "join_ref",			levelState.mCars[vehicleData.m_LocalIndex].mNetJoinId );
		Lua::AddDataMember( pState, "rank",				levelState.mCars[vehicleData.m_LocalIndex].mMultiplayerRank );
		Lua::AddDataMember( pState, "legend",			levelState.mCars[vehicleData.m_LocalIndex].mMultiplayerLegendLevel );
		Lua::AddDataMember( pState, "vehicle_index",	vehicleData.m_LocalIndex );

		// For AI drivers
		Lua::AddDataMember( pState, "ai_driver", ( levelState.mCars[vehicleData.m_LocalIndex].mAiAvatarId == -1 ) ? false : true );
		Lua::AddDataMember( pState, "ai_avatar_id", levelState.mCars[vehicleData.m_LocalIndex].mAiAvatarId );
		lua_rawseti( pState, -2, i+1 );
	}

	lua_rawset(pState, -3);


	if ( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsRanked() &&  
		 C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceMode() == Game::E_RaceMode_Multiplayer_Destruction )
	{
		Game::MpVehiclesData& sortedResults = Game::C_MpResults::Instance().m_SortedVehicleResults;
		for( U8 i = 0; i < levelState.mNumCars; i++ )
		{
			if( sortedResults[i].m_LocalIndex == 0 )
			{
				bool won   = sortedResults[i].m_Position == 1; // i.e. first in the array of sorted results 
				S32 points = Game::C_MpResults::Instance().m_SortedVehicleResults[i].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
				Game::C_StickerManager::Instance().CheckMotorman(won,points);
			}
		}
	}
	
	return 1;
}

static S32 GetTeamResults( lua_State* pState )
{
	// Cycle through teams
	const char* names[] =
	{
		"team_a",
		"team_b",
	};

	lua_newtable(pState);

	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	
	S32 winning_team = 0;
	S32 highest_score = 0;
	bool draw = false;

	for ( S32 team = 0; team < Game::C_MpResults::Instance().m_Teams.Size(); team++ )
	{
		lua_pushstring( pState, names[team] );
		lua_newtable(pState);

		const Game::NetTeamData& teamData = Game::C_MpResults::Instance().m_Teams[team];

		// calculate the winning team
		if ( teamData.m_TeamScore > highest_score )
		{
			highest_score = teamData.m_TeamScore;
			winning_team = team;
		}
		else
		if ( teamData.m_TeamScore == highest_score )
			draw = true;

		// Push team racer data
		for ( S32 i = 0; i < teamData.m_RacerOrder.Size(); i++ )
		{
			const S32 index = teamData.m_RacerOrder[i];
			const Game::NetVehicleData& vehicleData = Game::C_MpResults::Instance().m_SortedVehicleResults[index];

			lua_newtable( pState );
				Lua::AddDataMember( pState, "rank",				levelState.mCars[vehicleData.m_LocalIndex].mMultiplayerRank );
				Lua::AddDataMember( pState, "legend",			levelState.mCars[vehicleData.m_LocalIndex].mMultiplayerLegendLevel );
				Lua::AddDataMember( pState, "join_ref",			levelState.mCars[vehicleData.m_LocalIndex].mNetJoinId );
				Lua::AddDataMember( pState, "index",			index );
				Lua::AddDataMember( pState, "vehicle_index",	vehicleData.m_LocalIndex );
				Lua::AddDataMember( pState, "player",			vehicleData.m_LocalIndex == 0 );
			lua_rawseti( pState, -2, i+1 );
		}

		lua_rawset( pState, -3 );
	}

	Lua::AddDataMember( pState, "winner", names[winning_team] );
	Lua::AddDataMember( pState, "loser", names[!winning_team] );
	Lua::AddDataMember( pState, "draw", draw );

	return 1;
}

static S32 GetPlayerTeam( lua_State* pState )
{
	const char* names[] = { "team_a", "team_b", };

	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	lua_pushstring( pState, names[levelState.mCars[0].mTeam] );

	return 1;
}

static S32 VehiclesFinished( lua_State* pState )
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	lua_newtable( pState );
	Lua::AddDataMember( pState, "num_players",	levelState.mNumCars );
	Lua::AddDataMember( pState, "player", (S32)Game::C_MpResults::Instance().m_FinishedOrderPlayerIndex );

	lua_pushstring( pState, "list" );
	lua_newtable( pState );

	for ( S32 i = 0; i < Game::C_MpResults::Instance().m_FinishedOrder.Size(); i++ )
	{
		lua_newtable( pState );
		Lua::AddDataMember( pState, "index",	Game::C_MpResults::Instance().m_FinishedOrder[i].m_Index );
		Lua::AddDataMember( pState, "state",	Game::C_MpResults::Instance().m_FinishedOrder[i].m_Failed );

		U8 join_ref = 255;
		if ( Game::C_MpResults::Instance().m_FinishedOrder[i].m_Index > -1 )
			join_ref = levelState.mCars[Game::C_MpResults::Instance().m_FinishedOrder[i].m_Index].mNetJoinId;

		Lua::AddDataMember( pState, "join_ref",	join_ref );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset(pState, -3);

	return 1;
}


static S32 E3EnablePlayerControls( lua_State* pState )
{
	const Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( UI::GetPlayerVehicleIndex(0) );

	pLogicActor->SendMessageToComponent( Components::E_IF_VehicleComponent_E3Controller, Game::E_GameflowMessage_ToggleToPlayer );

	return 0;
}

static S32 RefreshLobby( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "RefreshLobby( id ) - incorrect arguments" );
		return 0;
	}

	bool result = false;
	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if( pObject )
	{
		if ( pObject->GetType() == E_UIGameButtonType_Lobby )
			result = ((UILobby*)pObject)->ShouldRefresh();
	}

	lua_pushboolean( pState, result );
	return 1;
}

static S32 TeamDataValid( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "TeamDataValid( id ) - incorrect arguments" );
		return 0;
	}

	bool result = false;
	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if( pObject )
	{
		if ( pObject->GetType() == E_UIGameButtonType_Lobby )
			result = ((UILobby*)pObject)->TeamDataValid();
	}

	lua_pushboolean( pState, result );
	return 1;
}

static S32 GetLobbySlots( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetLobbySlots( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tointeger( pState, 1 ) );
	if( !pObject )
		return 0;

	if ( pObject->GetType() == E_UIGameButtonType_Lobby )
	{
		((UILobby*)pObject)->SendToLua( pState );
		return 1;
	}

	return 0;
}

static S32 RefreshRacePreview( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "RefreshLobby( id ) - incorrect arguments" );
		return 0;
	}

	bool result = false;
	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if( pObject )
	{
		if ( pObject->GetType() == E_UIGameButtonType_RacePreview )
			result = ((UIRacePreview*)pObject)->ShouldRefresh();
	}

	lua_pushboolean( pState, result );
	return 1;
}

static S32 GetRacePreviewData( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetRacePreviewData( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tointeger( pState, 1 ) );
	if( !pObject )
		return 0;

	if ( pObject->GetType() == E_UIGameButtonType_RacePreview )
	{
		((UIRacePreview*)pObject)->SendToLua( pState );
		return 1;
	}

	return 0;
}

static S32 GetVehiclePosition( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetVehiclePosition( vehicleIndex ) - incorrect arguments" );
		return 0;
	}

	S32 sVehicleIndex = (S32)lua_tonumber( pState, 1 );
	U8 vehiclePosition = Game::C_MpResults::Instance().GetPlayerPosition(sVehicleIndex);

	lua_pushnumber( pState, vehiclePosition );

	return 1;
}

static S32 GetVehicleIndexByPosition( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetVehicleIndexByPosition( vehiclePosition ) - incorrect arguments" );
		return 0;
	}

	U8 vehiclePosition = (U8)lua_tonumber( pState, 1 );
	S32 vehicleIndex = Game::C_MpResults::Instance().GetVehicleIndexFromPosition(vehiclePosition);
	if(vehicleIndex == -1)
		vehicleIndex = 0;
		
	lua_pushnumber( pState, vehicleIndex );

	return 1;
}

static S32 GetRivalIndex( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetRivalIndex( viewportID ) - incorrect arguments" );
		return 0;
	}
	S32 sViewportIndex = (S32)lua_tonumber( pState, 1 ) - 1;
	Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( GetPlayerVehicleIndex( sViewportIndex ) );

	if(pLogicActor)
	{
		lua_pushnumber( pState, pLogicActor->pNewScoringComponent->GetRivalIndex() );
		return 1;
	}
	else
	{
		return 0;
	}
}

static S32 VehicleIDToJoinID( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, " VehicleIDToJoinID( vehicleId ) - incorrect arguments" );
		return 0;
	}

	S32 sVehicleIndex = (S32)lua_tonumber( pState, 1 ) - 1;
	Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor(  sVehicleIndex ); // check is valid
	if(pLogicActor)
	{
		Game::C_LevelState& levelState = GetLevelState();
		lua_pushnumber( pState, levelState.mCars[sVehicleIndex].mNetJoinId );
		return 1;
	}
	else
	{
		return 0;
	}
}

static S32 GetRivalsCountdownDelayActive( lua_State* pState )
{
	Game::C_RaceModeMultiplayer_Destruction* pRaceModeDestruction = (Game::C_RaceModeMultiplayer_Destruction*)(C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr());

	if ( !pRaceModeDestruction )
		return 0;

	bool active = pRaceModeDestruction->GetRivalsCountdownDelayActive();
	bool delay_active = ( pRaceModeDestruction->GetRivalsCommencementDelayTimer() > 0.0f );

	lua_pushboolean( pState, active );
	lua_pushboolean( pState, delay_active );

	return 2;
}

static S32 RaceFinished( lua_State* pState )
{
	// Returns true if all vehicles have finished and the race is over
	bool finished = false;

	Game::C_RaceModeBase* pRaceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();

	if ( pRaceMode )
		finished = pRaceMode->RaceFinished();

	lua_pushboolean( pState, finished );
	return 1;
}


static S32 PrimaryVehicleFinished( lua_State* pState )
{
	// Returns true if the player vehicle has finished
	bool finished = false;

	Game::C_RaceModeBase* pRaceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();

	if ( pRaceMode )
		finished = pRaceMode->PrimaryVehicleFinished();

	lua_pushboolean( pState, finished );
	return 1;
}


static S32 RaceTimeout( lua_State* pState )
{
	// Returns true if the player vehicle has finished
	bool timeout = false;

	Game::C_LevelState& levelState = GetLevelState();

	Game::C_RaceModeBase* pRaceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();

	if ( pRaceMode )
		timeout = ( pRaceMode->GetRaceTimeout() > 0.0f ) ? true : false;

	if ( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Elimination && levelState.GetMultiplayerSettings().mEliminationEvent == UI::E_EliminationEvent_Time )
		timeout = false;

	lua_pushboolean( pState, timeout );
	return 1;
}

static S32 FindPlayersTimer( lua_State* pState )
{
	S32 timer = (S32)g_FindPlayersTimer;
	if( -1 == timer )
		timer = C_MultiplayerData::Instance().m_pConfig->m_FindPlayersTimer;

	lua_pushnumber( pState, timer );
	return 1;
}

// SplitscreenResetScores
static S32 SplitscreenResetScores( lua_State* pState )
{
	//Reset the scores!
	Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	if(pGameModeBase)
	{
		if(pGameModeBase->GetID() == Game::E_GameMode_SplitScreen)
		{
			Game::C_GameModeSplitScreen* pGameMode = (Game::C_GameModeSplitScreen*)pGameModeBase;
			pGameMode->ResetScoring();
		}
	}

	return 0;
}

//GetSplitscreenScores
static S32 GetSplitscreenScores( lua_State* pState )
{
	bool bTotalScores = ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 1 ) != 0 ) : false;

	lua_newtable( pState );

	//Get the scores

	Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	if(pGameModeBase)
	{
		if(pGameModeBase->GetID() == Game::E_GameMode_SplitScreen)
		{
			Game::C_GameModeSplitScreen* pGameModeSplitScreen = (Game::C_GameModeSplitScreen*)pGameModeBase;

			const Game::SplitScreenScoring::Scores* scores;
			const Game::SplitScreenScoring::RaceResults* PositionResults;

			//See if we're looking for the total score or just the one from the last race
			if(bTotalScores)
			{
				scores = &pGameModeSplitScreen->GetTotalScores();
			}
			else
			{
				scores = &pGameModeSplitScreen->GetScoresForRace();
			}

			PositionResults = &pGameModeSplitScreen->GetResultsForRace();
			
			for(int i = 0; i < scores->Size(); i++)
			{
				lua_newtable( pState );
				Lua::AddDataMember( pState, "score", (*scores)[i] );
				Lua::AddDataMember( pState, "pos", (*PositionResults)[i] );
				lua_rawseti( pState, -2, i+1 );

			}
		}
	}
	return 1;
}

// GetSplitscreenPosition
static S32 GetSplitscreenPosition( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetSplitscreenPosition( player_index ) - incorrect arguments" );
		return 0;
	}

	S32 sViewportIndex = (S32)lua_tonumber( pState, 1 ) - 1;

	Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( GetPlayerVehicleIndex( sViewportIndex ) );

	if ( !pLogicActor )
		return 0;

	lua_pushnumber( pState, pLogicActor->GetPositionInfo().mRacePosition );

	return 1;
}

// Helper function 
bool GetIsCarRankUnlocked(const U32 carID, const int endRank )
{
	TLevelData levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;
	int levelingDataCount = levelData.count;

	// clamp - just in case.
	int end = levelingDataCount > endRank ? endRank:(levelingDataCount-1);

	for(int i=0; i<=end; i++)
	{
		for(int j=0; j < levelData[i].mVehicleUnlocks.count;j++)
		{
			AmaxVehicle* pVehicle = levelData[i].mVehicleUnlocks[j].Object();
			
			if(pVehicle->ID == carID)
			{
				// Then already unlocked
				return true;
			}
		}
	}

	// Not already unlocked
	return false; 
}


static S32 GetNonRankUnlockedDLCCars( lua_State* pState)
{
	// Get current level
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG( pGameProfile, "No game profile" );

	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
	BIZ_ASSERT_MSG( pProfile, "No multiplayer profile" );

	int endRank = pProfile->GetCurrentLevel();

	int levelingDataCount = C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count;

	// clamp - just in case.
	endRank = levelingDataCount > endRank ? endRank:(levelingDataCount-1);

	// Add all cars unlocked by DLC (unless instructed to ignore and car unlocked already)
	lua_newtable( pState );

	// For each cheat vehicle
	int index=0;
	for( int i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++ )
	{
		AmaxCheatBase* cheatBase = C_GameData::Instance().mCheatPacks[i];
		if(cheatBase->GetType() == E_CHEAT_TYPE_VEHICLE && cheatBase->dlcLicensed)
		{
			AmaxCheatVehicle* cheatVehicle = (AmaxCheatVehicle*)cheatBase;

			if(cheatVehicle->mAmaxVehicle.Object())
			{
				// Is this car already unlocked at this rank (avoid duplicates)
				bool isAlreadyRankUnlocked = GetIsCarRankUnlocked(cheatVehicle->mAmaxVehicle->ID, endRank);

				if(!isAlreadyRankUnlocked)
				{
					// Then Add
					lua_newtable( pState );
					Lua::AddDataMember( pState, "name_id",	(U32)cheatVehicle->mAmaxVehicle->loc_string.id );
					Lua::AddDataMember( pState, "id", cheatVehicle->mAmaxVehicle->ID );
					lua_rawseti( pState, -2, index+1 );
					index++; 
				}
			}
		}
	}

	return 1; 
}

static S32 ProfileProgression( lua_State* pState )
{
	lua_newtable( pState );

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	// To catch anything dumb that has happened
	BIZ_ASSERT_MSG_SPEED( pProfile->GetPreviousLevel() <= pProfile->GetCurrentLevel(), "Level Data has broke" );
	BIZ_ASSERT_MSG_SPEED( pProfile->NumPreviousFans() <= pProfile->NumFans(), "Level Data has broke" );

	// Work out max values and clamp other values so the UI doesn't banana out

	S32 maxRank = multiplayerConfig->m_LevelingData.count - 1;
	const S32 maxFans = multiplayerConfig->m_LevelingData[maxRank].mFansNeeded;

	const S32 fans = Maths::Clamp( pProfile->NumFans(), 0, maxFans );
	const S32 previousfans = Maths::Clamp( pProfile->NumPreviousFans(), 0, maxFans );
	
	Lua::AddDataMember( pState, "level",			pProfile->GetCurrentLevel() + 1 );
	Lua::AddDataMember( pState, "legend",			pProfile->GetLegendLevel() );
	Lua::AddDataMember( pState, "previous_level",	pProfile->GetPreviousLevel() + 1 );
	Lua::AddDataMember( pState, "level_ups",		pProfile->GetCurrentLevel() - pProfile->GetPreviousLevel() );
	Lua::AddDataMember( pState, "fans",				fans );
	Lua::AddDataMember( pState, "previous_fans",	previousfans );
	Lua::AddDataMember( pState, "maxed",			( maxFans == previousfans && pProfile->GetCurrentLevel() == maxRank ) ? true : false );

	return 1;
}

static S32 LevelProgressionData( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "LevelProgressionData( index ) - incorrect arguments" );
		return 0;
	}

	lua_newtable( pState );

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	// lua is going to give us 1-50 not 0-49
	const S32 rank = Maths::Clamp( (S32)lua_tonumber( pState, 1 ), 1, C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count ) - 1;
	S32 maxRank = C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count - 1;

	// Push number of fans needed for next level
	const S32 low_level = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank].mFansNeeded;
	const S32 high_level = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank + ( ( rank < maxRank ) ? 1 : 0 )].mFansNeeded;

	Lua::AddDataMember( pState, "fans_previous", low_level );
	Lua::AddDataMember( pState, "fans_needed", high_level );

	return 1;
}

static S32 NumberOfFans( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	lua_pushnumber( pState, pProfile->NumFans() );
	return 1;
}


static S32 ClearScoreboardGamerPictures( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ClearScoreboardGamerPictures( offset ) - incorrect arguments" );
		return 0;
	}

	const S32 offset = (S32)lua_tonumber( pState, 1 );

	I_GamerPictureManager* pGamerPictureManager = C_ProfileInterface::Instance().GamerPictureManager();
	for ( S32 i = offset; i < pGamerPictureManager->TotalSize(); i++ )
		pGamerPictureManager->ClearRemotePictureByIndex( i );

	return 0;
}

static S32 GetRaceMode( lua_State* pState )
{
	Game::C_RaceModeBase* pRaceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();

	lua_pushnumber( pState, pRaceMode->GetID() );

	return 1;
}

static S32 PostRankedUp( lua_State* pState )
{
	Game::C_RaceBookLayer* rbl=C_GameApp::GetInstance()->GetRaceBookLayer();
	if(rbl)
	{
		rbl->GetDataGathering().FlagPostEvent(Game::DATA_GATHERING_RECORD_TYPE_RANK_UP);
	}

	return 0;
}

// GetRaceSummary
static S32 GetRaceSummary( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	const Game::MpRaceSummary& raceSummary = pProfile->GetRaceSummary();

	lua_newtable( pState );

	Lua::AddDataMember( pState, "on_track_fans", raceSummary.m_OnTrackFans );
	Lua::AddDataMember( pState, "total_challenge_fans", raceSummary.m_TotalChallengeFans );
	Lua::AddDataMember( pState, "finish_bonus", raceSummary.m_FinishBonus );
	Lua::AddDataMember( pState, "car_reward_fan_bonus", raceSummary.m_CarRewardFanBonus );
	Lua::AddDataMember( pState, "total_new_fans", raceSummary.m_TotalNewFans );
	Lua::AddDataMember( pState, "showy_flourish_fans", raceSummary.m_ShowyFlourishContribution );
	Lua::AddDataMember( pState, "fan_favourite_fans", raceSummary.m_FanDemandContribution );
	Lua::AddDataMember( pState, "position", raceSummary.m_Position );
	Lua::AddDataMember( pState, "rank_up", raceSummary.m_RankUp );
	Lua::AddDataMember( pState, "rank", raceSummary.m_Rank );
	Lua::AddDataMember( pState, "team_bonus", raceSummary.m_TeamBonus );

	return 1;
}

static S32 MultiplayerVotePassed( lua_State* pState )
{
/*
	PostSkippedRoute();
#ifndef CFG_GOLD
	RacerNet::NetRaceSettings raceSettings;
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

	Game::C_GameModeBase* pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	UI::C_WideString32 wCityStr;
	UI::C_WideString32 wRouteStr;
	int numLaps = 0;

	if(pGameModeBase->IsRanked())
	{
		const MultiplayerEvent* pEv = C_MultiplayerCache::Instance().CurrentEvent();
		if(pEv)
		{
			Game::C_GameModeNetworkRanked* pGameModeRanked = (Game::C_GameModeNetworkRanked*)pGameModeBase;

			S32 raceIdx = pGameModeRanked->GetIndexShuffler().GetRaceIndex(raceSettings.m_CurrentRace);
			const MultiplayerRace* pRace = pEv->races[raceIdx];

			if(pRace)
			{
				wCityStr.Format(L"%s", UI::C_TextStringID(pRace->route->city->loc_string.id).String());
				wRouteStr.Format(L"%s", UI::C_TextStringID(pRace->route->loc_string.id).String());
				numLaps = pRace->laps;
			}
		}
	}
	//else
	//{
	//	RacerNet::RaceSettings settings = raceSettings.m_Races[raceSettings.m_CurrentRace];

	//	AmaxCity* city		= C_GameData::Instance().mCities[(GAMEDATA_ID)settings.m_xtCityId];
	//	AmaxRoute* route	= C_GameData::Instance().mRoutes[(GAMEDATA_ID)settings.m_xtRouteId];

	//	if(city)
	//		wCityStr.Format(L"%s", UI::C_TextStringID(city->loc_string.id).String());		

	//	if(route)
	//		wRouteStr.Format(L"%s", UI::C_TextStringID(route->loc_string.id).String());

	//	numLaps = settings.m_NumLaps;
	//}

	Util::C_String cityName		= wCityStr.AsString();
	Util::C_String routeName	= wRouteStr.AsString();
	AmaxDataLogging::LogEventSkipped(cityName.Chars(), routeName.Chars(), numLaps);

#endif // CFG_GOLD
*/
	return 0;
}

static S32 GetChallengePacks( lua_State* pState )
{
	if ( !C_MultiplayerData::Instance().m_pConfig && !C_MultiplayerData::Instance().m_pConfig->m_LevelingData && !C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks )
	{
		LOGERROR( g_Log, "Mulitplayer config, levelingData or challenge pack are missing" );
		return 0;
	}

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_MultiplayerChallengeSystem& challengeSystem = pProfile->GetChallengeSystem();
	
	// cache challenge progress
	challengeSystem.CachePackProgress();

	lua_newtable( pState );

	TLevelData levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;

	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++ )
	{
		XtHandle<MultiplayerChallengePack>& challengePack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];
		bool found = false;
		S32 level = 0;

		// Work out what level you need to be to unlock this pack and if its availible to us. What a crap system
		for ( level = 0; level < levelData.count; level++ )
		{
			// does this level unlock a challenge
			if ( !C_MultiplayerData::Instance().m_pConfig->m_LevelingData[level].mChallengeUnlocks.count )
				continue;

			// check the challenges
			for ( S32 pack = 0; pack < levelData[level].mChallengeUnlocks.count; pack++ )
			{
				if ( !levelData[level].mChallengeUnlocks[pack].Object() || !challengePack.Object() )
					continue;

				if ( levelData[level].mChallengeUnlocks[pack].Id() == challengePack.Id() )
				{
					found = true;
					break;
				}
			}

			// early return if found (prevent post increment of 'level' var, which 
			// meant all challenges were reported unlocked 1 level later than should be)
			if(found)
			{
				break; 
			}
		}

		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];

		const S32 packCount = ( pPack->mType == E_ChallengePack_Car ) ? challengeSystem.GetCarPackTotal() : pPack->mChallenges.count;

		lua_newtable( pState );
			Lua::AddDataMember( pState, "name",			pPack->mName.id );
			Lua::AddDataMember( pState, "desc",			pPack->mDesc.id );
			Lua::AddDataMember( pState, "icon_name",	pPack->mIconName );
			Lua::AddDataMember( pState, "category",		pPack->mType );
			Lua::AddDataMember( pState, "locked",		( level > pProfile->GetCurrentLevel() ) );
			Lua::AddDataMember( pState, "level",		level );
			Lua::AddDataMember( pState, "progress",		((F32)challengeSystem.GetPackProgressCounter( pPack->mType ) / packCount ) );
		lua_rawseti( pState, -2, i+1 );
	}

#ifndef CFG_GOLD
	//Data gathering
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	pLayer->GetDataGathering().PostChallengeSorceryInformation();
#endif //CFG_GOLD

	return 1;
}


static S32 GetChallenges( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetChallenges( pack ) - incorrect arguments" );
		return 0;
	}

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_MultiplayerChallengeSystem& challengeSystem = pProfile->GetChallengeSystem();

	// Imform the data finder which pack we are viewing
	challengeSystem.SetViewingPack( (E_ChallengePack)(S32)lua_tonumber( pState, 1 ) );

	// Get the requested pack
	const MultiplayerChallengePack* pPack = challengeSystem.GetViewingPack();
	if ( !pPack || pPack->mChallenges.count == 0 )
	{
		BIZ_ASSERT_MSG( pPack->mChallenges.count, "Challenge pack is empty" );
		return 0;
	}

	lua_newtable( pState );

	// Send the progress of each challenge
	for ( S32 i = 0; i < pPack->mChallenges.count; i++ )
	{
		const Game::C_MultiplayerChallenge& challenge = challengeSystem.GetChallenge( pPack->mChallenges[i]->mType );

		lua_newtable( pState );
			challenge.SendToLua( pState );
		lua_rawseti( pState, -2, i+1 );
	}

	return 1;
}

static S32 GetCarChallengeClasses( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
	Game::C_MultiplayerChallengeSystem& challengeSystem = pProfile->GetChallengeSystem();

	lua_newtable( pState );

	for ( S32 i = 0; i < E_VehicleClass_Max; i++ )
	{
		S32 amount = 0, max = 0;
		challengeSystem.GetCarClassProgress( (E_VehicleClass)i, amount, max );

		lua_pushnumber( pState, (F32)( amount / max ) );
		lua_rawseti( pState, -2, i+1 );
	}

	return 1;
}


static S32 GetCarChallenges( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetCarChallenges( class ) - incorrect arguments" );
		return 0;
	}

	if ( lua_type( pState, 2 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "GetCarChallenges( class ) - incorrect arguments" );
		return 0;
	}

	const AmaxCarClass carClass = (AmaxCarClass)((S32)lua_tonumber( pState, 1 ) - 1);
	const bool fetchLegendAndDLC = lua_toboolean( pState, 2 ) ? true : false;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
	Game::C_MultiplayerChallengeSystem& challengeSystem = pProfile->GetChallengeSystem();

	TLevelData& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;
	TLevelData& legendLevelData = C_MultiplayerData::Instance().m_pConfig->m_LegendLevelingData;

	lua_newtable( pState );

	// Cycle leveling data
	S32 index = 1;
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_MultiplayerVehicles.Size(); ++i )
	{
		const AmaxVehicle* pVehicle = C_MultiplayerData::Instance().m_MultiplayerVehicles[i];
		if ( !pVehicle )
			continue;

		if ( pVehicle->carClass == carClass )
		{
			if(!pVehicle->m_ParentVehicle || fetchLegendAndDLC) //Boss cars to be treated the same as DLC/Legend (ie. Not in challenge screen)
			{
				// Get the challenge data
				const Game::C_MultiplayerChallenge& repeatChallenge = challengeSystem.GetCarChallenge( pVehicle->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES );
				const Game::C_MultiplayerChallenge& winnerChallenge = challengeSystem.GetCarChallenge( pVehicle->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER );

				bool unlocked = repeatChallenge.IsUnlocked( pProfile->GetCurrentLevel() );

				// Secret code DLC cars
				if ( !unlocked )
				{
					for( Btl::Set<U32>::Iterator it = pGameProfile->mTempVehicleUnlocks.Begin(); it != pGameProfile->mTempVehicleUnlocks.End(); ++it )
					{
						XtHandle<AmaxVehicle> vehHandle = XtHandle<AmaxVehicle>(XtGetRecord(C_GameApp::GetInstance()->GetGlobalXtDatabase(), *it));

						if( vehHandle.Object() && vehHandle->ID == pVehicle->ID )
							unlocked = true;
					}
				}

				lua_newtable( pState );
					Lua::AddDataMember( pState, "id",				pVehicle->ID );
					Lua::AddDataMember( pState, "can_respray",		pVehicle->m_bUICanReSpray );
					Lua::AddDataMember( pState, "name",				pVehicle->loc_string.id );
					Lua::AddDataMember( pState, "texture",			pVehicle->manufacturer->mUiTexture.Text() );
					Lua::AddDataMember( pState, "texture_square",	pVehicle->manufacturer->mUiSquareTexture );
					Lua::AddDataMember( pState, "icon",				pVehicle->m_UiIconName.Text() );
					Lua::AddDataMember( pState, "sheet",			UI::GetVehicleSheet( pVehicle->carClass ) );
					Lua::AddDataMember( pState, "unlocked",			unlocked );
					Lua::AddDataMember( pState, "rank_needed",		repeatChallenge.GetUnlockLevel() + 1 );
					Lua::AddDataMember( pState, "progress",			repeatChallenge.GetProgress() );
					Lua::AddDataMember( pState, "complete",			repeatChallenge.IsCompleted() );

					lua_pushstring( pState, "repeat_offender" );
					lua_newtable( pState );
						repeatChallenge.SendToLua( pState );
					lua_rawset( pState, -3 );

					lua_pushstring( pState, "winner" );
					lua_newtable( pState );
						winnerChallenge.SendToLua( pState );
					lua_rawset( pState, -3 );

				lua_rawseti( pState, -2, index++ );
			}
		}
	}

	if(fetchLegendAndDLC)
	{
		// Legend
		for ( S32 level = 0; level < legendLevelData.count; level++ )
		{
			for ( S32 i = 0; i < legendLevelData[level].mVehicleUnlocks.count; i++ )
			{
				if ( legendLevelData[level].mVehicleUnlocks[i]->carClass == carClass )
				{
					const AmaxVehicle* pVehicle = legendLevelData[level].mVehicleUnlocks[i];

					// Get the challenge data
					const Game::C_MultiplayerChallenge& repeatChallenge = challengeSystem.GetCarChallenge( pVehicle->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES );
					const Game::C_MultiplayerChallenge& winnerChallenge = challengeSystem.GetCarChallenge( pVehicle->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER );

					lua_newtable( pState );
						Lua::AddDataMember( pState, "id",				pVehicle->ID );
						Lua::AddDataMember( pState, "name",				pVehicle->loc_string.id );
						Lua::AddDataMember( pState, "texture",			pVehicle->manufacturer->mUiTexture.Text() );
						Lua::AddDataMember( pState, "texture_square",	pVehicle->manufacturer->mUiSquareTexture );
						Lua::AddDataMember( pState, "icon",				pVehicle->m_UiIconName.Text() );
						Lua::AddDataMember( pState, "unlocked",			repeatChallenge.IsUnlocked( pProfile->GetCurrentLevel() ) );
						Lua::AddDataMember( pState, "rank_needed",		repeatChallenge.GetUnlockLevel() + 1 );

						lua_pushstring( pState, "repeat_offender" );
						lua_newtable( pState );
							repeatChallenge.SendToLua( pState );
						lua_rawset( pState, -3 );

						lua_pushstring( pState, "winner" );
						lua_newtable( pState );
							winnerChallenge.SendToLua( pState );
						lua_rawset( pState, -3 );
					lua_rawseti( pState, -2, index++ );
				}
			}
		}

		// DLC
		for( Btl::Set<U32>::Iterator it = pGameProfile->mVehicleUnlocks.Begin(); it != pGameProfile->mVehicleUnlocks.End(); it++ )
		{
			XtHandle<AmaxVehicle> vehHandle = XtHandle<AmaxVehicle>(XtGetRecord(C_GameApp::GetInstance()->GetGlobalXtDatabase(), *it));

			AmaxVehicle* pVehicleAM = vehHandle.Object();
			if( pVehicleAM && pVehicleAM->carClass == carClass)
			{
				// Get the challenge data
				const Game::C_MultiplayerChallenge& repeatChallenge = challengeSystem.GetCarChallenge( pVehicleAM->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES );
				const Game::C_MultiplayerChallenge& winnerChallenge = challengeSystem.GetCarChallenge( pVehicleAM->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER );

				lua_newtable( pState );
				Lua::AddDataMember( pState, "id",				pVehicleAM->ID );
				Lua::AddDataMember( pState, "name",				pVehicleAM->loc_string.id );
				Lua::AddDataMember( pState, "texture",			pVehicleAM->manufacturer->mUiTexture.Text() );
				Lua::AddDataMember( pState, "texture_square",	pVehicleAM->manufacturer->mUiSquareTexture );
				Lua::AddDataMember( pState, "icon",				pVehicleAM->m_UiIconName.Text() );
				Lua::AddDataMember( pState, "unlocked",			repeatChallenge.IsUnlocked( pProfile->GetCurrentLevel() ) );
				Lua::AddDataMember( pState, "rank_needed",		repeatChallenge.GetUnlockLevel() + 1 );

				lua_pushstring( pState, "repeat_offender" );
				lua_newtable( pState );
				repeatChallenge.SendToLua( pState );
				lua_rawset( pState, -3 );

				lua_pushstring( pState, "winner" );
				lua_newtable( pState );
				winnerChallenge.SendToLua( pState );
				lua_rawset( pState, -3 );
				lua_rawseti( pState, -2, index++ );
			}
		}
	}

	return 1;
}

static S32 GetCarChallengeRewards( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
	Game::C_MultiplayerChallengeSystem& challengeSystem = pProfile->GetChallengeSystem();

	lua_newtable( pState );

	// Get the challenge data
	const Game::C_MultiplayerChallenge& repeatChallenge = challengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES );
	const Game::C_MultiplayerChallenge& winnerChallenge = challengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER );

	lua_pushstring( pState, "repeat_offender" );
	lua_newtable( pState );
		for ( S32 i = 0; i < repeatChallenge.GetMaxTiers(); i++ )
		{
			lua_pushnumber( pState, repeatChallenge.GetReward( i ) );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "winner" );
	lua_newtable( pState );
		for ( S32 i = 0; i < winnerChallenge.GetMaxTiers(); i++ )
		{
			lua_pushnumber( pState, winnerChallenge.GetReward( i ) );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	return 1;
}

static S32 GetCurrentCarClass( lua_State* pState )
{
	Game::C_GameModeBase* pBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	if ( !pBase->IsMultiplayer() )
		return 0;

	lua_pushnumber( pState, ((Game::C_GameModeMultiPlayer*)pBase)->GetRaceCarClass() );
	return 1;
}

static S32 GetSplitscreenVehicles( lua_State* pState )
{
	Game::C_GameModeBase* pBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	if ( pBase->GetID() != Game::E_GameMode_SplitScreen )
	{
		BIZ_ASSERT_FAIL( "Game mode is not splitscreen!!" );
		return 0;
	}

	Game::C_GameModeSplitScreen* pGameMode = (Game::C_GameModeSplitScreen*)pBase;
	AmaxCarClass validClass = pGameMode->GetRaceCarClass();

	lua_newtable( pState );

	// Push a filtered list of vehicles based on the current class
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_SplitScreenVehicles.count; i++ )
	{
		const AmaxVehicle* pVehicle = C_MultiplayerData::Instance().m_pConfig->m_SplitScreenVehicles[i];

		if ( pVehicle && pGameMode->IsCarClassValid(validClass, pVehicle->carClass ) )
		{
			lua_pushboolean( pState, true );
			lua_rawseti( pState, -2, pVehicle->ID );
		}
	}

	return 1;
}

// GetSplitscreenResprays
static S32 GetSplitscreenResprays( lua_State* pState )
{
	lua_newtable( pState );

	C_UiXtConfig* pUiConfig = C_UiXtConfig::Instance();

	if ( !pUiConfig )
		return 1;

	S32 sCount = 0;

	for ( S32 i = 0; i < pUiConfig->m_GarageResprays.m_Splitscreen.count; i++ )
	{
		XtHandle< Vehicle::XtPaintJob > respray = pUiConfig->m_GarageResprays.m_Splitscreen[ i ];

		if ( !respray )
			continue;

		C_UIColour colour;
		colour.Set01( 1.0f, respray->m_Colours[ 0 ].r, respray->m_Colours[ 0 ].g, respray->m_Colours[ 0 ].b );

		lua_newtable( pState );
			Lua::AddDataMember( pState, "id", respray.Id() );
			Lua::AddDataMember( pState, "colour", colour.LuaString() );
		lua_rawseti( pState, -2, ++sCount );
	}

	return 1;
}

// GetGarage
static S32 GetGarage( lua_State* pState )
{
	if ( !C_MultiplayerData::Instance().m_pConfig || !C_MultiplayerData::Instance().m_pConfig->m_LevelingData )
		return 0;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ( !pGameProfile || !pGameProfile->GetMultiplayerProfile() )
		return 0;

	SP_SaveGame* pSave = SP_GetCurrentSaveGame();

	bool bInLobby = ( ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 1 ) != 0 ) );

	TLevelData& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;
	TLevelData& legendLevelData = C_MultiplayerData::Instance().m_pConfig->m_LegendLevelingData;

	S32 sCurrentLevel = pGameProfile->GetMultiplayerProfile()->GetCurrentLevel();
	S32 sCurrentLegendLevel = pGameProfile->GetMultiplayerProfile()->GetLegendLevel();

#ifndef CFG_GOLD
	if ( g_DebugGarageMpLevelOverride != -1 )
		sCurrentLevel = g_DebugGarageMpLevelOverride;
#endif

	lua_newtable( pState );

#ifndef CFG_GOLD
	Lua::AddDataMember( pState, "own_it_override", g_DebugCheatShowAllGarage );
#endif

	lua_pushstring( pState, "cars" );
	lua_newtable( pState );

	// DLC cars are in here too, as are legend cars
	for ( Btl::Map<U32, S32>::Iterator it = C_MultiplayerData::Instance().m_MultiplayerVehicleMap.Begin(); it != C_MultiplayerData::Instance().m_MultiplayerVehicleMap.End(); ++it )
	{
		AmaxVehicle* pVehicleAM = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)it->GetKey() ];

		if ( !pVehicleAM )
			continue;

		lua_newtable( pState );
		lua_rawseti( pState, -2, pVehicleAM->ID );
	}
		
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "rank" );
	lua_newtable( pState );

	for ( S32 i = 0; i < levelData.count; i++ )
	{
		for ( S32 j = 0; j < levelData[ i ].mVehicleUnlocks.count; j++ )
		{
			AmaxVehicle* pVehicleAM = levelData[ i ].mVehicleUnlocks[ j ].Object();

			if ( !pVehicleAM )
				continue;

			//Boss vehicles need to be unlocked in Single Player as well (And don't show "Unlock at rank X" unless we already have the SP version unlocked)
			if(!pVehicleAM->m_ParentVehicle || (pSave &&  SP_IsVehicleUnlocked(pSave, pVehicleAM->m_ParentVehicle->ID)))
			{
				lua_pushnumber( pState, i + 1 );
				lua_rawseti( pState, -2, pVehicleAM->ID );
			}
		}
	}

	lua_rawset( pState, -3 );

	lua_pushstring( pState, "garage" );
	lua_newtable( pState );

	//	Check whether this event is online - if not then we're in a LAN game and so have access to all the vehicles
	bool online = true;

	Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	if ( pGameModeBase )
		online = pGameModeBase->IsOnline();

	if ( !bInLobby )
		online = true;

	// cars unlocked by level
	for ( S32 i = 0; i < levelData.count; i++ )
	{
		if ( online && sCurrentLevel < i )
			break;

		for ( S32 j = 0; j < levelData[ i ].mVehicleUnlocks.count; j++ )
		{
			AmaxVehicle* pVehicleAM = levelData[ i ].mVehicleUnlocks[ j ].Object();

			if ( !pVehicleAM )
				continue;

			//Boss vehicles need to be unlocked in Single Player as well
			if(!pVehicleAM->m_ParentVehicle || (pSave &&  SP_IsVehicleUnlocked(pSave, pVehicleAM->m_ParentVehicle->ID)))
			{
				lua_pushboolean( pState, true );
				lua_rawseti( pState, -2, pVehicleAM->ID );
			}
		}
	}

	// Legend cars
	for ( S32 i = 0; i < legendLevelData.count; i++ )
	{
		if ( sCurrentLegendLevel < i )
			break;

		for ( S32 j = 0; j < legendLevelData[ i ].mVehicleUnlocks.count; j++ )
		{
			AmaxVehicle* pVehicleAM = legendLevelData[ i ].mVehicleUnlocks[ j ].Object();

			if ( !pVehicleAM )
				continue;

			lua_pushboolean( pState, true );
			lua_rawseti( pState, -2, pVehicleAM->ID );
		}
	}

	// Secret code DLC cars
	for( Btl::Set<U32>::Iterator it = pGameProfile->mVehicleUnlocks.Begin(); it != pGameProfile->mVehicleUnlocks.End(); it++ )
	{
		XtHandle<AmaxVehicle> vehHandle = XtHandle<AmaxVehicle>(XtGetRecord(C_GameApp::GetInstance()->GetGlobalXtDatabase(), *it));

		AmaxVehicle* pVehicleAM = vehHandle.Object();
		if( pVehicleAM )
		{
			lua_pushboolean( pState, true );
			lua_rawseti( pState, -2, pVehicleAM->ID );
		}
	}

	// Downloaded DLC cars
	for( Btl::Set<U32>::Iterator it = pGameProfile->mTempVehicleUnlocks.Begin(); it != pGameProfile->mTempVehicleUnlocks.End(); it++ )
	{
		lua_pushboolean( pState, true );
		lua_rawseti( pState, -2, *it );
	}
	
	lua_rawset( pState, -3 );


	//	teams restrictions
	lua_pushstring( pState, "team_restrictions" );
	lua_newtable( pState );

	for(int i = 0; i < C_MultiplayerData::Instance().m_InvalidTeamVehicleIds.Size(); i++)
	{
		lua_pushboolean( pState, true );
		lua_rawseti( pState, -2, C_MultiplayerData::Instance().m_InvalidTeamVehicleIds[i]);
	}

	lua_rawset( pState, -3 );

#ifndef CFG_GOLD
	//Data gathering
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	pLayer->GetDataGathering().PostVehicleSorceryInformation();
#endif //CFG_GOLD

	return 1;
}

static S32 ShowAllGarage( lua_State* pState )
{
#ifndef CFG_GOLD
	lua_pushboolean( pState, g_DebugCheatShowAllGarage );
#else
	lua_pushboolean( pState, false );
#endif
	return 1;
}

static S32 GetCurrentLoadOutType( lua_State* pState )
{
	bool bInLobby = ( ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 1 ) != 0 ) );

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	bool isRanked = !bInLobby || (!pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked());

	//Need a profile
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG( pGameProfile, "lua_Multiplayer::SetCurrentLoadOutType( newLoadOutType ) > No game profile" );

	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
	BIZ_ASSERT_MSG( pMultiplayerProfile, "lua_Multiplayer::SetCurrentLoadOutType( newLoadOutType ) > No multiplayer profile" );

	lua_pushnumber( pState, (S32)pMultiplayerProfile->GetLoadouts(isRanked).GetCurrentLoadOutType() );

	return 1;
}


static S32 SetCurrentLoadOutType( lua_State* pState )
{
	//Pass the enum type for the loadout 
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "lua_Multiplayer::SetCurrentLoadOutType( newLoadOutType ) > Incorrect argument" );
		return 0;
	}

	bool bInLobby = ( ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 2 ) != 0 ) );

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	bool isRanked = !bInLobby || (!pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked());

	//Need a profile
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG( pGameProfile, "lua_Multiplayer::SetCurrentLoadOutType( newLoadOutType ) > No game profile" );

	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
	BIZ_ASSERT_MSG( pMultiplayerProfile, "lua_Multiplayer::SetCurrentLoadOutType( newLoadOutType ) > No multiplayer profile" );

	//Store the multiplayer level (rank) the player's at currently
	const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType = (UI::E_MULTIPLAYER_LOADOUT_TYPE)(S32)lua_tonumber( pState, 1 );

	pMultiplayerProfile->GetLoadouts(isRanked).SetCurrentLoadOutType(eLoadOutType);

	return 0;
}


static S32 GetAbilityPacks( lua_State* pState )
{
	lua_newtable( pState );

	// Pass all pack names
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_AbilitySlots.count; i++ )
	{
		// Everything is name differently :-s sighhhhh
		const MultiplayerAbilitySlot* pPack = C_MultiplayerData::Instance().m_pConfig->m_AbilitySlots[i];

		lua_newtable( pState );
			Lua::AddDataMember( pState, "name", (U32)pPack->mName.id );
			Lua::AddDataMember( pState, "type", pPack->mType );
		lua_rawseti( pState, -2, i+1 );
	}
	return 1;
}

static S32 GetAbility( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetAbility( id, [bool] ) - incorrect arguements" );
		return 0;
	}

	bool bByEnum = ( lua_isboolean( pState, 2 ) && ( lua_toboolean( pState, 2 ) != 0 ) );

	const MultiplayerAbilityData* pAbilityData = NULL;

	if ( bByEnum )
	{
		E_MultiplayerVehicleAbilities eAbility = (E_MultiplayerVehicleAbilities)(S32)lua_tonumber( pState, 1 );

		if ( Game::C_Game::GetInstance() )
		{
			const Game::C_MultiplayerAbility* pAbility = Game::C_Game::GetInstance()->GetMultiplayerAbilities().GetAbility( eAbility );

			if ( pAbility )
				pAbilityData = pAbility->GetData();
		}
	}
	else
	{
		XtId id = (XtId)lua_tointeger( pState, 1 );
		pAbilityData = C_MultiplayerData::Instance().FindAbility( id );
	}

	if ( !pAbilityData )
		return 0;

	lua_newtable( pState );

	Lua::AddDataMember( pState, "name",			(U32)pAbilityData->mAbilityName.id );
	Lua::AddDataMember( pState, "desc",			(U32)pAbilityData->mAbilityDescription.id );
	Lua::AddDataMember( pState, "icon_name",	pAbilityData->mIconName );
	Lua::AddDataMember( pState, "type",			pAbilityData->mAbilityId );
	Lua::AddDataMember( pState, "pack",			pAbilityData->mAbilityCategory );

	return 1;
}

static S32 GetAbilities( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "lua_Multiplayer::GetAbilityInSlotInLoadOut( LOADOUTTYPE, slotType ) > Incorrect 1st argument" );
	}

	const UI::E_MultiplayerAbilityCategory packType = (UI::E_MultiplayerAbilityCategory)(S32)lua_tonumber( pState, 1 );

	lua_newtable( pState );

	// just incase we get a dodgy value
	if ( packType < C_MultiplayerData::Instance().m_pConfig->m_AbilitySlots.count )
	{
		const MultiplayerAbilitySlot* pPack = C_MultiplayerData::Instance().m_pConfig->m_AbilitySlots[packType];

		for ( S32 i = 0; i < pPack->mAbilities.count; i++ )
		{
			const MultiplayerAbilityData* pAbility = pPack->mAbilities[i];

			lua_newtable( pState );
				Lua::AddDataMember( pState, "name",			(U32)pAbility->mAbilityName.id );
				Lua::AddDataMember( pState, "desc",			(U32)pAbility->mAbilityDescription.id );
				Lua::AddDataMember( pState, "icon_name",	pAbility->mIconName );
				Lua::AddDataMember( pState, "type",			pAbility->mAbilityId );
				Lua::AddDataMember( pState, "pack",			pAbility->mAbilityCategory );
			lua_rawseti( pState, -2, i+1 );
		}
	}

#ifndef CFG_GOLD
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	if(pLayer)
	{
		pLayer->GetDataGathering().PostAbilitySorceryInformation();
	}
#endif //CFG_GOLD

	return 1;
}


static S32 GetAbilityUnlockMap( lua_State* pState )
{
	//Need a profile
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG( pGameProfile, "GetAbilityUnlockMap() > No game profile" );

	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
	BIZ_ASSERT_MSG( pMultiplayerProfile, "GetAbilityUnlockMap() > No multiplayer profile" );

	lua_newtable( pState );

	TLevelData& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;

	// Why so many levels of iteration? Rubbish data set up
	for ( S32 level = 0; level < levelData.count; level++ )
	{
		for ( S32 num = 0; num < levelData[level].mAbilityUnlocks.count; num++ )
		{
			MultiplayerAbilityData* pAbility = levelData[level].mAbilityUnlocks[num];

				lua_newtable( pState );
					Lua::AddDataMember( pState, "level", level ); 
					Lua::AddDataMember( pState, "unlocked", ( pMultiplayerProfile->GetCurrentLevel() >= level ) ? true : false );
				lua_rawseti( pState, -2, pAbility->mAbilityId);
		}
	}
	return 1;
}

static S32 GetAbilityLoadouts( lua_State* pState )
{
	bool bInLobby = ( ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 1 ) != 0 ) );

	//Need a profile
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG( pGameProfile, "lua_Multiplayer::SetCurrentLoadOutType( newLoadOutType ) > No game profile" );

	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
	BIZ_ASSERT_MSG( pMultiplayerProfile, "lua_Multiplayer::SetCurrentLoadOutType( newLoadOutType ) > No multiplayer profile" );

	//I'm cheerfully expecting the number of slots to change so hopefully this will handle that change gracefully
	lua_newtable( pState );

	const Game::C_MultiplayerAbilities& abilities = Game::C_Game::GetInstance()->GetMultiplayerAbilities();
	Game::C_GameModeBase* pGameMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	bool modsAllowed = pGameMode->IsNetwork() && ((Game::C_GameModeNetwork*)pGameMode)->GetModsAllowed();
	bool isRanked = !bInLobby || pGameMode->IsRanked(); //If we are not in a lobby we are accessing from the main multiplayer menu and so wan't to view ranked mods

	for ( S32 i = 0; i < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX; i++ )
	{
		const Game::MultiplayerLoadOut& loadOut = pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut( (UI::E_MULTIPLAYER_LOADOUT_TYPE)i);

		lua_newtable( pState );
		Lua::AddDataMember( pState, "index", i + 1 );

		lua_pushstring( pState, "slots" );
		lua_newtable( pState );

		// Push each slot
		bool can_use = true;
		for( S32 j = 0; j < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; j++)
		{
			lua_newtable( pState );

			bool empty = true;
			UI::E_MultiplayerVehicleAbilities type = E_MultiplayerVehicleAbilities_None;
			type = loadOut.GetAbilityInSlot( (UI::E_MULTIPLAYER_LOADOUT_SLOT)j);
			if(type == E_MultiplayerVehicleAbilities_None)
			{
				type = abilities.GetSorceryDefaultAbility((UI::E_MULTIPLAYER_LOADOUT_TYPE)i, (UI::E_MULTIPLAYER_LOADOUT_SLOT)j, isRanked);
			}

			// if its none then its not been set
			if ( type != E_MultiplayerVehicleAbilities_None )
			{
				const MultiplayerAbilityData* pAbility = C_MultiplayerData::Instance().FindAbility( type );

				if ( pAbility )
				{
					empty = false;
					Lua::AddDataMember( pState, "name",			(U32)pAbility->mAbilityName.id );
					Lua::AddDataMember( pState, "desc",			(U32)pAbility->mAbilityDescription.id );
					Lua::AddDataMember( pState, "icon_name",	pAbility->mIconName );
					Lua::AddDataMember( pState, "type",			pAbility->mAbilityId );
					Lua::AddDataMember( pState, "pack",			pAbility->mAbilityCategory );
				}
			}
			else
			{
				can_use = false;
			}

			Lua::AddDataMember( pState, "empty", empty );
			lua_rawseti( pState, -2, j+1 );
		}

		lua_rawset( pState, -3 );

		// Can we select this
		Lua::AddDataMember( pState, "can_use", can_use );
		//Lua::AddDataMember( pState, "default", i < ( E_MULTIPLAYER_LOADOUT_TYPE_CUSTOM1 ) ? true : false );
		lua_rawseti( pState, -2, i+1 );
	}
	return 1;
}


static S32 GetCurrentLoadout( lua_State* pState )
{
	lua_newtable( pState );

	const Game::C_MultiplayerAbilities& abilities = Game::C_Game::GetInstance()->GetMultiplayerAbilities();
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

	bool modsAllowed = pGameMode && pGameMode->GetModsAllowed();
	bool isRanked = pGameMode->IsRanked();

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	const UI::E_MULTIPLAYER_LOADOUT_TYPE type = pMultiplayerProfile->GetLoadouts(isRanked).GetCurrentLoadOutType();
	const Game::MultiplayerLoadOut& loadOut = pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut( type );

	lua_newtable( pState );
	Lua::AddDataMember( pState, "type", type );

	lua_pushstring( pState, "slots" );
	lua_newtable( pState );

	// Push the abilities in that loadout
	for ( S32 i = 0; i < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; i++ )
	{
		lua_newtable( pState );

		UI::E_MultiplayerVehicleAbilities ability = E_MultiplayerVehicleAbilities_None;
		if(modsAllowed)
		{
			ability = loadOut.GetAbilityInSlot( (UI::E_MULTIPLAYER_LOADOUT_SLOT)i);
			if(ability == E_MultiplayerVehicleAbilities_None)
			{
				ability = abilities.GetSorceryDefaultAbility(type, (UI::E_MULTIPLAYER_LOADOUT_SLOT)i, isRanked);
			}
		}
		
		bool empty = true;

		// if its none then its not been set
		if ( ability != E_MultiplayerVehicleAbilities_None )
		{
			const MultiplayerAbilityData* pAbility = C_MultiplayerData::Instance().FindAbility( ability );

			if ( pAbility )
			{
				empty = false;
				Lua::AddDataMember( pState, "icon_name",	pAbility->mIconName );
				Lua::AddDataMember( pState, "pack",			pAbility->mAbilityCategory );
				Lua::AddDataMember( pState, "type",			pAbility->mAbilityId );
			}
		}

		Lua::AddDataMember( pState, "empty", empty );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	return 1;
}

static S32 SetAbilityLoadout( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TTABLE )
	{
		g_pUILua->StackDump( pState, "SetAbilityLoadout( loadout(num),  ) > Incorrect 1st argument" );
	}

	bool bInLobby = ( ( lua_type( pState, 3 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 3 ) != 0 ) );

	//Need a profile
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	bool isRanked = !bInLobby || (!pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked());

	const UI::E_MULTIPLAYER_LOADOUT_TYPE loadoutIndex = (UI::E_MULTIPLAYER_LOADOUT_TYPE)(S32)lua_tonumber( pState, 1 );

	lua_settop(pState,2);

	const S32 size = luaL_getn(pState,-1);
	for ( S32 i = 0; i < size; i++ )
	{
		lua_rawgeti( pState,-1, i+1 ); // Push on index [i]

		S32 type = UI::E_MultiplayerVehicleAbilities_None;
		bool empty = false;

		// check to see if we have an empty mod
		Lua::GetDataMember( pState, "empty", empty );

		// pull out of the lua table
		if ( !empty )
			Lua::GetDataMember( pState, "type", type );

		// Change the ability
		pMultiplayerProfile->GetLoadouts(isRanked).SetAbilityInLoadOut( loadoutIndex, (UI::E_MULTIPLAYER_LOADOUT_SLOT)i, (UI::E_MultiplayerVehicleAbilities)type );

		lua_pop( pState, 1 );
	}
	return 0;
}


static S32 SetNameOfLoadOut( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetNameOfLoadOut( loadout(num),  ) > Incorrect 1st argument" );
	}

	//Need a profile
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	bool isRanked = true; //No renaming of unranked loadouts

	const UI::E_MULTIPLAYER_LOADOUT_TYPE loadoutIndex = (UI::E_MULTIPLAYER_LOADOUT_TYPE)(S32)lua_tonumber( pState, 1 );

	pMultiplayerProfile->GetLoadouts(isRanked).RenameLoadOut( loadoutIndex, UI::gDashboardInterface.GetString() );

	return 0;
}

static S32 GetRaceAwards( lua_State* pState )
{
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

	const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
	const Game::AwardSelection& awardSelection = awards.GetAwardSelection();

	const U8 playerIndex = 0;

	lua_newtable( pState );

	//Push the data for each award
	U8 numAwardsLocalPlayerWon = 0;
	for ( U8 i = 0; i < awards.NumAwardsDelt(); i++ )
	{
		const UI::E_MultiplayerAward type = awardSelection.m_Awards[i];
		if(type == UI::E_MultiplayerAward_None)
		{
			continue;
		}

		const MultiplayerAwardData* pAward = awards.GetAward(type).GetData();

		const U8 vehicleIndex = awardSelection.m_Owners[i];

		lua_newtable( pState );
			Lua::AddDataMember( pState, "icon_name",		pAward->mIconName );
			Lua::AddDataMember( pState, "category",			pAward->mAwardCategory );
			Lua::AddDataMember( pState, "name",				pAward->mAwardName.id );
			Lua::AddDataMember( pState, "description",		pAward->mAwardDescription.id );
			Lua::AddDataMember( pState, "join_ref",			levelState.mCars[vehicleIndex].mNetJoinId );
			Lua::AddDataMember( pState, "rank",				levelState.mCars[vehicleIndex].mMultiplayerRank );
			Lua::AddDataMember( pState, "legend",			levelState.mCars[vehicleIndex].mMultiplayerLegendLevel );
			Lua::AddDataMember( pState, "tied",				awardSelection.m_AwardTied[i] );
			

			Lua::AddDataMember( pState, "local_player",		( vehicleIndex == 0 ) );
			Lua::AddDataMember( pState, "viewport_index",	vehicleIndex );
			Lua::AddDataMember( pState, "pad_index",		levelState.mCars[vehicleIndex].mController );
		lua_rawseti( pState, -2, i+1 );

		if(vehicleIndex == 0)
		{
			numAwardsLocalPlayerWon++;
#ifndef CFG_GOLD
			//Log this players awards
			AmaxDataLogging::LogMpAwardReceived(type, awardSelection.m_AwardValues[i]);
#endif
		}
	}

	// Call Dominator, tell how many awards won. 
	if (pGameMode->IsRanked())
	{
		Game::C_StickerManager::Instance().CheckDominator(numAwardsLocalPlayerWon);
	}

#ifndef CFG_GOLD
	pLayer->GetDataGathering().PostAwardSorceryInformation();
#endif //CFG_GOLD

	return 1;
}

static S32 GetRaceAwardsInLobby( lua_State* pState )
{
	//we need this because the levelstate is not valid in the lobby!!
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

	const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
	const Game::AwardSelection& awardSelection = awards.GetAwardSelection();

	const U8 playerIndex = 0;

	lua_newtable( pState );

	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();


	//Push the data for each award
	U8 numAwardsLocalPlayerWon = 0;
	if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
	{				
		if( !socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
		{
			for ( U8 i = 0; i < awards.NumAwardsDelt(); i++ )
			{
				const UI::E_MultiplayerAward type = awardSelection.m_Awards[i];
				if(type == UI::E_MultiplayerAward_None)
				{
					continue;
				}

				const MultiplayerAwardData* pAward = awards.GetAward(type).GetData();

				const U8 vehicleIndex = awardSelection.m_Owners[i];
				
				lua_newtable( pState );
				Lua::AddDataMember( pState, "icon_name",		pAward->mIconName );
				Lua::AddDataMember( pState, "category",			pAward->mAwardCategory );
				Lua::AddDataMember( pState, "name",				pAward->mAwardName.id );
				Lua::AddDataMember( pState, "description",		pAward->mAwardDescription.id );
				Lua::AddDataMember( pState, "join_ref",			socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[vehicleIndex].mNetJoinId );
				Lua::AddDataMember( pState, "rank",				socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[vehicleIndex].mRank );
				Lua::AddDataMember( pState, "legend",			socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[vehicleIndex].mLegend );

				Lua::AddDataMember( pState, "local_player",		( vehicleIndex == 0 ) );
				Lua::AddDataMember( pState, "viewport_index",	vehicleIndex );
				Lua::AddDataMember( pState, "pad_index",		socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[vehicleIndex].mPadIndex );
				lua_rawseti( pState, -2, i+1 );
				

				if(vehicleIndex == 0)
				{
					numAwardsLocalPlayerWon++;
				}	
			}
		}
	}


	return 1;
}

static S32 GetCustomRaceDefaults( lua_State* pState )
{
	CustomRaceDefaults& defaults = C_MultiplayerData::Instance().m_pConfig->m_CustomRaceDefaults;
	BIZ_ASSERT( defaults.track );
	if ( !defaults.track )
		return 0;

	lua_newtable( pState );
	Lua::AddDataMember( pState, "game_mode",	defaults.game_mode );
	Lua::AddDataMember( pState, "city",			defaults.track->city->ID );
	Lua::AddDataMember( pState, "route",		defaults.track->ID );
	Lua::AddDataMember( pState, "laps",			defaults.laps );
	Lua::AddDataMember( pState, "time",			defaults.time );
	Lua::AddDataMember( pState, "class",		defaults.car_class );
	Lua::AddDataMember( pState, "damage",		defaults.damage );
	Lua::AddDataMember( pState, "respawn",		defaults.respawn );
	Lua::AddDataMember( pState, "timeout",		(S8)defaults.timeout );
	Lua::AddDataMember( pState, "mods",			defaults.mods );
	Lua::AddDataMember( pState, "upgrades",		defaults.upgrades );
	Lua::AddDataMember( pState, "lock_host",	(S8)defaults.lock_to_host_car );
	Lua::AddDataMember( pState, "ai",			defaults.ai );
	Lua::AddDataMember( pState, "ai_padding",	defaults.ai_padding );
	Lua::AddDataMember( pState, "handicap",		defaults.handicap );

	Lua::AddDataMember( pState, "powerup_respawn",	defaults.powerup_respawn );
	Lua::AddDataMember( pState, "random_powerups",	defaults.random_powerups );
	return 1;
}

static S32 GetCustomRaceLimits( lua_State* pState )
{
	CustomRaceDefaults& defaults = C_MultiplayerData::Instance().m_pConfig->m_CustomRaceDefaults;

	lua_newtable( pState );
	Lua::AddDataMember( pState, "max_laps",		defaults.max_laps );
	Lua::AddDataMember( pState, "max_time",		defaults.max_time );
	Lua::AddDataMember( pState, "time_step",	defaults.time_step );
	return 1;
}

static S32 GetCustomRaceSettings( lua_State* pState )
{
	Game::C_GameModeBase* pGameMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	if ( !pGameMode->IsMultiplayer() )
		return 0;

	lua_newtable( pState );
	((Game::C_GameModeMultiPlayer*)pGameMode)->GetCustomRace().SendToLua( pState );
	return 1;
}

static S32 GetCustomRaceCities( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetCustomRaceCities( mode(num) ) - incorrect arguments" );
		return 0;
	}

	const UI::E_RaceType mode = (UI::E_RaceType)(S32)lua_tonumber( pState, 1 );

	// use the correct route list
	const XtArray< XtHandle<AmaxRoute> >* pRoutes = NULL;
	switch ( mode )
	{
		case E_RaceType_Racing :
		case E_RaceType_Hardcore :
			pRoutes = &C_MultiplayerData::Instance().m_pConfig->m_Routes;
			break;
		case E_RaceType_Destruction :
			pRoutes = &C_MultiplayerData::Instance().m_pConfig->m_Arenas;
			break;

		default :
			BIZ_ASSERT_FAIL( "Game mode not supported" );
	}

	// build a list of cities from the valid routes
	Btl::Vector< const AmaxCity* > cityList;
	for ( S32 r = 0; r < pRoutes->count; r++ )
	{
		const AmaxRoute* pRoute = pRoutes->elements[r];
		BIZ_ASSERT_MSG_SPEED( pRoute, "NULL record in multiplayer route list" );

		if ( !pRoute )
			continue;

		bool found = false;
		for ( S32 c = 0; c < cityList.Size(); c++ )
		{
			if ( pRoute->city->ID == cityList[c]->ID )
			{
				found = true;
				break;
			}
		}

		// only add each city once
		if ( !found )
			cityList.PushBack( pRoute->city );
	}


	// Sort the city list alphabetically
	std::sort( cityList.Begin(), cityList.End(), CitySort );

	// Push cities to lua
	lua_newtable( pState );

	for ( S32 i = 0; i < cityList.Size(); i++ )
	{
		lua_newtable( pState );
			Lua::AddDataMember( pState, "id",			cityList[i]->ID );
			Lua::AddDataMember( pState, "name",			cityList[i]->loc_string.id );
			Lua::AddDataMember( pState, "icon",			cityList[i]->icon_name );
			Lua::AddDataMember( pState, "debug_tag",	cityList[i]->tag );
		lua_rawseti( pState, -2, i+1 );
	}

#ifndef CFG_GOLD
	//Data gathering
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	pLayer->GetDataGathering().PostRouteSorceryInformation();
#endif //CFG_GOLD

	return 1;
}

static S32 GetCustomRaceRoutes( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetCustomRaceRoutes( id(num) ) - incorrect arguments" );
		return 0;
	}

	const U32 cityID = (U32)lua_tonumber( pState, 1 );
	const UI::E_RaceType mode = (UI::E_RaceType)(S32)lua_tonumber( pState, 2 );

	// use the correct route list
	const XtArray< XtHandle<AmaxRoute> >* pRoutes = NULL;
	switch ( mode )
	{
		case E_RaceType_Racing :
		case E_RaceType_Hardcore :
			pRoutes = &C_MultiplayerData::Instance().m_pConfig->m_Routes;
			break;
		case E_RaceType_Destruction :
			pRoutes = &C_MultiplayerData::Instance().m_pConfig->m_Arenas;
			break;

		default :
			BIZ_ASSERT_FAIL( "Game mode not supported" );
	}
	
	// build a list of cities from the valid routes
	Btl::Vector< const AmaxRoute* > routeList;
	for ( S32 r = 0; r < pRoutes->count; r++ )
	{
		const AmaxRoute* pRoute = pRoutes->elements[r];
		BIZ_ASSERT_MSG_SPEED( pRoute, "NULL record in multiplayer route list" );

		if ( !pRoute || pRoute->city->ID != cityID )
			continue;

		routeList.PushBack( pRoute );
	}

	// Sort the route list alphabetically
	std::sort( routeList.Begin(), routeList.End(), RouteSort );

	lua_newtable( pState );

	for ( S32 i = 0; i < routeList.Size(); i++ )
	{
		lua_newtable( pState );
			Lua::AddDataMember( pState, "id",				routeList[i]->ID );
			Lua::AddDataMember( pState, "name",				routeList[i]->loc_string.id );
			Lua::AddDataMember( pState, "thumbnail",		routeList[i]->tag );
			Lua::AddDataMember( pState, "debug_tag",		routeList[i]->tag );
			Lua::AddDataMember( pState, "point_to_point",	routeList[i]->onlyAllowsSingleLap );
		lua_rawseti( pState, -2, i+1 );
	}

	return 1;
}

static S32 GetUnlocksForLevel( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetUnlocksForLevel( eSlotType, ability ) - incorrect arguments" );
		return 0;
	}

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	// lua is gonna pass 1-50
	const S32 rank = (S32)lua_tonumber( pState, 1 ) - 1;
	MultiplayerLevel& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank];

	// push new table
	lua_newtable( pState );

	// push abilities
	lua_pushstring( pState, "abilities" );
	lua_newtable( pState );
		for ( S32 i = 0; i < levelData.mAbilityUnlocks.count; i++ )
		{
			lua_newtable( pState );
				Lua::AddDataMember( pState, "name", levelData.mAbilityUnlocks[i]->mAbilityName.id );
				Lua::AddDataMember( pState, "icon", levelData.mAbilityUnlocks[i]->mIconName.string );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	// push cars
	lua_pushstring( pState, "cars" );
	lua_newtable( pState );
		for ( S32 i = 0; i < levelData.mVehicleUnlocks.count; i++ )
		{
			lua_newtable( pState );
				Lua::AddDataMember( pState, "name", levelData.mVehicleUnlocks[i]->loc_string.id );
				Lua::AddDataMember( pState, "icon", levelData.mVehicleUnlocks[i]->m_UiIconName );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	// push packs
	lua_pushstring( pState, "packs" );
	lua_newtable( pState );
		for ( S32 i = 0; i < levelData.mChallengeUnlocks.count; i++ )
		{
			lua_newtable( pState );
				Lua::AddDataMember( pState, "name", levelData.mChallengeUnlocks[i]->mName.id );
				Lua::AddDataMember( pState, "icon", levelData.mChallengeUnlocks[i]->mIconName.string );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	return 1;
}

static S32 GetProgressionForRace( lua_State* pState )
{
	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	const Game::MpRaceSummary& raceSummary = pProfile->GetRaceSummary();

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	// To catch anything dumb that has happened
	BIZ_ASSERT_MSG_SPEED( pProfile->GetPreviousLevel() <= pProfile->GetCurrentLevel(), "Level Data has broke" );
	BIZ_ASSERT_MSG_SPEED( pProfile->NumPreviousFans() <= pProfile->NumFans(), "Level Data has broke" );

	// Work out max values and clamp other values so the UI doesn't banana out
	S32 maxRank = multiplayerConfig->m_LevelingData.count - 1;

	const S32 maxFans = multiplayerConfig->m_LevelingData[maxRank].mFansNeeded;
	const S32 fans = Maths::Clamp( pProfile->NumFans(), 0, maxFans );
	const S32 previousfans = Maths::Clamp( pProfile->NumPreviousFans(), 0, maxFans );

	lua_newtable( pState );

	Lua::AddDataMember( pState, "level",			pProfile->GetCurrentLevel() + 1 );
	Lua::AddDataMember( pState, "legend",			pProfile->GetLegendLevel() );
	Lua::AddDataMember( pState, "previous_level",	pProfile->GetPreviousLevel() + 1 );
	Lua::AddDataMember( pState, "level_ups",		pProfile->GetCurrentLevel() - pProfile->GetPreviousLevel() );
	Lua::AddDataMember( pState, "fans",				fans );
	Lua::AddDataMember( pState, "previous_fans",	previousfans );
	Lua::AddDataMember( pState, "maxed",			( maxFans == previousfans && pProfile->GetCurrentLevel() == maxRank ) ? true : false );

	// bucket up unlocks
	Btl::Vector< MultiplayerAbilityData* >			abilities;
	Btl::Vector< AmaxVehicle* >						vehicles;
	Btl::Vector< MultiplayerChallengePack* >		challenges;
	Btl::Vector< MultiplayerFeature* >				features;
	Btl::Vector< MultiplayerFeature* >				playlists;
	Btl::Vector< Game::C_MultiplayerChallengeTracker::ChallengeComplete >	upgrades; // will only ever have 2 items so copy it

	for ( S32 rank = pProfile->GetPreviousLevel()+1; rank <= pProfile->GetCurrentLevel(); rank++ )
	{
		MultiplayerLevel& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank];

		for ( S32 i = 0; i < levelData.mAbilityUnlocks.count; i++ )
			abilities.PushBack( levelData.mAbilityUnlocks[i] );

		for ( S32 i = 0; i < levelData.mVehicleUnlocks.count; i++ )
		{
			if(!levelData.mVehicleUnlocks[i]->m_ParentVehicle)
			{
				vehicles.PushBack( levelData.mVehicleUnlocks[i] );
			}
		}

		for ( S32 i = 0; i < levelData.mChallengeUnlocks.count; i++ )
			challenges.PushBack( levelData.mChallengeUnlocks[i] );

		for ( S32 i = 0; i < levelData.mFeatureUnlocks.count; i++ )
		{
			// Dont add playlists
			if ( !levelData.mFeatureUnlocks[i]->mIsPlaylist )
				features.PushBack( levelData.mFeatureUnlocks[i] );
		}

		for ( S32 i = 0; i < levelData.mFeatureUnlocks.count; i++ )
		{
			// Add playlists only
			if ( levelData.mFeatureUnlocks[i]->mIsPlaylist )
				playlists.PushBack( levelData.mFeatureUnlocks[i] );
		}
	}

	// Build a list of upgrades for the car that we are using
	const Game::C_MultiplayerChallengeTracker::TChallengeList& completeChallenges = pProfile->GetChallengeSystem().GetTracker().GetChallengesComplete();

	for ( S32 i = 0; i < completeChallenges.Size(); i++ )
	{
		if ( completeChallenges[i].type == E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES )
			upgrades.PushBack( completeChallenges[i] );

		if ( completeChallenges[i].type == E_MULTIPLAYER_CHALLENGE_TYPE_WINNER )
		{
			// Check the car we are using, can we get paint upgrades?
			U32 vehicleID = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[0].mCarID;
			AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)vehicleID];

			if ( pVehicle && pVehicle->m_bUICanReSpray )
				upgrades.PushBack( completeChallenges[i] );
		}
	}

	// Create unlocks table
	lua_pushstring( pState, "unlocks" );
	lua_newtable( pState );

	// push abilities
	lua_pushstring( pState, "abilities" );
	lua_newtable( pState );
		for ( S32 i = 0; i < abilities.Size(); i++ )
		{
			lua_newtable( pState );
				Lua::AddDataMember( pState, "name", abilities[i]->mAbilityName.id );
				Lua::AddDataMember( pState, "desc", abilities[i]->mAbilityDescription.id );
				Lua::AddDataMember( pState, "slot", abilities[i]->mAbilityCategory );
				Lua::AddDataMember( pState, "icon", abilities[i]->mIconName.string );
				Lua::AddDataMember( pState, "sheet", "ability_icons" );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	// push cars
	lua_pushstring( pState, "cars" );
	lua_newtable( pState );
		for ( S32 i = 0; i < vehicles.Size(); i++ )
		{
			lua_newtable( pState );
				Lua::AddDataMember( pState, "id", vehicles[i]->ID );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	// push packs
	lua_pushstring( pState, "packs" );
	lua_newtable( pState );
		for ( S32 i = 0; i < challenges.Size(); i++ )
		{
			lua_newtable( pState );
				Lua::AddDataMember( pState, "name", challenges[i]->mName.id );
				Lua::AddDataMember( pState, "desc", challenges[i]->mDesc.id );
				Lua::AddDataMember( pState, "icon", challenges[i]->mIconName.string );
				Lua::AddDataMember( pState, "sheet", "common_icons" );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	// push features
	lua_pushstring( pState, "features" );
	lua_newtable( pState );
		for ( S32 i = 0; i < features.Size(); i++ )
		{
			lua_newtable( pState );
				Lua::AddDataMember( pState, "name", features[i]->mFeatureName.id );
				Lua::AddDataMember( pState, "desc", features[i]->mFeatureDescription.id );
				Lua::AddDataMember( pState, "icon", features[i]->mIconName );
				Lua::AddDataMember( pState, "sheet", features[i]->mIconSheet );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	// push playlists
	lua_pushstring( pState, "playlists" );
	lua_newtable( pState );
		for ( S32 i = 0; i < playlists.Size(); i++ )
		{
			lua_newtable( pState );
				Lua::AddDataMember( pState, "name",		playlists[i]->mFeatureName.id );
				Lua::AddDataMember( pState, "desc",		playlists[i]->mFeatureDescription.id );
				Lua::AddDataMember( pState, "icon",		playlists[i]->mIconName );
				Lua::AddDataMember( pState, "texture",	playlists[i]->mTexture );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	// push upgrades
	lua_pushstring( pState, "upgrades" );
	lua_newtable( pState );
		for ( S32 i = 0; i < upgrades.Size(); i++ )
		{
			// I can grab the dummy car challenge in the main system
			UI::E_MultiplayerVehicleUpgrades type = pProfile->GetChallengeSystem().GetChallenge( upgrades[i].type ).GetReward( upgrades[i].tier );

			lua_newtable( pState );
				Lua::AddDataMember( pState, "challenge",	upgrades[i].type );
				Lua::AddDataMember( pState, "tier",			upgrades[i].tier );
				Lua::AddDataMember( pState, "type",			type );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	lua_rawset( pState, -3 );

	// Check here wether unlock all 'mods/playlist/cars etc' has been achieved.
	Game::C_GameModeBase* pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	if(pGameModeBase->IsRanked())
	{
		Game::C_StickerManager::Instance().CheckRankFifty();
	}
	return 1;
}

static S32 GetProgressionForRank( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		return 0; 
	}

	// Returns all abilities unlocked at given rank.
	//Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	//Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	S32 Rank = (S32)lua_tonumber( pState, 1 );

	lua_newtable( pState );

	// bucket up unlocks
	Btl::Vector< MultiplayerAbilityData* >		abilities;
	Btl::Vector< AmaxVehicle* >					vehicles;
	Btl::Vector< MultiplayerChallengePack* >	challenges;
	Btl::Vector< MultiplayerFeature* >			features;

	for ( S32 rank = Rank -1; rank <= Rank; rank++ )
	{
		MultiplayerLevel& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank];

		for ( S32 i = 0; i < levelData.mAbilityUnlocks.count; i++ )
			abilities.PushBack( levelData.mAbilityUnlocks[i] );

		for ( S32 i = 0; i < levelData.mVehicleUnlocks.count; i++ )
			vehicles.PushBack( levelData.mVehicleUnlocks[i] );

		for ( S32 i = 0; i < levelData.mChallengeUnlocks.count; i++ )
			challenges.PushBack( levelData.mChallengeUnlocks[i] );

		for ( S32 i = 0; i < levelData.mFeatureUnlocks.count; i++ )
			features.PushBack( levelData.mFeatureUnlocks[i] );
	}

	// Create unlocks table
	lua_pushstring( pState, "unlocks" );
	lua_newtable( pState );

	// push abilities
	lua_pushstring( pState, "abilities" );
	lua_newtable( pState );
	for ( S32 i = 0; i < abilities.Size(); i++ )
	{
		lua_newtable( pState );
		Lua::AddDataMember( pState, "name", abilities[i]->mAbilityName.id );
		Lua::AddDataMember( pState, "icon", abilities[i]->mIconName.string );
		Lua::AddDataMember( pState, "sheet", "ability_icons" );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	// push cars
	lua_pushstring( pState, "cars" );
	lua_newtable( pState );
	for ( S32 i = 0; i < vehicles.Size(); i++ )
	{
		lua_newtable( pState );
		Lua::AddDataMember( pState, "name", vehicles[i]->loc_string.id );
		Lua::AddDataMember( pState, "icon", vehicles[i]->m_UiIconName );
		Lua::AddDataMember( pState, "sheet", "car_icons" );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	// push packs
	lua_pushstring( pState, "packs" );
	lua_newtable( pState );
	for ( S32 i = 0; i < challenges.Size(); i++ )
	{
		lua_newtable( pState );
		Lua::AddDataMember( pState, "name", challenges[i]->mName.id );
		Lua::AddDataMember( pState, "icon", challenges[i]->mIconName.string );
		Lua::AddDataMember( pState, "sheet", "common_icons" );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	// push features
	lua_pushstring( pState, "features" );
	lua_newtable( pState );
	for ( S32 i = 0; i < features.Size(); i++ )
	{
		lua_newtable( pState );
		Lua::AddDataMember( pState, "name", features[i]->mFeatureName.id );
		Lua::AddDataMember( pState, "icon", features[i]->mIconName );
		Lua::AddDataMember( pState, "sheet", features[i]->mIconSheet );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	lua_rawset( pState, -3 );

	return 1;
}

static S32 GetCarChallengeUnlocks( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetCarChallengeUnlocks() - incorrect arguments" );
		return 0;
	}

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	// TODO : Add car unlocks

	return 0;
}

static S32 GetCurrentSystemTimeAsU32( lua_State* pState )
{
	U32 sysTime = static_cast<U32>(Time::GetCurrentTime() / 100000);

	lua_pushinteger( pState, sysTime);
	return 1; 
}

// GetEliminationData
static S32 GetEliminationData( lua_State* pState )
{
	Game::C_RaceBookLayer* pRaceBookLayer = GetRaceBookLayer();
	Game::C_RaceLogicLayer* pRaceLogicLayer = GetRaceLogicLayer();

	if ( !pRaceBookLayer || !pRaceLogicLayer )
		return 0;

	Game::C_LevelState& levelState = GetLevelState();

	if ( levelState.mRaceMode != Game::E_RaceMode_Multiplayer_Elimination || levelState.GetMultiplayerSettings().mEliminationEvent != UI::E_EliminationEvent_Time )
		return 0;

	F32 fTime = ((Game::C_RaceModeMultiplayer_Elimination*)pRaceBookLayer->GetRaceModePtr())->GetRaceTimeout();
	S32 elimStartPos = ((Game::C_RaceModeMultiplayer_Elimination*)pRaceBookLayer->GetRaceModePtr())->GetEliminationStartPos();

	S32 sVehicleIndex = GetPlayerVehicleIndex( 0 );

	S32 sPositionMe  = Game::C_MpResults::Instance().GetPlayerPosition(sVehicleIndex);

	bool inDanger = (sPositionMe > elimStartPos) ? true : false;

	lua_pushnumber( pState, fTime );
	lua_pushboolean( pState, inDanger);

	return 2;
}

// IsPointsTarget
static S32 IsPointsTarget( lua_State* pState )
{
	bool bIsPointsTarget = false;

	Game::C_LevelState& levelState = GetLevelState();

	if ( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Points && levelState.GetMultiplayerSettings().mScoreLimit > 0 )
		bIsPointsTarget = true;

	lua_pushboolean( pState, bIsPointsTarget );
	return 1;
}

static S32 UnlockedLegend( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	lua_pushboolean( pState, pMultiplayerProfile->CanUnlockLegend() );

	return 1;
}

static S32 LegendModeReset( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
	
	pMultiplayerProfile->UnlockLegendMode();

	Game::C_AvatarAwardSystem::Instance().GiveAvatarAward(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), Game::BLUR_AVATAR_AWARD_LEGEND_TEE);

	// if we are in a party, then we need to update the players rank / legend level
	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	if (!ns.IsInitialised())
		return 0;

	if ( ns.Party().IsRunning() )
	{
		RacerNet::C_NetPlayerManager& mgr = ns.PlayerManager();
		for ( U8 i = 0; i < mgr.TotalSize(); i++ )
		{
			if ( mgr.Index( i )->LocalPlayer() )
			{
				mgr.Index( i )->m_ProfileData.m_Game.m_Rank	  = pMultiplayerProfile->GetCurrentLevel();
				mgr.Index( i )->m_ProfileData.m_Game.m_Legend = pMultiplayerProfile->GetLegendLevel();
			}
		}
	}

	return 0;
}

// GetPlayerLoadout
static S32 GetPlayerLoadouts( lua_State* pState )
{
	Btl::Vector<int>& loadOuts = GetLevelState().mCars[ GetPlayerVehicleIndex( 0 ) ].mLoadOuts;

	lua_newtable( pState );

	S32 sCount = 0;

	for ( S32 i = 0; i < loadOuts.Size(); i++ )
	{
		if ( loadOuts[ i ] == E_MultiplayerVehicleAbilities_None )
			continue;

		const MultiplayerAbilityData* pAbility = C_MultiplayerData::Instance().FindAbility( (E_MultiplayerVehicleAbilities)loadOuts[ i ] );

		if ( !pAbility )
			continue;

		lua_newtable( pState );
			Lua::AddDataMember( pState, "slot_index", ++sCount );
			Lua::AddDataMember( pState, "category", pAbility->mAbilityCategory );
		lua_rawseti( pState, -2, loadOuts[ i ] );
	}

	return 1;
}

static S32 CanChooseVehicles( lua_State* pState )
{
	RacerNet::NetRaceSettings raceSettings;
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

	// Some events have random cars so disable the car select
	const bool random_cars = ( raceSettings.m_Misc & ( 1 << UI::E_MiscSettings_RandomCars ) ) ? true : false;

	lua_pushboolean( pState, !random_cars );

	return 1;
}


static S32 CanChooseMods( lua_State* pState )
{
	RacerNet::NetRaceSettings raceSettings;
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

	lua_pushboolean( pState, ( raceSettings.m_Race.m_Mods == UI::E_PowerupMods_On ) ? true : false );

	return 1;
}


static S32 SetEventPresence( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetEventPresence( index(num) ) - incorrect arguments" );
		return 0;
	}

	const S32 index = (S32)lua_tonumber( pState, 1 );

	const MultiplayerEvent* pEvent = C_MultiplayerCache::Instance().Event( index );
	if ( pEvent )
		U64 temp = pEvent->ID;

	//LOGINFO(g_Log, "Setting the event presence to: %i ", index);

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		services.SetUserGroup(index);
	}
	
	return 0;
}

static S32 SetDefaultPresence( lua_State* pState )
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		// 1983 Is the default group id, you change this here you must change it
		// in the IMP file
		services.SetUserGroup(1983);
	}

	return 0;
}

static S32 GetMaxRank( lua_State* pState )
{
	lua_pushnumber( pState, C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count );

	return 1;
}

// GetSplitscreenRouteThumbnail
static S32 GetSplitscreenRouteThumbnail( lua_State* pState )
{
	AmaxRoute* pRoute = C_GameData::Instance().mRoutes[	(GAMEDATA_ID)GetLevelState().mRoute ];

	if ( !pRoute )
		return 0;

	lua_pushstring( pState, pRoute->tag.Text() );
	return 1;
}


static S32 GetRaceChallengesComplete( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	lua_pushnumber( pState, pMultiplayerProfile->GetChallengeSystem().GetTracker().GetNumChallengesComplete() );

	return 1;
}

static S32 GetRaceChallengesCompleteFans( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	if(pMultiplayerProfile)
	{
		Game::C_MultiplayerChallengeSystem& challengeSystem = pMultiplayerProfile->GetChallengeSystem();

		const Game::C_MultiplayerChallengeTracker::TChallengeList& CompleteChallenges = challengeSystem.GetTracker().GetChallengesComplete();

		Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

		U32 VehicleId = 0;

		//we need to get the vehicle loc id from the last race
		if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
		{
			Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;		

			for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
				vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
			{
				U32 padIndex = (vehicleIterator)->mPadIndex;
				U64 systemId = (vehicleIterator)->mSystemID;

				//We know that the pad index for the player will always be 0
				if(SysProfile::C_Manager::Instance().GetPrimaryPadNo() == padIndex && (systemId != 0))
				{
					VehicleId = (vehicleIterator)->mVehicleId;
					break;
				}
			}
		}

		lua_newtable( pState );

		lua_pushstring(pState, "numFans");
		lua_newtable( pState );

		for(int i = 0; i < CompleteChallenges.Size(); i++)
		{		
			lua_pushnumber( pState, CompleteChallenges[i].fansEarned);
			lua_rawseti( pState, -2, i+1);
		}

		lua_rawset( pState, -3 );

		lua_pushstring(pState, "ChallengeName");
		lua_newtable( pState );

		for(int i = 0; i < CompleteChallenges.Size(); i++)
		{		
			const Game::C_MultiplayerChallenge& challenge = challengeSystem.GetChallenge( CompleteChallenges[i].type );
			lua_pushinteger( pState, challenge.GetName().id );
			lua_rawseti( pState, -2, i+1);
		}

		lua_rawset( pState, -3 );

		lua_pushstring(pState, "ChallengeReward");
		lua_newtable( pState );

		for(int i = 0; i < CompleteChallenges.Size(); i++)
		{		
			const Game::C_MultiplayerChallenge& challenge = challengeSystem.GetChallenge( CompleteChallenges[i].type );
			lua_newtable( pState );
				Lua::AddDataMember( pState, "Reward",		challenge.GetReward(CompleteChallenges[i].tier) );
				Lua::AddDataMember( pState, "Type",			CompleteChallenges[i].type );
				Lua::AddDataMember( pState, "Tier",			CompleteChallenges[i].tier );
				Lua::AddDataMember( pState, "VehicleId",	VehicleId);
			lua_rawseti( pState, -2, i+1);
		}

		lua_rawset( pState, -3 );
		
		Lua::AddDataMember( pState, "Total_Fans", pMultiplayerProfile->GetChallengeSystem().GetTracker().GetFansEarned() );

		return 1;
	}

	return 0;

}

static S32 GetLobbyPlayerCar( lua_State* pState )
{	
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if ( !services.IsInitialised() )
		return 0;

	RacerNet::NetRaceSettings raceSettings;
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

	// Find the local player
	RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
	RacerNet::C_NetPlayer* pPlayer = NULL;

	for (U8 i=0; i<mgr.TotalSize(); i++)
	{
		pPlayer = mgr.Index(i);

		if ( pPlayer->PrimaryPlayer() )
			break;
	}

	if ( pPlayer )
	{
		const bool random_cars = ( raceSettings.m_Misc & ( 1 << UI::E_MiscSettings_RandomCars ) ) ? true : false;
		const AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)pPlayer->m_RaceData.m_VehicleData.m_xtVehicleId];
		
		if (!pVehicle)
			return 0;

		lua_newtable( pState );

		Lua::AddDataMember( pState, "id",			pVehicle->ID );
		Lua::AddDataMember( pState, "random",		random_cars );
		Lua::AddDataMember( pState, "icon_sheet",	UI::GetVehicleSheet( pVehicle->carClass ) );
		Lua::AddDataMember( pState, "icon_name",	pVehicle->m_UiIconName );

		// colour
		XtHandle< Vehicle::XtPaintJob > paintJob;
		
		if ( C_UiXtConfig::Instance() )
			paintJob = C_UiXtConfig::Instance()->m_GarageResprays.Find( pPlayer->m_RaceData.m_VehicleData.m_xtColourId );

		bool bColourMan = paintJob ? false : true;

		Lua::AddDataMember( pState, "colour_man", bColourMan );

		if ( !bColourMan )
		{
			C_UIColour col;
			col.Set01( 1.0f, paintJob->m_Colours[ 0 ].r, paintJob->m_Colours[ 0 ].g, paintJob->m_Colours[ 0 ].b );
			Lua::AddDataMember( pState, "colour", col.LuaString() );
		}

		return 1;
	}

	return 0;
}


static S32 GetNewUnlocks( lua_State* pState )
{
	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_MultiplayerUnlockHistory& unlocks = pProfile->GetRecentUnlockHistory();


	bool mods = false, cars = false, challenges = false;
	
	// Check for new mods
	const Game::C_MultiplayerUnlockHistory::AbilitiesContainerType& abilities=unlocks.GetAbilities();
	for( S32 i = 0; i < abilities.Size(); i++ )
	{
		if (abilities[i])
		{
			mods = true;
			break;
		}
	}

	if (!mods)
	{
		mods=unlocks.GetFeatures()[UI::E_MultiplayerFeature_ModShop];
	}

	// Check for new cars
	Game::C_MultiplayerUnlockHistory::CarsMapType::ConstIterator it;
	for( it = unlocks.GetCars().Begin(); it != unlocks.GetCars().End(); it++ )
	{
		if ( (*it).GetValue() == true )
		{
			//cars = true; // Cant add this until mp garage is complete
			break;
		}
	}

	// Check for new challenge packs
	const Game::C_MultiplayerUnlockHistory::ChallengePacksContainerType& challengePacks=unlocks.GetChallengePacks();
	for( S32 i = 0; i < challengePacks.Size(); i++ )
	{
		if ( challengePacks[i] )
		{
			challenges = true;
			break;
		}
	}

	// Check for new car challenges
	for( it = unlocks.GetCarChallenges().Begin(); it != unlocks.GetCarChallenges().End(); it++ )
	{
		if ( (*it).GetValue() == true )
		{
			//challenges = true; // Cant sort this until all the car data is valid
			break;
		}
	}

	bool events=unlocks.GetEvents().Size()!=0;

	lua_newtable( pState );
	Lua::AddDataMember( pState, "mods",			mods );
	Lua::AddDataMember( pState, "cars",			cars );
	Lua::AddDataMember( pState, "challenges",	challenges );
	Lua::AddDataMember( pState, "events",		events );

	return 1;
}

static S32 GetNewAbilitiesMap( lua_State* pState )
{
	lua_newtable( pState );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_MultiplayerUnlockHistory& unlocks = pProfile->GetRecentUnlockHistory();

	// Check for new mods
	const Game::C_MultiplayerUnlockHistory::AbilitiesContainerType& abilities=unlocks.GetAbilities();
	for( int i = 0; i != abilities.Size(); i++ )
	{
		lua_pushboolean( pState, abilities[i] );
		lua_rawseti( pState, -2, i );
	}

	return 1;
}

static S32 GetNewChallengePacksMap( lua_State* pState )
{
	lua_newtable( pState );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_MultiplayerUnlockHistory& unlocks = pProfile->GetRecentUnlockHistory();

	// Check for new challenge packs
	const Game::C_MultiplayerUnlockHistory::ChallengePacksContainerType& challengePacks=unlocks.GetChallengePacks();
	for( int i = 0; i != challengePacks.Size(); i++ )
	{
		lua_pushboolean( pState, challengePacks[i] );
		lua_rawseti( pState, -2, i+1 );
	}

	return 1;
}

static S32 GetNewCarsMap( lua_State* pState )
{
	lua_newtable( pState );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_MultiplayerUnlockHistory& unlocks = pProfile->GetRecentUnlockHistory();

	SP_SaveGame* pSave = SP_GetCurrentSaveGame();

	// Check for new cars
	S32 i = 0;
	Game::C_MultiplayerUnlockHistory::CarsMapType::ConstIterator it;
	bool isNew = false;
	for( it = unlocks.GetCars().Begin(); it != unlocks.GetCars().End(); it++, i++ )
	{
		if((*it).GetValue())
		{
			AmaxVehicle* pVehicle = XtHandle<AmaxVehicle>(XtGetRecord(C_GameApp::GetInstance()->GetGlobalXtDatabase(), it->GetKey())).Object();
			if(pSave && pVehicle && pVehicle->m_ParentVehicle)
			{
				if(SP_IsVehicleUnlocked(pSave, it->GetKey()))
					isNew = true;
				else
					isNew = false;
			}
			else
			{
				isNew = true;
			}
		}
		else
		{
			isNew = false;
		}

		lua_pushboolean( pState, isNew );
		lua_rawseti( pState, -2, it->GetKey() );
	}

	return 1;
}

static S32 GetNewCarChallengesMap( lua_State* pState )
{
	lua_newtable( pState );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_MultiplayerUnlockHistory& unlocks = pProfile->GetRecentUnlockHistory();

	// Check for new car challenges
	S32 i = 0;
	Game::C_MultiplayerUnlockHistory::CarsMapType::ConstIterator it;
	for( it = unlocks.GetCarChallenges().Begin(); it != unlocks.GetCarChallenges().End(); it++, i++ )
	{
		lua_pushboolean( pState, (*it).GetValue() );
		lua_rawseti( pState, -2, i+1 );
	}

	return 1;
}

static S32 GetNewCarsForClass( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetNewCarsForClass( class(num) ) - incorrect arguments" );
		return 0;
	}

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_MultiplayerUnlockHistory& unlocks = pProfile->GetRecentUnlockHistory();

	// get car class for race
	const AmaxCarClass currentClass = (AmaxCarClass)(S32)lua_tonumber( pState, 1 );

	// Check for new cars
	bool newCars = false;
	Game::C_MultiplayerUnlockHistory::CarsMapType::ConstIterator it;
	for( it = unlocks.GetCars().Begin(); it != unlocks.GetCars().End(); it++ )
	{
		// only check items that are new
		if ( (*it).GetValue() )
		{
			AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)it->GetKey() ];

			if ( pVehicle && pVehicle->carClass == currentClass )
			{
				newCars = true;
				break;
			}
		}
	}

	lua_pushboolean( pState, newCars );
	return 1;
}

static S32 AbilityViewed( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "AbilityViewed( id(num) ) - incorrect arguments" );
		return 0;
	}

	const E_MultiplayerVehicleAbilities type = (E_MultiplayerVehicleAbilities)(S32)lua_tonumber( pState, 1 );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	pProfile->GetRecentUnlockHistory().SetAbilityUnlocked( type, false );

	return 0;
}

static S32 FeatureViewed( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetFeatureViewed( id(num) ) - incorrect arguments" );
		return 0;
	}

	const E_MultiplayerFeature feature = (E_MultiplayerFeature)(S32)lua_tonumber( pState, 1 );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	pProfile->GetRecentUnlockHistory().SetFeatureUnlocked(feature, false);

	return 0;
}

static S32 EventViewed( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "EventViewed( id ) - incorrect arguments" );
		return 0;
	}

	U32 eventIndex=(U32)lua_tonumber( pState, 1 );

	const XtId event = C_MultiplayerCache::Instance().Event(eventIndex)->ID;

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	pProfile->GetRecentUnlockHistory().SetEventUnlocked(event, false);

	return 0;
}

static S32 ChallengePackViewed( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ChallengePackViewed( id(num) ) - incorrect arguments" );
		return 0;
	}

	const UI::E_ChallengePack pack = (E_ChallengePack)(S32)lua_tonumber( pState, 1 );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	pProfile->GetRecentUnlockHistory().SetChallengePackUnlocked( pack, false );

	return 0;
}

static S32 CarViewed( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "CarViewed( id(num) ) - incorrect arguments" );
		return 0;
	}

	const XtId id = (XtId)(U32)lua_tointeger( pState, 1 );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	pProfile->GetRecentUnlockHistory().SetCarUnlocked( id, false );

	return 0;
}

static S32 CarChallengeViewed( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "CarViewed( id(num) ) - incorrect arguments" );
		return 0;
	}

	const XtId id = (XtId)(U32)lua_tointeger( pState, 1 );

	// Returns all abilities currently unlocked.
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	pProfile->GetRecentUnlockHistory().SetCarChallengeUnlocked( id, false );

	return 0;
}

// GetGarageCarByID
static S32 GetGarageCarByID( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ( !pGameProfile )
		return 0;

	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
		lua_pushinteger( pState, pGameProfile->mMultiplayerProfile.GetLastCarUsed( (AmaxCarClass)(S32)lua_tonumber( pState, 1 ) ) );
	else
		lua_pushinteger( pState, pGameProfile->mMultiplayerProfile.GetLastCarUsed() );

	return 1;
}

// SetGarageCarByID
static S32 SetGarageCarByID( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetGarageCarByID( id ) - incorrect arguments" );
		return 0;
	}

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ( !pGameProfile )
		return 0;

	//No changes allowed on lock to host mode unless you are the host 
	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_Online)
	{
		Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
		if(pGameMode->GetPlayMode() == Game::E_PlayMode_CustomRace)
		{
			const Game::C_CustomRace& customRace = pGameMode->GetCustomRace();
			if(customRace.GetLockToHostCar())
			{
				RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
				RacerNet::C_NetPlayer* hostPlayer = services.PlayerManager().HostPlayer(false);

				if(hostPlayer && !hostPlayer->LocalPlayer())
				{
					return 0;
				}
			}
		}
	}

	pGameProfile->mMultiplayerProfile.SetLastCarUsed( (U32)lua_tointeger( pState, 1 ) );

	return 0;
}

static S32 SetupLevelState( lua_State* pState )
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	if ( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer() )
		((Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr())->SetupLevelState( pState, levelState);

	return 0;
}

static S32 CyclePlaylist( lua_State* pState )
{
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	if ( pGameMode && pGameMode->IsMultiplayer() )
		((Game::C_GameModeMultiPlayer*)pGameMode)->CyclePlaylist( pGameMode->GetNumberOfPlayers() );

	return 0;
}

static S32 IsLastPlaylistRace( lua_State* pState )
{
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	bool result = false;
	if ( pGameMode && pGameMode->IsMultiplayer() )
		result = ((Game::C_GameModeMultiPlayer*)pGameMode)->GetPlaylist().IsLastRace();

	lua_pushboolean( pState, result );
	return 1;
}

static S32 StartRaceVoting( lua_State* pState )
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	if ( pLayer->GetGameModePtr()->IsMultiplayer() )
		((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr())->CalculateVoteIndices();

	return 0;
}

static S32 GetVoteRaceIndicies( lua_State* pState )
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

	const Game::VoteInfo& voteInfo = pGameMode->GetVoteInfo();

	lua_pushnumber( pState, voteInfo.indexA );
	lua_pushnumber( pState, voteInfo.indexB );

	return 2;
}

static S32 GetVotingValues( lua_State* pState )
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

	// Get the vote info structure
	const Game::VoteInfo& voteInfo = pGameMode->GetVoteInfo();

	const S32 numPlayers = pGameMode->GetNumberOfPlayers();
	const S32 total_votes = voteInfo.countA + voteInfo.countB;
	const S32 majorityCount = numPlayers / 2;

	lua_pushboolean( pState, ( total_votes == numPlayers || voteInfo.countA > majorityCount || voteInfo.countB > majorityCount ) );
	lua_pushnumber( pState, voteInfo.countA );
	lua_pushnumber( pState, voteInfo.countB );

	return 3;
}

static S32 GetVotingResults( lua_State* pState )
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

	const Game::VoteInfo& voteInfo = pGameMode->GetVoteInfo();
	
	lua_pushnumber( pState, voteInfo.result );
	lua_pushboolean( pState, ( voteInfo.countA == voteInfo.countB ) );

	return 2;
}

static S32 CalculateLocalVoteResults( lua_State* pState )
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

	pGameMode->CalculateVoteResults();

	return 0;
}

static S32 CastLocalVote( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "CastLocalVote( vote ) - incorrect arguments" );
		return 0;
	}

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

	pGameMode->VoteOnRace( (S8)lua_tonumber( pState, 1 ) );

	return 0;
}

static S32 IsVotingEnabled( lua_State* pState )
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

	// Voting isnt availible in custom race
	bool voting = false;
	if ( pGameMode->GetPlayMode() == Game::E_PlayMode_Playlist )
	{
		const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();

		if(pEvent->races.count > 1 && (pEvent->misc & ( 1 << UI::E_MiscSettings_Voting)))
			voting = true;
	}

#ifndef CFG_GOLD
	//If we don't get to vote we can stop logging data now
	if(!voting)
	{
		AmaxDataLogging::LogRaceEnd();
	}
#endif

	lua_pushboolean( pState, voting );
	return 1;
}

static S32 GetRaceHistory( lua_State* pState )
{
	int num=1;
	int index=Game::C_SocialNetworkManager::Instance().mRaceHistoryStoreIndex;

	lua_newtable(pState);

	//make a table from the start of the circular buffer, and go backwards for chronological order
	do
	{
		lua_newtable( pState );

		index--;
		if(index == -1)
			index=Game::MAX_SOCIAL_NETWORK_RACE_HISTORY-1;

		Lua::AddDataMember( pState, "num_cars",	Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index].mNumCars );
		Lua::AddDataMember( pState, "index",	index );
		Lua::AddDataMember( pState, "my_pos", Game::C_SocialNetworkManager::Instance().GetMyRaceHistoryPos(index) );
		Lua::AddDataMember( pState, "type", Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index].mRaceType );

		const AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index].mRoute];
		if( pRoute )
		{
			Lua::AddDataMember( pState, "city", pRoute->city->ID );
		}

		int num_awards=0;
		for(int a=0;a<Game::MAX_SOCIAL_NETWORK_RACE_AWARDS;a++)
		{
			if(Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index].mPlayerAwards[a] != UI::E_MultiplayerAward_None)
				num_awards++;
		}
		Lua::AddDataMember( pState, "num_awards", num_awards );

		lua_rawseti( pState, -2, num );

		num++;
	
	}while(index != Game::C_SocialNetworkManager::Instance().mRaceHistoryStoreIndex);
	
	return 1;
}

static S32 GetRaceHistoryAwardIconNames( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER  )
	{
		g_pUILua->StackDump( pState, "GetRaceHistoryAwardIconNames( raceIndex ) - incorrect arguments" );
		return 0;
	}

	int raceIndex = (int)lua_tonumber(pState,1);

	if( raceIndex >= Game::C_SocialNetworkManager::Instance().mRaceHistoryStore.Size() || raceIndex < 0 )
	{
		g_pUILua->StackDump( pState, "GetRaceHistoryAwardIconNames( raceIndex ) - raceIndex out of bounds" );
		return 0;
	}
	
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

	const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
	
	lua_newtable( pState );

	for( int i=0; i< Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[raceIndex].mPlayerAwards.Size(); i++ )
	{
		UI::E_MultiplayerAward award_enum = Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[raceIndex].mPlayerAwards[i];
		if( award_enum != UI::E_MultiplayerAward_None )
		{
			const Game::C_MultiplayerAward& award = awards.GetAward( award_enum );
			const MultiplayerAwardData* awardData = award.GetData();

			lua_pushstring(pState, awardData->mIconName );
			lua_rawseti( pState, -2, i+1 );
		}
	}

	return 1;
}

static S32 GetRaceHistoryVehicle( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetRaceHistoryVehicle( raceIndex, vehIndex ) - incorrect arguments" );
		return 0;
	}

	int raceIndex = (int)lua_tonumber(pState,1);
	int vehPosition = (int)lua_tonumber(pState,2);

	if( raceIndex >= Game::C_SocialNetworkManager::Instance().mRaceHistoryStore.Size() || raceIndex < 0 )
	{
		g_pUILua->StackDump( pState, "GetRaceHistoryVehicle( raceIndex, vehPosition ) - raceIndex out of bounds" );
		return 0;
	}

	Game::AmaxSocialNetworkRaceVehicleStore* pVehicle = NULL;
	for(int a=0;a<Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[raceIndex].mNumCars;a++)
	{
		if( vehPosition == Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[raceIndex].mVehicles[a].mPosition )
		{
			pVehicle = &Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[raceIndex].mVehicles[a];
		}
	}

	if(pVehicle)
	{
		lua_newtable( pState );

		Lua::AddDataMember( pState, "name",				pVehicle->mName );
		Lua::AddDataMember( pState, "rank",				pVehicle->mRank );
		Lua::AddDataMember( pState, "legend",			pVehicle->mLegend );
		Lua::AddDataMember( pState, "position",			pVehicle->mPosition );

		return 1;
	}

	return 0;
}

static S32 LookingAtRaceHistory( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "LookingAtRaceHistory( index ) - incorrect arguments" );
		return 0;
	}

	Game::C_SocialNetworkManager::Instance().mLookingAtRaceHistoryStoreIndex=(int)lua_tonumber(pState,1);

	return 0;
}

static S32 POBGameInfo( lua_State* pState )
{
	lua_newtable( pState );

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
	Game::C_MultiplayerPreviousSessionStats POBStats = pMultiplayerProfile->GetPreviouslyOnBlurStats();

	//Lua::AddDataMember( pState, "LastNemesis",					POBStats.m_LastNemesis);
	//Lua::AddDataMember( pState, "m_LastPrey",					POBStats.m_LastPrey);

	Lua::AddDataMember( pState, "Races",						POBStats.m_Races);
	Lua::AddDataMember( pState, "Wins",							POBStats.m_Wins);
	Lua::AddDataMember( pState, "NumRacePoints",				POBStats.m_NumRacePoints);
	Lua::AddDataMember( pState, "FansEarned",					POBStats.m_FansEarned);
	Lua::AddDataMember( pState, "RanksGained",					POBStats.m_RanksGained);									  
	Lua::AddDataMember( pState, "LastChallengeCompleted",		POBStats.m_LastChallengeCompleted);
	Lua::AddDataMember( pState, "LastChallengeCompletedLevel",	POBStats.m_LastChallengeCompletedLevel);
	Lua::AddDataMember( pState, "LastChallengeCarId",			POBStats.m_LastChallengeCarId);
	Lua::AddDataMember( pState, "Legend",						pMultiplayerProfile->GetLegendLevel());

	return 1;									  
}		

static S32 ClearPOBGameInfo( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
	Game::C_MultiplayerPreviousSessionStats POBStats = pMultiplayerProfile->GetPreviouslyOnBlurStats();
	POBStats.Clear();

	return 0;
}

static S32 POBUpcomingUnlocksInfo( lua_State* pState )
{
	lua_newtable( pState );

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	const S32 numFans = pMultiplayerProfile->NumFans();

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	// lua is going to give us 1-50 not 0-49
	const S32 rank = pMultiplayerProfile->GetCurrentLevel();

	S32 maxRank = C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count - 1;

	// Push number of fans needed for next level
	const S32 low_level = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank].mFansNeeded;
	const S32 high_level = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank + ( ( rank < maxRank ) ? 1 : 0 )].mFansNeeded;

	S32 NumFansNeeded = high_level - low_level;
	S32 NumFansGotNow = numFans - low_level;

	bool rankUp = false;

	if( rank < maxRank )
	{
		if((((float)numFans/(float)high_level)*100.0f) > 75.0f)
		{
			rankUp = true;
		}
	}

	Lua::AddDataMember( pState, "rankUp",	rankUp);


	return 1;

}		

static S32 POBGetCurrentRank( lua_State* pState )
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	const S32 rank = (pMultiplayerProfile->GetCurrentLevel() + 1);

	lua_pushnumber(pState, rank);

	return 1;
}

static S32 POBGetPlaylistUnlocks( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "POBGetUnlocks( level ) - incorrect arguments" );
		return 0;
	}

	lua_newtable(pState);

	S32 rank = lua_tonumber(pState,1);

	MultiplayerConfig* multiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
	
	lua_pushstring(pState, "playlists");
	lua_newtable(pState);

	if(multiplayerConfig)
	{
		if( (rank) < multiplayerConfig->m_LevelingData.count )	
		{
		
			MultiplayerLevel  levelingData = multiplayerConfig->m_LevelingData[rank];

			for( S32 i = 0; i < multiplayerConfig->m_OnlineEvents.count; i++ )
			{
				const MultiplayerEvent* pEvent = multiplayerConfig->m_OnlineEvents[i];
				if(pEvent->min_rank == rank)
				{
					lua_newtable(pState);
						Lua::AddDataMember( pState, "name", pEvent->name.id ); 
						Lua::AddDataMember( pState, "description", pEvent->desc.id ); 
						Lua::AddDataMember( pState, "filename", pEvent->image_name ); 
						Lua::AddDataMember( pState, "icon", pEvent->icon_name ); 
						Lua::AddDataMember( pState, "sheet", "common_icons" );
					lua_rawseti( pState, -2, i + 1);
				}
			}
		}
	}

	lua_rawset(pState, -3);

	return 1;
}

static S32 POBProgressionForRank( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		return 0; 
	}

	// Returns all abilities unlocked at given rank.
	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	S32 Rank = (S32)lua_tonumber( pState, 1 );

	lua_newtable( pState );

	// bucket up unlocks
	Btl::Vector< MultiplayerAbilityData* >		abilities;
	Btl::Vector< AmaxVehicle* >					vehicles;
	Btl::Vector< MultiplayerChallengePack* >	challenges;
	Btl::Vector< MultiplayerFeature* >			features;

	for ( S32 rank = Rank -1; rank < Rank; rank++ )
	{
		if(rank < C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count)
		{
			MultiplayerLevel& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank];

			for ( S32 i = 0; i < levelData.mAbilityUnlocks.count; i++ )
				abilities.PushBack( levelData.mAbilityUnlocks[i] );

			for ( S32 i = 0; i < levelData.mVehicleUnlocks.count; i++ )
			{	
				//no boss or legend cars please
				if(levelData.mVehicleUnlocks[i])
				{
					if(levelData.mVehicleUnlocks[i]->m_ParentVehicle == NULL)
					{
						vehicles.PushBack( levelData.mVehicleUnlocks[i] );
					}
				}
			}
			
			for ( S32 i = 0; i < levelData.mChallengeUnlocks.count; i++ )
			challenges.PushBack( levelData.mChallengeUnlocks[i] );

			for ( S32 i = 0; i < levelData.mFeatureUnlocks.count; i++ )
				features.PushBack( levelData.mFeatureUnlocks[i] );
		}
	}

	// Create unlocks table
	lua_pushstring( pState, "unlocks" );
	lua_newtable( pState );

	// push abilities
	lua_pushstring( pState, "abilities" );
	lua_newtable( pState );
	for ( S32 i = 0; i < abilities.Size(); i++ )
	{
		lua_newtable( pState );
		
		Lua::AddDataMember( pState, "name", abilities[i]->mAbilityName.id );
		Lua::AddDataMember( pState, "description", abilities[i]->mAbilityDescription.id );
		Lua::AddDataMember( pState, "icon", abilities[i]->mIconName.string );
		Lua::AddDataMember( pState, "sheet", "ability_icons" );
		Lua::AddDataMember( pState, "category", abilities[i]->mAbilityCategory);
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	// push cars
	lua_pushstring( pState, "cars" );
	lua_newtable( pState );
	for ( S32 i = 0; i < vehicles.Size(); i++ )
	{
		lua_newtable( pState );
		Lua::AddDataMember( pState, "id", vehicles[i]->ID );
		Lua::AddDataMember( pState, "name", vehicles[i]->loc_string.id );
		Lua::AddDataMember( pState, "icon", vehicles[i]->m_UiIconName );
		Lua::AddDataMember( pState, "style", vehicles[i]->m_eUiCarStyle );
		Lua::AddDataMember( pState, "class", vehicles[i]->carClass );
		
		// full name
		if ( vehicles[i]->locStringFull.id != 0 )
			Lua::AddDataMember( pState, "name_full", vehicles[i]->locStringFull.id );	// override
		else
		{
			Btl::String64 string;
			string.Printf( "GAME_VEHICLE_%u_FULL_NAME", vehicles[i]->ID );
			Lua::AddDataMember( pState, "name_full", string.Chars() );	// datafinder lookup for manufacturer name + car name
		}

		Lua::AddDataMember( pState, "sheet", GetVehicleSheet(vehicles[i]->carClass) );

		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	// push packs
	lua_pushstring( pState, "packs" );
	lua_newtable( pState );
	for ( S32 i = 0; i < challenges.Size(); i++ )
	{
		lua_newtable( pState );
		Lua::AddDataMember( pState, "name", challenges[i]->mName.id );
		Lua::AddDataMember( pState, "icon", challenges[i]->mIconName.string );
		Lua::AddDataMember( pState, "sheet", "common_icons" );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	// push features
	lua_pushstring( pState, "features" );
	lua_newtable( pState );
	for ( S32 i = 0; i < features.Size(); i++ )
	{
		lua_newtable( pState );
		Lua::AddDataMember( pState, "name", features[i]->mFeatureName.id );
		Lua::AddDataMember( pState, "icon", features[i]->mIconName );
		Lua::AddDataMember( pState, "sheet", features[i]->mIconSheet );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	lua_rawset( pState, -3 );

	return 1;
}

static S32 IsElimination( lua_State* pState )
{
	bool isElimination = false;
	if(C_GameApp::GetInstance()->GetGame()->GetLevelState().mWreckedBehaviour == Game::E_AMAXWRECKED_ELIMINATED)
	{
		isElimination = true;
	}
	lua_pushboolean( pState, isElimination  );
	return 1;
}

static S32 IsEliminated( lua_State* pState )
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	bool isEliminated = levelState.mCars[0].mState == Game::E_VEHICLE_STATE_ELIMINATED;

	lua_pushboolean( pState, isEliminated );
	return 1;
}

static S32 GetLobbyRaceResults( lua_State* pState )
{
	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();
	
	lua_newtable( pState );

	if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
	{
		S32 num = 0;
		Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;

		for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
			  vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
		{
			lua_newtable( pState );
			Lua::AddDataMember( pState, "position",  (vehicleIterator)->mPosition); 
			Lua::AddDataMember( pState, "gamertag",  (vehicleIterator)->mName); 
			Lua::AddDataMember( pState, "statistic",  (vehicleIterator)->mStatistic); 
			lua_rawseti( pState, -2, num + 1);

			num++;
		}
	
	}

	return 1;
	
}

static S32 GetLobbyRaceResultsLegendLevel( lua_State* pState )
{
	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

	S32 num = 0;

	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
		{
			Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;

			S32 position = (S32)lua_tonumber( pState, 1 );

			for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
				vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
			{
				if((vehicleIterator)->mPosition == position)
				{
					lua_pushnumber( pState, (vehicleIterator)->mLegend); 
					num++;
					break;
				}				
			}
		}
	}

	if(num == 0)
	{
		lua_pushnumber( pState, -1 ); 
		num++;
	}

	return num;
}

static S32 GetLobbyRaceResultsRank( lua_State* pState )
{
	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

	S32 num = 0;

	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
		{
			Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;

			
			S32 position = (S32)lua_tonumber( pState, 1 );

			for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
				vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
			{
				if((vehicleIterator)->mPosition == position)
				{
					lua_pushnumber( pState, (vehicleIterator)->mRank ); 
					num++;

					break;
				}

			}
		}

	}

	if(num == 0)
	{
		lua_pushnumber( pState, -1 ); 
		num++;
	}

	return num;
}

static S32 GetLobbyRaceResultsTeam( lua_State* pState )
{
	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
		{
			Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;
			S32 position = (S32)lua_tonumber( pState, 1 );

			for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
				vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
			{
				if((vehicleIterator)->mPosition == position)
				{
					S8 team = (vehicleIterator)->mTeam;
					lua_pushnumber( pState, team ); 
					return 1;
				}

			}
		}

	}

	return 0;
}

static S32 GetLobbyRaceResultsIsMe( lua_State* pState )
{
	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

	S32 num = 0;

	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
		{
			Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;
			
			S32 position = (S32)lua_tonumber( pState, 1 );

			for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
				vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
			{
				if((vehicleIterator)->mPosition == position)
				{
					U32 padIndex = (vehicleIterator)->mPadIndex;

					const bool isMe = (padIndex == SysProfile::C_Manager::Instance().GetPrimaryPadNo());

					lua_pushboolean( pState, isMe); 
					return 1;
				}
			}
		}
	}

	return 0;
}

static S32 GetLobbyRaceResultsNemesisJoinId( lua_State* pState )
{
	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

	S32 num = 0;

	if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
	{

		S8 NemesisId = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mMyNemesis;
		U8 NemesisJoinId = -1;

		SysProfile::UserIdentifier	Uid;

		if(NemesisId != -1)
		{
			NemesisJoinId = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mNemesisNetJoinId;
			Uid.m_Uid  =	socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mNemesisUid.m_Uid;
			WCHAR* name = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[NemesisId].mName;
			
			//Make sure this guy still exists
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			RacerNet::C_NetPlayerManager& playerManager = services.PlayerManager();

			RacerNet::C_NetPlayer* pPlayer = playerManager.FindFromJoinId(NemesisJoinId, false);

			if(pPlayer && pPlayer->m_User == Uid)
			{
				lua_pushnumber( pState, NemesisJoinId ); 
				num++;
			}			
		}				
	}

	if(num == 0)
	{
		lua_pushnumber( pState, -1 ); 
		num++;
	}

	return num;
}

static S32 GetLobbyRaceResultsPreyJoinId( lua_State* pState )
{
	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

	S32 num = 0;

	if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
	{

		S8 PreyId = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mMyPrey;
		U8 PreyJoinId = -1;

		SysProfile::UserIdentifier	Uid;

		if(PreyId != -1)
		{
			PreyJoinId = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mPreyNetJoinId;
			Uid.m_Uid  =	socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mPreyUid.m_Uid;
			WCHAR* name = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[PreyId].mName;

			//Make sure this guy still exists
			RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
			RacerNet::C_NetPlayerManager& playerManager = services.PlayerManager();

			RacerNet::C_NetPlayer* pPlayer = playerManager.FindFromJoinId(PreyJoinId, false);

			if(pPlayer && pPlayer->m_User == Uid)
			{
				lua_pushnumber( pState, PreyJoinId ); 
				num++;
			}			
		}				
	}

	if(num == 0)
	{
		lua_pushnumber( pState, -1 ); 
		num++;
	}

	return num;
}

static S32 GetLobbyRaceResultsGameModeIsDestruction( lua_State* pState )
{
	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

	if(socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mRaceType == UI::E_RaceType_Destruction)
	{
		lua_pushboolean(pState, true);
	}
	else
	{
		lua_pushboolean(pState, false);
	}

	return 1;
}

static S32 GetRaceResultsValid( lua_State* pState )
{
	if(Game::C_MpResults::Instance().m_ResultsValid == true)
	{
		lua_pushboolean(pState, true);
	}
	else
	{
		lua_pushboolean(pState, false);
	}

	return 1;
}

static S32 SwitchSpectateCam( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER  )
	{
		g_pUILua->StackDump( pState, "SwitchSpectateCam( index, vehicle ) - incorrect arguments" );
		return 0;
	}

	int camIndex=(S32)lua_tonumber( pState, 1 );
	int vehIndex=(S32)lua_tonumber( pState, 2 );

	if (camIndex==0)
	{
		ActorMsg::C_BlendAnimatedCameraMsg		msg;
		msg.m_Duration = 0.5f;
		msg.m_Viewport = 0;
		msg.m_VehicleIndex = vehIndex;
		msg.m_FutureFeed = false;
		Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
	}
	else
	{
		if (camIndex-1>Actors::E_ANIMCAMERASEQ_MPSPECTATE_END-Actors::E_ANIMCAMERASEQ_MPSPECTATE1)
		{
			LOGERROR( g_Log, "SwitchSpectateCam, index is outside limits" );
			camIndex=1;
		}

		ActorMsg::C_StartAnimatedCameraMsg msg;
		msg.m_Sequence = (Actors::E_ANIMCAMERASEQ_TYPE)(Actors::E_ANIMCAMERASEQ_MPSPECTATE1 + camIndex-1);
		msg.m_Viewport = 0;
		msg.m_VehicleIndex = vehIndex;
		msg.m_StartPaused = false;
		msg.m_FutureFeed = false;
		Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
	}
	return 0;
} 

static S32 IsNemesisValid( lua_State* pState )
{
	bool nemesisValid = false;

	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();		

	if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
	{
		S8 NemesisId = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mMyNemesis;
		if(NemesisId != -1)
		{
			nemesisValid = true;
		}							
	}

	lua_pushboolean( pState, nemesisValid );

	return 1;
}

static S32 IsPreyValid( lua_State* pState )
{
	bool preyValid = false;

	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();		

	if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
	{
		S8 PreyId = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mMyPrey;
		if(PreyId != -1)
		{
			preyValid = true;
		}							
	}

	lua_pushboolean( pState, preyValid );

	return 1;
}

static S32 ChallengeTypeIsRace( lua_State* pState )
{
	bool typeRace = false;

	if ( lua_type( pState, 1 ) != LUA_TNUMBER  )
	{
		g_pUILua->StackDump( pState, "ChallengeTypeIsRace( challengeType ) - incorrect arguments" );
		return 0;
	}

	UI::E_MULTIPLAYER_CHALLENGE_TYPE challengeType = (UI::E_MULTIPLAYER_CHALLENGE_TYPE)lua_tointeger( pState, 1 );

	if(challengeType == UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES)
	{
		typeRace = true;
	}

	lua_pushboolean( pState, typeRace );

	return 1;
}

static S32 ChallengeTypeIsWinner( lua_State* pState )
{
	bool typeWinner = false;

	if ( lua_type( pState, 1 ) != LUA_TNUMBER  )
	{
		g_pUILua->StackDump( pState, "ChallengeTypeIsRace( challengeType ) - incorrect arguments" );
		return 0;
	}

	UI::E_MULTIPLAYER_CHALLENGE_TYPE challengeType = (UI::E_MULTIPLAYER_CHALLENGE_TYPE)lua_tointeger( pState, 1 );

	if(challengeType == UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER)
	{
		typeWinner = true;
	}

	lua_pushboolean( pState, typeWinner );

	return 1;
}

static S32 PostFlaggedDataEvents( lua_State* pState )
{
	bdOnlineUserID userID;
	bool working = SysProfile::C_Manager::Instance().GetDemonwareUserId(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), userID);
	if(!working)
	{
		return 0;
	}

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

	if(!pLayer)
	{
		return 0;
	}

	//Flush out all pending demonware events
	pLayer->GetDataGathering().PostFlaggedEvents(userID);

	return 0;
}
												  
// the library									  m_Timestamp;
static const luaL_reg multiplayer[] = 
{
	{ "FindPlaylistFromID",				FindPlaylistFromID },
	{ "GetPlaylist",					GetPlaylist },
	{ "GetPlaylists",					GetPlaylists },
	{ "GetPlaylistRace",				GetPlaylistRace },
	{ "GetCurrentRace",					GetCurrentRace },
	{ "GetRaceResults",					GetRaceResults },
	{ "GetTeamResults",					GetTeamResults },
	{ "GetPlayerTeam",					GetPlayerTeam },
	{ "SetCurrentEventIndex",			SetCurrentEventIndex },
	{ "GetCurrentEventIndex",			GetCurrentEventIndex },
	{ "GetCurrentRaceIndex",			GetCurrentRaceIndex },
	{ "GetRaceChallengesComplete",		GetRaceChallengesComplete },

	{ "CachePlaylists",					CachePlaylists },
	{ "CacheNetRaceSettings",			CacheNetRaceSettings },
	{ "CacheInviteSettings",			CacheInviteSettings },

	{ "E3EnablePlayerControls",			E3EnablePlayerControls },
	{ "VehiclesFinished",				VehiclesFinished },
	{ "RefreshLobby",					RefreshLobby },
	{ "TeamDataValid",					TeamDataValid },
	{ "GetLobbySlots",					GetLobbySlots },
	{ "RefreshRacePreview",				RefreshRacePreview },
	{ "GetRacePreviewData",				GetRacePreviewData },

	{ "GetVehiclePosition",				GetVehiclePosition },
	{ "GetVehicleIndexByPosition",		GetVehicleIndexByPosition },
	{ "GetRivalIndex",					GetRivalIndex },
	{ "VehicleIDToJoinID",				VehicleIDToJoinID },
	{ "GetRivalsCountdownDelayActive",	GetRivalsCountdownDelayActive },
	
	{ "ProfileProgression",				ProfileProgression },
	{ "LevelProgressionData",			LevelProgressionData },
	{ "NumberOfFans",					NumberOfFans },
	{ "GetChallengePacks",				GetChallengePacks },
	{ "GetChallenges",					GetChallenges },
	{ "GetCarChallenges",				GetCarChallenges },
	{ "GetCarChallengeClasses",			GetCarChallengeClasses },
	{ "GetCarChallengeRewards",			GetCarChallengeRewards },

	{ "RaceFinished",					RaceFinished },
	{ "PrimaryVehicleFinished",			PrimaryVehicleFinished },
	{ "RaceTimeout",					RaceTimeout },
	{ "FindPlayersTimer",				FindPlayersTimer },
	{ "ClearScoreboardGamerPictures",	ClearScoreboardGamerPictures },

	{ "SplitscreenResetScores",			SplitscreenResetScores },
	{ "GetSplitscreenScores",			GetSplitscreenScores },
	{ "GetSplitscreenPosition",			GetSplitscreenPosition },

	{ "GetRaceMode",					GetRaceMode },
	{ "MultiplayerVotePassed",			MultiplayerVotePassed },
	{ "GetNonRankUnlockedDLCCars",		GetNonRankUnlockedDLCCars},

	{ "GetFeatureUnlocked",				GetFeatureUnlocked },
	{ "GetFeatureUnlockedMap",			GetFeatureUnlockedMap },
	{ "GetUnlocksForLevel",				GetUnlocksForLevel },
	{ "GetCarChallengeUnlocks",			GetCarChallengeUnlocks },
	{ "GetProgressionForRace",			GetProgressionForRace },
	{ "GetAbilityPacks",				GetAbilityPacks },
	{ "GetAbility",						GetAbility },
	{ "GetAbilities",					GetAbilities },
	{ "GetAbilityLoadouts",				GetAbilityLoadouts },
	{ "GetAbilityUnlockMap",			GetAbilityUnlockMap },
	{ "SetAbilityLoadout",				SetAbilityLoadout },
	{ "GetCurrentLoadout",				GetCurrentLoadout },
	{ "GetCurrentLoadOutType",			GetCurrentLoadOutType },
	{ "SetCurrentLoadOutType",			SetCurrentLoadOutType },
	{ "SetNameOfLoadOut",				SetNameOfLoadOut },
	{ "GetRaceAwards",					GetRaceAwards },
	{ "UnlockedLegend",					UnlockedLegend },
	{ "LegendModeReset",				LegendModeReset },
	{ "GetMaxRank",						GetMaxRank },

	{ "GetNewUnlocks",					GetNewUnlocks },
	{ "GetNewAbilitiesMap",				GetNewAbilitiesMap },
	{ "GetNewChallengePacksMap",		GetNewChallengePacksMap },
	{ "GetNewCarChallengesMap",			GetNewCarChallengesMap },
	{ "GetNewCarsForClass",				GetNewCarsForClass },
	{ "GetNewCarsMap",					GetNewCarsMap },
	{ "AbilityViewed",					AbilityViewed },
	{ "FeatureViewed",					FeatureViewed },
	{ "ChallengePackViewed",			ChallengePackViewed },
	{ "CarViewed",						CarViewed },
	{ "EventViewed",					EventViewed },
	{ "CarChallengeViewed",				CarChallengeViewed },
	
	{ "GetCurrentCarClass",				GetCurrentCarClass },
	{ "GetSplitscreenVehicles",			GetSplitscreenVehicles },
	{ "GetSplitscreenResprays",			GetSplitscreenResprays },
	{ "GetGarage",						GetGarage },
	{ "ShowAllGarage",					ShowAllGarage },
	{ "CanChooseVehicles",				CanChooseVehicles },
	{ "CanChooseMods",					CanChooseMods },
	{ "GetLobbyPlayerCar",				GetLobbyPlayerCar },
	
	{ "SetNameOfLoadOut",				SetNameOfLoadOut },				//Set an ability in a specific slot within a specific loadout
	{ "GetCustomRaceDefaults",			GetCustomRaceDefaults },
	{ "GetCustomRaceLimits",			GetCustomRaceLimits },
	{ "GetCustomRaceSettings",			GetCustomRaceSettings },
	{ "GetCustomRaceCities",			GetCustomRaceCities },
	{ "GetCustomRaceRoutes",			GetCustomRaceRoutes },

	{"GetCurrentSystemTimeAsU32",		GetCurrentSystemTimeAsU32  },	//Get the current system time a s aU32 (used for seeding random etc when selecting events in SS world tour etc)

	{ "GetEliminationData",				GetEliminationData },
	{ "IsPointsTarget",					IsPointsTarget },

	{ "GetPlayerLoadouts",				GetPlayerLoadouts },
	{ "SetEventPresence",				SetEventPresence },
	{ "SetDefaultPresence",				SetDefaultPresence },

	{ "GetSplitscreenRouteThumbnail",	GetSplitscreenRouteThumbnail },

	{ "GetGarageCarByID",				GetGarageCarByID },				
	{ "SetGarageCarByID",				SetGarageCarByID },
	{ "PostRankedUp",					PostRankedUp },
	{ "GetRaceSummary",					GetRaceSummary },

	{ "SetupLevelState",				SetupLevelState },
	{ "CyclePlaylist",					CyclePlaylist },
	{ "IsLastPlaylistRace",				IsLastPlaylistRace },

	{ "StartRaceVoting",				StartRaceVoting },
	{ "GetVoteRaceIndicies",			GetVoteRaceIndicies },
	{ "GetVotingValues",				GetVotingValues },
	{ "GetVotingResults",				GetVotingResults },
	{ "CalculateLocalVoteResults",		CalculateLocalVoteResults },
	{ "CastLocalVote",					CastLocalVote },
	{ "IsVotingEnabled",				IsVotingEnabled },

	{ "IsChampionshipMode",				IsChampionshipMode },
	{ "ResetChampionship",				ResetChampionship },

	{ "GetRaceHistory",					GetRaceHistory },
	{ "GetRaceHistoryVehicle",			GetRaceHistoryVehicle },
	{ "GetRaceHistoryAwardIconNames",	GetRaceHistoryAwardIconNames },
	{ "LookingAtRaceHistory",			LookingAtRaceHistory },
	{ "POBGameInfo",					POBGameInfo },
	{ "ClearPOBGameInfo",				ClearPOBGameInfo },
	{ "POBUpcomingUnlocksInfo",			POBUpcomingUnlocksInfo },
	{ "POBGetPlaylistUnlocks",			POBGetPlaylistUnlocks },
	{ "POBGetCurrentRank",				POBGetCurrentRank },
	{ "POBProgressionForRank",			POBProgressionForRank },
	{ "IsElimination",					IsElimination },
	{ "IsEliminated",					IsEliminated },

	{ "GetChampionshipStandings",		GetChampionshipStandings},

	{ "SwitchSpectateCam",				SwitchSpectateCam},
	{ "ChallengeTypeIsRace",			ChallengeTypeIsRace},
	{ "ChallengeTypeIsWinner",			ChallengeTypeIsWinner},

	//race summary lobby screen
	{ "GetLobbyRaceResults",						GetLobbyRaceResults },
	{ "GetLobbyRaceResultsLegendLevel",				GetLobbyRaceResultsLegendLevel },
	{ "GetLobbyRaceResultsRank",					GetLobbyRaceResultsRank },
	{ "GetLobbyRaceResultsIsMe",					GetLobbyRaceResultsIsMe },
	{ "GetLobbyRaceResultsTeam",					GetLobbyRaceResultsTeam },
	{ "GetLobbyRaceResultsNemesisJoinId",			GetLobbyRaceResultsNemesisJoinId },
	{ "GetLobbyRaceResultsPreyJoinId",				GetLobbyRaceResultsPreyJoinId },
	{ "GetRaceAwardsInLobby",						GetRaceAwardsInLobby },
	{ "GetLobbyRaceResultsGameModeIsDestruction",	GetLobbyRaceResultsGameModeIsDestruction },
	{ "GetRaceChallengesCompleteFans",				GetRaceChallengesCompleteFans },
	{ "GetProgressionForRank",						GetProgressionForRank },
	{ "GetRaceResultsValid",						GetRaceResultsValid },
	{ "IsNemesisValid",								IsNemesisValid },
	{ "IsPreyValid",								IsPreyValid },
	//~race summary lobby screen

	//Data gathering
	{ "PostFlaggedDataEvents",						PostFlaggedDataEvents },

	// end of list
	{ NULL, NULL }
};

LUALIB_API int luaopen_multiplayer( lua_State* pState )
{
	luaL_openlib( pState, "Multiplayer", multiplayer, 0 );
	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
