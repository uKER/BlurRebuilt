#ifndef GAME_ACTOR_CONTEXT_PROVIDER_H
#define GAME_ACTOR_CONTEXT_PROVIDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <game_context/PhysicsContext.h>
#include <game_context/RenderContext.h>
#include <misc_shared/context/ContextProvider.h>

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace Gravity
{
	class C_Scene;
}

namespace GameResource
{
	class C_LevelSceneManager;
}

namespace Game
{

class C_GameWorldContextProvier : public Sys::I_ContextProvider
{
public:
	C_GameWorldContextProvier(SceneGraph::C_SceneGraph* pSceneGraph, GameResource::C_LevelSceneManager* pLevelSceneManager, Gravity::C_Scene* pPhysicsScene, Gravity::C_ExternalConstants* pDefaultPhysicsExternalConstants) 
		: m_RenderContext(pSceneGraph, pLevelSceneManager)
		, m_PhysicsContext(pPhysicsScene, pDefaultPhysicsExternalConstants)
	{
	}

	// from Sys::I_ContextProvider
	virtual const Sys::I_Anything* GetContext(const Sys::C_AnythingUid typeId) const
	{
		switch (typeId)
		{
		case GameContext::I_RenderContext::kClassUid:	return &m_RenderContext;
		case GameContext::I_PhysicsContext::kClassUid:	return &m_PhysicsContext;
		default:										return NULL;
		}
	}

private:
	GameContext::C_RenderContext m_RenderContext;
	GameContext::C_PhysicsContext m_PhysicsContext;
};

} // namespace Game

#endif //  GAME_ACTOR_CONTEXT_PROVIDER_H
