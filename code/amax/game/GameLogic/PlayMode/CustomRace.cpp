#include "stdafx.h"
#include "GameLogic/PlayMode/CustomRace.h"

#include "Ui/game/Multiplayer/MultiplayerHelpers.h"

#include "GameLogic/Structures/LevelState.h"
#include "Network/Misc/NetRaceSettings.h"
#include <Ui\UiHelpers.h>
#include "GameLogic\RaceLogicCore\Core\RaceLogicCore.h"
#include "GameLogic\RaceLogicCore\Database\RLDatabase.h"

extern Debug::Logger &g_MpLog;

namespace Game
{

#define MP_CUSTOM_RACE_MIN_PLAYERS		2
#define MP_CUSTOM_RACE_MAX_PLAYERS		20
#define MP_CUSTOM_RACE_FAN_MULTIPLIER	1.0f
#define MP_CUSTOM_RACE_TEAMS			false

C_CustomRace::C_CustomRace()
{
}

void C_CustomRace::Initialise()
{
}

void C_CustomRace::SetupNetRaceSettings( RacerNet::NetRaceSettings& settings )
{
	settings.Clear();
	settings.m_PackVersion			= C_MultiplayerData::Instance().m_pConfig->m_PackVersion;
	settings.m_EventId				= 0;
	settings.m_CustomRace			= true;
	settings.m_Championship			= false;

	settings.m_FanMultiplier 		= MP_CUSTOM_RACE_FAN_MULTIPLIER;
	settings.m_Teams				= MP_CUSTOM_RACE_TEAMS;

	// Copy race data
	settings.m_Race.Clear();
	settings.m_Race.m_xtCityId		= m_CityID;
	settings.m_Race.m_xtRouteId		= m_RouteID;
	settings.m_Race.m_MinPlayers	= MP_CUSTOM_RACE_MIN_PLAYERS;
	settings.m_Race.m_MaxPlayers	= MP_CUSTOM_RACE_MAX_PLAYERS;

	settings.m_Race.m_Type			= m_RaceMode;
	settings.m_Race.m_NumLaps		= m_LapLimit;
	settings.m_Race.m_TimeLimit		= m_TimeLimit;
	settings.m_Race.m_Class			= m_CarClass;

	settings.m_Race.m_DamageMode	= m_DamageMode;
	settings.m_Race.m_Respawn		= m_Respawn;
	settings.m_Race.m_Timeout		= m_Timeout;
	settings.m_Race.m_Mods			= m_Mods;
	settings.m_Race.m_Upgrades		= m_Upgrades;
	settings.m_Race.m_LockToHostCar = m_LockToHostCar;
	settings.m_Race.m_AI			= m_AI;
	settings.m_Race.m_AIMaxPadding	= m_AIMaxPadding;
	settings.m_Race.m_Handicap		= m_Handicap;

	settings.m_Race.m_Pickups			 = m_Powerups;
	settings.m_Race.m_RandomPickups		 = m_RandomPowerups;
	settings.m_Race.m_PickupsRespawnMode = m_PowerupRespawnMode;

	LOGINFO( g_MpLog, "Custom Race successfully filled out NetRaceSettings" );
}

void C_CustomRace::SetupLevelState( C_LevelState& levelState )
{
	// Playlist data
	levelState.mFanMultiplier						= MP_CUSTOM_RACE_FAN_MULTIPLIER;

	// Race data
	levelState.mRaceMode							= UI::ConvertUiToGame_RaceMode((UI::E_RaceType)m_RaceMode);
	levelState.mUiRaceType							= (UI::E_RaceType)m_RaceMode;
	levelState.mCity								= m_CityID;
	levelState.mRoute								= m_RouteID;
	levelState.mLaps								= m_LapLimit;
	levelState.mTimeLimit							= m_TimeLimit;

	levelState.mDamageMode							= UI::ConvertUiToGame_DamageMode((UI::E_DamageMode)m_DamageMode);
	levelState.mDamageMultiplier					= UI::ConvertUiToGame_DamageMultiplier((UI::E_DamageMode)m_DamageMode);
	levelState.mWreckedBehaviour					= ( m_Respawn == UI::E_LastManStanding_On ) ? E_AMAXWRECKED_RESPAWN : E_AMAXWRECKED_ELIMINATED;
	
	// Mp specific settings
	LevelStateMultiplayerSettings& multiplayerSettings = levelState.GetMultiplayerSettings();
	multiplayerSettings.mAIMode						= (UI::E_AiMode)m_AI;
	multiplayerSettings.mUpgrades					= (UI::E_CarUpgrades)m_Upgrades;
	multiplayerSettings.mHandicap					= (UI::E_Handicap)m_Handicap;
	multiplayerSettings.mModSettings				= (UI::E_PowerupMods)m_Mods;
	multiplayerSettings.mAlowTimeout				= ( m_Timeout ) ? true : false;
	multiplayerSettings.mLockToHostCar				= ( m_LockToHostCar ) ? true : false;

	// Powerup setup
	levelState.mPerkDroppingWhenUsed				= false;
	levelState.mPerksRespawn						= !levelState.mPerkDroppingWhenUsed; // If we're dropping pickups when they're fired, we don't want them to respawn (and vice versa)
	levelState.mPickupDisableTime					= 5.f;			//Bug 12525 - Default network pickup disable time...fairly horrible hard-coding sorry
	levelState.mPerksRespawnRandomType				= ( m_RandomPowerups ) ? true : false;
	levelState.mPickupRespawnType					= UI::ConvertUiToGame_PowerUpRespawnMode( (UI::E_PowerupRespawnMode)m_PowerupRespawnMode );

	// Turn powerups on or off
	Game::Actors::RaceLogicCore* pRLC = UI::GetRLC();
	bool pickupsEnabled = false;
	for(int i = 0; i < UI::E_Pickup_Max; i++)
	{
		E_PICKUP_TYPE pickupType = UI::ConvertUiToGame_Pickup((UI::E_Pickup)i);
		Game::LevelPickupProbability levelPickupProb(pickupType, 0.f);

		if( (m_Powerups & (1 << i)) && m_RaceMode != UI::E_RaceType_Hardcore )
		{
			const RLPickupType* pPickup	= pRLC->GetPickupType(pickupType);
			levelPickupProb.probability = pPickup->mBaseProbability;
			pickupsEnabled = true;
		}

		levelState.mPickupProbabilities.PushBack(levelPickupProb);
	}

	levelState.mPickupsEnabled = pickupsEnabled;

	LOGINFO( g_MpLog, "Custom Race successfully filled out LevelState" );
}


void C_CustomRace::SetupRace( RacerNet::NetRaceSettings& settings )
{
	m_CityID				= settings.m_Race.m_xtCityId;
	m_RouteID				= settings.m_Race.m_xtRouteId;
	m_MaxPlayers			= settings.m_Race.m_MaxPlayers;

	m_RaceMode				= settings.m_Race.m_Type;
	m_LapLimit				= settings.m_Race.m_NumLaps;
	m_TimeLimit				= settings.m_Race.m_TimeLimit;
	m_CarClass				= settings.m_Race.m_Class;

	m_DamageMode			= settings.m_Race.m_DamageMode;
	m_Respawn				= settings.m_Race.m_Respawn;	
	m_Timeout				= settings.m_Race.m_Timeout;
	m_Mods					= settings.m_Race.m_Mods;	
	m_Upgrades				= settings.m_Race.m_Upgrades;
	m_LockToHostCar 		= settings.m_Race.m_LockToHostCar;
	m_AI					= settings.m_Race.m_AI;
	m_AIMaxPadding			= settings.m_Race.m_AIMaxPadding;
	m_Handicap				= settings.m_Race.m_Handicap;

	m_Powerups				= settings.m_Race.m_Pickups;
	m_RandomPowerups		= settings.m_Race.m_RandomPickups;
	m_PowerupRespawnMode	= settings.m_Race.m_PickupsRespawnMode;

	LOGINFO( g_MpLog, "Custom Race settings updated from NetRaceSettings" );
}

void C_CustomRace::SetupRace( lua_State* pState )
{
	BIZ_ASSERT( pState );
	if ( !pState )
		return;

	// Check that a table has come
	if ( lua_type( pState, 1 ) != LUA_TTABLE )
		return;

	Lua::GetDataMember( pState, "game_mode",	m_RaceMode );
	Lua::GetDataMember( pState, "city_id",		m_CityID );
	Lua::GetDataMember( pState, "route_id",		m_RouteID );
	Lua::GetDataMember( pState, "laps",			m_LapLimit );
	Lua::GetDataMember( pState, "time",			m_TimeLimit );
	Lua::GetDataMember( pState, "class",		m_CarClass );
	Lua::GetDataMember( pState, "damage",		m_DamageMode );
	Lua::GetDataMember( pState, "respawn",		m_Respawn );
	Lua::GetDataMember( pState, "timeout",		m_Timeout );
	Lua::GetDataMember( pState, "mods",			m_Mods );
	Lua::GetDataMember( pState, "upgrades",		m_Upgrades );
	Lua::GetDataMember( pState, "lock_host",	m_LockToHostCar );
	Lua::GetDataMember( pState, "ai",			m_AI );
	Lua::GetDataMember( pState, "ai_padding",	m_AIMaxPadding );
	Lua::GetDataMember( pState, "handicap",		m_Handicap );

	Lua::GetDataMember( pState, "random_powerups",	m_RandomPowerups );
	Lua::GetDataMember( pState, "powerup_respawn", 	m_PowerupRespawnMode );

	if( Lua::DataMemberType( pState, "powerups") == LUA_TTABLE )
	{
		m_Powerups = 0;
		UI::UiXt::C_PointerMap pointerMap;
		lua_pushstring( pState, "powerups" );
		lua_gettable( pState, -2 );
			UI::UiXt::GetLuaFromDefImpl( pState, XtGetType<UI::E_Pickup>(), (U8 *)&m_Powerups, m_Allocator, pointerMap, NULL );
		lua_pop( pState, 1 );
	}

	// Some safetly validations due to crazy conbinations causing crashes and strange behaviour
	switch( m_RaceMode )
	{
		case UI::E_RaceType_Racing : 
		{
			m_TimeLimit = 0;
			break;
		}
		case UI::E_RaceType_Destruction : 
		{
			m_LapLimit = 0;		
			m_Powerups = ( m_Powerups & ~( 1 << UI::E_Pickup_Shock ) ); // Force shock off
			break;
		}
	}

	LOGINFO( g_MpLog, "Custom Race settings updated from lua" );
}

void C_CustomRace::SendToLua( lua_State* pState ) const
{
	BIZ_ASSERT( pState );
	if ( !pState )
		return;

	Lua::AddDataMember( pState, "game_mode",	m_RaceMode );
	Lua::AddDataMember( pState, "city_id",		m_CityID );
	Lua::AddDataMember( pState, "route_id",		m_RouteID );
	Lua::AddDataMember( pState, "laps",			m_LapLimit );
	Lua::AddDataMember( pState, "time",			m_TimeLimit );
	Lua::AddDataMember( pState, "class",		m_CarClass );
	Lua::AddDataMember( pState, "damage",		m_DamageMode );
	Lua::AddDataMember( pState, "respawn",		m_Respawn );
	Lua::AddDataMember( pState, "timeout",		m_Timeout );
	Lua::AddDataMember( pState, "mods",			m_Mods );
	Lua::AddDataMember( pState, "upgrades",		m_Upgrades );
	Lua::AddDataMember( pState, "lock_host",	m_LockToHostCar );
	Lua::AddDataMember( pState, "ai",			m_AI );
	Lua::AddDataMember( pState, "ai_padding",	m_AIMaxPadding );
	Lua::AddDataMember( pState, "handicap",		m_Handicap );

	Lua::AddDataMember( pState, "random_powerups",	m_RandomPowerups );
	Lua::AddDataMember( pState, "powerup_respawn", 	m_PowerupRespawnMode );

	UI::UiXt::C_PointerMap pointerMap;
	lua_pushstring( pState, "powerups" );
		UI::UiXt::MakeLuaFromDefImpl( pState, XtGetType<UI::E_Pickup>(), (const U8 *)&m_Powerups, pointerMap, NULL );
	lua_rawset( pState, -3 );
}

};// namespace Game
