#include "stdafx.h"
#include <GameLogic/Pickups/PickupClasses.h>	// [Blur2PC]

#include <GameLogic/Pickups/Mods/PickupBlindfireShotgunReverse.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

extern Debug::Float g_debugBlindfireShotgunAngle;
extern Debug::Int g_debugBlindfireShotgunNumShots;

namespace Game
{
	C_PickupBlindfireShotgunReverse::C_PickupBlindfireShotgunReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX)
		: C_PickupBase(E_PickupClass_BlindfireShotgunReverse, id, firingIdx, E_PICKUP_TYPE_BLINDFIRE, subType, noFiringVehicleFX)
	{
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireShotgunReverse::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);

		if(VehicleIsLocal(mFiringIdx))
		{
			SetPostDelayMatrix(matrix);

			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, matrix);
			}
		}
	}

	// --------------------------------------------------------------------------------

	bool C_PickupBlindfireShotgunReverse::Update()
	{
		if(!mActivated)
			return false;

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

		float angleInRad = Maths::DegToRad((F32)g_debugBlindfireShotgunAngle);

		Maths::HMat43 reverseMat = pActor->GetWorldMatrix();
		reverseMat.AZ(-reverseMat.AZ());

		for( int i = -g_debugBlindfireShotgunNumShots; i <= g_debugBlindfireShotgunNumShots; i++ )
		{
			Maths::HMat43 matrix		= reverseMat;
			Maths::HMat43 invMatrix		= matrix.InverseRotTrans();
			Maths::HMat43 localMatrix	= invMatrix * matrix;

			Maths::HMat43 rotMat	= Maths::HMat43(Maths::RotY(angleInRad * i));
			localMatrix				= localMatrix * rotMat;
			matrix					= localMatrix * matrix;

			PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BLINDFIRE, E_PICKUP_SUB_TYPE_SUB_MOD_REVERSED, mFiringIdx);
			pickupInfo.mDisableVehicleFX	= true;
			pickupInfo.mOverrideMatrix		= true;
			pickupInfo.mMatrix				= matrix;
			C_PickupsManager::Instance().CreatePickup(pickupInfo);
		}

		return true;
	}
}