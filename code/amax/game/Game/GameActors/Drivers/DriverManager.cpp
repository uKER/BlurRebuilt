#include "stdafx.h"
#include "DriverManager.h"
#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/GameFlowMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/Structures/LevelResult.h>
#include <RaceBook/Data/RaceBookData.h>
#include <Game/Game.h>
#include <animation/task/TaskManager.h>
#include <animated_model/TaskManager.h>
#include <Camera2/stock/DebugFeed.h>
#include "Game/AmaxCam/CameraSystem.h"

C_DriverManager* C_DriverManager::s_pInstance = NULL;

C_DriverManager::C_DriverManager():
	m_pSceneGraph(NULL),
	m_pVehicleCubeMapTexture(NULL)
{
	BIZ_ASSERT(!s_pInstance);
	s_pInstance = this;
}

C_DriverManager::~C_DriverManager()
{
	Clear();
	s_pInstance = NULL;
}

void C_DriverManager::SetSceneGraph(SceneGraph::C_SceneGraph* pSceneGraph)
{
	m_pSceneGraph = pSceneGraph;
}

void C_DriverManager::Clear()
{
	for (S32 i = 0; i < m_driverActors.Size(); ++i)
	{
		DestroyDriver(i);
	}
	m_driverActors.Clear();
	m_pVehicleCubeMapTexture = NULL;
}

void C_DriverManager::CreateDriver(U32 driverIndex, const Game::C_DriverActorCreate& createInfo)
{
	m_pVehicleCubeMapTexture = NULL;

	BIZ_ASSERT(m_pSceneGraph);

	U32 numDrivers = m_driverActors.Size();
	if (driverIndex >= (U32)m_driverActors.Size())
	{
		m_driverActors.Resize(driverIndex + 1);
		for (U32 j = numDrivers; j <= driverIndex; ++j)
		{
			m_driverActors[j] = NULL;
		}
	}

	if (m_driverActors[driverIndex])
	{
		DestroyDriver(driverIndex);
	}
	Game::C_DriverActor* pDriverActor = new Game::C_DriverActor;

	// Begin priming the driver actor, which will finish creation once its resources are loaded.
	bool ok = pDriverActor->Prime(createInfo, m_pSceneGraph);

	if (!ok)
	{
		delete pDriverActor;
		pDriverActor = NULL;
	}

	m_driverActors[driverIndex] = pDriverActor;
}

void C_DriverManager::DestroyDriver(U32 driverIndex)
{
	if (driverIndex >= static_cast<U32>(m_driverActors.Size()))
		return;

	delete m_driverActors[driverIndex];
	m_driverActors[driverIndex] = NULL;
}

void C_DriverManager::ResetDriver(U32 driverIndex)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->Reset();
	}
}

void C_DriverManager::SetVehicleModelInput(U32 driverIndex, const Vehicle::C_ModelInput& modelInput)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->SetVehicleModelInput(modelInput);
	}
}

void C_DriverManager::SetVehicleLightSampler(U32 driverIndex, const WorldRender::C_LightSampler* pVehicleLightSampler)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->SetVehicleLightSampler(pVehicleLightSampler);
	}
}

Debug::Bool gDebugDisableDriverEnvMap("ActorDebug.Actors.Driver.DisableDriverEnvMap", false);

void C_DriverManager::SetVehicleCubeMapTexture(const void* pCubeMapTexture)
{
	if (gDebugDisableDriverEnvMap)
	{
		pCubeMapTexture = NULL;
	}
	if (pCubeMapTexture != m_pVehicleCubeMapTexture)
	{
		m_pVehicleCubeMapTexture = pCubeMapTexture;
		for (U32 i = 0, count = m_driverActors.Size(); i < count; ++i)
		{
			Game::C_DriverActor* pDriverActor = GetDriverActor(i);
			if (pDriverActor)
			{
				pDriverActor->SetVehicleCubeMapTexture(pCubeMapTexture);
			}
		}
	}
}

void C_DriverManager::SetDynamicSHLightShaderParameters(U32 driverIndex, const F32 additiveSHLightData[16])
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->SetDynamicSHLightShaderParameters(additiveSHLightData);
	}
}


void C_DriverManager::SetDriverVisibility(U32 driverIndex, bool isVisible, U32 viewportIndex)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->SetVisibility(isVisible, viewportIndex);
	}
}

void C_DriverManager::SetDriverVisibilityDetectionFlags(U32 driverIndex, U32 flags)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->SetVisibilityDetectionFlags(flags);
	}
}


void C_DriverManager::PhotoMoveDriver(U32 driverIndex, const Maths::HVec3& offset)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->PhotoMoveDriver(offset);
	}
}

Maths::C_QuatTrans C_DriverManager::GetDriverHeadTransform(U32 driverIndex) const
{
	Maths::C_QuatTrans qt;
	const Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		qt = pDriverActor->GetHeadTransform();
	}
	return qt;
}

#ifndef CFG_GOLD
Debug::Bool gDebugDriverViewDistanceFromDebugFeed("ActorDebug.Actors.Driver.ViewDistanceFromDebugFeed", true);
#endif

void C_DriverManager::UpdateDrivers(F32 deltaTime)
{
	const U32 kMaxViewports = 4;
	Maths::HVec4 lodScaleAndOffset[kMaxViewports];

	// Calculate LOD scale and offset factors for each viewport.
	U32 numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	BIZ_ASSERT(numViewports <= kMaxViewports);
	for (U32 i = 0; i < numViewports; ++i)
	{
		const Cam2::I_Feed* feed = &AmaxCam::CameraSystem().GameFeed(i);
	#ifndef CFG_GOLD
		if (gDebugDriverViewDistanceFromDebugFeed)
		{
			feed = &AmaxCam::CameraSystem().DebugFeed(i);
		}
	#endif
		Maths::HMat43 cameraMatrix = feed->Mat43();
		Maths::HVec3 cameraPos = cameraMatrix.Translation();

		Maths::HF32 fov = feed->Fov();
		Maths::HVec3 lodScale = cameraMatrix.AZ() * fov;
		Maths::HF32 lodOffset = -lodScale | cameraMatrix.Translation();

		lodScaleAndOffset[i] = lodScale || lodOffset;
	}

	// Update all drivers.
	for (U32 i = 0, count = m_driverActors.Size(); i < count; ++i)
	{
		Game::C_DriverActor* pDriverActor = GetDriverActor(i);
		if (pDriverActor)
		{
			pDriverActor->Update(deltaTime, lodScaleAndOffset, numViewports);
		}
	}

	// Ensure that any batched up animation and animated model tasks are kicked off now.
	Animation::TaskManager().ScheduleQueuedTasks();
	AnimatedModel::TaskManager().ScheduleQueuedTasks();
}

void C_DriverManager::OnCollision(U32 driverIndex, const Game::C_ImpactMsg* pImpactMsg)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->OnImpactMessage(pImpactMsg);
	}
}

void C_DriverManager::OnOvertake(U32 overtakerIndex, U32 overtakeeIndex, U32 overtakerRacePosition)
{
	Game::C_DriverActor* pOvertaker = GetDriverActor(overtakerIndex);
	Game::C_DriverActor* pOvertakee = GetDriverActor(overtakeeIndex);

	if (!pOvertaker || !pOvertakee)
	{
		return;
	}

	pOvertaker->Overtake(pOvertakee->GetPosition(), true);
	pOvertakee->Overtake(pOvertaker->GetPosition(), false);
}

void C_DriverManager::OnPlayDriverAnimation(U32 driverIndex, const ActorMsg::C_SetDriverAnimMsg* pSetDriverAnimMsg)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->OnPlayDriverAnimation(pSetDriverAnimMsg);
	}
}

void C_DriverManager::OnFatallyDamaged(U32 driverIndex)
{
	Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
	if (pDriverActor)
	{
		pDriverActor->VehicleFatallyDamaged();
	}
}


void C_DriverManager::SubscribeToMessages(Messaging::C_MessageSubscriberSystem* pSubscriberSystem)
{
	RemoveAllSubscribers();

	SetSubscriberSystem(pSubscriberSystem);

	AddSubscriber(E_VehicleLogicMsg_HitByPerk);
	AddSubscriber(Game::E_GameflowMessage_RaceFinished);
	AddSubscriber(Game::E_VehicleMessage_FatallyDamaged);
	AddSubscriber(Game::E_VehicleMessage_ResetFatalDamage);
	AddSubscriber(Game::E_VehicleMessage_LeftRace);
	AddSubscriber(ActorMsg::E_CutSceneMsg_SetDriverAnim);
}


bool C_DriverManager::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	PERF_EVENT("C_DriverActor::MessageHandler");

	switch (pMessage->Type())
	{
		case E_VehicleLogicMsg_HitByPerk:
		{
			const C_VehicleLogicMsg_HitByPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);
			Game::C_DriverActor* pDriverActor = GetDriverActor(msg->mVehicleIndex);
			if (pDriverActor)
			{
				pDriverActor->HitByPerk();
			}
			break;
		}

		case Game::E_GameflowMessage_RaceFinished:
		{
			const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

			for (U32 driverIndex = 0, count = m_driverActors.Size(); driverIndex < count; ++driverIndex)
			{
				BIZ_ASSERT(driverIndex < (U32)results.mCars.Size());
				if (driverIndex >= (U32)results.mCars.Size())
				{
					break;
				}

				Game::C_DriverActor* pDriverActor = GetDriverActor(driverIndex);
				if (!pDriverActor)
				{
					continue;
				}
			
				// Check level results to decide on driver's response.
				bool hasPassedEvent = true;

				const Game::LevelResultCar& car = results.mCars[driverIndex];
				for (U32 i = 0, count = car.mPassedRule.Size(); i < count && hasPassedEvent; ++i)
				{
					if(!car.mPassedRule[i])
					{
						hasPassedEvent = false;
					}
				}

				for (U32 i = 0, count = car.mFailedRule.Size(); i < count && hasPassedEvent; ++i)
				{
					if (car.mFailedRule[i])
					{
						hasPassedEvent = false;
					}
				}

				if (hasPassedEvent)
				{
					pDriverActor->PassedEvent();
				}
				else
				{
					pDriverActor->FailedEvent();
				}
			}
			break;
		}

		case Game::E_VehicleMessage_LeftRace:
		{
			const Game::Actors::C_VehicleLeftRaceMsg* pMsg =  Messaging::MessageCast<Game::Actors::C_VehicleLeftRaceMsg>(pMessage);
			DestroyDriver(pMsg->m_VehicleIndex);
			break;
		}

		case Game::E_VehicleMessage_FatallyDamaged:
		{
			const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);
			OnFatallyDamaged(pMsg->m_VehicleIndex);
			break;
		}
		
		default:
			return false;
			break;
	}
	return true;
}

#ifndef CFG_GOLD
void C_DriverManager::DebugRender(DbUtil::I_DebugRender* debugRenderPtr) const
{
	for (U32 i = 0; i < (U32)m_driverActors.Size(); ++i)
	{
		if (m_driverActors[i])
		{
			m_driverActors[i]->DebugRender(debugRenderPtr);
		}
	}
}
#endif
