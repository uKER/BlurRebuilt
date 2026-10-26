#include "stdafx.h"
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>	// [Blur2PC]

#include <GameLogic/Pickups/Double/PickupDoubleShield.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

Debug::Float g_debugDoubleShieldMass("Perks.DoubleShield.Mass", 10000.f);

extern Debug::Float g_debugShieldScaleX;
extern Debug::Float g_debugShieldScaleY;
extern Debug::Float g_debugShieldScaleZ;

extern Debug::Float g_debugBumperTranslationX;
extern Debug::Float g_debugBumperTranslationY;
extern Debug::Float g_debugBumperTranslationZ;

extern Debug::Bool g_DebugShowMpAbilityActivateSpam;

namespace Game
{
	C_PickupDoubleShield::C_PickupDoubleShield(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) : 
		C_PickupShield(id, firingIdx, subType, noFiringVehicleFX)
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringIdx);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringIdx);
		Gravity::C_Body* pBody				= pVehActor->GetCollisionBody();

		mOldMass = pBody->Mass();
		pBody->SetMass((F32)g_debugDoubleShieldMass);
	}

	// --------------------------------------------------------------------------------

	bool C_PickupDoubleShield::Update()
	{
		if(!mActivated)
			return false;

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringIdx);

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		pActor->mShieldTime -= frameTime;

		if(pActor->mShieldTime <= 0.f)
		{
			//send end msg to audio
			C_VehicleLogicMsg_PerkEnded pMsg(mFiringIdx,mUniqueId,E_PICKUP_TYPE_SHIELD,Game::E_BodyUserDataType_Actor,E_PICKUP_SUB_TYPE_DOUBLE_TAP);
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);

			pActor->GetGraphicalActor(mFiringIdx)->GetCollisionBody()->SetMass(mOldMass);
			return true;
		}

		return false;
	}
}