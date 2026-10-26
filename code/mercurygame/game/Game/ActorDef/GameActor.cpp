#include "stdafx.h"

#include <Game/ActorDef/GameActor.h>
#include <Game/ActorDef/ActorTypes.h>
#include <game_resource/LevelSceneManager.h>

#include <actor_system/ActorMessages.h>

namespace Game
{

namespace Actors
{

// C_GameActor

C_GameActor::C_GameActor()
{
}

bool C_GameActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	case Actor::E_ActorMsg_Kill:
		Kill();
		break;
	default:
		return C_Actor::OnMessage(pMessage);
	}

	return true;
}

SceneGraph::C_SceneGraph* C_GameActor::GetSceneGraph()
{
	return GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
}

Gravity::C_Scene* C_GameActor::GetPhysicsScene()
{
	return GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();
}

Gravity::C_ExternalConstants* C_GameActor::GetPhysicsExternalConstants()
{
	return GetContext<GameContext::I_PhysicsContext>()->GetDefaultExternalConstants();
}

GameResource::C_LevelSceneManager* C_GameActor::GetLevelSceneManager()
{
	return GetContext<GameContext::I_RenderContext>()->GetLevelSceneManager();
}

WorldRender::C_Scene* C_GameActor::GetWorldRenderScene()
{
	return GetLevelSceneManager()->GetWorldRenderScene();
}


} // namespace Actors


} // namespace Game
