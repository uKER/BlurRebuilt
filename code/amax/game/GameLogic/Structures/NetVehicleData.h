#ifndef NET_VEHICLE_DATA_H
#define NET_VEHICLE_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <Network\Misc\NetDefines.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>

#include <GameLogic/Structures/LevelState.h>

namespace Game
{


struct NetVehicleData
{
	void Clear(void)
	{
		m_State = E_VEHICLE_STATE_UNKNOWN;

		for(int i = 0; i < UI::E_MultiplayerStat_Count; i++)
			m_Statistics[i] = 0.f;

		m_SkillRating = 0;
		m_SkillTeam = 0;
				
		m_Position = 0;		//	This should NEVER be set until after the race has finished and we're setting up the results
		m_LocalIndex = -1;	//	Don't want to confuse ourselves with "local" indices from a remote player
		m_NetJoinId = 0;	//	Corresponding global net join ID for this player
		m_Idle=false;
	}

	bool IsRacing(void)
	{
		if(m_State == E_VEHICLE_STATE_RACING)
			return true;
		else
			return false;
	}

	bool IsOutOfRace(void)
	{
		if(m_State == E_VEHICLE_STATE_LEFT_RACE
			|| m_State == E_VEHICLE_STATE_ELIMINATED)
			return true;
		else
			return false;
	}

	E_VEHICLE_STATE								m_State;						//	Vehicle state

	S32											m_SkillRating;					//	needs to be set when results are calculated on xbox360 for trueskill purposes.
	S32											m_SkillTeam;					//	needs to be set when results are calculated on xbox360 for trueskill purposes.

	S8											m_Position;						//	Final position at end of event (Calculated in SetupResults())
	S8											m_LocalIndex;					//	Vehicle index (locally)
	U8											m_Team;							//	Which team this player belongs to
	U8											m_NetJoinId;

	bool										m_Idle;

	Btl::Vector<F32, Btl::VectorParameters_Array<UI::E_MultiplayerStat_Count> > m_Statistics;
};

typedef Btl::Vector< NetVehicleData, Btl::VectorParameters_Array< RacerNet::NET_MAX_VEHICLES> > MpVehiclesData;

}	//	namespace Game

#endif // NET_VEHICLE_DATA_H
