#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <GameLogic/Pickups/Double/PickupDoubleShunt.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Pickups/PickupsManager.h>
#include <GameLogic/Pickups/PickupClasses.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <GameApp.h>

namespace Game
{
	C_PickupDoubleShunt::C_PickupDoubleShunt(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupBase(E_PickupClass_DoubleShunt, id, firingIdx, E_PICKUP_TYPE_SHUNT, subType, noFiringVehicleFX), mTimer(0.f), mShotsFired(0)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHUNT);
		mpShuntData							= (RLShuntDoubleBalancingData*)pPickup->mDoubleTap->mBalancingData;

		BIZ_ASSERT(mpShuntData);
	}

	// --------------------------------------------------------------------

	bool C_PickupDoubleShunt::Update()
	{
		if(!mActivated)
			return false;

		mTimer -= C_GameApp::GetInstance()->GetTimeStep();

		if(mTimer <= 0.f)
		{
			C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

			float bestEval = 0;
			int bestTarget = -1;
			for( int i = 0; i < pLayer->GetNumVehicles(); i++ )
			{
				if(mFiringIdx == i || mVehiclesHitMajor[i])
					continue;

				// No longer used - if we resurrect double shunt, fix this!
				float nextEval = 0.f;//pActor->EvaluateShuntTarget(i, pActor->mVehicleProximities[i].mRelativeHeading);
				if(bestTarget == -1 || nextEval < bestEval )
				{
					bestEval = nextEval;
					bestTarget = i;
				}
			}

			PickupCreationStruct pickupInfo(E_PICKUP_TYPE_SHUNT, E_PICKUP_SUB_TYPE_NORMAL, mFiringIdx);
			pickupInfo.mTargetIdx			= bestTarget;
			pickupInfo.mDisableVehicleFX	= true;

			C_PickupsManager::Instance().CreatePickup(pickupInfo);

			if(bestTarget != -1)
				mVehiclesHitMajor[bestTarget] = true;

			mShotsFired++;
			mTimer = mpShuntData->mShotDelay;
		}

		if(mShotsFired >= 3)
			return true;

		return false;
	}

	// --------------------------------------------------------------------

	void C_PickupDoubleShunt::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);

		if(VehicleIsLocal(mFiringIdx))
		{
			C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

			SetPostDelayMatrix(pActor->GetWorldMatrix());

			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, pActor->GetWorldMatrix());
			}
		}
	}
}