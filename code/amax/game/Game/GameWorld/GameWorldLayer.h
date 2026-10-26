#ifndef GAME_WORLD_LAYER_H
#define GAME_WORLD_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <Framework/Layer/Layer.h>
//
//#include <Game/GameWorld/GameWorld.h>
//#include <Resource/Resource.h>
//#include <game_resource/LevelResource.h>
//#include <game_resource/ModelResource.h>
//#include <world_render/ModelInstance.h>
//#include "gravity/narrowphase/Ray.h"
//
//namespace Render 
//{ 
//	namespace Sky { class C_SkyDome; }
//}
//
//
//namespace Game
//{
//
//class C_QuitLevelMenuItem;
//
////! C_GameWorldLayer
//
//class C_GameWorldLayer : public Framework::C_Layer, public Messaging::C_MessageHandler, public Resource::C_Listener
//{
//public:
//
//	C_GameWorldLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
//	virtual ~C_GameWorldLayer();
//
//	HRESULT ProcessMessage(const Framework::E_LayerMessage message);
//
//	HRESULT Create();
//	HRESULT Destroy();
//	HRESULT Update();
//	HRESULT PreRender();
//	HRESULT RenderSplitscreenWithAA();
//	HRESULT RenderSplitscreenWithoutAA();
//	HRESULT RenderFullscreen();
//
//	void ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
//
//	// For game messages
//	bool MessageHandler(const Messaging::C_MessageBase* pMessage);
//
//	C_GameWorld* GetGameWorld();
//
//private:
//	C_GameWorld*						m_pGameWorld;
//
//	GameResource::C_LevelHandle			m_LevelHandle;
//	GameResource::C_LevelResource*		m_pLevelResource;
//	GameResource::C_LevelInstance*		m_pLevelInstance;
//
//	GameResource::C_ModelHandle					m_ModelHandle;
//	GameResource::C_ModelResource*				m_pModelResource;
//	Btl::Vector<WorldRender::C_ModelInstance*>	m_ModelInstances;
//
//	Collision::C_CollisionFilter		m_TempRayCollisionFilter;
//
//
//	Collision::C_Ray					m_TempRay;
//
//	C_QuitLevelMenuItem*				m_pQuitLevelMenuItem;
//};
//
//} // namespace Game
//
#endif // GAME_LAYER_H
