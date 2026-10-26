#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <gravity/sdk/PhysicsSDK.h>
#include <actor_system/ActorSys.h>
#include <btl/containers/list.h>
#include <Game/GameDef/Polyflags.h>
#include <gameview/cubemapview.h>
#include "gravity/sharedphysics/ContactStreamIterator.h"
#include <fx/Fx.h>

namespace SceneGraph
{
	class C_SceneGraph;
	class C_BackFrontSorter;
	class C_FrontBackSorter;
	class C_View;
	class C_MultiView;
	class C_TaskList;
	class C_RenderGroupSettings;
	struct RenderStates;
}

namespace Gravity
{
	class C_Body;
	class C_Scene;
}

namespace GamePhysics
{
	class C_World;

	struct C_CollisionEvent;
}

namespace GameResource
{
	class C_LevelSceneManager;
}

namespace FX
{
	class I_EnvironmentSettings;
}

namespace Effect
{
namespace Sky
{
	class C_TexturedSkyDome;
}
}

namespace DynamicEffect
{
	class C_Renderer;

	namespace Particle
	{
		class C_ParticleManager;
	}
}

namespace AnimatedCrowd
{
	class C_CrowdSystem;
}

namespace Actors
{
class C_VisibilityDetector;
}

namespace Render
{
	namespace Reduced
	{
		class C_Manager;
	}
}

namespace Sys
{
	class I_ContextProvider;
}

namespace Game
{

class C_GameWorldContextProvier;
class C_GameActorList;


enum E_RenderPass
{
	E_RenderPass_Solid,
	E_RenderPass_Decal,
	E_RenderPass_Punch,
	E_RenderPass_Refract,
	E_RenderPass_Alpha,
	E_RenderPass_CustomSolid,
	E_RenderPass_EffectSolid,
	E_RenderPass_EffectDecal,
	E_RenderPass_Sky
};

enum E_RenderGroup
{
	E_RenderGroup_WithAA,
	E_RenderGroup_WithoutAA,

	E_RenderGroup_Count
};

class C_ContactDrawer
{
public:
	static void Clear();

	static void AddContact(const GamePhysics::C_CollisionEvent& collisionEvent);

	static void DebugRender(DbUtil::I_DebugRender* pDebugRender);

private:
	struct C_Item;

	static Btl::Vector<C_Item>	s_Items;
	static S32					s_Counter;
	static Gravity::C_Body*		s_pLastBodyA;
	static Gravity::C_Body*		s_pLastBodyB;

	C_ContactDrawer(); // can not instantiate
	~C_ContactDrawer(); // can not instantiate

	C_ContactDrawer(const C_ContactDrawer& copy); // can not copy
	C_ContactDrawer& operator = (const C_ContactDrawer& copy); // can not copy

};

class C_CubeMap : public GameView::C_CubeMapView
{
public:
	C_CubeMap(SceneGraph::C_SceneGraph* pSceneGraph) : m_pSceneGraph(pSceneGraph) {}
	~C_CubeMap() {}

	void	Create(U32 subtechnique, U32 disableFlags);
	void	Destroy();

	void			Gather();
	virtual void	GatherFace(const U32 face);

	void			Schedule();
	virtual void	ScheduleFace(const U32 face);
	
	void			Render();
	virtual void	RenderFace(const U32 face);

private:
	SceneGraph::C_SceneGraph*			m_pSceneGraph;
	SceneGraph::C_View*					m_pView[6];
	SceneGraph::C_MultiView*			m_pMultiView;
	SceneGraph::C_RenderGroupSettings*	m_pSettings;
};

class C_GameWorld //: public GameView::I_CubeMapRenderHelper
{
public:
	C_GameWorld();
	~C_GameWorld();

	bool Create( Actor::C_ActorFactory* pActorFactory );
	bool Destroy();

	void BeginFrame();
	void BeginSplitscreen();
	void ScheduleOnce();
	void ScheduleSplitscreen();
	void RenderOnce();
	void PreRenderSplitscreen();
	void RenderSplitscreenWithAA();
	void RenderSplitscreenWithoutAA();
	void EndFrame();

	void DebugRender( DbUtil::I_DebugRender* pDebugRender );

	void Update();
	void GlobalUpdate();

	Gravity::C_ExternalConstants& GetPhysicsExternalConstants();
	
	void SetSunDirection( Maths::HVec3 PCREF sunDir ) { m_SunDirection = sunDir; }// temporary, this needs to be replaced with better scene settings

public:

	C_GameActorList* GetActorList();
	Gravity::C_Scene* GetPhysicsScene();
	GameResource::C_LevelSceneManager* GetLevelSceneManager();
	SceneGraph::C_SceneGraph* GetSceneGraph();
	AnimatedCrowd::C_CrowdSystem* GetCrowdSystem();
	Sys::I_ContextProvider* GetContextProvider();

private:
	bool CreateActorList( Actor::C_ActorFactory* pActorFactory );
	bool CreateSceneGraph();
	bool CreateStreamer();
	bool CreateLevelSceneManager();
	bool CreateRenderers();
	bool CreateSceneGraphView();
	bool CreatePhysics();
	bool CreateAnimatedModelTaskManager();
	bool CreateAnimationTaskManager();
	bool CreateCrowdSystem();

	void CreateEnvironmentSettingsFromSorcery();

	void UpdatePhysics();

	void SetSharedFxParameters();
	void SetSharedFxParameters(S32 viewIndex);

private:
	// actors
	C_GameActorList*							m_pActorList;
	C_GameWorldContextProvier*					m_pContextProvider;
	
	FX::I_EnvironmentSettings*					m_pEnvironmentSettings;

	FX::SamplerStateHandle						m_ClampPoint, m_ClampLinear;

	// scene graph
	SceneGraph::C_SceneGraph*					m_pSceneGraph;
	SceneGraph::C_BackFrontSorter*				m_pBackFront;
	SceneGraph::C_FrontBackSorter*				m_pFrontBack;
	SceneGraph::RenderStates*					m_pColourWriteAllStates;
	SceneGraph::C_RenderGroupSettings*			m_pWithAAsettings;
	SceneGraph::C_RenderGroupSettings*			m_pWithoutAAsettings;
	
	// renderers
	Effect::Sky::C_TexturedSkyDome*				m_pSky;
	DynamicEffect::C_Renderer*					m_pDynamicEffectRenderer;
	DynamicEffect::Particle::C_ParticleManager*	m_pParticleManager;
	Render::Reduced::C_Manager*					m_pReducedRender;

	GameResource::C_LevelSceneManager*			m_pLevelSceneManager;

	// physics
	Gravity::C_ExternalConstants				m_PhysicsExternalConstants;
	Gravity::C_Scene*							m_pPhysicsScene;
	GamePhysics::C_World*						m_pGamePhysicsWorld;

	// Timers
	S32											m_GameFrame;

	// animated crowd
	AnimatedCrowd::C_CrowdSystem*				m_pCrowdSystem;
	
	// misc
	Display::C_TextureCube						m_TextureCube;
	Maths::HVec3								m_SunDirection;

	// TODO: very temporary. view management needs work
	SceneGraph::C_View*							m_pView[4];
	SceneGraph::C_MultiView*					m_pMultiView[4];
	S32											m_MultiViewCount;

	C_CubeMap*									m_pCubeMap;

	U32											m_iRenderGroup;
	U32											m_iAlphaLuminanceTechniqueHandle;
	U32											m_iLODTechniqueHandle;

	::Actors::C_VisibilityDetector				*m_pVisibilityDetector;
};

} // namespace Game

#endif // GAME_WORLD_H
