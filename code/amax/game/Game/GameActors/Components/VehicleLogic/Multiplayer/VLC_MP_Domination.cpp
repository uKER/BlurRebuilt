#include "stdafx.h"

#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Domination.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <Game\GameActors\Vehicles\VehiclePerformanceModifiers.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <Game\AmaxCam\CameraSystem.h>
#include "Game/AmaxCam/VehicleFeed.h"

#include <Game\Messaging\GameflowMessages.h>
#include <Game\Messaging\VehicleActorMessages.h>
#include <racingphysics\vehicles\car\car.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic\Structures\LevelState.h>
#include <GameLogic\Structures\NetGameVars.h>

#include <AI\AI_Amax.h>
#include "Game\Game.h"

//Debug::Float g_debugMaxBoostForce("Vehicle.Lol.Force", 10000.0f, Debug::Range(0.0f, 10000.0f), Debug::Step(0.001f));
//Debug::Float g_debugMinAssistDistance("Amax.Multiplayer.Balance.MinAssistDistance", 50.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(0.001f));
//Debug::Float g_debugMaxAssistDistance("Amax.Multiplayer.Balance.MaxAssistDistance", 700.0f, Debug::Range(0.0f, 10000.0f), Debug::Step(0.001f));

Debug::Logger &g_MpBalancingLog(Debug::GetLogger("Amax.Multiplayer.Balance"));

Debug::Bool g_debugMpDominationEnabled("Amax.Multiplayer.Balance.DominationEnabled", false, Debug::Persist());
Debug::Float g_debugDominationDistance("Amax.Multiplayer.Balance.DominationDistance", 500.0f, Debug::Range(50.0f, 1000.0f), Debug::Step(1.0f));

const F32	DOMINATION_TIMER_RESET = 10.0f;
const F32	DOMINATION_DISPLAY_LENGTH = 3.0f;
const F32	DOMINATION_FANS_PER_SECOND = 1.0f;


namespace Components
{

//	OLD DOMINATION

//bool C_VehicleComponent_MP_Domination::CreateImpl()
//{
//	const I_VehicleComponent_MP_Domination_CreateInfo* creation_data = static_cast<const I_VehicleComponent_MP_Domination_CreateInfo *>(GetCreator().GetData());
//
//	if(creation_data)
//	{
//	 	mVehicleIndex=creation_data->mVehicleIndex;
//		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
//	}
//
//	AddSubscriber( GameMessaging::E_GameMsg_Update );
//	AddSubscriber( E_VehicleLogicMsg_Domination );
//	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
//	AddSubscriber( Game::E_GameLogicMessage_LapIncremented );
//
//	m_DeltaTime = 0.0f;
//	mRaceStarted=false;
//	
//	//	DOMINATION MODE
//	m_CurrentMode = E_DominationMode_Idle;
//	m_DominationDisplayTimer = 0.0f;
//	m_DominationFans = 0.0f;
//
//	return true;
//}
//
//void C_VehicleComponent_MP_Domination::Update()
//{
//	if(!g_debugMpDominationEnabled)
//		return;
//
//	if(!mRaceStarted || C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars <= 1)
//		return;	
//
//	Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
//
//	bool first = (pLogicActor->mPositionInfo.mRacePosition == 1);
//
//	switch(m_CurrentMode)
//	{
//	case E_DominationMode_Idle:
//		{
//			if(first)
//				m_CurrentMode = E_DominationMode_Dominating;
//		}
//		break;
//
//	case E_DominationMode_Dominating:
//		{
//			if(first)
//			{
//				//	Ok, this vehicle is in 1st place - so they're earning Domination Fans.
//				m_DominationFans += (DOMINATION_FANS_PER_SECOND * m_DeltaTime);
//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mPosition+Maths::HVec3(0,2.0f,0), Util::C_String().Printf("%d Domination Fans at next Lap", (U32)m_DominationFans));
//			}
//			else
//			{
//				//	Vehicle *was* dominating, but has been overtaken so loses all the cached Domination Fans
//				m_DisplayFansValue = m_DominationFans;
//				m_DominationFans = 0.0f;
//				//	Do some visual messaging to tell the player they're no longer Dominating, possibly a big red X over the displayed number of Dom. fans.
//				m_CurrentMode = E_DominationMode_Lost;
//				m_DominationDisplayTimer = DOMINATION_DISPLAY_LENGTH;
//			}
//		}
//		break;
//
//	case E_DominationMode_Banking:
//		{
//			if(m_DominationDisplayTimer > 0.0f)
//			{
//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mPosition+Maths::HVec3(0,2.0f,0), Util::C_String().Printf("Banked %d Domination Fans!", (U32)m_DisplayFansValue));
//				m_DominationDisplayTimer -= m_DeltaTime;
//			}
//			else
//				m_CurrentMode = E_DominationMode_Idle;
//		}
//		break;
//
//	case E_DominationMode_Lost:
//		{
//			if(m_DominationDisplayTimer > 0.0f)
//			{
//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mPosition+Maths::HVec3(0,2.0f,0), Util::C_String().Printf("Overtaken! Lost %d possible Domination Fans", (U32)m_DisplayFansValue));
//				m_DominationDisplayTimer -= m_DeltaTime;
//			}
//			else
//				m_CurrentMode = E_DominationMode_Idle;
//		}
//		break;
//	}
//}
//
//void C_VehicleComponent_MP_Domination::CrossedLapLine()
//{
//	m_DisplayFansValue = m_DominationFans;
//	m_DominationFans = 0.0f;
//
//	//	This is where the number of fans would actually get awarded permanently
//	C_VehicleLogicDominationMsg bankedMsg(mVehicleIndex, (U32)m_DominationFans);
//	Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
//	if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Scoring))
//	{
//		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Scoring, &bankedMsg);
//	}
//
//	m_CurrentMode = E_DominationMode_Banking;
//	m_DominationDisplayTimer = DOMINATION_DISPLAY_LENGTH;
//}
//
//bool C_VehicleComponent_MP_Domination::OnMessage(const Messaging::C_MessageBase *pMessage)
//{
//	switch(pMessage->Type())
//	{
//	case GameMessaging::E_GameMsg_Update:
//		{
//			m_DeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
//			Update();
//
//			return true;
//		}
//		break;
//
//	case E_VehicleLogicMsg_Domination:
//		{
//			return true;
//		}
//		break;
//
//	case Game::E_GameLogicMessage_LapIncremented:
//	{
//		const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);
//		if(mVehicleIndex == pMsg->mVehicleIndex)
//			CrossedLapLine();
//		return true;
//	}
//	break;
//
//	case Game::E_GameflowMessage_RaceStarted:
//		{
//			mRaceStarted=true;
//			return true;
//		}
//		break;
//
//	default:
//		break;
//	}
//
//	return I_GameActorComponent::OnMessage(pMessage);
//}

bool C_VehicleComponent_MP_Domination::CreateImpl()
{
	const I_VehicleComponent_MP_Domination_CreateInfo* creation_data = static_cast<const I_VehicleComponent_MP_Domination_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{
	 	mVehicleIndex=creation_data->mVehicleIndex;
		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
	}

	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( E_VehicleLogicMsg_Domination );
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );

	m_DeltaTime = 0.0f;
	mRaceStarted=false;
	
	//	DOMINATION MODE
	m_CurrentMode = E_DominationMode_Idle;
	m_DominationDisplayTimer = 0.0f;

	return true;
}

void C_VehicleComponent_MP_Domination::Update()
{
	if(!g_debugMpDominationEnabled)
		return;

	if(!mRaceStarted || C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars <= 1)
		return;	

	Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	bool first = (pLogicActor->mPositionInfo.mRacePosition == 1);

	switch(m_CurrentMode)
	{
	case E_DominationMode_Idle:
		{
			if(m_DominationDisplayTimer > 0.0f)
			{
				m_DominationDisplayTimer -= m_DeltaTime;
				if(m_DominatingVehicleLocalIndex != mVehicleIndex)
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mPosition+Maths::HVec3(0.0f, 1.5f, 0.0f), Util::C_String().Printf("%S DOMINATED", C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[m_DominatingVehicleLocalIndex].mNameAboveCar.Chars()));
				}
			}
			else if(first)
			{
				m_CurrentMode = E_DominationMode_Dominating;
			}
		}
		break;

	case E_DominationMode_Dominating:
		{
			if(first)
			{
				//	Ok, this vehicle is in 1st place - so they're getting towards domination
				int theirIndex = 0;
				float theirLogicalDistance = 0.0f;
				for(theirIndex = 0; theirIndex < mRaceLogicLayerPtr->GetNumVehicles(); theirIndex++)
				{
					if(mRaceLogicLayerPtr->GetLogicVehicleActor(theirIndex)->mPositionInfo.mRacePosition == 2)
					{
						theirLogicalDistance = mRaceLogicLayerPtr->GetLogicVehicleActor(theirIndex)->mPositionInfo.mLogicPositionCombined;
						break;
					}
				}

				//	Find logical distance to next player
				float logicalDistanceToPlayer = pLogicActor->mPositionInfo.mLogicPositionCombined - theirLogicalDistance;

				//	Find distance to player in metres
				float trackLengthInMetres = AIAmax::AmaxAIR::Instance().DistanceFromStartOfSpline(1);
				float distanceToPlayer = logicalDistanceToPlayer * trackLengthInMetres;


				float displayStartDistance = g_debugDominationDistance - 200.0f;
				if(distanceToPlayer > displayStartDistance)
				{
					if(distanceToPlayer < g_debugDominationDistance)
					{
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mPosition+Maths::HVec3(0.0f, 1.5f, 0.0f),Util::C_String().Printf("%2.1f metres from Domination Zone", g_debugDominationDistance - distanceToPlayer));
					}
					else
					{
						//	Find the position of the vehicle we're going to reset the Dominator to
						//	 - if there are more than 5 racers this will be the vehicle currently in 5th, otherwise it's the vehicle in last place - 1

						U8 resetVehiclePosition = 0;
						if(mRaceLogicLayerPtr->GetNumVehicles() > 5)
						{
							//	We want the vehicle in 5th
							resetVehiclePosition = 5;
						}
						else if(mRaceLogicLayerPtr->GetNumVehicles() == 2)
						{
							//	We want the vehicle in 2nd
							resetVehiclePosition = 2;
						}
						else
						{
							//	We want the vehicle in 2nd-last
							resetVehiclePosition = mRaceLogicLayerPtr->GetNumVehicles() - 1;
						}

						U8 resetVehicleIndex = mVehicleIndex;
						do
						{
							if(resetVehiclePosition <= 1)
								return;
							for(int i = 0; i <mRaceLogicLayerPtr->GetNumVehicles(); i++)
							{
								if(mRaceLogicLayerPtr->GetLogicVehicleActor(i)->mPositionInfo.mRacePosition == resetVehiclePosition)
								{
									resetVehicleIndex = i;
									break;
								}
							}
							resetVehiclePosition--;
						}while(mRaceLogicLayerPtr->GetLogicVehicleActor(resetVehicleIndex)->IsFatallyDamaged());

						if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod))
						{
							Game::Actors::C_VehicleResetAtVehiclePositionMsg resetMsg(resetVehicleIndex);
							pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_HandOfGod, &resetMsg);
						}

						Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
						const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();

						//netGameVars.m_EventManager->AddEvent_VehicleDominated(levelState.mCars[mVehicleIndex].mNetJoinId);

						//	Player has successfully dominated the race!
						m_DominationDisplayTimer = DOMINATION_DISPLAY_LENGTH -1.0f;
						m_DominatingVehicleLocalIndex = mVehicleIndex;

						m_CurrentMode = E_DominationMode_Dominated;
					}
				}
			}
			else
				m_CurrentMode = E_DominationMode_Idle;
		}
		break;

	case E_DominationMode_Dominated:
		{
			if(m_DominationDisplayTimer > 0.0f)
			{
				m_DominationDisplayTimer -= m_DeltaTime;
				AmaxCam::CameraSystem().VehicleFeed()->Reset();
			}
			else
				m_CurrentMode = E_DominationMode_Idle;
		}
		break;
	}
}

bool C_VehicleComponent_MP_Domination::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_Update:
		{
			m_DeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
			Update();

			return true;
		}
		break;

	case E_VehicleLogicMsg_Domination:
		{
			m_DominatingVehicleLocalIndex = Messaging::MessageCast<C_VehicleLogicDominationMsg>(pMessage)->m_DominatingVehicleIndex;
			m_DominationDisplayTimer = DOMINATION_DISPLAY_LENGTH;
			
			m_CurrentMode = E_DominationMode_Idle;

			return true;
		}
		break;

	case Game::E_GameflowMessage_RaceStarted:
		{
			mRaceStarted=true;
			return true;
		}
		break;

	default:
		break;
	}

	return I_GameActorComponent::OnMessage(pMessage);
}

} // namespace Components