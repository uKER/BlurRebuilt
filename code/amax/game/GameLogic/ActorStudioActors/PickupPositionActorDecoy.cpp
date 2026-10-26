#include <GameApp.h>
#include <Game/Game.h>	// [Blur2PC]

#include "../Layers/RaceBookLayer.h"
#include "../../Game/Messaging/PickupActorMessages.h"

#include <Game/GameWorld/GameWorld.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <shared_actors/Utility/VisibilityDetection.h>

#include "PickupPositionActorDecoy.h"

Debug::Bool		gRotateDecoysInOppositeDirection("Multiplayer.Upgrades.DecoyDrop.RotateInOppositeDirection", true, Debug::Persist());
Debug::Float	gDecoyRotationSpeedY("Multiplayer.Upgrades.DecoyDrop.RotationSpeedYAxis", 0.2f, Debug::Range(0.f, 2*Maths::Pi), Debug::Persist());
Debug::Float	gDecoyRotationSpeedX("Multiplayer.Upgrades.DecoyDrop.RotationSpeedXAxis", 0.0f, Debug::Range(0.f, 2*Maths::Pi), Debug::Persist());
Debug::Bool		gDecoyBob("Multiplayer.Upgrades.DecoyDrop.BobDecoy", false, Debug::Persist());
Debug::Float	gDecoyBobAmount("Multiplayer.Upgrades.DecoyDrop.BobMagnitude", 1.0f, Debug::Range(0.f, 100.f), Debug::Persist());
Debug::Float	gDecoyBobSpeed("Multiplayer.Upgrades.DecoyDrop.BobSpeed", 1.0f, Debug::Range(0.f, 100.f), Debug::Persist());
Debug::Bool		gDecoyFlipGraphic0("Multiplayer.Upgrades.DecoyDrop.FlipGraphic0", false );
Debug::Bool		gDecoyFlipGraphic90("Multiplayer.Upgrades.DecoyDrop.FlipGraphic90", false );
Debug::Bool		gDecoyFlipGraphic180("Multiplayer.Upgrades.DecoyDrop.FlipGraphic180", false );
Debug::Bool		gDecoyResetToSpawn("Multiplayer.Upgrades.DecoyDrop.ResetToOriginalSpawnMatrix", false);

extern Debug::Float gDebugRotationSpeed;
extern Debug::Int gDebugPickupActiveLevelForDebug;
extern Debug::Int gDebugPickupActiveLevel;
extern Debug::Bool g_debugUsePhysicsTriggersForPickups;

namespace Actors
{
	void PickupPositionActorDecoy::Update(const float frameTime)
	{
		if(!GetGameWorldPtr())
			return;

		//reset desired?
		if(gDecoyResetToSpawn)
		{
			SetWorldMatrix( Load(GetCreationData()->m_WorldMatrix) );
			gDecoyResetToSpawn = false;
		}

		//rotation on x axis
		if(gDecoyRotationSpeedX != 0.f)
		{
			Maths::HVec3 pos = GetWorldMatrix().AW();
			SetWorldPosition(Maths::HVec3());
			SetWorldMatrix( Maths::HMat43(Maths::RotX(gDecoyRotationSpeedX)) * GetWorldMatrix() );
			SetWorldPosition(pos);
		}

		//bobbing behaviour
		if(gDecoyBob)
		{
			Maths::HVec3 pos = Maths::HVec3(0.f, gDecoyBobAmount * Maths::Cos(GetOneShotTimer()*gDecoyBobSpeed), 0.f);
			SetWorldPosition( Load(GetCreationData()->m_WorldMatrix).AW() + pos );
		}

		//rotate speed on y axis
		mRotationSpeed = gDecoyRotationSpeedY;

		//rotate direction
		if(gRotateDecoysInOppositeDirection)
			mRotationSpeed = mRotationSpeed * -1.f;

		//rotate around z
		if(gDecoyFlipGraphic0 || gDecoyFlipGraphic90 || gDecoyFlipGraphic180)
		{	
			Maths::HMat43 finalMtx;

			if( gDecoyFlipGraphic90 )
				finalMtx = Maths::HMat43( Maths::RotZ( Maths::Pi * 0.5f ) );
			else if( gDecoyFlipGraphic180 )
				finalMtx = Maths::HMat43( Maths::RotZ( Maths::Pi ) );
			else if( gDecoyFlipGraphic0 )
				finalMtx = Maths::HMat43( Maths::RotZ( 0.f ) );

			SetWorldMatrix( finalMtx * Load(GetCreationData()->m_WorldMatrix) );

			gDecoyFlipGraphic180 = false;
			gDecoyFlipGraphic90 = false;
			gDecoyFlipGraphic0 = false;
		}

		PickupPositionActor::Update(frameTime);

		mRotationSpeed = gDebugRotationSpeed;
	}

	bool PickupPositionActorDecoy::SetRandomPickupType(bool checkAgainstNext, bool forceChange)
	{
		// Do nothing - we are hijacking this to stop decoys being reassigned new types
		return false;
	}

	bool PickupPositionActorDecoy::CheckPickup(VehiclePickupPosition& vehiclePosition)
	{
		if(!vehiclePosition.mCollided)
			return false;

		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		if( levelState.mCars[vehiclePosition.mVehicleIndex].mUpgrades.GetMultiplayerAbilities()
			->GetAbility(UI::E_MultiplayerVehicleAbilities_Shielded_Bay)->GetActive() )
		{
			Game::Actors::C_VehicleLogicActor *pLogicActor = mpRaceLogicLayer->GetLogicVehicleActor(vehiclePosition.mVehicleIndex);
			if(pLogicActor)
			{
				// [Blur2PC] retail Blur PC (0x00b6e3d0): CanHavePickUp checks the free slots too
				if(pLogicActor->GetIsInLastGaspMode() || !pLogicActor->CanHavePickUp(mPickupType))
					return false;
			}
		}
		
		return true;
	}

	void PickupPositionActorDecoy::GrantPickup(VehiclePickupPosition& vehiclePosition)
	{
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		if( levelState.mCars[vehiclePosition.mVehicleIndex].mUpgrades.GetMultiplayerAbilities()
			->GetAbility(UI::E_MultiplayerVehicleAbilities_Shielded_Bay)->GetActive() )
		{
			levelState.mCars[vehiclePosition.mVehicleIndex].mUpgrades.GetMultiplayerAbilities()->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Shielded_Bay, true);
			PickupPositionActor::GrantPickup(vehiclePosition);
		}
		else
		{
			if(Game::VehicleIsLocal(vehiclePosition.mVehicleIndex))
			{
				Game::C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				Game::Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(vehiclePosition.mVehicleIndex);

				Game::PickupHitInfo hitInfo;
				Maths::HMat43 mat = pActor->GetGraphicalActor(vehiclePosition.mVehicleIndex)->GetMatrix();
				hitInfo.mPosition = vehiclePosition.mWorldPosition;
				hitInfo.mDirection = vehiclePosition.mWorldPosition - vehiclePosition.mWorldPosition;
				hitInfo.mDirection.Normalise();

				if(pActor && pActor->pPickupComponent)
					pActor->pPickupComponent->AttemptHitByPickup( mPickupType, E_PICKUP_SUB_TYPE_DECOY, E_PICKUP_HIT_TYPE_MAJOR, GetCreationData()->m_DroppedFromVehicleIndex, E_TARGETTING_DIRECTION_NONE, GetUniqueDropID(), false, hitInfo, 0);
			}
		}

		Destroy();
		Kill();
	}

	void PickupPositionActorDecoy::SetupVisualPickupIcon()
	{
		const RLPickupType* pPickup	= m_pRLC->GetPickupType(mPickupType);

		if(pPickup)
		{
			if (m_pPickupSpecialFX)
			{
				m_pPickupSpecialFX->Stop();
				SAFE_DELETE_PTR(m_pPickupSpecialFX);
			}
		
			SpecialFX::I_SpecialFXInstance* pickupFX = NULL;
			if(pPickup->mPickupSpecialFXDecoy.Object())
				pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFXDecoy, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);

			if(pickupFX)
			{
				m_pPickupSpecialFX = pickupFX;
			}
		}
	}
}
