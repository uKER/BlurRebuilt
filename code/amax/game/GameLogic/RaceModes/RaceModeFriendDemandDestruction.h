#ifndef RACEMODE_FRIEND_DEMAND_DESTRUCTION_H
#define RACEMODE_FRIEND_DEMAND_DESTRUCTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/RaceModes/RaceModeDestruction.h>

namespace Game
{

class C_RaceModeFriendDemandDestruction: public C_RaceModeDestruction
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

#endif // RACEMODE_FRIEND_DEMAND_DESTRUCTION_H
