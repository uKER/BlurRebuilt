#include "stdafx.h"

#include <Game/GameActors/Crowds/CrowdGameActor.h>

#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/GameFlowMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <GameLogic/Messaging/GameLogicMessages.h>

#include <crowd_system/CrowdSystem.h>
#include <crowd_system/CrowdRandom.h>

#include <GameApp.h>
#include <GameLogic/GameData/GameEffectData.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/ActorDef/GameActorList.h>

#include <game_resource/LevelSceneManager.h>
#include "Game/Game.h"

namespace Game
{
	Debug::Int		g_CrowdCameraFlashesSpawnRate("Game.Crowd.Effects.CameraFlashes.SpawnRate", 8, Debug::Range(0, 100), Debug::Persist());
	Debug::Float	g_CrowdCameraFlashesWaitTimeLower("Game.Crowd.Effects.CameraFlashes.WaitTimeLower", 1.5f, Debug::Range(0.f, 2.f), Debug::Persist());
	Debug::Float	g_CrowdCameraFlashesWaitTimeUpper("Game.Crowd.Effects.CameraFlashes.WaitTimeUpper", 3.0f, Debug::Range(2.f, 4.f), Debug::Persist());

	C_CrowdGameManager* C_CrowdGameManager::s_pInstance = NULL;

	namespace Crowd
	{
		//
		// C_VehicleInfo
		//
		C_VehicleInfo::C_VehicleInfo()
			: m_RacePosition(0),
			m_PreviousLapTime(-FLT_MAX),
			m_PreviousLapOwnage(0),
			m_MegaFonzies(0)
		{ }

		void C_VehicleInfo::Initialise(const U8 racePosition)
		{
			m_RacePosition = racePosition;
		}

		void C_VehicleInfo::UpdateMegaFonzies(const U8 racePosition, const F32 lapTime)
		{
			// TODO: [rthomas]
		}

		const S32 C_VehicleInfo::GetMegaFonzies() const
		{
			return m_MegaFonzies;
		}

		//
		// C_VehicleEvent
		//
		C_VehicleEvent::C_VehicleEvent()
			: m_EventType(E_EVENT_UNKNOWN),
			m_Duration(0.f),
			m_Lifetime(0.f)
		{ }

		C_VehicleEvent::C_VehicleEvent(const VehicleEventType eventType)
			: m_EventType(eventType),
			m_Duration(0.f),
			m_Lifetime(0.f)
		{ }

		void C_VehicleEvent::Set(const VehicleEventType eventType, const Maths::HVec3& position)
		{
			if (HasExpired() || GetPriority(eventType) >= m_Priority)
			{
				m_EventType = eventType;
				m_Priority = GetPriority(eventType);
				m_Position = position;
				m_Direction = Maths::HVec3(0.f, 0.f, 1.f);
				m_Duration = GetDuration(eventType);
				m_Lifetime = m_Duration;
			}
		}

		void C_VehicleEvent::Set(const VehicleEventType eventType, const Maths::HVec3& position, const Maths::HVec3& direction)
		{
			if (HasExpired() || GetPriority(eventType) >= m_Priority)
			{
				m_EventType = eventType;
				m_Priority = GetPriority(eventType);
				m_Position = position;
				m_Direction = direction;
				m_Duration = GetDuration(eventType);
				m_Lifetime = m_Duration;
			}
		}

		void C_VehicleEvent::Update(const F32 deltaTime)
		{
			m_Lifetime -= deltaTime;
		}

		const VehicleEventType C_VehicleEvent::GetEventType() const
		{
			return m_EventType;
		}

		const bool C_VehicleEvent::HasExpired() const
		{
			return (m_Lifetime <= 0.f);
		}

		const Maths::HVec3& C_VehicleEvent::GetPosition() const
		{
			return m_Position;
		}

		const Maths::HVec3& C_VehicleEvent::GetDirection() const
		{
			return m_Direction;
		}

		const F32 C_VehicleEvent::GetDuration(const VehicleEventType eventType)
		{
			static F32 durations[Crowd::E_EVENT_COUNT + 1] =
			{
				0.1f,	// E_EVENT_CAR_NEARBY
				0.25f,	// E_EVENT_OVERTAKE
				0.5f,	// E_EVENT_CRASH,
				0.1f,	// E_EVENT_CAR_WRONG_WAY

				0.f		// E_EVENT_UNKNOWN
			};

			return durations[eventType];
		}

		const U8 C_VehicleEvent::GetPriority(const VehicleEventType eventType)
		{
			static U8 priorities[Crowd::E_EVENT_COUNT + 1] =
			{
				1,	// E_EVENT_CAR_NEARBY
				5,	// E_EVENT_OVERTAKE
				10,	// E_EVENT_CRASH
				2,	// E_EVENT_CAR_WRONG_WAY

				0	// E_EVENT_UNKNOWN
			};

			return priorities[eventType];
		}

	} // namespace Crowd

	//
	// C_CrowdManager
	//
	C_CrowdGameManager::C_CrowdGameManager()
		: m_pCrowdSystem(NULL),
		m_RaceLoaded(0)
	{
		BIZ_ASSERT(!s_pInstance);
		s_pInstance = this;
	}

	C_CrowdGameManager::~C_CrowdGameManager()
	{
		s_pInstance = NULL;
	}

	bool C_CrowdGameManager::Create(AnimatedCrowd::C_CrowdSystem* pCrowdSystem, Messaging::C_MessageSubscriberSystem* pSubscriberSystem)
	{
		m_pCrowdSystem = pCrowdSystem;

		if (!m_pCrowdSystem || !pSubscriberSystem)
			return false;

		RemoveAllSubscribers();

		SetSubscriberSystem(pSubscriberSystem);

		m_RaceLoaded = 0;

		// Preliminary list of events I want the crowds to react to
		AddSubscriber(GameMessaging::E_GameMsg_Update);

		AddSubscriber(Game::E_GameflowMessage_RaceLoaded);
		AddSubscriber(Game::E_GameflowMessage_IntroStarted);
		AddSubscriber(Game::E_GameflowMessage_IntroFinished);
		AddSubscriber(Game::E_GameflowMessage_RaceStarted);
		AddSubscriber(Game::E_GameflowMessage_RaceFinished);

		AddSubscriber(Game::E_GameLogicMessage_LapIncremented);

		AddSubscriber(Game::E_PickupMessage_PickupInitiated);

		AddSubscriber(Game::E_GameflowMessage_StopGameUpdate);
		//AddSubscriber(E_VehicleLogicMsg_HitByPerk);

		//AddSubscriber(Game::E_VehicleMessage_FatallyDamaged);
		//AddSubscriber(Game::E_VehicleMessage_ResetFatalDamage);
		//AddSubscriber(Game::E_VehicleMessage_LeftRace);
		
		return true;
	}

	bool C_CrowdGameManager::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		PERF_EVENT("CrowdGameActor.MessageHandler");

		if (m_pCrowdSystem)
		{
			switch (pMessage->Type())
			{
				case Game::E_GameflowMessage_StopGameUpdate:
					{
						RemoveSubscriber(GameMessaging::E_GameMsg_Update);
					}
					break;

				case GameMessaging::E_GameMsg_Update:
					{
						OnUpdate(Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue());
					}
					break;

				case Game::E_GameflowMessage_RaceLoaded:
					{
						AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_INTERESTED);
						m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);
						m_RaceLoaded = 1;
						AddSubscriber(GameMessaging::E_GameMsg_Update);
					}
					break;

				case Game::E_GameflowMessage_IntroStarted:
					{
						AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_INTERESTED);
						m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);
					}
					break;

				case Game::E_GameflowMessage_IntroFinished:
					{
						AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_EXCITED);
						m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);
					}
					break;

				case Game::E_GameflowMessage_RaceStarted:
					{
						AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_ECSTATIC);
						m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);
					}
					break;

				case Game::E_GameflowMessage_RaceFinished:
					{
						AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_ECSTATIC);
						m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);
					}
					break;

				case Game::E_GameflowMessage_Failed:
				case Game::E_GameflowMessage_Wrecked:
					{
						AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_UNHAPPY);
						m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);
					}
					break;

				case Game::E_GameLogicMessage_LapIncremented:
					{
						AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_INTERESTED);
						m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);

						const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);
						const VehiclePositionInfo* pVehiclePositionInfo = GetVehiclePositionInfo(pMsg->mVehicleIndex);
						
						if (pVehiclePositionInfo)
							m_VehicleInfo[pMsg->mVehicleIndex].UpdateMegaFonzies(static_cast<U8>(pVehiclePositionInfo->mRacePosition), pMsg->mTime);
						
						//TODO: [rthomas]	Increase global (race) Mega Fonzies?
						//LOGINFO( Debug::GetLogger("Amax.Crowd"), "Lap finished" );
					}
					break;

				case Game::E_PickupMessage_PickupInitiated:
					{
						//TODO: [rthomas]	Local high interest event
					}
					break;

				case E_VehicleLogicMsg_HitByPerk:
					{
						//TODO: [rthomas]	Local evade/look event (depending on distance)
					}
					break;

				case Game::E_VehicleMessage_FatallyDamaged:
					{
						//TODO: [rthomas]	Local unhappy event (fonzies depending)
					}
					break;

				case Game::E_VehicleMessage_ResetFatalDamage:
					{
						//TODO: [rthomas]	No idea :)
					}
					break;

				case Game::E_VehicleMessage_LeftRace:
					{
						//TODO: [rthomas]	No idea :)
					}
					break;
					
				default:
					return false;
					break;
			}
		}
		return true;
	}

	AnimatedCrowd::CrowdEventType DeterminExcitementFromMegaFonzies(const S32 megaFonzies)
	{
		// TODO: [rthomas]	Properly use mega fonzies to see how excited the crowd will be
		return AnimatedCrowd::E_CROWD_EVENT_LOCAL_EXCITED;
		//return AnimatedCrowd::E_CROWD_EVENT_LOCAL_UNHAPPY;
	}

	void C_CrowdGameManager::OnUpdate(const F32 deltaTime)
	{
		PERF_EVENT("CrowdGameActor.Update");

		// TODO: [rthomas]	Could do initial update of crowd here and run some async update tasks

		if (!m_pCrowdSystem || !m_pCrowdSystem->Ready() || !m_RaceLoaded)
			return;

		m_pCrowdSystem->EnableEffectSystem(AnimatedCrowd::E_CROWD_EFFECT_SYSTEM_CAMERA_FLASHES, static_cast<U32>(g_CrowdCameraFlashesSpawnRate.Value()),
			AnimatedCrowd::C_Random::Instance().RandomF32(g_CrowdCameraFlashesWaitTimeLower, g_CrowdCameraFlashesWaitTimeUpper));

#ifndef CFG_GOLD
		Game::C_AmaxWorldLayer* pWorldLayer = GetAmaxWorldLayer();
		
		const Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		const AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute];

		if (pWorldLayer && pWorldLayer->GetLevelInstance())
			m_pCrowdSystem->UpdateRouteMask(pWorldLayer->GetLevelInstance()->GetRequiredRenderRoutes(), ((pRoute != NULL) ? &*pRoute->CrowdPopulation : NULL));
#endif

		const Game::Actors::RaceLogicCore* pRaceLogicCore = GetRaceLogicCore();
		
		if (pRaceLogicCore)
		{
			if (!pRaceLogicCore->GetNumVehicles())
			{
				const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
				m_pCrowdSystem->SetLocalEvent(AnimatedCrowd::C_Event(AnimatedCrowd::E_CROWD_EVENT_LOCAL_EXCITED, view.m_View.GetViewSettings3D().GetCameraPosition()), 0);
			}
			else
			{
				for (U32 i = 0, count = pRaceLogicCore->GetNumVehicles(); i < count; ++i)
				{
					const Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = GetVehicleLogicActor(i);
					if (pVehicleLogicActor)
					{
						if (m_VehicleEvents[i].HasExpired())
						{
							if (pVehicleLogicActor->GetPositionInfo().mGoingWrongWay)
								m_VehicleEvents[i].Set(Crowd::E_EVENT_CAR_WRONGWAY, pVehicleLogicActor->GetPositionInfo().mWorldPosition);
							else
								m_VehicleEvents[i].Set(Crowd::E_EVENT_CAR_NEARBY, pVehicleLogicActor->GetPositionInfo().mWorldPosition);
						}
						
						m_VehicleEvents[i].Update(deltaTime);
						
						switch (m_VehicleEvents[i].GetEventType())
						{
						case Crowd::E_EVENT_OVERTAKE: // TODO: [rthomas]	Look at cars involved, compare mega fonzies, cheer unless overtaker is hated (low mega fonzies), in which case boo :)
							m_pCrowdSystem->SetLocalEvent(AnimatedCrowd::C_Event(AnimatedCrowd::E_CROWD_EVENT_LOCAL_ECSTATIC, m_VehicleEvents[i].GetPosition()), i);
							break;
						case Crowd::E_EVENT_CRASH:
							m_pCrowdSystem->SetLocalEvent(AnimatedCrowd::C_Event(AnimatedCrowd::E_CROWD_EVENT_LOCAL_REACT_TO_DANGER, m_VehicleEvents[i].GetPosition(), m_VehicleEvents[i].GetDirection()), i);
							break;
						case Crowd::E_EVENT_CAR_WRONGWAY:
							m_pCrowdSystem->SetLocalEvent(AnimatedCrowd::C_Event(AnimatedCrowd::E_CROWD_EVENT_LOCAL_UNHAPPY, m_VehicleEvents[i].GetPosition()), i);
							break;
						case Crowd::E_EVENT_CAR_NEARBY:
						default:
							m_pCrowdSystem->SetLocalEvent(AnimatedCrowd::C_Event(DeterminExcitementFromMegaFonzies(m_VehicleInfo[i].GetMegaFonzies()), m_VehicleEvents[i].GetPosition()), i);
							break;
						}
					}
				}
			}
		}
	}

	void C_CrowdGameManager::OnOvertake(U32 overtakerIndex, U32 overtakeeIndex, U32 overtakerRacePosition)
	{
		// TODO: [rthomas]	Handle this better
		const Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = GetVehicleLogicActor(overtakerIndex);
		
		if (pVehicleLogicActor)
		{
			if (overtakerIndex < static_cast<U32>(m_VehicleEvents.Size()))
				m_VehicleEvents[overtakerIndex].Set(Crowd::E_EVENT_OVERTAKE, pVehicleLogicActor->GetPositionInfo().mWorldPosition);
		}
	}

	void C_CrowdGameManager::OnImpact(const S32 vehicleIndex, const Game::C_ImpactMsg* pImpactMessage)
	{
		if (pImpactMessage && pImpactMessage->mCollisionInfo.m_GameplayImpulse >= 0.025f && vehicleIndex >= 0 && vehicleIndex < m_VehicleEvents.Size())
		{
			const Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = GetVehicleLogicActor(vehicleIndex);

			if (pVehicleLogicActor)
				m_VehicleEvents[vehicleIndex].Set(Crowd::E_EVENT_CRASH, pVehicleLogicActor->GetPositionInfo().mWorldPosition, pVehicleLogicActor->GetPositionInfo().mDirection);
		}
	}

	void C_CrowdGameManager::OnFanDemand(const bool demandPassed)
	{
		if (!m_pCrowdSystem)
			return;

		if (demandPassed)
		{
			AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_ECSTATIC);
			m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);
		}
		else
		{
			AnimatedCrowd::C_Event crowdEvent(AnimatedCrowd::E_CROWD_EVENT_GLOBAL_UNHAPPY);
			m_pCrowdSystem->AddDeferredGlobalEvent(crowdEvent);
		}
	}

	// Some helper functions
	C_GameApp* C_CrowdGameManager::GetGameApp() const
	{
		return C_GameApp::GetInstance();
	}

	Game::C_RaceBookLayer* C_CrowdGameManager::GetRaceBookLayer() const
	{
		if (GetGameApp())
			return GetGameApp()->GetRaceBookLayer();
		return NULL;
	}

	Game::C_RaceLogicLayer* C_CrowdGameManager::GetRaceLogicLayer() const
	{
		if (GetRaceBookLayer())
			return GetRaceBookLayer()->GetRaceLogicLayer();
		return NULL;
	}

	Game::C_AmaxWorldLayer* C_CrowdGameManager::GetAmaxWorldLayer() const
	{
		if (GetRaceBookLayer())
			return GetRaceBookLayer()->GetAmaxWorldLayer();
		return NULL;
	}

	Game::C_GameWorld* C_CrowdGameManager::GetGameWorld() const
	{
		if (GetAmaxWorldLayer())
			return GetAmaxWorldLayer()->GetGameWorld();
		return NULL;
	}

	Game::C_GameActorList* C_CrowdGameManager::GetGameActorList() const
	{
		if (GetGameWorld())
			return GetGameWorld()->GetActorList();
		return NULL;
	}

	const Actor::C_Actor* C_CrowdGameManager::GetActorPtr(const Actor::C_ActorHandle& actorHandle) const
	{
		if (GetGameActorList())
			return GetGameActorList()->GetActorPtr(actorHandle);
		return NULL;
	}

	Game::Actors::RaceLogicCore* C_CrowdGameManager::GetRaceLogicCore() const
	{
		if (GetAmaxWorldLayer())
			return GetAmaxWorldLayer()->GetRaceLogicCore();
		return NULL;
	}

	Game::Actors::C_VehicleLogicActor* C_CrowdGameManager::GetVehicleLogicActor(const U8 vehicleIndex) const
	{
		if (GetRaceLogicLayer())
			return GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
		return NULL;
	}

	const VehiclePositionInfo* C_CrowdGameManager::GetVehiclePositionInfo(const U8 vehicleIndex) const
	{
		if (GetVehicleLogicActor(vehicleIndex))
			return &GetVehicleLogicActor(vehicleIndex)->GetPositionInfo();
		return NULL;
	}

	const VehicleLapsInfo* C_CrowdGameManager::GetVehicleLapsInfo(const U8 vehicleIndex) const
	{
		if (GetVehicleLogicActor(vehicleIndex))
			return &GetVehicleLogicActor(vehicleIndex)->GetLapInfo();
		return NULL;
	}

} // namespace Game
