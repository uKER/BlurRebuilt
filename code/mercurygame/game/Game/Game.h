#ifndef GAME_GAME_H
#define GAME_GAME_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorSys.h>
#include <actor_system/ActorFactory.h>
#include <misc_shared/Messaging/Messaging.h>

namespace Game
{

extern Debug::Logger& g_Log;

// C_LevelSettings
class C_LevelSettings
{
public:
	C_LevelSettings();
	~C_LevelSettings();

public:
	Util::C_String m_LevelFilename;
	Util::C_String m_ActorsFilename;
};

//! C_Game
class C_Game
{
public:
	C_Game();
	~C_Game();

	static C_Game* GetInstance();

	bool Initialise();

	Actor::C_ActorFactory* GetActorFactory();

	Messaging::C_MessageSubscriberSystem&	GetSubscriberSystem(void) {return m_MessageSubscriberSystem;}

	C_LevelSettings& GetLevelSettings() { return m_LevelSettings; }

private:
	bool InitialiseActors();

private:
	Actor::C_ActorFactory* m_pActorFactory;
	Messaging::C_MessageSubscriberSystem m_MessageSubscriberSystem;

	C_LevelSettings m_LevelSettings;
};

} // namespace Game

#endif // GAME_GAME_H
