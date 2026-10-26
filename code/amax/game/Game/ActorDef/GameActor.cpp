#include "stdafx.h"

#include <Game/ActorDef/GameActor.h>
#include <Game/ActorDef/ActorTypes.h>
#include <game_resource/LevelSceneManager.h>

#include <actor_system/ActorMessages.h>
#include <misc_shared/messaging/GameMessaging.h>

namespace Game
{

namespace Actors
{

// C_GameActor

C_GameActor::C_GameActor()
{
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

// switch on and off the update for the actor
void C_GameActor::SetUpdate(const bool yn = true)	
{
	if(yn == FlagUpdate())
		return;

	if(yn)
	{
		SetActorFlag(Actor::E_ActorFlag_Update, true);
		//SetActiveGroup(true);
		AddSubscriber(GameMessaging::E_GameMsg_Update);

		// Send Wake Message - has to be in physics RW phase at present
		if(FlagAsleep())	// only if actor has previously been put to sleep
		{
			SendMessageToActor(GetHandle(),Actor::E_ActorMsg_WakeUp);//, E_ActorMsgQueue_PhysicsWorldUpdate);
			SetActorFlag(Actor::E_ActorFlag_Asleep, false);
		}
	}
	else
	{
		SetActorFlag(Actor::E_ActorFlag_Update, false);
		//SetActiveGroup(false);
		RemoveSubscriber(GameMessaging::E_GameMsg_Update);

		// Send Sleep Message - has to be in physics RW phase at present
		SendMessageToActor(GetHandle(),Actor::E_ActorMsg_GoToSleep);//, E_ActorMsgQueue_PhysicsWorldUpdate);
		SetActorFlag(Actor::E_ActorFlag_Asleep, true);
	}
}

} // namespace Actors


} // namespace Game
