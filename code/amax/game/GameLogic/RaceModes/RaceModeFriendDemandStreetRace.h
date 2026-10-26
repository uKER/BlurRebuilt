#ifndef RACEMODE_FRIEND_DEMAND_STREETRACE_H
#define RACEMODE_FRIEND_DEMAND_STREETRACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/RaceModes/RaceModeStreetRace.h>

namespace Game
{

class C_RaceModeFriendDemandStreetRace : public C_RaceModeStreetRace
{
public:
	void Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	void Update();

	void StartLevel();

	void StartRace();

	void SetupResults();

	void UpdateHUD();

	bool MessageHandler(const Messaging::C_MessageBase* pMessage);
};


} // namespace Game

#endif // RACEMODE_FRIEND_DEMAND_STREETRACE_H
