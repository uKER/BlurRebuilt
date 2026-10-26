#include "stdafx.h"
#include <GameLogic/Pickups/PickupClasses.h>	// [Blur2PC]

#include <GameLogic/Pickups/Double/PickupDoubleNitrous.h>
#include "Game/Game.h"
#include "GameApp.h"
#include "Layers/RaceBookLayer.h"
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"

namespace Game
{
	C_PickupDoubleNitrous::C_PickupDoubleNitrous(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupBase(E_PickupClass_DoubleNitrous, id, firingIdx, E_PICKUP_TYPE_NITROUS, subType, noFiringVehicleFX), mTimer(0.f)
	{
		mpNitrousData = &C_Game::GetInstance()->GetLevelState().GetPickupInfo(firingIdx).NitrousDoubleTapInfo();
		BIZ_ASSERT(mpNitrousData);

		Game::C_LevelStateMultiplayerVehicleAbilities* pMpAbilities = Game::C_Game::GetInstance()->GetLevelState().mCars[firingIdx].mUpgrades.GetMultiplayerAbilities();
		if(pMpAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_NitroRift)->GetActive())
		{
			pMpAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_NitroRift, true, false, firingIdx);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupDoubleNitrous::PostDelayInitialise( Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);
		C_PickupBase::SetPostDelayMatrix(matrix);
	}

	// --------------------------------------------------------------------------------

	bool C_PickupDoubleNitrous::Update()
	{
		if(!mActivated)
			return false;

		if(mTimer >= mpNitrousData->mRiftTime)
			return true;

		Game::C_LevelState& levelState		= C_Game::GetInstance()->GetLevelState();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

		for(U32 a = 0; a < levelState.mNumCars; a++)
		{
			if(a == mFiringIdx)
				continue;

			Actors::C_VehicleLogicActor* pOtherLogicActor = pLayer->GetLogicVehicleActor(a);

			float angle		= pActor->mVehicleProximities[a].mRelativeHeading;
			float checkDist	= pActor->mVehicleProximities[a].mDistanceSquared;
			float radiusSq	= mpNitrousData->mRiftRadius * mpNitrousData->mRiftRadius;

			bool inRange = (angle >= mpNitrousData->mRiftAngleRange && checkDist < radiusSq);

			if(inRange)
			{
				if(!CheckCanHit(a, E_PICKUP_HIT_TYPE_MINOR))
					continue;

				RegisterHit(a, E_PICKUP_HIT_TYPE_MINOR);
			}
		}

		mTimer += C_GameApp::GetInstance()->GetTimeStep();

		return false;
	}

	// --------------------------------------------------------------------------------

	E_TARGETTING_DIRECTION C_PickupDoubleNitrous::GetHitDirection(Actors::C_VehicleLogicActor* pActor)
	{
		E_TARGETTING_DIRECTION hitDir = E_TARGETTING_DIRECTION_NONE;

		if(pActor)
		{
			Maths::HVec3 dir = Maths::Normalise(mMatrix.AW() - pActor->GetWorldMatrix().AW());
			float dotProdX = Maths::DotProduct(dir, pActor->GetWorldMatrix().AX());
			if( dotProdX < 0 )
			{
				hitDir = E_TARGETTING_DIRECTION_LEFT;
			}
			else
			{
				hitDir = E_TARGETTING_DIRECTION_RIGHT;
			}
		}

		return hitDir;
	}
}

