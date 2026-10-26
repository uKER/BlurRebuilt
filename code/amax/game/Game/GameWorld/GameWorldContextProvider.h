#ifndef GAME_WORLD_CONTEXT_PROVIDER_H
#define GAME_WORLD_CONTEXT_PROVIDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <game_context/PhysicsContext.h>
#include <game_context/RenderContext.h>
#include <game_context/LightingContext.h>
#include <misc_shared/context/ContextProvider.h>

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace Gravity
{
	class C_Scene;
}

namespace Game
{

class C_GameWorldContextProvider : public Sys::I_ContextProvider
{
public:
	C_GameWorldContextProvider(SceneGraph::C_SceneGraph* pSceneGraph, GameResource::C_LevelSceneManager* pLevelSceneManager, Gravity::C_Scene* pPhysicsScene, Gravity::C_ExternalConstants* pDefaultPhysicsExternalConstants, DynamicEffect::Light::C_Manager* pLightingManager) 
		: m_RenderContext(pSceneGraph, pLevelSceneManager)
		, m_PhysicsContext(pPhysicsScene, pDefaultPhysicsExternalConstants)
		, m_LightingContext(pLightingManager)
	{
	}

	// from Sys::I_ContextProvider
	virtual const Sys::I_Anything* GetContext(const Sys::C_AnythingUid typeId) const
	{
		switch (typeId)
		{
		case GameContext::I_RenderContext::kClassUid:	return &m_RenderContext;
		case GameContext::I_PhysicsContext::kClassUid:	return &m_PhysicsContext;
		case GameContext::I_LightingContext::kClassUid:	return &m_LightingContext;
		default:										return NULL;
		}
	}

	void SetPhysicsContext(Gravity::C_Scene* pPhysicsScene, Gravity::C_ExternalConstants* pDefaultPhysicsExternalConstants)
	{
		m_PhysicsContext.SetPhysicsScene(pPhysicsScene);
		m_PhysicsContext.SetDefaultExternalConstants(pDefaultPhysicsExternalConstants);
	}

private:
	GameContext::C_RenderContext m_RenderContext;
	GameContext::C_PhysicsContext m_PhysicsContext;
	GameContext::C_LightingContext m_LightingContext;
};

} // namespace Game

#endif //  GAME_WORLD_CONTEXT_PROVIDER_H
