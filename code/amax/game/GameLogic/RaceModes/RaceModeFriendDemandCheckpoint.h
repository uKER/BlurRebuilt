#ifndef RACEMODE_FRIEND_DEMAND_CHECKPOINT_H
#define RACEMODE_FRIEND_DEMAND_CHECKPOINT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/RaceModes/RaceModeCheckpoint.h>

namespace Game
{

class C_RaceModeFriendDemandCheckpoint : public C_RaceModeCheckpoint
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

#endif // RACEMODE_FRIEND_DEMAND_CHECKPOINT_H
