#include "stdafx.h"

#include <game/Game.h>
#include <actor_system/ActorFactory.h>
#include "ActorDef/ActorTypes.h"
#include "GameLogic/Structures/MultiplayerAbilities.h"
#include <GameLogic/Structures/NetGameVars.h>


namespace Game
{

Debug::Logger& g_Log = Debug::GetLogger("game");

C_Game* g_pGame = NULL;

// TODO: move actor stuff elsewhere
enum E_ActorType
{
	E_ActorType_Null = 0,
	E_ActorType_Crate,

	E_ActorType_Count
};

C_Game::C_Game()
	:	m_pActorFactory( NULL ),
	m_NetGameVars(new Game::C_NetGameVars()),
	m_MultiplayerAbilities(new Game::C_MultiplayerAbilities())
{
	assert(g_pGame == NULL);
	g_pGame = this;
}

C_Game::~C_Game()
{
	assert(g_pGame == this);
	g_pGame = NULL;
	SAFE_DELETE_PTR( m_MultiplayerAbilities );
	SAFE_DELETE_PTR( m_NetGameVars );
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

Messaging::C_MessageSubscriberSystem& C_Game::GetSubscriberSystem( void )
{
	return m_MessageSubscriberSystem;
}

C_MultiplayerAbilities& C_Game::GetMultiplayerAbilities()
{
	BIZ_ASSERT(m_MultiplayerAbilities!=0);
	return *m_MultiplayerAbilities;
}

C_LevelResult& C_Game::GetLevelResult()
{
	return m_LevelResult;
}

C_NetGameVars& C_Game::GetNetGameVars()
{
	BIZ_ASSERT(m_NetGameVars!=0);
	return *m_NetGameVars;
}

C_LevelState& C_Game::GetLevelState()
{
	return m_LevelState;
}

} // namespace Game
