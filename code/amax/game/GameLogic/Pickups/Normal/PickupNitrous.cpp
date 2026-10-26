#include "stdafx.h"
#include <GameAudio/GameAudioLayer.h>	// [Blur2PC]

#include <GameLogic/Pickups/Normal/PickupNitrous.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>
#include <Game/Messaging/PickupActorMessages.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>

#include <RaceBook\RaceBookManager.h>

#include <Vehicle/VehicleModel.h>
#include <GameLogic/Structures/LevelStateMultiplayerAbilities.h>
#include <GameLogic/StickerManager.h>

#include "GameAudio/PickupSubsystem.h"
#include <AI/AI_Amax.h>

Debug::Bool		g_debugNitrousRecordSpeeds("Perks.Nitrous.Debug.RecordSpeeds", false, Debug::Persist());
Debug::Float	g_debugNitrousMinSpeedMultiplier("Perks.Nitrous.MinSpeedMultiplier", 1.1f);
Debug::Float	g_debugNitrousSlowDownForce("Perks.Nitrous.SlowDownForce", 15.f);

namespace Game
{
	// -----------------------------------------------------------------------

	C_PickupEffectsNitrous::C_PickupEffectsNitrous(E_PICKUP_TYPE type, int vehIdx, float time)
		: C_PickupLocalEffectsBase(type, vehIdx, time), mDisableSlowdown(false), mClassMult(1.f)
	{
		LevelStateCar& levelCar = C_Game::GetInstance()->GetLevelState().mCars[vehIdx];
		mpNitrousData = &levelCar.mPickupInfo.NitrousInfo();

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mVehicleIndex);

		if(time == 0.f)
		{
			mDuration = mpNitrousData->mDuration;

			Game::C_RaceBookLayer* pRBLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
			if( pRBLayer && (pRBLayer->GetRaceMode() == E_RaceMode_StreetRace || 
				pRBLayer->GetRaceMode() == E_RaceMode_FriendDemand_StreetRace || 
				pRBLayer->GetRaceMode() == E_RaceMode_Multiplayer_Racing) )
			{
				float relativePos = (float)pActor->GetPositionInfo().mRacePosition / (float)pLayer->GetNumVehicles();
				if(relativePos > 0.75f)
				{
					mDuration += 0.5f;
				}
				else if(relativePos > 0.5f)
				{
					mDuration += 0.25f;
				}
			}
		}
		else
		{
			mDuration = time;
		}

		float graphVal	= 200.f; // 1.0f on the graph == 200mph
		mPrevSpeed = pActor->GetPositionInfo().mSpeedInMph;
		float speed		= mPrevSpeed / graphVal;
		float speedMult = CalcSpeedFromGraph(speed);

		AmaxVehicle* pVeh = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)levelCar.mCarID];
		
		switch(pVeh->carClass)
		{
		case AmaxCarClass_A:
			mClassMult = mpNitrousData->mClassAModifier;
			break;

		case AmaxCarClass_B:
			mClassMult = mpNitrousData->mClassBModifier;
			break;

		case AmaxCarClass_C:
			mClassMult = mpNitrousData->mClassCModifier;
			break;

		case AmaxCarClass_D:
			mClassMult = mpNitrousData->mClassDModifier;
			break;
		}

		speedMult = Maths::Clamp(speedMult, 0.f, speedMult);
		float newSpeed = mClassMult * speedMult * graphVal;
		pActor->GetGraphicalActor(mVehicleIndex)->GetFakeSpeed()->Start(Game::Actors::E_FAKE_SPEED_ACCELERATE, newSpeed);

#ifndef CFG_GOLD
		if(g_debugNitrousRecordSpeeds)
			DebugLn("Increased speed to %f", mClassMult * speedMult * graphVal);
#endif

	}

	bool C_PickupEffectsNitrous::Update()
	{
		PERF_EVENT("PickupNitrous.Update");

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mVehicleIndex);

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		mDuration -= frameTime;

		if(mTimer > 0.f)
		{
			C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);
			pActor->GetGraphicalActor(mVehicleIndex)->GetFakeSpeed()->Stop();
		}

		if(mDisableSlowdown)
		{
			//send end msg to audio
			C_VehicleLogicMsg_PerkEnded pMsg(mVehicleIndex,mUniqueId,E_PICKUP_TYPE_NITROUS,Game::E_BodyUserDataType_Actor);//	= Messaging::MessageCast<C_VehicleLogicMsg_PerkEnded>(pMessage);
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);
			Game::C_StickerManager::Instance().MessageHandler(&pMsg); //we also need to know about this for a sticker

			return true;
		}

		if(mDuration <= 0.f)
		{
			float slowDownVal = mpNitrousData->mSlowDownVal;

			Game::C_RaceBookLayer* pRBLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
			if( pRBLayer && (pRBLayer->GetRaceMode() == E_RaceMode_StreetRace || 
				pRBLayer->GetRaceMode() == E_RaceMode_FriendDemand_StreetRace || 
				pRBLayer->GetRaceMode() == E_RaceMode_Multiplayer_Racing) )
			{
				float relativePos = (float)pActor->GetPositionInfo().mRacePosition / (float)pLayer->GetNumVehicles();

				if(relativePos > 0.75f && pLayer->GetNumVehicles() > 10)
				{
					slowDownVal -= 0.1f;
				}
				else if(relativePos > 0.5f)
				{
					slowDownVal -= 0.05f;
				}
			}

			float graphVal	= 200.f; // 1.0f on the graph == 200mph
			float speed		= (pActor->GetPositionInfo().mSpeedInMph / graphVal) - (mpNitrousData->mSlowDownVal * mClassMult);
			float speedMult = CalcSpeedFromGraph(speed);

			speedMult = Maths::Clamp(speedMult, 0.f, speedMult);
			float newSpeed = Maths::Max(mPrevSpeed * g_debugNitrousMinSpeedMultiplier, speedMult * graphVal);
			pActor->GetGraphicalActor(mVehicleIndex)->GetFakeSpeed()->Start(Game::Actors::E_FAKE_SPEED_BRAKE, newSpeed, g_debugNitrousSlowDownForce);

#ifndef CFG_GOLD
			if(g_debugNitrousRecordSpeeds)
				DebugLn("Reduced speed to %f", speedMult * graphVal);
#endif

			//send end msg to audio
			C_VehicleLogicMsg_PerkEnded pMsg(mVehicleIndex,mUniqueId,E_PICKUP_TYPE_NITROUS,Game::E_BodyUserDataType_Actor);//	= Messaging::MessageCast<C_VehicleLogicMsg_PerkEnded>(pMessage);
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);
			Game::C_StickerManager::Instance().MessageHandler(&pMsg); //we also need to know about this for a sticker

			return true;
		}

		return false;
	}

	float C_PickupEffectsNitrous::CalcSpeedFromGraph(float graphVal)
	{
		RLPickupType* pPickup		= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS);
		RLBalancingGraph *pGraph	= &pPickup->mNormal->mBalancingGraph;
		return pGraph->mEffectCurve.Evaluate(graphVal);
	}
}

// -----------------------------------------------------------------------

Debug::Float	g_debugNitrousVehicleSpeed("Perks.Nitrous.Debug.VehicleSpeed", 0.f, Debug::Persist());
Debug::Float	g_debugNitrousDistanceToTravel("Perks.Nitrous.Debug.DistanceToTravel", 250.f, Debug::Persist());
Debug::Float	g_debugNitrousFireDelay("Perks.Nitrous.Debug.SecondsToFireNitrous", -1.f, Debug::Persist());
Maths::HMat43	g_debugNitrousSavedPosition;
float			g_debugNitrousTotalTravelledDistance;
float			g_debugNitrousTravelledDistance;
float			g_debugNitrousPrevTravelledDistance;
float			g_debugNitrousTimeTaken;
bool			g_debugNitrousDebugActive;
bool			g_debugNitrousFired;

void DebugNitrousTestBedReset()
{
	g_debugNitrousDebugActive = g_debugNitrousFired = false;
}

void DebugNitrousTestBedUpdate()
{
	Game::C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	
	if (!pRaceLogicLayer)
		return;
	
	Game::Actors::C_VehicleLogicActor* pActor = pRaceLogicLayer->GetLogicVehicleActor(0);

	if(AIAmax::AmaxAIR::Instance().GetTrackDataSystem() && AIAmax::AmaxAIR::Instance().GetTrackDataSystem()->GetTrackData() && pActor && g_debugNitrousDebugActive)
	{
		float lapDistance = AIAmax::AmaxAIR::Instance().GetTrackDataSystem()->GetTrackData()->m_TrackLogicData.m_noSamples * 
			AIAmax::AmaxAIR::Instance().GetTrackDataSystem()->GetTrackData()->m_TrackLogicData.m_stepSize;

		float logicPos = pActor->GetPositionInfo().mLogicPositionCombined;

		g_debugNitrousPrevTravelledDistance		= g_debugNitrousTravelledDistance;
		g_debugNitrousTravelledDistance			= logicPos * lapDistance;

		if(g_debugNitrousPrevTravelledDistance > 0.f && g_debugNitrousTravelledDistance >= g_debugNitrousPrevTravelledDistance)
		{
			g_debugNitrousTotalTravelledDistance	+= (g_debugNitrousTravelledDistance - g_debugNitrousPrevTravelledDistance);
			g_debugNitrousTimeTaken					+= C_GameApp::GetInstance()->GetTimeStep();

			if(g_debugNitrousTimeTaken >= g_debugNitrousFireDelay && g_debugNitrousFireDelay != -1.f && !g_debugNitrousFired)
			{
				Game::Actors::C_GrantPickupMsg grantPickupMsg(0, E_PICKUP_TYPE_NITROUS);
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
				Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(0, E_PICKUP_TYPE_NITROUS);
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);

				g_debugNitrousFired = true;
			}

			if(g_debugNitrousTotalTravelledDistance >= g_debugNitrousDistanceToTravel)
			{
				DebugLn("Time taken: %f", g_debugNitrousTimeTaken);
				DebugLn("-----------------------");

				DebugNitrousTestBedReset();
			}
		}
	}
}

static void DebugNitrousSaveVehiclePosition(void *)
{
	Game::C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	
	if (!pRaceLogicLayer)
		return;

	Game::Actors::C_VehicleActor* pActor = pRaceLogicLayer->GetLogicVehicleActor(0)->GetGraphicalActor(0);
	g_debugNitrousSavedPosition = pActor->GetMatrix();
}
Debug::Function	g_debugNitrousSaveVehiclePosition("Perks.Nitrous.Debug.SavePosition", DebugNitrousSaveVehiclePosition, 0);

static void DebugNitrousLoadVehiclePosition(void *)
{
	Game::C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	
	if (!pRaceLogicLayer)
		return;

	Game::Actors::C_VehicleActor* pActor = pRaceLogicLayer->GetLogicVehicleActor(0)->GetGraphicalActor(0);

	Game::Actors::C_VehicleResetAtWorldPositionMsg pMsg(pActor->GetHandle(), 0, g_debugNitrousSavedPosition, Maths::HVec3(0,0,0));
	pActor->MessageHandler(&pMsg);

	pActor->GetFakeSpeed()->Start(Game::Actors::E_FAKE_SPEED_ACCELERATE, g_debugNitrousVehicleSpeed);
	g_debugNitrousTravelledDistance = g_debugNitrousTotalTravelledDistance = g_debugNitrousPrevTravelledDistance = 0.f;
	g_debugNitrousDebugActive = true;
	g_debugNitrousTimeTaken = 0.f;
	g_debugNitrousFired = false;
}
Debug::Function	g_debugNitrousLoadVehiclePosition("Perks.Nitrous.Debug.LoadPosition", DebugNitrousLoadVehiclePosition, 0);