#include "stdafx.h"

#include <GameLogic/Pickups/Upgraded/PickupPulse.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>

#include <AI\AI_Amax.h>
#include <RaceBook\RaceBookManager.h>

extern Debug::Float g_debugPerkStartDelay;
Debug::Float g_debugPulseScaleCutOff("Perks.Pulse.ScaleCutOff",3.f,Debug::Persist());
Debug::Float g_debugPulseDistanceScaleMultiplier("Perks.Pulse.DistanceScaleMultiplier",0.001f,Debug::Persist());

namespace Game
{
	bool C_PickupFirePulse::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		Actors::RaceLogicCore* pRLC			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		RLPickupType* pPickup				= (RLPickupType*)pRLC->GetPickupType(E_PICKUP_TYPE_PULSE);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);

		if (!pPickup || mLevel >= pPickup->mLevels.count || !pActor || !pLayer || !pRLC )
			return false;

		if(mDelay)
		{
			mDelay -= frameTime;
			WarnPlayers();

			if(mDelay < 0.f)
			{
				float l = g_debugPerkStartDelay / mDelay;
				l=Maths::Clamp(l,0.f,1.f);

				Maths::HVec3 pos = pLayer->mVehiclePositions[mFiringVehicle].mPosition;

				mDelay = 0.f;

				mMatrix = pLayer->mVehiclePositions[mFiringVehicle].mMatrix;

				SendFirePerkMessage();
			}

			return false;
		}

		for(U32 a=0;a<levelState.mNumCars;a++)
		{
			Actors::C_VehicleLogicActor* pOtherActor = pLayer->GetLogicVehicleActor(a);

			if(mFiringVehicle == a || pActor->mPositionInfo.mRacePosition < pOtherActor->mPositionInfo.mRacePosition )
				continue;

			if(p_IsMultiplayer)
			{
				if(!CanHitMp(a))
					continue;
			}
			else
			{
				if(!CanHit(a))
					continue;
			}

			mAlreadyHit[a]=true;

			Maths::HVec3 vehicle_vector = pLayer->mVehiclePositions[mFiringVehicle].mPosition - pLayer->mVehiclePositions[a].mPosition;
			float dot_prod1 = Maths::DotProduct(pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AX(), vehicle_vector);
			float dot_prod2 = Maths::DotProduct(pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AZ(), pLayer->mVehiclePositions[a].mMatrix.AZ());

			bool flip = (dot_prod1 <=0);
			if( dot_prod2 <= 0 )
				flip = !flip;

			C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(GetType(), mLevel, a, mFiringVehicle, mUniqueId, mDroppedByIndex, -1);
			pActor->GetActorList()->BroadcastMessage(&hit_by_msg);
			C_InGameSpecialFXManager::Instance().MessageHandler(&hit_by_msg);

			if( C_InGameDialogueManager::Instance().IsEnabled() )
				C_InGameDialogueManager::Instance().MessageHandler(&hit_by_msg);

			Game::Actors::VehiclePerformanceModifiers mods;
			mods.Initialise();
			mods.mCutThrottle.Set(true, true);
			mods.mForce.Set(true, Maths::HVec3(0,-1000000.f,0));
			mods.mPosition.Set(true, pLayer->mVehiclePositions[a].mPosition);

			Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), a, mods);
			pActor->GetActorList()->BroadcastMessage(&perfModMsg);
		}

		pActor->mPickupInfo.mCurrentlyUsingType = E_PICKUP_TYPE_PULSE;

		return true;
	}

	void C_PickupFirePulse::WarnPlayers()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);

		for(int a = 0; a < pLayer->GetNumVehicles(); a++)
		{
			Actors::C_VehicleLogicActor* pOtherActor = pLayer->GetLogicVehicleActor(a);

			if(mFiringVehicle == a || pActor->mPositionInfo.mRacePosition < pOtherActor->mPositionInfo.mRacePosition )
				continue;

			pOtherActor->mTargetInDanger[mFiringVehicle] = true;
		}
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------

	bool C_PickupHitPulse::Update(int vehIdx, bool p_IsMultiplayer)
	{
		RLPickupType* pPickup						= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_PULSE);
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor			= pLayer->GetLogicVehicleActor(vehIdx);
		Actors::C_VehicleLogicActor* pFiringActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		float frameTime								= C_GameApp::GetInstance()->GetTimeStep();

		if (!pPickup || mLevel >= pPickup->mLevels.count || !pLayer || !pActor )
		{
			//BIZ_ASSERT_FAIL("!hu || !pPickup || hu->mLevel >= pPickup->mLevels.count");
			return false;
		}

		RLPulseBalancingData* pPulseData = (RLPulseBalancingData *)pPickup->mLevels[(int)mLevel]->mBalancingData;

		float scaler = mDistanceToPlayer * g_debugPulseDistanceScaleMultiplier;

		if(!mTimer) // Catch the first run through or we overwrite our existing perf mods
		{
			Game::Actors::C_VehicleApplyShockedEffectMsg shocked_sfx(vehIdx, pPulseData->mTimeShocked + scaler);
			pActor->GetActorList()->BroadcastMessage(&shocked_sfx);

			mTimer += frameTime;

			float logicalDistanceToPlayer = pActor->mPositionInfo.mLogicPositionCombined - pFiringActor->mPositionInfo.mLogicPositionCombined;
			float trackLengthInMetres = AIAmax::AmaxAIR::Instance().DistanceFromStartOfSpline(1);
			mDistanceToPlayer = logicalDistanceToPlayer * trackLengthInMetres;
			
			return false;
		}

		if(scaler > g_debugPulseScaleCutOff)
			scaler = g_debugPulseScaleCutOff;

		mTimer += frameTime;
		if(mTimer > pPulseData->mTimeShocked + scaler)
		{
			Game::Actors::VehiclePerformanceModifiers mods;
			mods.Initialise();
			mods.mCutThrottle.Set(true, false);

			Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), vehIdx, mods);
			/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
			pActor->SendMessageToActor(pActor->GetGraphicalActor(vehIdx)->GetHandle(), &perfModMsg);

			//audio
			C_VehicleLogicMsg_PerkAudioTriggerMsg msg(E_PICKUP_AUDIO_TRIGGER_REACTIVATE_VEHICLE);

			pActor->GetActorList()->SendMessageToActor(pActor->GetGraphicalActor(vehIdx)->GetHandle(),&msg);

			return true;
		}

		Game::Actors::VehiclePerformanceModifiers mods;
		mods.Initialise();
		mods.mCutThrottle.Set(true, true);

		Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), vehIdx, mods);
		/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
		pActor->SendMessageToActor(pActor->GetGraphicalActor(vehIdx)->GetHandle(), &perfModMsg);

		return false;
	}
}
