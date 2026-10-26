#include "stdafx.h"

#include <GameLogic/Pickups/Upgraded/PickupRift.h>
#include <GameLogic/Pickups/PickupMineBase.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <GameLogic\InGameSpecialFXManager.h>

#include <RaceBook\RaceBookManager.h>

#include <Vehicle/VehicleModel.h>

Debug::Float g_debugBullBarMaxForce("Perks.BullBar.MaxForce",1500.f);
Debug::Float g_debugBullBarMinForce("Perks.BullBar.MinForce",500.f);
Debug::Float g_debugRiftMineShiftingForce("Perks.BullBar.MineShiftingForce", 100000.f);

//extern Debug::Float g_debugLowSpeedNitrousBoost;

namespace Game
{
	bool C_PickupFireRift::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		RLPickupType* pPickup				= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_RIFT);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringVehicle);

		if (!pPickup || mLevel >= pPickup->mLevels.count)
			return true;

		RLRiftBalancingData* pRiftData = (RLRiftBalancingData *)pPickup->mLevels[(int)mLevel]->mBalancingData;

		if(mDelay)
		{
			mDelay -= frameTime;

			//WarnPlayers();

			if(mDelay < 0.f)
			{
				mDelay = 0.f;
				mMatrix = pLayer->mVehiclePositions[mFiringVehicle].mMatrix;
				SendFirePerkMessage();
			}

			return false;
		}

		mTimer += frameTime * pRiftData->mSpeed;

		if(mTimer > 1.f)
		{
			Game::Actors::VehiclePerformanceModifiers mods;
			mods.Initialise();
			mods.mBoost.Set(true, 0.f);

			Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), mFiringVehicle, mods);
			/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
			pActor->SendMessageToActor(pVehActor->GetHandle(), &perfModMsg);

			return true;
		}

		RLBalancingGraph *pGraph = &pPickup->mLevels[(int)mLevel]->mBalancingGraph;

		RLHermiteKeyFrame keyFrame;
		//RLHermiteCurve effectCurve;
		Curve::C_HermiteCurve effectCurve;

		//Build the effect curve
		for(int j = 0; j < pGraph->mEffectGraph.count; j++)
		{
			keyFrame = pGraph->mEffectGraph[j];

			effectCurve.AddKeyFrame(keyFrame.mTime, 
				keyFrame.mValue, 
				keyFrame.mInTangent, 
				keyFrame.mOutTangent);
		}
		//DEBUG DEBUG DEBUG

		Game::Actors::VehiclePerformanceModifiers mods;
		mods.Initialise();
		float boost = effectCurve.Evaluate(mTimer) * pGraph->mMaxBoost;

		Game::Actors::C_VehicleActor *pGActor = pActor->GetGraphicalActor(mFiringVehicle);
		float speedInMetresPerSecond = abs( Magnitude( pGActor->GetVehicleInput()->m_vVelocity ) );
		float mph= (speedInMetresPerSecond * 3600.f) / 1609.f;

		mph=Maths::Clamp(mph,0.f,60.f);

		if(60.f-mph)
		{
			float low_speed_boost=(60.f-mph) * 0.03f;
			boost *= low_speed_boost;
		}

		mods.mBoost.Set(true, boost);

		Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), mFiringVehicle, mods);
		/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
		pActor->SendMessageToActor(pVehActor->GetHandle(), &perfModMsg);

		RLRiftBalancingData* pBullBarsData = (RLRiftBalancingData *)pPickup->mLevels[(int)mLevel]->mBalancingData;
		BIZ_ASSERT_MSG(pBullBarsData,"Unable to find BullBars balancing data");

		int shortestIdx = -1;
		for(U32 a=0; a < levelState.mNumCars; a++)
		{
			//Maths::HVec3 dist=mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mPosition;
			if(mFiringVehicle == a)
				continue;

			float inRange = pActor->InRangeOfPickup(GetType(), mLevel, a);

			if(inRange <= 1.0f)
			{
				AMAX_PICKUPS_CANT_HIT_REASON reason=AMAX_PICKUPS_CANT_HIT_REASON_NONE;
				if(!CanHit(a,&reason))
					continue;
				
				float normalised_dist=pBullBarsData->mRadius / pActor->mVehicleProximities[a].mDistanceSquared;
				float force=g_debugBullBarMinForce + ((g_debugBullBarMaxForce-g_debugBullBarMinForce) * 1.0f-normalised_dist);

				Maths::HVec3 pos = mMatrix.AW();
				Maths::HVec3 heading= mMatrix.AZ();

				Maths::HVec3 vehicle_vector = pLayer->mVehiclePositions[mFiringVehicle].mPosition - pLayer->mVehiclePositions[a].mPosition;
				vehicle_vector = Maths::Normalise(vehicle_vector);

				float dot_prod1 = Maths::DotProduct(pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AX(), vehicle_vector);
				bool flip = (dot_prod1 <=0);

				Maths::HVec3 force_dir = pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AX() * (flip ? 1.f : -1.f);
				force_dir = Maths::Normalise(force_dir);
				force_dir *= force;

				Game::Actors::VehiclePerformanceModifiers mods;
				mods.Initialise();
				mods.mForce.Set(true, force_dir);
				mods.mPosition.Set(true, pLayer->mVehiclePositions[a].mPosition);

				Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), a, mods);
				/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
				pActor->SendMessageToActor(pVehActor->GetHandle(), &perfModMsg);

				mAlreadyHit[a] = true;

				C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(GetType(), mLevel, a, mFiringVehicle, mUniqueId, mDroppedByIndex, -1);
				pActor->GetActorList()->BroadcastMessage(&hit_by_msg);
				C_InGameSpecialFXManager::Instance().MessageHandler(&hit_by_msg);
			}	
		}

		for(int i = 0; i < pLayer->mLiveMines.Size(); i++)
		{
			C_PickupFireMineBase* pickup = pLayer->mLiveMines[i];

			float dist = pickup->mMatrix.AW().Distance(pActor->mPositionInfo.mWorldPosition);

			if(dist > pBullBarsData->mRadius)
				break;

			Maths::HVec3 vehicle_vector = pLayer->mVehiclePositions[mFiringVehicle].mPosition - pickup->mMatrix.AW();
			vehicle_vector = Maths::Normalise(vehicle_vector);

			float dot_prod1 = Maths::DotProduct(pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AX(), vehicle_vector);
			bool flip = (dot_prod1 <= 0);

			Maths::HVec3 force_dir = pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AX() * (flip ? 1.f : -1.f);
			force_dir = Maths::Normalise(force_dir);

			pickup->mMovementForce = g_debugRiftMineShiftingForce * ( 1 - ( dist / pBullBarsData->mRadius ) );
			pickup->mMovementDir = force_dir;
		}

		pActor->mPickupInfo.mCurrentlyUsingType=E_PICKUP_TYPE_RIFT;

		return false;
	}
}
