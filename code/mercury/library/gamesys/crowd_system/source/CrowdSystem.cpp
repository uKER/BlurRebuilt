#include "CrowdSystemPch.h"

#include <crowd_system/CrowdSystem.h>
#include <crowd_system/CrowdSystemActor.h>
#include <crowd_system/CrowdMessaging.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdWorld.h>
#include <crowd_system/CrowdSpectators.h>
#include <crowd_system/CrowdDebug.h>
#include <crowd_system/CrowdRandom.h>
#include <crowd_system/CrowdTextureCache.h>
#include <crowd_system/CrowdBlockRef.h>

#include <crowd_system/effects/CrowdFlags.h>
#include <crowd_system/effects/CrowdCameraFlashes.h>

#include <animation/task/TaskManager.h>

#include <scenegraph/scenegraph.h>
#include <actor_system/ActorSys.h>
#include <game_resource/CrowdData.h>
#include <misc_shared/Messaging/MessageDebug.h>
#include <misc_shared/Messaging/Messaging.h>
#include <system/Perf.h>

#ifdef CROWD_DEBUG_TIME_CREATE_WORLD
#include <system/time/time.h>
#endif

DEBUG_ENUM(AnimatedCrowd::CrowdGlobalMood)
	DE_OPTION(AnimatedCrowd::E_CROWD_MOOD_UNHAPPY,				"Unhappy")
	DE_OPTION(AnimatedCrowd::E_CROWD_MOOD_EXCITEMENT_LOW,		"Idle")
	DE_OPTION(AnimatedCrowd::E_CROWD_MOOD_EXCITEMENT_MEDIUM,	"Interested")
	DE_OPTION(AnimatedCrowd::E_CROWD_MOOD_EXCITEMENT_HIGH,		"Excited")
	DE_OPTION(AnimatedCrowd::E_CROWD_MOOD_EXCITEMENT_VERY_HIGH,	"Ecstatic")
END_DEBUG_ENUM

namespace AnimatedCrowd
{
Debug::Bool						g_Disable("GameSys.CrowdSystem.DisableCreationOfCrowdSystem",		false, Debug::Persist());

#ifndef CFG_GOLD
Debug::Bool						g_DebugRenderStats("GameSys.CrowdSystem.DrawStats",					false, Debug::Persist());
#endif // CFG_GOLD

Debug::Enum<CrowdGlobalMood>	g_GlobalMood("GameSys.CrowdSystem.Spectators.GlobalMood",			E_CROWD_MOOD_EXCITEMENT_LOW);
static Debug::Function			UpdateGlobalMood("GameSys.CrowdSystem.Spectators.UpdateGlobalMood",	C_CrowdSystem::UpdateGlobalMood, 0);

Debug::Float					g_FlagWidth("GameSys.CrowdSystem.Effects.Flags.Width",					1.143f, Debug::Range(0.5f, 4.f));
Debug::Float					g_FlagHeight("GameSys.CrowdSystem.Effects.Flags.Height",				0.9144f, Debug::Range(0.5f, 4.f));
Debug::Float					g_FlagOffset("GameSys.CrowdSystem.Effects.Flags.Offset",				1.15f);
Debug::Float					g_FlagStretchFactor("GameSys.CrowdSystem.Effects.Flags.StretchFactor",	0.98f, Debug::Range(0.f, 2.f));
Debug::Float					g_FlagWindCoeff("GameSys.CrowdSystem.Effects.Flags.WindCoeff",			0.1f, Debug::Range(0.f, 1.f));
Debug::Float					g_FlagParticleMass("GameSys.CrowdSystem.Effects.Flags.ParticleMass",	0.01f, Debug::Range(0.f, 1.f));

bool C_CrowdSystem::DisableCreation()
{
	return g_Disable;
}

CrowdGlobalMood C_CrowdSystem::s_DeferredGlobalMood	= E_CROWD_MOOD_EXCITEMENT_LOW;

C_CrowdSystem::C_CrowdSystem()
	: m_pSceneGraph(NULL),
	m_pResources(NULL),
	m_pWorld(NULL),
	m_pSpectators(NULL),
	m_State(E_CROWD_STATE_CREATED),
	m_FrameCount(0),
	m_JobQueuePolicyHandle(-1),
	m_pRoutePopulation(NULL),
	m_WorldInsertedIntoSceneGraph(false),
	m_pFlags(NULL),
	m_pCameraFlashes(NULL),
	m_TextureCacheDirty(false),
	m_IsPaused(true),
	m_WasPaused(true)
{
}

C_CrowdSystem::~C_CrowdSystem()
{
	Destroy();
}

void C_CrowdSystem::Create(SceneGraph::C_SceneGraph* pSceneGraph, Actor::C_ActorList* pActorList, const GameResource::C_CrowdData* pCrowdData,
						   const U32 routeMask, const C_XtCrowdPopulation* pCrowdPopulation, const U32 viewTypeFlags)
{
	m_pSceneGraph = pSceneGraph;

	m_pRoutePopulation = pCrowdPopulation;
	m_RouteMask = routeMask;

	if (!pActorList || !pCrowdData)
		return;

	AnimatedCrowd::C_Random::Instance().Initialise(pCrowdData->GetRandomSeed());

	Actor::C_ActorCreationDataFactoryHelper<C_CrowdSystemCreate> pCrowdSystemActorCreate;
	pCrowdSystemActorCreate->m_pCrowdSystem	= this;
	
	Actor::C_ActorHandle crowdActorHandle = pActorList->Spawn(pCrowdSystemActorCreate.Get());

	if (!m_pResources)
		m_pResources = new C_CrowdResources(pCrowdData, pSceneGraph, routeMask);

	BIZ_ASSERT(m_pResources);

	pActorList->GetSubscriberSystem().BroadcastMessage( E_CrowdMsg_LoadResources );

	if (!m_pWorld)
		m_pWorld = new C_CrowdWorld(m_pSceneGraph, viewTypeFlags);

	BIZ_ASSERT(m_pWorld);

	if (!m_pSpectators)
		m_pSpectators = new C_CrowdSpectators(m_pResources);

	BIZ_ASSERT(m_pSpectators);

	SetState(E_CROWD_STATE_INITIALISED);

	s_DeferredGlobalMood = E_CROWD_MOOD_EXCITEMENT_LOW;
	m_pWorld->SetGlobalMood(s_DeferredGlobalMood);

	m_FrameCount = 0;
	m_TextureCacheDirty = true;
}

void C_CrowdSystem::Destroy()
{
	DestroyWorld();

	ClearActiveEvents();

	for (U32 i = 0, count = E_TextureListUsage_Count; i < count; ++i)
		m_TextureCache[i].Destroy();

	SAFE_DELETE_PTR(m_pCameraFlashes);

	SAFE_DELETE_PTR(m_pSpectators);

	SAFE_DELETE_PTR(m_pResources);
}

void C_CrowdSystem::Reset()
{
	SetState(E_CROWD_STATE_RESET);

	// TODO: [rthomas]	Does anything need reset?
}

void C_CrowdSystem::Reactivate(Actor::C_ActorList* pActorList)
{
	if (!pActorList || (m_State != E_CROWD_STATE_RESET))
		return;

	Actor::C_ActorCreationDataFactoryHelper<C_CrowdSystemCreate> pCrowdSystemActorCreate;
	pCrowdSystemActorCreate->m_pCrowdSystem = this;

	Actor::C_ActorHandle crowdActorHandle = pActorList->Spawn(pCrowdSystemActorCreate.Get());

	SetState(E_CROWD_STATE_READY);
}

void C_CrowdSystem::Update(const Maths::HVec3& cameraPosition, const F32 deltaTime)
{
	PERF_EVENT("CrowdUpdate");

	switch (GetState())
	{
	case E_CROWD_STATE_INITIALISED:
		{
			if (m_pResources)
			{
				if (m_pResources->IsLoaded())
					SetState(E_CROWD_STATE_RESOURCES_LOADED);
			}
		}
		break;
	case E_CROWD_STATE_RESOURCES_LOADED:
		{
			if (m_pResources)
			{
#ifdef CROWD_DEBUG_TIME_CREATE_WORLD
				//################################ GET RID OF ME ################################//
				Time::C_Time timeBegin = Time::GetPerformanceCounter();
#endif

				if (CreateWorld())
					SetState(E_CROWD_STATE_POST_INITIALISE);

#ifdef CROWD_DEBUG_TIME_CREATE_WORLD
				Time::C_Time timeEnd = Time::GetPerformanceCounter();

				Time::C_Time duration = timeEnd - timeBegin;
				LOGWARN(g_CrowdLog, "Creating crowd world: %s", Time::FormatProfileDurationString(duration).Chars());
				//################################ GET RID OF ME ################################//
#endif
			}
		}
		break;
	case E_CROWD_STATE_POST_INITIALISE:
		{
			CreateEffectSystems();
			SetState(E_CROWD_STATE_READY);
		}
		break;
	case E_CROWD_STATE_READY:
		{
			OnUpdate(deltaTime);
		}
		break;
	case E_CROWD_STATE_RELOAD_WORLD:
		{
			OnUpdate(deltaTime);
			m_pWorld->Destroy();
			SetState(E_CROWD_STATE_RESOURCES_LOADED);
		}
		break;
	case E_CROWD_STATE_RESET:
		{
			// TODO: [rthomas]	Does anything need doing here?
		}
		break;
	default:
		break;
	}
}

void C_CrowdSystem::UpdateEffects(const F32 deltaTime)
{
	if (!Ready())
		return;

	if (!m_pResources->IsLoaded())
		return;

	if (m_pWorld)
	{
		PERF_EVENT("CrowdUpdate.Effects");
			
		if (m_pCameraFlashes)
			m_pCameraFlashes->Update(deltaTime, m_pWorld->GetVisibleBlockData(), m_pResources->GetCrowdData());

		if (m_pFlags)
		{
#ifndef CFG_GOLD
			if (g_FlagWidth != m_pFlags->GetFlagWidth() || g_FlagHeight != m_pFlags->GetFlagHeight()
				|| g_FlagStretchFactor != m_pFlags->GetFlagStretchFactor() || g_FlagWindCoeff != m_pFlags->GetFlagWindCoeff() || g_FlagParticleMass != m_pFlags->GetFlagParticleMass())
			{
				m_pFlags->ResizeFlagMesh(g_FlagWidth, g_FlagHeight, g_FlagStretchFactor, g_FlagParticleMass, g_FlagWindCoeff);
				m_pFlags->SetFlags(NULL, 0);
			}
			else
				m_pFlags->SetFlags(m_pResources->GetFlags(), m_pResources->GetNumFlags());
#else
			m_pFlags->SetFlags(m_pResources->GetFlags(), m_pResources->GetNumFlags());
#endif
		}

		m_pWorld->UpdateEffects(deltaTime);
	}
}

void C_CrowdSystem::BeginFrame(const bool isPaused) const
{
	PERF_EVENT("CrowdUpdate.BeginFrame");

	m_WasPaused = m_IsPaused;
	m_IsPaused = isPaused;

	if (Ready())
	{
		if (m_pFlags)
		{
			for (U32 i = 0, count = m_pResources->GetNumFlagAnimations(); i < count; ++i)
			{
				if (m_pResources->GetFlagAnimation(i) >= 0)
				{
					C_CrowdSharedAnimation* pSharedAnimation = m_pResources->GetSharedAnimationPool()->GetAnimation(m_pResources->GetFlagAnimation(i));
					if (pSharedAnimation && (pSharedAnimation->GetAnimation().GetIndex() != 0xff) && pSharedAnimation->GetAnimation().IsValid())
						m_pFlags->UpdateClothPins(i, pSharedAnimation->GetAnimation().GetJointTransform(m_pResources->GetPropJointIndex()));
				}
			}
			
			if (!(m_WasPaused && m_IsPaused))
				m_pFlags->ScheduleClothSimulationTask(m_JobQueuePolicyHandle, (1.f / 30.f));
		}
	}
}

void C_CrowdSystem::Schedule() const
{
	PERF_EVENT("CrowdUpdate.Schedule");

	if (Ready())
	{
		if (m_pWorld)
			m_pWorld->ScheduleBlockVisibilityTasks(&m_ActiveEvents, m_JobQueuePolicyHandle);

		if (!m_IsPaused || (!m_WasPaused && m_IsPaused))
		{
			if (m_pFlags)
				m_pFlags->FlushClothSimulationTask();
		}
	}
}

#ifndef CFG_GOLD
void C_CrowdSystem::RenderFullscreen()
{
	if (!g_DebugRenderStats)
		return;

	if (m_pWorld)
		m_pWorld->RenderStats();
}
#endif

void C_CrowdSystem::AddListener(C_CrowdSystemListener* pListener)
{
	m_Listeners.PushBack(pListener);
}

void C_CrowdSystem::RemoveListener(C_CrowdSystemListener* pListener)
{
	for (Btl::Vector<C_CrowdSystemListener*>::Iterator iter = m_Listeners.Begin(); iter != m_Listeners.End(); ++iter)
	{
		if (*iter == pListener)
		{
			m_Listeners.Erase(iter);
			return;
		}
	}
}

void C_CrowdSystem::SetFlagRenderer(C_CrowdFlags* pFlags)
{
	m_pFlags = pFlags;
	if (m_pFlags)
		m_pFlags->SetTextureCache(&m_TextureCache[E_TextureListUsage_Flags]);
}

void C_CrowdSystem::UpdateGlobalMood(void *)
{
	s_DeferredGlobalMood = g_GlobalMood;
}

const bool C_CrowdSystem::Ready() const
{
	return (GetState() == E_CROWD_STATE_READY);
}

const bool C_CrowdSystem::Deactivated() const
{
	return (GetState() == E_CROWD_STATE_RESET);
}

#ifndef CFG_GOLD
void C_CrowdSystem::UpdateRouteMask(const U32 routeMask, const C_XtCrowdPopulation* pPopulation)
{
	if (m_RouteMask != routeMask)
	{
		if (Ready() && m_pWorld)
		{
			m_pRoutePopulation = pPopulation;
			m_RouteMask = routeMask;

			// TODO: [rthomas]	Flushing causes crashes ...sometimes
			/*for (Btl::Vector<C_CrowdSystemListener*>::Iterator iter = m_Listeners.Begin(); iter != m_Listeners.End(); ++iter)
			{
				if (*iter)
					(*iter)->OnCrowdSystemRouteChanged();
			}*/

			SetState(E_CROWD_STATE_RELOAD_WORLD);
		}
	}
}
#endif

bool C_CrowdSystem::Flush()
{
	//Animation::TaskManager().Flush();

	if (m_pResources)
	{
		if (!m_pResources->Flush())
			return false;
	}

	if (m_pSpectators)
	{
		if (!m_pSpectators->Flush())
			return false;
	}

	if (m_pWorld)
	{
		if (!m_pWorld->Flush())
			return false;
	}

	return true;
}

#ifndef CFG_GOLD
void C_CrowdSystem::DebugRenderEvents(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	debugRenderPtr->SetDepthEnable(false);
	for (U32 i = 0, count = static_cast<U32>(m_ActiveEvents.Size()); i < count; ++i)
	{
		if (m_ActiveEvents[i].GetAreaType() == C_Event::E_AREA_TYPE_CYLINDER)
		{
			debugRenderPtr->Draw3dCircle(m_ActiveEvents[i].GetPosition() - Maths::HVec3(0.f, m_ActiveEvents[i].GetHalfHeight(), 0.f),
				Maths::HVec3(0.f, 1.f, 0.f), m_ActiveEvents[i].GetRadius(), CROWD_COLOUR_WHITE);
			debugRenderPtr->Draw3dCircle(m_ActiveEvents[i].GetPosition() + Maths::HVec3(0.f, m_ActiveEvents[i].GetHalfHeight(), 0.f),
				Maths::HVec3(0.f, 1.f, 0.f), m_ActiveEvents[i].GetRadius(), CROWD_COLOUR_WHITE);
		}
	}
	debugRenderPtr->SetDepthEnable(true);
}
#endif

const CrowdSystemState C_CrowdSystem::GetState() const
{
	return m_State;
}

void C_CrowdSystem::SetState(const CrowdSystemState state)
{
	m_State = state;
}

void C_CrowdSystem::PreUpdateWorld(const F32 deltaTime)
{
	if (!Ready())
		return;

	if (!m_pResources->IsLoaded())
		return;

	if (m_TextureCacheDirty)
		UpdateTextureCache();

	m_pResources->ClearFlags();

	if (m_pWorld)
	{
		if (C_CrowdWorld::GetGlobalMood() != s_DeferredGlobalMood)
			m_pWorld->SetGlobalMood(s_DeferredGlobalMood);

		{
			PERF_EVENT("CrowdUpdate.WorldPreUpdate");
			m_pWorld->PreUpdate(m_ActiveEvents, &m_TextureCache[E_TextureListUsage_Flags], m_JobQueuePolicyHandle, deltaTime, m_FrameCount); // Last frame's events
		}
	}
}

void C_CrowdSystem::OnUpdate(const F32 deltaTime)
{
	if (!Ready())
		return;

	if (!m_pResources->IsLoaded())
		return;

	if (m_pWorld)
	{
		PERF_EVENT("CrowdUpdate.World");
		m_pWorld->Update(deltaTime, m_FrameCount); // Last frame's events
	}

	{
		PERF_EVENT("CrowdUpdate.Events");
		UpdateEvents(deltaTime);
	}

	Animation::TaskManager().ScheduleQueuedTasks();

	if (m_pResources)
	{
		PERF_EVENT("CrowdUpdate.Purge");
		m_pResources->PurgeAnimationPools();
	}

	m_FrameCount++;
}

bool C_CrowdSystem::CreateWorld()
{
	if (!m_pWorld)
		return false;

	if (m_pWorld->Create(m_pResources, m_pSpectators, m_RouteMask))
	{
		if (!m_WorldInsertedIntoSceneGraph)
		{
			SceneGraph::Insert(m_pSceneGraph, m_pWorld);
			m_WorldInsertedIntoSceneGraph = true;
		}

		return true;
	}

	return false;
}

void C_CrowdSystem::DestroyWorld()
{
	if (!m_pWorld)
		return;

	m_pWorld->Destroy();
	SceneGraph::Remove(m_pSceneGraph, m_pWorld);
	m_WorldInsertedIntoSceneGraph = false;
	
	SAFE_DELETE_PTR(m_pWorld);
}

void C_CrowdSystem::CreateEffectSystems()
{
	if (!m_pResources)
		return;
	
	const C_CrowdPopulation& population = m_pResources->GetPopulation();

	for (U32 i = 0, count = E_CROWD_EFFECT_SYSTEM_COUNT; i < count; ++i)
	{
		DynamicEffect::Particle::C_Event* pEvent = population.GetEffectSystemEvent(static_cast<CrowdEffectSystemType>(i));

		if (pEvent)
		{
			switch (i)
			{
			case E_CROWD_EFFECT_SYSTEM_CAMERA_FLASHES:
				{
					if (!m_pCameraFlashes)
					{
						m_pCameraFlashes = new C_CrowdCameraFlashes(pEvent);

						// TODO: [rthomas] Put in sorcery or something
						m_pCameraFlashes->EnableBehaviour(population.GetBehaviourIndex(Btl::Hash("Standing")));
						m_pCameraFlashes->EnableBehaviour(population.GetBehaviourIndex(Btl::Hash("Standing_Phone")));
						m_pCameraFlashes->EnableBehaviour(population.GetBehaviourIndex(Btl::Hash("Standing_HandsHips")));
						m_pCameraFlashes->EnableBehaviour(population.GetBehaviourIndex(Btl::Hash("Standing_HandsHips_Phone")));
					}
				}
				break;
			default:
				break;
			}
		}
	}

	S32 flagWaverIndex = population.GetBehaviourIndex(Btl::Hash("Flag_Waver"));
	
	if (flagWaverIndex >= 0)
	{
		U32 maxNumFlagWavers = m_pResources->GetMaxSharedAnimationsByBehaviour(flagWaverIndex);

		if (maxNumFlagWavers)
		{
			if (m_pFlags)
			{
				m_pFlags->BuildFlagMesh(g_FlagWidth, g_FlagHeight, 5, 4, maxNumFlagWavers, g_FlagStretchFactor, g_FlagParticleMass, g_FlagWindCoeff);
				
				m_pResources->SetFlagInstanceNodeTypes(m_pFlags);
				m_pResources->SetFlagBehaviourIndex(m_pResources->GetPopulation().GetBehaviourIndex(Btl::Hash("Flag_Waver")));
			}
		}
	}
}

void C_CrowdSystem::UpdateEvents(const F32 elapsedTime)
{
	ClearActiveEvents();

	ProcessDeferredGlobalEvents();

	// Add some active events here

	if (!m_ActiveGlobalEvent.HasExpired())
	{
		m_ActiveGlobalEvent.Update(elapsedTime);
		AddActiveEvent(m_ActiveGlobalEvent);
	}

	for (U32 i = 0, count = CROWD_MAX_LOCAL_EVENTS; i < count; ++i)
	{
		if (m_ActiveGlobalEvent.HasExpired() || m_ActiveGlobalEvent.GetPriority() <= m_LocalEvents[i].GetPriority())
			AddActiveEvent(m_LocalEvents[i]);
		
		m_LocalEvents[i].Update(elapsedTime);
	}
}

void C_CrowdSystem::ClearActiveEvents()
{
	m_ActiveEvents.Clear();
}

void C_CrowdSystem::AddActiveEvent(const C_Event& e)
{
	if (m_ActiveEvents.Size() < m_ActiveEvents.Capacity())
	{
		if (!e.HasExpired())
			m_ActiveEvents.PushBack(e);
	}
	else
		LOGWARN ( g_CrowdLog, "Active event buffer full." );
}

void C_CrowdSystem::AddDeferredGlobalEvent(const C_Event& e)
{
	if (m_DeferredGlobalEvents.Size() < m_DeferredGlobalEvents.Capacity())
		m_DeferredGlobalEvents.PushBack(e);
	else
		LOGWARN ( g_CrowdLog, "Deferred global event buffer full." );
}

void C_CrowdSystem::SetLocalEvent(const C_Event& e, const S32 index)
{
	BIZ_ASSERT(index < CROWD_MAX_LOCAL_EVENTS);

	if (m_LocalEvents[index].HasExpired() || e.GetPriority() >= m_LocalEvents[index].GetPriority())
	{
		m_LocalEvents[index] = e;
	}
}

void C_CrowdSystem::EnableEffectSystem(const CrowdEffectSystemType type, const U32 spawnRate, const F32 effectWaitTime)
{
	if (type < E_CROWD_EFFECT_SYSTEM_COUNT)
	{
		switch (type)
		{
		case E_CROWD_EFFECT_SYSTEM_CAMERA_FLASHES:
			{
				if (m_pCameraFlashes)
					m_pCameraFlashes->Enable(spawnRate, effectWaitTime);
			}
			break;
		default:
			break;
		}
	}
}

void C_CrowdSystem::DisableEffectSystem(const CrowdEffectSystemType type)
{
	if (type < E_CROWD_EFFECT_SYSTEM_COUNT)
	{
		switch (type)
		{
		case E_CROWD_EFFECT_SYSTEM_CAMERA_FLASHES:
			{
				if (m_pCameraFlashes)
					m_pCameraFlashes->Disable();
			}
			break;
		default:
			break;
		}
	}
}

void C_CrowdSystem::AddTextureList(const C_XtCrowdTextureList* pTextureList)
{
	if (!pTextureList)
		return;

	m_TextureLists.PushBack(pTextureList);

	m_TextureCacheDirty = true;
}

void C_CrowdSystem::RemoveTextureList(const C_XtCrowdTextureList* pTextureList)
{
	if (!pTextureList)
		return;

	U32 i = 0;
	while (i < static_cast<U32>(m_TextureLists.Size()))
	{
		if (pTextureList == m_TextureLists[i])
			m_TextureLists.EraseByIndexNonOrdered(i);
		else
			i++;
	}

	m_TextureCacheDirty = true;
}

void C_CrowdSystem::ProcessDeferredGlobalEvents()
{
	for (S32 i = 0, count = m_DeferredGlobalEvents.Size(); i < count; ++i)
	{
		const C_Event& crowdEvent(m_DeferredGlobalEvents[i]);

		if (crowdEvent.GetPriority() >= m_ActiveGlobalEvent.GetPriority() || m_ActiveGlobalEvent.HasExpired())
		{
			switch (crowdEvent.GetType())
			{
			case E_CROWD_EVENT_GLOBAL_UNHAPPY:
			case E_CROWD_EVENT_GLOBAL_IDLE:
			case E_CROWD_EVENT_GLOBAL_INTERESTED:
			case E_CROWD_EVENT_GLOBAL_EXCITED:
			case E_CROWD_EVENT_GLOBAL_ECSTATIC:
			case E_CROWD_EVENT_STATE_OVERRIDE:
				m_ActiveGlobalEvent = crowdEvent;
				break;
			default:
				BIZ_ASSERT_FAIL("Only global events should be handled here");
				break;
			}
		}
	}

	m_DeferredGlobalEvents.Clear();
}

void C_CrowdSystem::UpdateTextureCache()
{
	for (U32 i = 0, count = E_TextureListUsage_Count; i < count; ++i)
		m_TextureCache[i].BeginBuild();

	for (U32 i = 0, count = static_cast<U32>(m_TextureLists.Size()); i < count; ++i)
	{
		if (m_TextureLists[i])
			m_TextureCache[m_TextureLists[i]->m_Usage].AddTextureList(m_TextureLists[i]);
	}

	for (U32 i = 0, count = E_TextureListUsage_Count; i < count; ++i)
		m_TextureCache[i].EndBuild();

	m_TextureCacheDirty = false;
}

C_CrowdWorld* C_CrowdSystem::GetWorld() const
{
	return m_pWorld;
}

C_CrowdResources* C_CrowdSystem::GetResources() const
{
	return m_pResources;
}

#ifndef CFG_GOLD

void RegisterMessageDebugInfo()
{
	const char* pMessageNames [] = 
	{
		"E_CrowdMsg_LoadResources",
	};

	REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, E_CrowdMsg_Count, E_CrowdMsg_Begin);
}

#endif

}; // namespace AnimatedCrowd
