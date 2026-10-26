#include "stdafx.h"

#include <GameLogic/Pickups/DoubleReverse/PickupDoubleShuntReverse.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Pickups/PickupClasses.h>

Debug::Float g_debugDoubleShuntReverseAngle("Perks.DoubleShuntReverse.Angle", 10.f);
Debug::Int g_debugDoubleShuntReverseNumShots("Perks.DoubleShuntReverse.NumShotsOffCentre", 1);

namespace Game
{
	C_PickupDoubleShuntReverse::C_PickupDoubleShuntReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupBase(E_PickupClass_DoubleShuntReverse, id, firingIdx, E_PICKUP_TYPE_SHUNT, subType, noFiringVehicleFX)
	{
	}

	// --------------------------------------------------------------------

	bool C_PickupDoubleShuntReverse::Update()
	{
		if(!mActivated)
			return false;

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

		float angleInRad = Maths::DegToRad((F32)g_debugDoubleShuntReverseAngle);

		for( int i = -g_debugDoubleShuntReverseNumShots; i <= g_debugDoubleShuntReverseNumShots; i++ )
		{
			Maths::HMat43 matrix		= pActor->GetWorldMatrix();
			Maths::HMat43 invMatrix		= matrix.InverseRotTrans();
			Maths::HMat43 localMatrix	= invMatrix * matrix;

			Maths::HMat43 rotMat	= Maths::HMat43(Maths::RotY(angleInRad * i));
			localMatrix				= localMatrix * rotMat;
			matrix					= localMatrix * matrix;


			PickupCreationStruct pickupInfo(E_PICKUP_TYPE_SHUNT, E_PICKUP_SUB_TYPE_REVERSED, mFiringIdx);
			pickupInfo.mDisableVehicleFX	= true;
			pickupInfo.mOverrideMatrix		= true;
			pickupInfo.mMatrix				= matrix;
			C_PickupsManager::Instance().CreatePickup(pickupInfo);
		}

		return true;
	}

	// --------------------------------------------------------------------

	void C_PickupDoubleShuntReverse::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
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