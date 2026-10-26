#ifndef GAMEMODE_SINGLEPLAYER_H
#define GAMEMODE_SINGLEPLAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameModes/GameModeBase.h>

namespace Game
{

class C_GameModeSinglePlayer : public C_GameModeBase
{
public:
	void Initialise();
	void Update();
	void Start();
	void SetupRace(void *data);
	void StartRace();
	void InterpretRaceResults();
};


} // namespace Game

#endif // GAMEMODE_SINGLEPLAYER_H
