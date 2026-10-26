#include "stdafx.h"
#include "GameLogic/PlayMode/Playlist.h"

#include "Ui/Game/Multiplayer/MultiplayerCache.h"
#include "Ui/game/Multiplayer/MultiplayerHelpers.h"

#include "GameLogic/RaceLogicCore/Database/RLDatabase.h"
#include "GameLogic/Structures/LevelState.h"
#include <GameLogic\StickerManager.h>

#include "Network/Misc/NetRaceSettings.h"
#include <Ui\UiHelpers.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

extern Debug::Logger &g_MpLog;

namespace Game
{

C_Playlist::C_Playlist()
:	m_RaceIndex( 0 ),
	m_Seed( 0 )
{
}

void C_Playlist::Initialise()
{
	m_RaceIndex = 0; // Reset
	m_Seed = 0;
}

const MultiplayerEvent*	C_Playlist::GetPlaylistData() const
{
	const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
	BIZ_ASSERT( pEvent );

	return pEvent;
}

const MultiplayerRace* C_Playlist::GetRaceData( const S16 sRaceIndex ) const
{
	const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
	if ( !pEvent )
		return NULL;

	if ( sRaceIndex < -1 || sRaceIndex >= m_Races.Size() )
		return NULL;

	return ( sRaceIndex != -1 ) ? pEvent->races[m_Races[sRaceIndex].m_Index] : pEvent->races[m_Races[m_RaceIndex].m_Index];
}

void C_Playlist::Shuffle( const U32 uSeed )
{
	m_Random.Seed( uSeed );
	m_Seed = uSeed;

	const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
	BIZ_ASSERT_MSG( pEvent, "No event" );
	if ( !pEvent )
		return;

	BIZ_ASSERT_MSG( pEvent->races.count, "No event races" );
	if ( pEvent->races.count <= 0 )
		return;

	m_IsChampionship = pEvent->is_championship;
	m_ChampionshipInProgress = false;

	m_Races.Clear();
	m_Races.Reserve( pEvent->races.count );

	if (!m_IsChampionship)
	{
		GenerateSingleRaces(pEvent);
	}
	else
	{
		GenerateChampionship(pEvent);
	}

	LOGINFO( g_MpLog, "Playlist shuffled %d races using the seed %u", m_Races.Size(), uSeed );
}

void C_Playlist::SetRaceIndex( const S16 sIndex )
{
	BIZ_ASSERT( sIndex >= 0 && sIndex < m_Races.Size() && m_Races.Size() );

	m_RaceIndex = sIndex;
	LOGINFO( g_MpLog, "Playlist race index changed to %d", m_RaceIndex );
}

S16 C_Playlist::GetRaceDataIndex( const S16 sRaceIndex ) const
{
	BIZ_ASSERT( sRaceIndex < m_Races.Size() && m_Races.Size() );

	return ( sRaceIndex != -1 ) ? m_Races[sRaceIndex].m_Index : m_Races[m_RaceIndex].m_Index;
}

UI::E_VehicleClass C_Playlist::GetCarClass( const S16 sRaceIndex ) const
{
	BIZ_ASSERT( sRaceIndex < m_Races.Size() && m_Races.Size() );

	return ( sRaceIndex != -1 ) ? m_Races[sRaceIndex].m_CarClass : m_Races[m_RaceIndex].m_CarClass;
}

void C_Playlist::ResetCycle( const S32 sNumPlayers )
{
	const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();

	m_RaceIndex = 0;
	if ( !IsRaceValid( pEvent, m_Races[m_RaceIndex].m_Index, sNumPlayers ) )
		m_RaceIndex = FindNextValidRace( sNumPlayers, m_RaceIndex );

	LOGINFO( g_MpLog, "Playlist cycle reset to %d with %d active player", m_RaceIndex, sNumPlayers );
}

S16 C_Playlist::FindNextValidRace( const S32 sNumPlayers, const S16 sCurrentIndex )
{
	const MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();

	// Find the next valid race for us to play
	S16 index = sCurrentIndex;
	bool finished = false;

	while( !finished )
	{
		index++;
		if ( index >= (S16)pEvent->races.count ||
			(m_IsChampionship && index >= pEvent->championship_length))
		{
			index = 0;
		}

		// If we dont have enough players to play the event then the lobby will wait until the requirements are met, 
		// but this would get into an infinite loop so we take the min off to give the range of (0 - X) so that a valid first race is found.
		finished = IsRaceValid( pEvent, m_Races[index].m_Index, sNumPlayers );
	}

	return index;
}

bool C_Playlist::IsRaceValid( const MultiplayerEvent* pEvent, const S16 sRaceDataIndex, const S32 sNumPlayers )
{
	const S32 minPlayers = ( sNumPlayers < pEvent->min_players ) ? pEvent->races[sRaceDataIndex]->min_players - pEvent->min_players : pEvent->races[sRaceDataIndex]->min_players;

	// Check the race requirements
	return ( sNumPlayers >= minPlayers && sNumPlayers <= pEvent->races[sRaceDataIndex]->max_players );
}

UI::E_RaceType C_Playlist::GetRaceMode() const
{
	return GetRaceData()->type;
}

U32	C_Playlist::GetCity() const
{
	return GetRaceData()->route->city->ID;
}

U32	C_Playlist::GetRoute() const
{
	return GetRaceData()->route->ID;
}

S8 C_Playlist::GetLapLimit() const
{
	return GetRaceData()->laps;
}

S32 C_Playlist::GetTimeLimit() const
{
	return GetRaceData()->time;
}

void C_Playlist::SetupNetRaceSettings( RacerNet::NetRaceSettings& settings )
{
	// Get current playlist
	const MultiplayerEvent* pPlaylist = UI::C_MultiplayerCache::Instance().CurrentEvent();
	if ( !pPlaylist )
	{
		BIZ_ASSERT_FAIL( "Playlist not set" );
		return;
	}

	// Get current race data
	const MultiplayerRace* pRace = pPlaylist->races[GetRaceDataIndex()];
	if ( !pRace )
	{
		BIZ_ASSERT_FAIL( "Invalid playlist race" );
		return;
	}

	// Copy playlist data
	settings.m_PackVersion			= C_MultiplayerData::Instance().m_pConfig->m_PackVersion;
	settings.m_EventId				= pPlaylist->ID;
	settings.m_CustomRace			= false;
	settings.m_Name					= pPlaylist->name.id;
	settings.m_Description			= pPlaylist->desc.id;
	settings.m_MinDrivers 			= pPlaylist->min_players;
	settings.m_MaxDrivers 			= pPlaylist->max_players;
	settings.m_FanMultiplier 		= pPlaylist->fan_multiplier;
	settings.m_Teams				= pPlaylist->teams;
	settings.m_Misc					= pPlaylist->misc;
	settings.m_Championship			= pPlaylist->is_championship;
	settings.m_PlaylistSeed			= m_Seed;
	settings.m_PlaylistIndex		= m_RaceIndex;

	// Copy race data
	settings.m_Race.Clear();
	settings.m_Race.m_xtCityId		= pRace->route->city->ID;
	settings.m_Race.m_xtRouteId		= pRace->route->ID;
	settings.m_Race.m_MinPlayers	= pRace->min_players;
	settings.m_Race.m_MaxPlayers	= pRace->max_players;

	settings.m_Race.m_Type			= pRace->type;
	settings.m_Race.m_NumLaps		= pRace->laps;
	settings.m_Race.m_TimeLimit		= pRace->time;
	settings.m_Race.m_Class			= GetCarClass();

	settings.m_Race.m_DamageMode	= pRace->damage_mode;
	settings.m_Race.m_Pickups		= pRace->powerups;
	settings.m_Race.m_Respawn		= pRace->lastManStanding;
	settings.m_Race.m_Timeout		= pRace->timeout;
	settings.m_Race.m_Mods			= pRace->powerupMods;
	settings.m_Race.m_Upgrades		= pRace->carUpgrades;
	settings.m_Race.m_LockToHostCar = pRace->lock_to_host_car;
	settings.m_Race.m_AI			= pRace->aiMode;
	settings.m_Race.m_Handicap		= pRace->handicap;

	LOGINFO( g_MpLog, "Playlist successfully filled out NetRaceSettings" );
}

void C_Playlist::SetupLevelState( C_LevelState& levelState )
{
	// Get current playlist
	const MultiplayerEvent* pPlaylist = UI::C_MultiplayerCache::Instance().CurrentEvent();
	if ( !pPlaylist )
	{
		BIZ_ASSERT_FAIL( "Playlist not set" );
		return;
	}

	// Get current race data
	const MultiplayerRace* pRace = pPlaylist->races[GetRaceDataIndex()];
	if ( !pRace )
	{
		BIZ_ASSERT_FAIL( "Invalid playlist race" );
		return;
	}

	// Playlist data
	levelState.mFanMultiplier						= pPlaylist->fan_multiplier;
	levelState.mTeams								= pPlaylist->teams;

	// Race data
	levelState.mRaceMode							= UI::ConvertUiToGame_RaceMode( pRace->type);
	levelState.mUiRaceType							= pRace->type;
	levelState.mCity								= pRace->route->city->ID;
	levelState.mRoute								= pRace->route->ID;
	levelState.mLaps								= pRace->laps;
	levelState.mTimeLimit							= pRace->time;
	levelState.mDamageMode							= UI::ConvertUiToGame_DamageMode(pRace->damage_mode);
	levelState.mDamageMultiplier					= UI::ConvertUiToGame_DamageMultiplier(pRace->damage_mode);
	levelState.mWreckedBehaviour					= (pRace->lastManStanding == UI::E_LastManStanding_Off) ? E_AMAXWRECKED_RESPAWN : E_AMAXWRECKED_ELIMINATED;
	
	// Mp specific settings
	LevelStateMultiplayerSettings& multiplayerSettings = levelState.GetMultiplayerSettings();
	multiplayerSettings.mAIMode						= pRace->aiMode;
	multiplayerSettings.mUpgrades					= pRace->carUpgrades;
	multiplayerSettings.mHandicap					= pRace->handicap;
	multiplayerSettings.mModSettings				= pRace->powerupMods;
	multiplayerSettings.mAlowTimeout				= pRace->timeout;
	multiplayerSettings.mLockToHostCar				= pRace->lock_to_host_car;

	// Powerup setup
	levelState.mPerkDroppingWhenUsed				= false;
	levelState.mPerksRespawn						= !levelState.mPerkDroppingWhenUsed; // If we're dropping pickups when they're fired, we don't want them to respawn (and vice versa)
	levelState.mPickupDisableTime					= 5.f;			//Bug 12525 - Default network pickup disable time...fairly horrible hard-coding sorry
	levelState.mPerksRespawnRandomType				= ( pRace->randomPowerups == UI::E_RandomPowerups_On ) ? true : false;
	levelState.mPickupRespawnType					= ConvertUiToGame_PowerUpRespawnMode( pRace->powerupRespawnMode );

	// Turn powerups on or off
	Game::Actors::RaceLogicCore* pRLC = UI::GetRLC();
	bool pickupsEnabled = false;
	for(int i = 0; i < UI::E_Pickup_Max; i++)
	{
		E_PICKUP_TYPE pickupType = UI::ConvertUiToGame_Pickup((UI::E_Pickup)i);
		Game::LevelPickupProbability levelPickupProb(pickupType, 0.f);

		if( (pRace->powerups & (1 << i)) && pRace->type != UI::E_RaceType_Hardcore )
		{
			const RLPickupType* pPickup	= pRLC->GetPickupType(pickupType);
			levelPickupProb.probability = pPickup->mBaseProbability;
			pickupsEnabled = true;
		}

		levelState.mPickupProbabilities.PushBack(levelPickupProb);
	}

	levelState.mPickupsEnabled = pickupsEnabled;

	LOGINFO( g_MpLog, "Playlist successfully filled out LevelState" );
}

void C_Playlist::FindValidVotingIndicies( const S32 sNumPlayers, const U32 uSeed, const S16 sRaceIndex, S16& sIndexA, S16& sIndexB )
{
	sIndexA = FindNextValidRace( sNumPlayers, sRaceIndex );
	sIndexB = FindNextValidRace( sNumPlayers, sIndexA );

	// Check to see if we have finished the cycle
	if( sIndexA < m_RaceIndex || sIndexB < m_RaceIndex )
	{
		Shuffle( uSeed );
		ResetCycle( sNumPlayers );
		
		sIndexA = m_RaceIndex;
		sIndexB = FindNextValidRace( sNumPlayers, sIndexA );
	}

	LOGINFO( g_MpLog, "Playlist set vote indicies to %i, %i valid for %i players", sIndexA, sIndexB, sNumPlayers );
}

void C_Playlist::CycleRaceIndex( const S32 sNumPlayers, const U32 uSeed )
{
	const S16 index = FindNextValidRace( sNumPlayers, m_RaceIndex );

	// Check to see if we have finished the cycle
	if ( index < m_RaceIndex )
	{
		Shuffle( uSeed );
		ResetCycle( sNumPlayers );
		
		// TO DO [GavinY] - Move this to the Champ. results screen once it's in and working
		/*if( m_ChampionshipInProgress )
		{
			Game::C_StickerManager::Instance().CheckChampionshipWinner( GetLocalIndexFromJoinId(
				((C_GameModeNetwork*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr())->GetSortedChampionshipStandings()[0].mJoinId) );
		}*/
		m_ChampionshipInProgress = false;
	}
	else
		m_RaceIndex = index;

	
	if( m_IsChampionship )
	{

	}

	LOGINFO( g_MpLog, "Playlist move on to race %i, valid for %i players", m_RaceIndex, sNumPlayers );
}

//Championship
const bool C_Playlist::IsChampionship() const
{
	return m_IsChampionship;
}

const bool C_Playlist::IsChampionshipInProgress() const
{
	return m_ChampionshipInProgress;
}

void C_Playlist::StartChampionship()
{
	BIZ_ASSERT_MSG(m_IsChampionship, "C_Playlist::StartChampionship - Trying to start a championship event which has not been declared as one");
	
	m_ChampionshipInProgress = true;
}

void C_Playlist::GenerateSingleRaces( const MultiplayerEvent* pEvent )
{
	// Add the data for each race in the playlist
	AddRacesRange(0, pEvent->races.count, pEvent);

	// Shuffle race order
	for ( S32 i = 0; i < m_Races.Size(); i++ )
	{
		Btl::Swap( m_Races[i], m_Races[m_Random.Rand(m_Races.Size())] );
	}
}

void C_Playlist::GenerateChampionship( const MultiplayerEvent* pEvent )
{
	BIZ_ASSERT(pEvent!=0);

	int championshipLength=1;
	if (pEvent->championship_length!=0)
	{
		championshipLength=pEvent->championship_length;
	}

	int number_of_championships=pEvent->races.count/championshipLength;
	int firstRace=m_Random.Rand(number_of_championships)*championshipLength;
	int lastRace=firstRace+championshipLength;
	if (lastRace>pEvent->races.count)
		lastRace=pEvent->races.count;
	
	AddRacesRange(firstRace, lastRace, pEvent);
}

void C_Playlist::AddRacesRange( int start, int end, const MultiplayerEvent* pEvent )
{
	RaceShuffleData raceData;
	for ( S32 i = start; i < end; i++ )
	{
		// Shuffle car classes
		Btl::Vector<UI::E_VehicleClass> classes;
		for ( S32 j = UI::E_VehicleClass_A; j < UI::E_VehicleClass_Max; j++ )
		{
			if ( pEvent->races[i]->classes & ( 1 << j ) )
				classes.PushBack( (UI::E_VehicleClass)j );
		}
		BIZ_ASSERT_MSG( classes.Size(), "No car classes set for this race %d in this event", i );

		raceData.m_Index = (S16)i;
		raceData.m_CarClass = classes[m_Random.Rand(classes.Size())];

		m_Races.PushBack( raceData );
	}
}

};// namespace Game
