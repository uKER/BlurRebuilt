#include "stdafx.h"

#include "system\datalogging\DataLogging.h"

#include "Automation\GamePerfLogging.h"

#include "Framework/Application/PerformanceLogging.h"

#include <GameApp.h>
#include <Game/game.h>
#include "Game/Messaging/GameflowMessages.h"
#include "GameLogic/ActorStudioActors/ActorUtils.h"
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include "dynamic\particle\manager.h"
#include "dynamic\particle\effect3D.h"

#include "misc_shared/archive/archive.h"
#include "misc_shared\archive\serialiser.h"

// [Rog] I include this so I can get to the StreamData DebugInfo values
#define RENDERINGDATA_ALLOW_INCLUDE
//#include "renderingdata/build.h"
#include "renderingdata/target.h"
#include "renderingdata/streaming.h"
#include "world_render/renderlevel.h"

#include <Lod/LodManager.h>

#include "Game/GameUtil/LoadingPerf.h"

namespace AmaxPerfLogging
{
using namespace DataLog;
using namespace PerformanceLogging;

Debug::Logger &g_Log( Debug::GetLogger("performance.logging") );

// Temp variables
Time::C_Time g_timeFillStreamerStart;

class SystemTimers
{
public:
	int		m_count;
	float	m_time;
};

int g_uniqueLevelObjects;

SystemTimers g_systemTimers[kMaxUpdateTimer];

// Table members
DataLogU32*	m_dltLocationId			= NULL;
DataLogU32*	m_dltRouteId			= NULL;
DataLogU8*	m_dltNumCars			= NULL;
DataLogU32*	m_dltNumDynObj			= NULL;
DataLogU32* m_dltNumPhyObj			= NULL;
DataLogF32*	m_dltFillStreamerTime	= NULL;
DataLogU32* m_dltUniqueObjects		= NULL;

// Perf Stat
DataLogU32*	m_dltCommandBufferSize			= NULL;
DataLogU32*	m_dltStreamBytesRequested		= NULL;
DataLogU32*	m_dltStreamBytesLoaded			= NULL;
DataLogU32*	m_dltStreamBytesLoadedWanted	= NULL;

DataLogU32*	m_dltParticleEvents			= NULL;
DataLogU32*	m_dltParticleEmitters		= NULL;
DataLogU32*	m_dltParticleEventInstances	= NULL;
DataLogU32*	m_dltParticleCount			= NULL;

DataLogU8*	m_dltPhysicsUpdateCount		= NULL;
DataLogF32*	m_dltAiUpdate				= NULL;
DataLogF32*	m_dltPhysicsUpdate			= NULL;
DataLogF32*	m_dltUiUpdate				= NULL;
DataLogF32*	m_dltActorUpdate			= NULL;
DataLogF32*	m_dltLevelSceneUpdate		= NULL;
DataLogF32*	m_dltDynEffectRenUpdate		= NULL;
DataLogF32*	m_dltGeneric				= NULL;

DataLogU8*	m_dltPerkCount				= NULL;

DataLogF32*	m_dltLodCpuDetail			= NULL;
DataLogF32*	m_dltLodGpuDetail			= NULL;

void CreateTables(ePerfLogType type, DataLogTable* dlt)
{
	switch (type)
	{
	case kPerfSubSession:
		m_dltLocationId			= dlt->AddU32("LocationId");;
		m_dltRouteId			= dlt->AddU32("RouteId");
		m_dltNumCars			= dlt->AddU8("NumCars");
		m_dltNumDynObj			= dlt->AddU32("DynamicObjects");
		m_dltNumPhyObj			= dlt->AddU32("PhysicalObjects");
		m_dltFillStreamerTime	= dlt->AddF32("FillStreamer");
		m_dltUniqueObjects		= dlt->AddU32("UniqueObjects");
		break;
	
	case kPerfStat:
		m_dltCommandBufferSize			= dlt->AddU32("CmdBuffSize");
		m_dltStreamBytesRequested		= dlt->AddU32("StreamRequested");
		m_dltStreamBytesLoaded			= dlt->AddU32("StreamLoaded");
		m_dltStreamBytesLoadedWanted	= dlt->AddU32("StreamLoadedWanted");

		m_dltParticleEvents			= dlt->AddU32("ParticleEvents");
		m_dltParticleEmitters		= dlt->AddU32("ParticleEmitters");
		m_dltParticleEventInstances	= dlt->AddU32("ParticleEventInstances");
		m_dltParticleCount			= dlt->AddU32("ParticleCount");

		m_dltPhysicsUpdateCount		= dlt->AddU8("PhysicsUpdateCount");
		m_dltAiUpdate				= dlt->AddF32("AiUpdate");
		m_dltPhysicsUpdate			= dlt->AddF32("PhysicsUpdate");
		m_dltUiUpdate				= dlt->AddF32("UiUpdate");
		m_dltActorUpdate			= dlt->AddF32("ActorUpdate");
		m_dltLevelSceneUpdate		= dlt->AddF32("LevelSceneUpdate");
		m_dltDynEffectRenUpdate		= dlt->AddF32("DynEffectRenUpdate");
		m_dltGeneric				= dlt->AddF32("Generic");

		m_dltPerkCount				= dlt->AddU8("PerkCount");

		m_dltLodCpuDetail			= dlt->AddF32("LodCpuDetail");
		m_dltLodGpuDetail			= dlt->AddF32("LodGpuDetail");
		
		break;
	}
}

void ResourceCallback(const Resource::C_ResourceBase* pResource)
{
	const char* matchString = "models\\objects\\";

	if (!strnicmp(pResource->GetResourceFilename().Chars(), matchString, strlen(matchString)))
	{
		g_uniqueLevelObjects++;
	}
}

void UpdateTables(ePerfLogType type, DataLogEvent* e)
{
	switch (type)
	{
	case kPerfSubSession:
		{
			const Time::C_Time timeLoadEnded = PerformanceLogging::QueryCounter(kGameSubSessionLoaded);

			float streamerFillTime = (F32)(timeLoadEnded - g_timeFillStreamerStart) / (F32)Time::GetPerformanceFrequency();

			LOGINFO(g_Log, "PERF:        Streamer: %.2fsecs\n", streamerFillTime);

			if (e != NULL)
			{
				const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

				e->Set(m_dltLocationId,			levelState.mCity);
				e->Set(m_dltRouteId,			levelState.mRoute);
				e->Set(m_dltNumCars,			levelState.mNumCars);
				e->Set(m_dltNumDynObj,			::Actors::C_ActorUtils::Instance().NumDynamicActors());
				e->Set(m_dltNumPhyObj,			::Actors::C_ActorUtils::Instance().NumPhysicalActors());
				e->Set(m_dltFillStreamerTime,	streamerFillTime);
				e->Set(m_dltUniqueObjects,		g_uniqueLevelObjects);
			}
		}
		break;
	
	case kPerfStat:
		if (e != NULL)
		{
			int cmdBufferSize = Display::RingBuffer()->GetUsedSize();
			e->Set(m_dltCommandBufferSize, cmdBufferSize);

			Game::C_GameWorld *pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();

			// Streamer info
			if (GameResource::C_LevelSceneManager *pLevelSceneManager = pGameWorld->GetLevelSceneManager())
			{
				if (WorldRender::C_Scene* pWorldRenderScene = pLevelSceneManager->GetWorldRenderScene())
				{
					const Btl::List<WorldRender::C_LevelInstance>& rlil = pWorldRenderScene->GetRenderLevelInstanceList();

					Btl::List<WorldRender::C_LevelInstance>::ConstIterator i = rlil.Begin();
					for (; i != rlil.End(); ++i)
					{
						const RenderingData::Target::StreamData& streamData = i->GetRenderLevel()->GetScene().GetStreamData();

						RenderingData::Target::StreamData::DebugInfoStats stats;

						streamData.DebugInfo(stats);

						e->Set(m_dltStreamBytesRequested,		(U32)(stats.bytesRequested / 1024));
						e->Set(m_dltStreamBytesLoaded,			(U32)(stats.bytesLoaded / 1024));
						e->Set(m_dltStreamBytesLoadedWanted,	(U32)(stats.bytesLoadedAndRequested / 1024));
					}
				}
			}

			// Particle stats
			DynamicEffect::Particle::C_ParticleManager* particleManager = pGameWorld->GetParticleManager();

			e->Set(m_dltParticleEvents, particleManager->GetEvents().Size());
			e->Set(m_dltParticleEmitters, particleManager->GetEmitterList()->m_ActiveElements);
			e->Set(m_dltParticleEventInstances, particleManager->GetEventInstanceList().m_ActiveElements);

			S32 particleCount = 0;
			for (int i=0; i < particleManager->GetActiveEffects().Size(); ++i)
				particleCount += particleManager->GetActiveEffects()[i]->GetNumParticles();

			e->Set(m_dltParticleCount, particleCount);

			e->Set(m_dltPhysicsUpdateCount,	g_systemTimers[kPhysicsUpdate].m_count);
			e->Set(m_dltAiUpdate,			g_systemTimers[kAiUpdate].m_time);
			e->Set(m_dltPhysicsUpdate,		g_systemTimers[kPhysicsUpdate].m_time);
			e->Set(m_dltUiUpdate,			g_systemTimers[kUiUpdate].m_time);
			e->Set(m_dltActorUpdate,		g_systemTimers[kActorUpdate].m_time);
			e->Set(m_dltLevelSceneUpdate,	g_systemTimers[kLevelSceneUpdate].m_time);
			e->Set(m_dltDynEffectRenUpdate,	g_systemTimers[kDynEffectRenderUpdate].m_time);
			e->Set(m_dltGeneric,			g_systemTimers[kGeneric].m_time);

			e->Set(m_dltPerkCount,			(U8)g_systemTimers[kPerkInfo].m_time);

			e->Set(m_dltLodCpuDetail,		Lod::C_LodManager::Instance().GetCpuDetail());
			e->Set(m_dltLodGpuDetail,		Lod::C_LodManager::Instance().GetGpuDetail());
		}
		break;
	}
}

void UpdateCounters(ePerfStage pi)
{
#ifndef CFG_GOLD
	if (pi == kGameBeginFrame)
	{
		memset(g_systemTimers, 0, sizeof(g_systemTimers));
	}
	else if (pi == kGameSubSessionLoaded)
	{
		// Count up all the unique level objects
		g_uniqueLevelObjects = 0;
		Resource::C_Manager::GetInstance()->DebugForEach(ResourceCallback);
	}
#endif
}

void Initialise()
{
	PerformanceLogging::Initialise("AmaxPerf", AmaxPerfLogging::CreateTables, AmaxPerfLogging::UpdateTables, AmaxPerfLogging::UpdateCounters);
}

void BeginFillStreamer()
{
	g_timeFillStreamerStart = Time::GetPerformanceCounter();
}

// TODO: Need to clear these out
void AccumulateDuration(eDurationTimer timer, float duration)
{
	g_systemTimers[timer].m_time += duration;
	g_systemTimers[timer].m_count++;
}

/*
	Handle relevant messages sent by the game code
*/
MessageController::MessageController(Messaging::C_MessageSubscriberSystem* pSubscriberSystem)
{
#ifndef CFG_GOLD
	SetSubscriberSystem(pSubscriberSystem);

	AddSubscriber(Game::E_GameflowMessage_StartRaceLoad);
	AddSubscriber(Game::E_GameflowMessage_StreamerFull);
	AddSubscriber(Game::E_GameflowMessage_StartUILoad);
	AddSubscriber(Game::E_GameflowMessage_UILoaded);
#endif
}

MessageController::~MessageController()
{
}

bool MessageController::MessageHandler( const Messaging::C_MessageBase *pMessage )
{
	bool handledMessage = true;

	switch (pMessage->Type())
	{
	case Game::E_GameflowMessage_StartRaceLoad:
		#ifdef ENABLE_LOAD_PERF
		// Sorry Roger, I borrowed your message handler - Paul M
		LoadPerf::Push("Loading");
		#endif
		PerformanceLogging::UpdateCounters(PerformanceLogging::kGameSubSessionStarted);
		break;
	
	case Game::E_GameflowMessage_StreamerFull:
		#ifdef ENABLE_LOAD_PERF
		// Sorry Roger, I borrowed your message handler - Paul M
		if (LoadPerf::ProfilingStarted())
		{
			LoadPerf::Pop("LoadDuringUpdate"); // matches push at end of gameworld create
			LoadPerf::Pop();
		}
		#endif
		PerformanceLogging::UpdateCounters(PerformanceLogging::kGameSubSessionLoaded);
		break;

	case Game::E_GameflowMessage_StartUILoad:

		// Don't do this coming back from a debug race as it's all a bit different
		if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_Debug)
		{
			#ifdef ENABLE_LOAD_PERF
			LoadPerf::Push("Loading");
			#endif
		}

		PerformanceLogging::UpdateCounters(PerformanceLogging::kGameSubSessionEnded);
		break;

	case Game::E_GameflowMessage_UILoaded:
#ifdef ENABLE_LOAD_PERF
		if (LoadPerf::ProfilingStarted())
		{
			LoadPerf::Pop("Loading");
		}
#endif
		break;

	default:
		handledMessage = false;
		break;
	}

	return handledMessage;
}

}
