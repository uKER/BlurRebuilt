#include "stdafx.h"

#include <game/Game.h>
#include <Game/GameDef/Materials.h>
#include <Game/ActorDef/GameActor.h>
#include "ActorComponents/GameComponents.h"

namespace Game
{

Debug::Logger& g_Log = Debug::GetLogger("game");

C_LevelSettings::C_LevelSettings()
{
}

C_LevelSettings::~C_LevelSettings()
{
}

C_Game* g_pGame = NULL;

// TODO: move actor stuff elsewhere
enum E_ActorType
{
	E_ActorType_Null = 0,
	E_ActorType_Crate,

	E_ActorType_Count
};

C_Game::C_Game()
	:	m_pActorFactory( NULL )
{
	assert(g_pGame == NULL);
	g_pGame = this;
}

C_Game::~C_Game()
{
	assert(g_pGame == this);
	g_pGame = NULL;
	SAFE_DELETE_PTR( m_pActorFactory );
}

C_Game* C_Game::GetInstance()
{
	return g_pGame;
}

bool C_Game::InitialiseActors()
{
	//LOGINFO(g_Log,"C_Actor size = %d bytes\n", sizeof(Actor::C_Actor));
	//LOGINFO(g_Log,"C_GameActor size = %d bytes\n", sizeof(Actors::C_GameActor));
	Components::RegisterGameComponents();

	m_pActorFactory = new Actor::C_ActorFactory( E_ActorType_Count );
    Game::RegisterActorTypes( *m_pActorFactory );

	return true;
}

bool C_Game::Initialise()
{
	bool success = true;
		
	success = C_PhysicsMaterials::SetupMaterials();
	if(!success)
		return false;

	success = InitialiseActors();
	if(!success)
		return false;

	return success;
}

Actor::C_ActorFactory* C_Game::GetActorFactory()
{
	return m_pActorFactory;
}

} // namespace Game
