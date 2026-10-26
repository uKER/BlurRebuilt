#ifndef GAME_GAME_ACTOR_H
#define GAME_GAME_ACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/Actor.h>
#include <Game/ActorDef/ActorTypes.h>
#include <game_context/PhysicsContext.h>
#include <game_context/RenderContext.h>

namespace SceneGraph
{
	class C_SceneGraph;
};

namespace Gravity
{
	class C_Scene;
	struct C_ExternalConstants;
};

namespace GameResource 
{ 
	class C_LevelSceneManager; 
};

namespace WorldRender
{
	class C_Scene;
};

namespace Game
{

namespace Actors
{

// Actor Info
class C_GameActor : public Actor::C_Actor
{
public:
	C_GameActor();

	bool	OnMessage(const Messaging::C_MessageBase *pMessage);

	SceneGraph::C_SceneGraph* GetSceneGraph();

	Gravity::C_Scene* GetPhysicsScene();
	Gravity::C_ExternalConstants* GetPhysicsExternalConstants();
	
	GameResource::C_LevelSceneManager* GetLevelSceneManager();
	WorldRender::C_Scene* GetWorldRenderScene();
private:
};

} // namespace Actors


} // namespace Game

#endif //  GAME_GAME_ACTOR_H
