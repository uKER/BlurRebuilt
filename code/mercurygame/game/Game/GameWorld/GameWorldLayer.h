#ifndef GAME_WORLD_LAYER_H
#define GAME_WORLD_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>

#include <Resource/Resource.h>
#include <game_resource/LevelResource.h>

namespace Render 
{ 
	namespace Sky { class C_SkyDome; }
}

#ifndef CFG_GOLD
namespace GameDebug
{
	class C_GameDebug;
}
#endif

namespace ActorStudioMsgGame
{
	class C_ActorStudioMessaging;
}

namespace Game
{

class C_GameWorld;
class C_QuitLevelMenuItem;


//! C_GameWorldLayer

class C_GameWorldLayer : public Framework::C_Layer, public Messaging::C_MessageHandler, public Resource::C_Listener
{
public:

	C_GameWorldLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
	virtual ~C_GameWorldLayer();

	HRESULT ProcessMessage(const Framework::E_LayerMessage message);

	HRESULT Create();
	HRESULT Destroy();
	HRESULT Update();
	HRESULT GlobalUpdate();
	HRESULT BeginFrame();
	HRESULT PreRenderSplitscreen();
	HRESULT RenderSplitscreenWithAA();
	HRESULT PreRestoreWindow();
	HRESULT RenderSplitscreenWithoutAA();
	HRESULT RenderFullscreen();
	HRESULT EndFrame();

	void ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	C_GameWorld* GetGameWorld();

	GameResource::C_LevelResource*		GetLevelResource() { return m_pLevelResource;}


private:
	C_GameWorld*							m_pGameWorld;
	
	ActorStudioMsgGame::C_ActorStudioMessaging*
											m_pActorStudioMessaging;

	GameResource::C_LevelHandle				m_LevelHandle;
	GameResource::C_LevelResource*			m_pLevelResource;
	GameResource::C_LevelInstance*			m_pLevelInstance;

	C_QuitLevelMenuItem*					m_pQuitLevelMenuItem;

	#ifndef CFG_GOLD
	GameDebug::C_GameDebug*					m_pGameDebug;
	#endif
};

} // namespace Game

#endif // GAME_LAYER_H
