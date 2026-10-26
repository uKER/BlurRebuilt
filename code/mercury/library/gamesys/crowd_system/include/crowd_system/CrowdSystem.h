#ifndef CROWD_SYSTEM_H
#define CROWD_SYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdEvent.h>
#include <crowd_system/XtCrowd.h>
#include <crowd_system/CrowdTextureCache.h>

#include <system/scheduler/jobqueue.h>

#ifndef CFG_GOLD
#include <system/Debug/I_DebugRender.h>
#endif

//typedef Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >	EventVector;

namespace SceneGraph
{
	class C_SceneGraph;
	class C_View;
};

namespace Actor
{
	class C_ActorList;
};

namespace GameResource
{
	class C_CrowdData;
};

namespace WorldRender
{
	class C_LightSamplePointScene;
};

namespace DynamicEffect
{
	namespace Particle
	{
		class C_Event;
	};
};

namespace AnimatedCrowd
{
	class C_CrowdResources;
	class C_CrowdWorld;
	class C_CrowdSpectators;
	class C_CrowdSystemActor;
	
	class C_CrowdFlags;
	class C_CrowdCameraFlashes;
};

namespace AnimatedCrowd
{

//! C_CrowdSystemListener
class C_CrowdSystemListener : public Btl::ListElementDouble<C_CrowdSystemListener>
{
public:
	C_CrowdSystemListener() {}
	virtual ~C_CrowdSystemListener() {}

	virtual void	OnCrowdSystemRouteChanged() {}
};

//! C_CrowdSystem

class C_CrowdSystem
{
public:
	static bool				DisableCreation();

	C_CrowdSystem();
	~C_CrowdSystem();

	void					Create(SceneGraph::C_SceneGraph* pSceneGraph, Actor::C_ActorList* pActorList, const GameResource::C_CrowdData* pCrowdData,
								const U32 routeMask = 0, const C_XtCrowdPopulation* pCrowdPopulation = NULL, const U32 viewTypeFlags = 0xffffffff);
	
	void					Destroy();

	void					Reset();
	void					Reactivate(Actor::C_ActorList* pActorList);

	void					SetJobQueuePolicyHandle(Scheduler::PolicyHandle jobQueuePolicyHandle) { m_JobQueuePolicyHandle = jobQueuePolicyHandle; }

	void					PreUpdateWorld(const F32 deltaTime);
	void					Update(const Maths::HVec3& cameraPosition, const F32 deltaTime);
	void					UpdateEffects(const F32 deltaTime);

	void					BeginFrame(const bool isPaused = false) const;
	void					Schedule() const;

#ifndef CFG_GOLD
	void					RenderFullscreen();
#endif

	void					AddListener(C_CrowdSystemListener* pListener);
	void					RemoveListener(C_CrowdSystemListener* pListener);

	void					SetFlagRenderer(C_CrowdFlags* pFlags);

	static void				UpdateGlobalMood(void *);

	void					AddDeferredGlobalEvent(const C_Event& e);
	void					SetLocalEvent(const C_Event& e, const S32 index);

	void					EnableEffectSystem(const CrowdEffectSystemType type, const U32 spawnRate, const F32 effectWaitTime);
	void					DisableEffectSystem(const CrowdEffectSystemType type);

	void					AddTextureList(const C_XtCrowdTextureList* pTextureList);
	void					RemoveTextureList(const C_XtCrowdTextureList* pTextureList);

	const bool				Ready() const;
	const bool				Deactivated() const;

	const C_XtCrowdPopulation*
							GetRoutePopulation() const { return m_pRoutePopulation; }

#ifndef CFG_GOLD
	void					UpdateRouteMask(const U32 routeMask, const C_XtCrowdPopulation* pPopulation);
#endif

	bool					Flush();

#ifndef CFG_GOLD
	void					DebugRenderEvents(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

private:
	const CrowdSystemState	GetState() const;
	void					SetState(const CrowdSystemState state);

	void					OnUpdate(const F32 deltaTime);

	void					LoadCrowdResources();
	void					DestroyCrowdActors();

	bool					CreateWorld();
	void					DestroyWorld();

	void					CreateEffectSystems();

	// Events
	void					UpdateEvents(const F32 elapsedTime);

	void					ClearActiveEvents();
	void					AddActiveEvent(const C_Event& e);

	void					ProcessDeferredGlobalEvents();

	static void				UpdateGlobalMood(const CrowdEventType eventType);

	void					UpdateTextureCache();

private:
	C_CrowdWorld*			GetWorld() const;
	C_CrowdResources*		GetResources() const;

	//typedef Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >					EventVector;
	typedef Btl::Vector<C_Event, Btl::VectorParameters_Array<CROWD_MAX_LOCAL_EVENTS> >				LocalEventArray;
	typedef Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_GLOBAL_DEFERRED_EVENTS>	>	GlobalEventVector;

private:
	SceneGraph::C_SceneGraph*			m_pSceneGraph;

	C_CrowdResources*					m_pResources;

	C_CrowdWorld*						m_pWorld;

	C_CrowdSpectators*					m_pSpectators;
	
	CrowdSystemState					m_State;
	U32									m_FrameCount;

	Scheduler::PolicyHandle				m_JobQueuePolicyHandle;

	// Listeners
	Btl::Vector<C_CrowdSystemListener*>	m_Listeners;

	// Events
	Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >
										m_ActiveEvents;
	C_Event								m_ActiveGlobalEvent;

	LocalEventArray						m_LocalEvents;
	GlobalEventVector					m_DeferredGlobalEvents;

	// Effects
	C_CrowdFlags*						m_pFlags;
	C_CrowdCameraFlashes*				m_pCameraFlashes;

	const C_XtCrowdPopulation*			m_pRoutePopulation;
	U32									m_RouteMask;

	Btl::Vector<const C_XtCrowdTextureList*>
										m_TextureLists;

	bool								m_TextureCacheDirty;
	C_CrowdTextureCache					m_TextureCache[E_TextureListUsage_Count];

	bool								m_WorldInsertedIntoSceneGraph;

	mutable bool						m_IsPaused;
	mutable bool						m_WasPaused;

	static CrowdGlobalMood				s_DeferredGlobalMood;

	friend class C_CrowdSystemActor;
};

}; // namespace AnimatedCrowd

#endif // CROWD_WORLD_H
