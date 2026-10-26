#ifndef GAMEMODE_DEBUG_H
#define GAMEMODE_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameModes/GameModeBase.h>

namespace Game
{

class C_GameModeDebug : public C_GameModeBase
{
public:
	void Initialise();
	void Update();
	void Start();
	void SetupRace(void *data);
	void StartRace();
	void InterpretRaceResults();

private:

	void LoadDebugValues();
	void SaveDebugValues() const;
};


} // namespace Game

#endif // GAMEMODE_DEBUG_H
