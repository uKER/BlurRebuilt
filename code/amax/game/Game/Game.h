#ifndef GAME_GAME_H
#define GAME_GAME_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Structures/LevelState.h>
#include <GameLogic/Structures/LevelResult.h>

#include "misc_shared\Messaging\MessageSubscriberSystem.h"

namespace Game
{
	class C_MultiplayerAbilities;
	class C_NetGameVars;
}

namespace Actor
{

	class C_ActorFactory;
}

namespace Game
{

extern Debug::Logger& g_Log;

//! C_Game
class C_Game
{
public:
	C_Game();
	~C_Game();

	static C_Game* GetInstance();

	bool Initialise();

	Actor::C_ActorFactory* GetActorFactory();

	Messaging::C_MessageSubscriberSystem&	GetSubscriberSystem(void);

	C_LevelState& GetLevelState();
	
	C_LevelResult& GetLevelResult();
	C_NetGameVars& GetNetGameVars();

	C_MultiplayerAbilities& GetMultiplayerAbilities();

private:
	bool InitialiseActors();

private:
	Actor::C_ActorFactory* m_pActorFactory;
	Messaging::C_MessageSubscriberSystem m_MessageSubscriberSystem;

	C_LevelState m_LevelState;

	C_LevelResult m_LevelResult;
	C_NetGameVars* m_NetGameVars;

	C_MultiplayerAbilities* m_MultiplayerAbilities;
};

} // namespace Game

#endif // GAME_GAME_H
