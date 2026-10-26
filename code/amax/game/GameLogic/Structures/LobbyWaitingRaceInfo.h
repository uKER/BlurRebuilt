#ifndef LOBBY_RACE_INFO_H
#define LOBBY_RACE_INFO_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network\Misc\NetDefines.h>

namespace Game
{

	struct LobbyRaceVehicleInfo
	{
		U8	m_JoinId;
		U8	m_RaceState;
		F32 m_StatData;
		F32	m_Xposition;
		F32 m_Zposition;
	};

	struct LobbyWaitingRaceInfo
	{
		void Reset()
		{
			m_Timeleft = -1.0f;
			m_Vehicles.Clear();
		}

		Btl::Vector< LobbyRaceVehicleInfo, Btl::VectorParameters_Static < RacerNet::NET_MAX_VEHICLES > > m_Vehicles;
		F32						m_Timeleft;
	};

} // namespace Game

#endif // namespace Game