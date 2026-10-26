
#include "stdafx.h"
#include "ui/Game/Multiplayer/MultiplayerRace.h"

#include <lua_shared/LuaHelper.h>

#include <GameLogic/GameData/GameData.h>
#include <GameLogic/GameData/MultiplayerData.h>

namespace UI
{

C_MultiplayerRace::C_MultiplayerRace()
:	m_CityID( 0 ), 
	m_RouteID( 0 ),
	m_Type( E_RaceType_Racing ),
	m_Laps( 1 ),
	m_ScoringRules( 0 ),
	m_ElimationEvent( E_EliminationEvent_LapFirst ),
	m_ElimationType( E_EliminationType_LastPlace )
{
}

void C_MultiplayerRace::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	stream	<< m_CityID
			<< m_RouteID
			<< (S32)m_Type
			<< m_Laps
			<< m_Limits.Size();

	for(S32 lim = 0; lim < m_Limits.Size(); lim++)
	{
		stream	<< m_Limits[lim].maxCars
				<< m_Limits[lim].scoreLimit
				<< m_Limits[lim].timeLimit;
	}


	stream	<< m_ScoringRules
			<< (S32)m_ElimationEvent
			<< (S32)m_ElimationType;
}

void C_MultiplayerRace::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	S32 type, elim_event, elim_type, noOfLimits;

	stream	>> m_CityID
			>> m_RouteID
			>> type
			>> m_Laps
			>> noOfLimits;

	U8 maxCars, scoreLimit;
	U16 timeLimit;

	for(S32 lim = 0; noOfLimits; lim++)
	{
		stream	>> maxCars
				>> scoreLimit
				>> timeLimit;
		
		m_Limits.PushBack( MultiplayerRaceLimit() );
		MultiplayerRaceLimit& limit = m_Limits.Back();

		limit.maxCars = maxCars;
		limit.scoreLimit = scoreLimit;
		limit.timeLimit = timeLimit;
	}

	stream	>> m_ScoringRules
			>> elim_event
			>> elim_type;

	m_Type = (E_RaceType)type;
	m_ElimationEvent = (E_EliminationEvent)elim_event;
	m_ElimationType = (E_EliminationType)elim_type;

	m_CityLoc = C_GameData::Instance().mCities[(GAMEDATA_ID)m_CityID]->loc_string;
	m_RouteLoc = C_GameData::Instance().mRoutes[(GAMEDATA_ID)m_RouteID]->loc_string;
}

void C_MultiplayerRace::SendToLua( lua_State* pState ) const
{
	Lua::AddDataMember( pState, "city",					m_CityID );
	Lua::AddDataMember( pState, "city_name",			m_CityLoc.id );
	Lua::AddDataMember( pState, "route",				m_RouteID );
	Lua::AddDataMember( pState, "route_name",			m_RouteLoc.id );
	Lua::AddDataMember( pState, "type",					m_Type );

	Lua::AddDataMember( pState, "laps",					m_Laps );
	Lua::AddDataMember( pState, "time",					m_Time );
	Lua::AddDataMember( pState, "score_limit",			m_ScoreLimit );
	Lua::AddDataMember( pState, "elimination_event",	m_ElimationEvent );
	Lua::AddDataMember( pState, "elimination_type",		m_ElimationType );

	AmaxRoute* pRoute = C_GameData::Instance().mRoutes[ (GAMEDATA_ID)m_RouteID ];

	if ( pRoute )
	{
		Lua::AddDataMember( pState, "thumbnail", pRoute->thumbnail );
		Lua::AddDataMember( pState, "route_tag", pRoute->tag );
		Lua::AddDataMember( pState, "only_allow_single_lap", pRoute->onlyAllowsSingleLap );
		Lua::AddDataMember( pState, "fans_to_unlock_splitscreen", pRoute->fansToUnlockInSplitScreen );

		if ( pRoute->city )
		{
			Lua::AddDataMember( pState, "city_tag", pRoute->city->tag );
			Lua::AddDataMember( pState, "city_icon", pRoute->city->icon_name );
		}
	}
}

void C_MultiplayerRace::EditFromLua( lua_State* pState )
{
	S32 raceType = E_RaceType_Racing;
	S32 eliminationEvent = E_EliminationEvent_LapFirst;
	S32 eliminationType = E_EliminationType_LastPlace;

	Lua::GetDataMember( pState, "type",					raceType );

	Lua::GetDataMember( pState, "laps",					m_Laps );
	Lua::GetDataMember( pState, "time",					m_Time );
	Lua::GetDataMember( pState, "score_limit",			m_ScoreLimit );
	Lua::GetDataMember( pState, "elimination_event",	eliminationEvent );
	Lua::GetDataMember( pState, "elimination_type",		eliminationType );

	m_Type = (E_RaceType)raceType;
	m_ElimationEvent = (E_EliminationEvent)eliminationEvent;
	m_ElimationType = (E_EliminationType)eliminationType;
}

}; // namespace UI
