
#include "stdafx.h"

#include "UI/Game/Multiplayer/UserEventManager.h"
#include "Ui/Game/Multiplayer/MultiplayerCache.h"

#include <ui/Game/Profiles/ProfileInterface.h>
#include <ui/Game/UILua.h>

#include "GameLogic/GameData/MultiplayerData.h"
#include "GameLogic/GameData/GameData.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

C_UserEventManager C_UserEventManager::m_sInstance;

C_UserEventManager::C_UserEventManager()
:	m_Dirty( false ),
	m_EventSlot( -1 ),
	m_RaceSlot( -1 )
{
}

void C_UserEventManager::Destroy()
{
	m_UserEvents.Clear();
}


const bool C_UserEventManager::SaveGroups( Sys::BinarySerialiserAutoEndian& serial )
{
	const bool result = SaveImpl( serial );

	if ( !result )
		return false;

	m_Dirty = false;
	m_EventSlot = -1;
	m_RaceSlot = -1;

	return true;
}


const bool C_UserEventManager::LoadGroups( Sys::BinaryDeserialiserAutoEndian& serial )
{
	Destroy();

	const bool result = LoadImpl( serial );

	if ( !result )
		return false;

	m_Dirty = false;
	m_EventSlot = -1;
	m_RaceSlot = -1;

	return true;
}


const bool C_UserEventManager::SaveImpl( Sys::BinarySerialiserAutoEndian& serial )
{
	serial << m_FileVersion;

	S32 size = m_UserEvents.Size();

	serial << size;
	for ( S32 i = 0; i < m_UserEvents.Size(); i++ )
		serial << m_UserEvents[i];

	return true;
}


const bool C_UserEventManager::LoadImpl( Sys::BinaryDeserialiserAutoEndian& serial )
{
	U32 version = 0;
	serial >> version;

	if ( version != m_FileVersion )
		return false;

	S32 size = 0;
	serial >> size;

	for ( S32 i = 0; i < size; i++ )
	{
		C_MultiplayerEvent event;

		serial >> event;

		m_UserEvents.PushBack( event );
	}

	return true;
}


void C_UserEventManager::CacheEvents()
{
	C_MultiplayerCache::Instance().ClearEventCache();

	for ( S32 i = 0; i < m_UserEvents.Size(); i++ )
		C_MultiplayerCache::Instance().CacheUserEvent( &m_UserEvents[i] );
}


void C_UserEventManager::EditEventFromLua( lua_State *pState )
{
	BIZ_ASSERT_MSG( m_EventSlot != -1, "MountEvent() wasnt called" );

	m_UserEvents[ m_EventSlot ].EditFromLua( pState );
	m_Dirty = true;
}


void C_UserEventManager::SendEventToLua( const S32 sSlot, lua_State *pState )
{
	if ( sSlot >= m_UserEvents.Size() )
		return;

	lua_newtable( pState );
	m_UserEvents[ sSlot ].SendToLua( pState );
}


const bool C_UserEventManager::AddNewEvent()
{
	if ( m_UserEvents.Size() < MAX_USER_EVENTS )
	{
		m_UserEvents.PushBack( C_MultiplayerEvent() );
		C_MultiplayerEvent& event = m_UserEvents.Back();

		event.m_Name			= UI::C_TextStringID( "MP_EVENT_NAME" ).String();
		event.m_Description		= UI::C_TextStringID( "MP_EVENT_DESCRIPTION" ).String();

		C_MultiplayerCache::Instance().CacheUserEvent( &event );
		m_Dirty = true;

		MountEvent( m_UserEvents.Size() - 1 );
		return true;
	}
	return false;
}


void C_UserEventManager::RemoveEvent( const S32 uIndex )
{
	m_UserEvents.EraseByIndex( uIndex );
	m_Dirty = true;
}


void C_UserEventManager::RemoveNewEvent()
{
	// New event always appear at the back for now.
	m_UserEvents.PopBack();
	m_Dirty = false; // just removed what was added so no longer dirty
}


void C_UserEventManager::ChangeEventName( const WCHAR* pStr )
{
	BIZ_ASSERT_MSG( m_EventSlot != -1, "MountEvent() wasnt called" );

	m_UserEvents[ m_EventSlot ].m_Name = pStr;
	m_Dirty = true;
}

void C_UserEventManager::ChangeEventDesc( const WCHAR* pStr )
{
	BIZ_ASSERT_MSG( m_EventSlot != -1, "MountEvent() wasnt called" );

	m_UserEvents[ m_EventSlot ].m_Description = pStr;
	m_Dirty = true;
}


const bool C_UserEventManager::AddNewRace()
{
	BIZ_ASSERT_MSG( m_EventSlot != -1, "MountEvent() wasnt called" );

	if ( m_UserEvents[m_EventSlot].NumRaces() >= MAX_EVENT_RACES )
		return false;

	m_Dirty = true;
	m_RaceSlot = m_UserEvents[m_EventSlot].m_Races.Size();

	m_UserEvents[m_EventSlot].m_Races.PushBack( C_MultiplayerRace() );

	return true;
}

void C_UserEventManager::RemoveRace( const S32 sIndex )
{
	if ( m_UserEvents[m_EventSlot].m_Races.Size() <= 0 )
		return; 

	m_UserEvents[m_EventSlot].m_Races.EraseByIndex( sIndex );
}


void C_UserEventManager::ChangeRaceCityRoute( const U32 uCityID, const U32 uRouteID )
{
	BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

	C_MultiplayerRace& race = m_UserEvents[m_EventSlot].m_Races[m_RaceSlot];

	race.m_CityID = uCityID;
	race.m_RouteID = uRouteID;

	// lets so some slow code :-s oh how you fail us GAME_DATA_ID
	race.m_CityLoc = C_GameData::Instance().mCities[(GAMEDATA_ID)race.m_CityID]->loc_string;
	race.m_RouteLoc = C_GameData::Instance().mRoutes[(GAMEDATA_ID)race.m_RouteID]->loc_string;

	// Incase we get Haruna :-s
	if ( C_GameData::Instance().mRoutes[(GAMEDATA_ID)race.m_RouteID]->onlyAllowsSingleLap == true )
	{
		// Force to race
		race.m_Type = E_RaceType_Racing;
		race.m_Laps = 1;
		race.m_ScoreLimit = 0;
		race.m_Time = 0;
	}
	m_Dirty = true;
}


void C_UserEventManager::ChangeRaceMode( const E_RaceType eType )
{
	BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_Type = eType;

	// Check we dont have the survivor route
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_SurvivorRoutes.count; i++ )
	{
		if ( m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].Route() == C_MultiplayerData::Instance().m_pConfig->m_SurvivorRoutes[i]->ID )
		{
			if ( eType != E_RaceType_Destruction )
			{
				m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_CityID = C_MultiplayerData::Instance().m_pConfig->m_Cities[0]->ID;
				m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_RouteID = C_MultiplayerData::Instance().m_pConfig->m_Routes[0]->ID;

				// store loc ids
				m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_CityLoc = C_MultiplayerData::Instance().m_pConfig->m_Cities[0]->loc_string;
				m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_RouteLoc = C_MultiplayerData::Instance().m_pConfig->m_Routes[0]->loc_string;
			}
		}
	}
}

void C_UserEventManager::ChangeRaceLaps( const S32 sLaps )
{
	BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_Laps = sLaps;
	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_Time = 0;
	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_ScoreLimit = 0;
}

void C_UserEventManager::ChangeRaceTime( const S32 sTime )
{
	BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_Time = sTime;
	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_Laps = 0;
	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_ScoreLimit = 0;
}

void C_UserEventManager::ChangeRaceScore( const S32 sScore )
{
	BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_ScoreLimit = sScore;
	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_Time = 0;
	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_Laps = 0;
}

void C_UserEventManager::ChangeRaceEliminationEvent( const E_EliminationEvent eEvent )
{
	BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_ElimationEvent = eEvent;
}

void C_UserEventManager::ChangeRaceEliminationType( const E_EliminationType eType )
{
	BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

	m_UserEvents[m_EventSlot].m_Races[m_RaceSlot].m_ElimationType = eType;
}

}; // namespace UI


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif