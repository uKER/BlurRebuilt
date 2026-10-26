#include "stdafx.h"
#include "MultiplayerHelpers.h"

#include <ui/Game/UILua.h>
#include "ui/Game/Multiplayer/MultiplayerDefines.h"

#include "GameLogic/GameData/GameData.h"
#include "GameLogic/GameData/MultiplayerData.h"

#include <Network/Core/NetScoreboards.h>
#include <Network/Player/NetPlayerManager.h>

#include <GameLogic/Profile/MultiplayerProfileStorage.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

Debug::Float g_UIDamageMultiplayerHigh( "Multiplayer.DamageMultiplayerHigh", 4.0f, Debug::Range( 0.0f, 10.0f ) );

void CreateMpLuaEnums( lua_State* pState )
{
	lua_pushstring( pState, "StorageReaderState" );
	lua_newtable( pState );
			Lua::AddDataMember(pState,"InProgress",				Game::E_MPS_Pending );				
			Lua::AddDataMember(pState,"Failed",					Game::E_MPS_RequestFailed );				
			Lua::AddDataMember(pState,"Successful",				Game::E_MPS_Ok );				
			Lua::AddDataMember(pState,"FileDoesNotExist",		Game::E_MPS_DoesNotExist );				
			Lua::AddDataMember(pState,"Corrupted",				Game::E_MPS_Corrupted );				
			Lua::AddDataMember(pState,"VersionError",			Game::E_MPS_VersionMismatch );				
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "StorageWriterState" );
	lua_newtable( pState );
			Lua::AddDataMember(pState,"InProgress",				Game::E_MPS_Pending );				
			Lua::AddDataMember(pState,"Failed",					Game::E_MPS_RequestFailed );				
			Lua::AddDataMember(pState,"Successful",				Game::E_MPS_Ok );				
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpRaceType" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Racing",		E_RaceType_Racing );
		Lua::AddDataMember( pState, "Elimination",	E_RaceType_Elimination );
		Lua::AddDataMember( pState, "Points",		E_RaceType_Points );
		Lua::AddDataMember( pState, "Destruction",	E_RaceType_Destruction );
		Lua::AddDataMember( pState, "Hardcore",		E_RaceType_Hardcore );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpPickup" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Nitro",		E_Pickup_Nitro );
		Lua::AddDataMember( pState, "Shock",		E_Pickup_Shock );
		Lua::AddDataMember( pState, "Barge",		E_Pickup_Barge );
		Lua::AddDataMember( pState, "Mine",			E_Pickup_Mine );
		Lua::AddDataMember( pState, "Shunt",		E_Pickup_Shunt );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpVehicleClass" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "ClassA",		E_VehicleClass_A );
		Lua::AddDataMember( pState, "ClassB",		E_VehicleClass_B );
		Lua::AddDataMember( pState, "ClassC",		E_VehicleClass_C );
		Lua::AddDataMember( pState, "ClassD",		E_VehicleClass_D );
		Lua::AddDataMember( pState, "ClassE",		E_VehicleClass_E );
		Lua::AddDataMember( pState, "Any",			E_VehicleClass_Any );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpScoreboard" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "TotalWins",		RacerNet::eMpScoreboardTotalWins );
		Lua::AddDataMember( pState, "PerkRatio",		RacerNet::eMpScoreboardPerkRatio );
		Lua::AddDataMember( pState, "TotalFans",		RacerNet::eMpScoreboardTotalFans );
		Lua::AddDataMember( pState, "Legends",			RacerNet::eMpScoreboardLegends );
		Lua::AddDataMember( pState, "Lobby",			RacerNet::eMpScoreboardLobby );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpRaceMode" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Racing",			Game::E_RaceMode_Multiplayer_Racing );
		Lua::AddDataMember( pState, "Elimination",		Game::E_RaceMode_Multiplayer_Elimination );
		Lua::AddDataMember( pState, "Points",			Game::E_RaceMode_Multiplayer_Points );
		Lua::AddDataMember( pState, "Destruction",		Game::E_RaceMode_Multiplayer_Destruction );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpEventFilter" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Bizarre",			E_EventFilter_Bizarre );
		Lua::AddDataMember( pState, "QuickRace",		E_EventFilter_QuickRace );
		Lua::AddDataMember( pState, "UserCreated",		E_EventFilter_UserCreated );
		Lua::AddDataMember( pState, "Favourties",		E_EventFilter_Favourties );
		Lua::AddDataMember( pState, "RecentlyPlayed",	E_EventFilter_RecentlyPlayed );
		Lua::AddDataMember( pState, "HighestRated",		E_EventFilter_HighestRated );
		Lua::AddDataMember( pState, "Newest",			E_EventFilter_Newest );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpLobbyState");
	lua_newtable( pState );
		Lua::AddDataMember( pState, "FindingPlayers",	E_LobbyState_FindingPlayers );
		Lua::AddDataMember( pState, "Waiting",			E_LobbyState_Waiting );
		Lua::AddDataMember( pState, "WaitingToVote",	E_LobbyState_WaitingToVote );
		Lua::AddDataMember( pState, "Voting",			E_LobbyState_Voting );
		Lua::AddDataMember( pState, "Countdown",		E_LobbyState_Countdown );
		Lua::AddDataMember( pState, "TeamCountdown",	E_LobbyState_TeamCountdown );
		Lua::AddDataMember( pState, "TeamBalanceWait",	E_LobbyState_TeamBalanceWait );
		Lua::AddDataMember( pState, "WaitingForHost",	E_LobbyState_WaitingForHost );
		Lua::AddDataMember( pState, "Ready",			E_LobbyState_Ready );
	lua_rawset( pState, -3 );	

	lua_pushstring( pState, "MpDamageMode" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Off",			E_DamageMode_Off );
		Lua::AddDataMember( pState, "Low",			E_DamageMode_Low );
		Lua::AddDataMember( pState, "Medium",		E_DamageMode_Medium );
		Lua::AddDataMember( pState, "High",			E_DamageMode_High );
		Lua::AddDataMember( pState, "OneHit",		E_DamageMode_OneHit );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpAbilitySlot" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Slot1",		E_MultiplayerAbilityCategory_Slot1 );
		Lua::AddDataMember( pState, "Slot2",		E_MultiplayerAbilityCategory_Slot2 );
		Lua::AddDataMember( pState, "Slot3",		E_MultiplayerAbilityCategory_Slot3 );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpChallengePack" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Ace",			E_ChallengePack_Ace );
		Lua::AddDataMember( pState, "Car",			E_ChallengePack_Car );
		Lua::AddDataMember( pState, "Daily",		E_ChallengePack_Daily );
		Lua::AddDataMember( pState, "Demolition",	E_ChallengePack_Demolition );
		Lua::AddDataMember( pState, "DriversEd",	E_ChallengePack_DriversED );
		Lua::AddDataMember( pState, "Specialist",	E_ChallengePack_Specialist );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpChallengeRewards" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "HealthSlot",	E_MultiplayerVehicleUpgrades_HealthSlot );
		Lua::AddDataMember( pState, "BonusFans",	E_MultiplayerVehicleUpgrades_FanBonus );
		Lua::AddDataMember( pState, "SpeedBoost",	E_MultiplayerVehicleUpgrades_SpeedBoost );
		Lua::AddDataMember( pState, "Pearlescent",	E_MultiplayerVehicleUpgrades_Pearlescent );
		Lua::AddDataMember( pState, "Metallic",		E_MultiplayerVehicleUpgrades_Metallic );
		Lua::AddDataMember( pState, "Candy",		E_MultiplayerVehicleUpgrades_Candy );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MPFeatures" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Default", E_MultiplayerFeature_Default );
		Lua::AddDataMember( pState, "ModShop", E_MultiplayerFeature_ModShop );
		Lua::AddDataMember( pState, "ResprayCar", E_MultiplayerFeature_ResprayCar );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MatchingRegion" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Any",			RacerNet::eMatchingRegionUnknown );
		Lua::AddDataMember( pState, "Asia",			RacerNet::eMatchingRegionAsia );
		Lua::AddDataMember( pState, "Australia",	RacerNet::eMatchingRegionAunz );
		Lua::AddDataMember( pState, "Europe",		RacerNet::eMatchingRegionEurope );
		Lua::AddDataMember( pState, "NorthAmerica", RacerNet::eMatchingRegionNorthAmerica );
		Lua::AddDataMember( pState, "SouthAmerica", RacerNet::eMatchingRegionSouthAmerica );
	lua_rawset( pState, -3 );
	
	lua_pushstring( pState, "MatchingRule" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Any",			RacerNet::eMatchingRuleAny );
		Lua::AddDataMember( pState, "Preferred",	RacerNet::eMatchingRulePreferred );
		Lua::AddDataMember( pState, "Enforced",		RacerNet::eMatchingRuleEnforced );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "NatType" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Open",			RacerNet::eNatOpen );
		Lua::AddDataMember( pState, "Moderate",		RacerNet::eNatModerate );
		Lua::AddDataMember( pState, "Strict",		RacerNet::eNatStrict );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpUpgrades" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Off",			E_CarUpgrades_Off );
		Lua::AddDataMember( pState, "On",			E_CarUpgrades_On );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpAI" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Off",			E_AiOff );
		Lua::AddDataMember( pState, "Easy",			E_AiEasy );
		Lua::AddDataMember( pState, "Medium",		E_AiMedium );
		Lua::AddDataMember( pState, "Hard",			E_AiHard );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpMods" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Off",			E_PowerupMods_Off );
		Lua::AddDataMember( pState, "On",			E_PowerupMods_On );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpRespawn" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Off",			E_LastManStanding_Off );
		Lua::AddDataMember( pState, "On",			E_LastManStanding_On );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpHandicap" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Off",			E_HandicapOff );
		Lua::AddDataMember( pState, "Low",			E_HandicapLow );
		Lua::AddDataMember( pState, "Medium",		E_HandicapMedium );
		Lua::AddDataMember( pState, "High",			E_HandicapHigh );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpPowerupRespawnMode" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Normal",		E_PowerupRespawnMode_Normal );
		Lua::AddDataMember( pState, "Long",			E_PowerupRespawnMode_Long );
		Lua::AddDataMember( pState, "VeryLong",		E_PowerupRespawnMode_VeryLong );
		Lua::AddDataMember( pState, "AlwaysOn",		E_PowerupRespawnMode_AlwaysOn );
		Lua::AddDataMember( pState, "None",			E_PowerupRespawnMode_None );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "MpUpgradeChallenges" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Races",	E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES );
		Lua::AddDataMember( pState, "Winner",	E_MULTIPLAYER_CHALLENGE_TYPE_WINNER );
	lua_rawset( pState, -3 );
}

Game::E_RaceMode ConvertUiToGame_RaceMode( E_RaceType p_Event )
{
	Game::E_RaceMode raceMode = Game::E_RaceMode_Nothing;
	switch(p_Event)
	{
	case E_RaceType_Racing:
	case E_RaceType_Hardcore:
		raceMode = Game::E_RaceMode_Multiplayer_Racing;
		break;

	case E_RaceType_Elimination:
		raceMode = Game::E_RaceMode_Multiplayer_Elimination;
		break;

	case E_RaceType_Points:
		raceMode = Game::E_RaceMode_Multiplayer_Points;
		break;

	case E_RaceType_Destruction:
		raceMode = Game::E_RaceMode_Multiplayer_Destruction;
		break;
	}
	return raceMode;
}

E_PICKUP_TYPE ConvertUiToGame_Pickup( E_Pickup p_Pickup )
{
	E_PICKUP_TYPE pickup = E_PICKUP_TYPE_NITROUS;
	switch(p_Pickup)
	{
	case E_Pickup_Nitro:
		pickup = E_PICKUP_TYPE_NITROUS;
		break;
	case E_Pickup_Shock:
		pickup = E_PICKUP_TYPE_SHOCK;
		break;
	case E_Pickup_Barge:
		pickup = E_PICKUP_TYPE_BARGE;
		break;
	case E_Pickup_Mine:
		pickup = E_PICKUP_TYPE_BOMB;
		break;
	case E_Pickup_Shunt:
		pickup = E_PICKUP_TYPE_SHUNT;
		break;
	case E_Pickup_Bolt:
		pickup = E_PICKUP_TYPE_BLINDFIRE;
		break;
	case E_Pickup_Repair:
		pickup = E_PICKUP_TYPE_REPAIR;
		break;
	case E_Pickup_Shield:
		pickup = E_PICKUP_TYPE_SHIELD;
		break;

	default :
		BIZ_ASSERT_FAIL( "Invalid pickup type" );
	}
	return pickup;
}

E_DAMAGE_MODE ConvertUiToGame_DamageMode( E_DamageMode eDamageMode )
{
	E_DAMAGE_MODE mode = E_DAMAGE_MODE_NORMAL;
	switch(eDamageMode)
	{
	case E_DamageMode_Off:
		mode = E_DAMAGE_MODE_INVULNERABLE;
		break;
	case E_DamageMode_Low:
		mode = E_DAMAGE_MODE_MULTIPLIER;
		break;
	case E_DamageMode_Medium:
		mode = E_DAMAGE_MODE_MULTIPLIER;
		break;
	case E_DamageMode_High:
		mode = E_DAMAGE_MODE_MULTIPLIER;
		break;
	case E_DamageMode_OneHit:
		mode = E_DAMAGE_MODE_ONE_SHOT_KILL;
		break;
	}
	return mode;
}

F32 ConvertUiToGame_DamageMultiplier( E_DamageMode eDamageMode )
{
	if ( eDamageMode == E_DamageMode_High )
		return C_MultiplayerData::Instance().m_pConfig->m_DamageMultiplierValues.m_High;
	else if( eDamageMode == E_DamageMode_Medium )
		return C_MultiplayerData::Instance().m_pConfig->m_DamageMultiplierValues.m_Medium;
	else if( eDamageMode == E_DamageMode_Low )
		return C_MultiplayerData::Instance().m_pConfig->m_DamageMultiplierValues.m_Low;

	return 0.0f;
}

const E_PICKUP_RESPAWN_TYPE ConvertUiToGame_PowerUpRespawnMode(const enum E_PowerupRespawnMode ePowerupRespawnMode)
{
	switch(ePowerupRespawnMode)
	{
	case E_PowerupRespawnMode_Normal:
		{
			return E_PICKUP_RESPAWN_TYPE_NORMAL;
			break;
		}
	case E_PowerupRespawnMode_AlwaysOn:
		{
			return E_PICKUP_RESPAWN_TYPE_ALWAYS_ON;
			break;
		}
	case E_PowerupRespawnMode_Long:
		{
			return E_PICKUP_RESPAWN_TYPE_LONG;
			break;
		}
	case E_PowerupRespawnMode_VeryLong:
		{
			return E_PICKUP_RESPAWN_TYPE_VERY_LONG;
			break;
		}
	case E_PowerupRespawnMode_None:
		{
			return E_PICKUP_RESPAWN_TYPE_NONE;
			break;
		}
	};

	BIZ_ASSERT_MSG(false, "ConvertUiToGame_RespawnMode() > Failed to handle new form of respawn mode");
	return E_PICKUP_RESPAWN_TYPE_NONE;
};


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
