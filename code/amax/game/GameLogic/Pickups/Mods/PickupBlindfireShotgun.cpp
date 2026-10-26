#include "stdafx.h"

#include <GameLogic/Pickups/Mods/PickupBlindfireShotgun.h>
#include <GameLogic/Pickups/PickupClasses.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game\Game.h>

Debug::Float g_debugBlindfireShotgunAngle("Perks.BlindfireShotgun.Angle", 2.f);
Debug::Int g_debugBlindfireShotgunNumShots("Perks.BlindfireShotgun.NumShotsOffCentre", 1);

namespace Game
{
	C_PickupBlindfireShotgun::C_PickupBlindfireShotgun(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX)
		: C_PickupBase(E_PickupClass_BlindfireShotgun, id, firingIdx, E_PICKUP_TYPE_BLINDFIRE, subType, noFiringVehicleFX)
	{
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireShotgun::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
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

	bool C_PickupBlindfireShotgun::Update()
	{
		if(!mActivated)
			return false;

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

		float angleInRad = Maths::DegToRad((F32)g_debugBlindfireShotgunAngle);

		for( int i = -g_debugBlindfireShotgunNumShots; i <= g_debugBlindfireShotgunNumShots; i++ )
		{
			Maths::HMat43 matrix		= pActor->GetWorldMatrix();
			Maths::HMat43 invMatrix		= matrix.InverseRotTrans();
			Maths::HMat43 localMatrix	= invMatrix * matrix;

			Maths::HMat43 rotMat	= Maths::HMat43(Maths::RotY(angleInRad * i));
			localMatrix				= localMatrix * rotMat;
			matrix					= localMatrix * matrix;

			PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BLINDFIRE, E_PICKUP_SUB_TYPE_SUB_MOD, mFiringIdx);
			pickupInfo.mDisableVehicleFX	= true;
			pickupInfo.mOverrideMatrix		= true;
			pickupInfo.mMatrix				= matrix;
			C_PickupsManager::Instance().CreatePickup(pickupInfo);
		}

		return true;
	}
}