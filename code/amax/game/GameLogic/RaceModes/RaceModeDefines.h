#ifndef RACEMODE_DEFINES_H
#define RACEMODE_DEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

enum E_RaceMode
{
	E_RaceMode_Nothing,

	E_RaceMode_StreetRace,
	E_RaceMode_Multiplayer_Racing,
	E_RaceMode_Multiplayer_Elimination,
	E_RaceMode_Multiplayer_Points,
	E_RaceMode_Multiplayer_Destruction,

	E_RaceMode_Destruction,
	E_RaceMode_Checkpoint,
	E_RaceMode_FanDemand,
	E_RaceMode_BossBattle,

	E_RaceMode_FriendDemand_StreetRace,
	E_RaceMode_FriendDemand_Destruction,
	E_RaceMode_FriendDemand_Checkpoint,
	E_RaceMode_FriendDemand_FanDemand,
	E_RaceMode_FriendDemand_BossBattle,

	E_RaceMode_MAX,
};

} // namespace Game

#endif // RACEMODE_DEFINES_H
