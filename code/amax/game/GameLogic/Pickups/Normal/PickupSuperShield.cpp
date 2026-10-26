#include "stdafx.h"
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>	// [Blur2PC]

#include <GameLogic/Pickups/Normal/PickupSuperShield.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic/Pickups/PickupsManager.h>
#include <GameLogic/Pickups/PickupClasses.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameAudio/GameAudioLayer.h>
#include <GameAudio/PickupSubsystem.h>
#include <Layers/AmaxWorldLayer.h>

Debug::Bool g_ForceAllShieldsToBeSuper("Perks.SuperShield.ForceAllShieldsToBeSuper", false);
Debug::Bool g_SuperShieldFinishNowPlease("Perks.SuperShield.FinishNowPlease", false);
Debug::Bool g_InfiniShield("Perks.SuperShield.InfiniShield", false);

static Debug::Vector g_SuperShieldScale("Perks.SuperShield.ScaleTest",  1.0f, 1.25f, 0.47f);
static Debug::Vector g_SuperShieldBumperTranslation("Perks.SuperShield.BumperTranslation",  0.f, -0.65f, 1.f);



namespace Game
{
	C_PickupSuperShield::C_PickupSuperShield(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) : 
					C_PickupBase(E_PickupClass_SuperShield, id, firingIdx, E_PICKUP_TYPE_SUPER_SHIELD, subType, noFiringVehicleFX), 
					mOriginalMass(0.f),
					m_pBalancingData(0)

	{
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pLogicActor	= pLayer->GetLogicVehicleActor(mFiringIdx);
		Actors::C_VehicleActor* pGraphicalActor		= pLogicActor->GetGraphicalActor(mFiringIdx);
		Gravity::C_Body* pBody						= pGraphicalActor->GetCollisionBody();

		const RLPickupType* pPickup					= (const RLPickupType*)::RLDatabase::Instance().GetPickupType(mPickupType);
		BIZ_ASSERT_MSG(pPickup, "C_PickupSuperShield() > Couldn't get a pointer to the pickup type of the super shield");

		const RLPickupLevel* pPickupLevel			= pPickup->mNormal;
		BIZ_ASSERT_MSG(pPickupLevel, "C_PickupSuperShield() > Couldn't get a pointer to the 'normal' pickupLevel of the super shield");

		m_pBalancingData							= (RLSuperShieldBalancingData *)pPickupLevel->mBalancingData;
		BIZ_ASSERT_MSG(m_pBalancingData, "C_PickupSuperShield() > Couldn't get a pointer to the balancing data of the super shield");


		//See if the player's already using one of these bad boys
		if(pLogicActor->mSuperShield)
		{
			KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_REPLACED);
			return;
		}

		pLogicActor->mShieldTime = m_pBalancingData->mLifetime;
		pLogicActor->mSuperShield = true;

		InitialiseSFX(pGraphicalActor, pLogicActor, pPickup, pPickupLevel);


		//Store the original mass of the vehicle as of now
		StoreOriginalMass(pBody);

		if(m_pBalancingData->mIncreaseMassDuringUse)
		{
			//Increase the mass of the vehicle, so we can blat our enemies out of the way with impecunity!
			IncreaseMass(pBody, m_pBalancingData->mIncreasedMass);
		}

		if(m_pBalancingData->mNitroOnStartup)
		{
			//Give the vehicle a fake physics kick up the arse, pazzooowww
			pGraphicalActor->GetFakeSpeed()->Start(Game::Actors::E_FAKE_SPEED_ACCELERATE, m_pBalancingData->mNitroTargetSpeedInMPH);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::InitialiseSFX(const Actors::C_VehicleActor* pGraphicalActor, 
											const Actors::C_VehicleLogicActor* pLogicActor, 
											const RLPickupType *pPickup, 
											const RLPickupLevel* pPickupLevel) const
	{
		XtArray<RLPickupSpecialFX>* sfx = &((RLShieldSpecialFX*)pPickup->mNormal->mSpecialFX)->mShieldDomeSpecialFX;

		AmaxVehicle* pXtVehicle	= C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)pGraphicalActor->GetXtIndex()];

		Maths::HVec3 extents = pGraphicalActor->GetExtents();
		Maths::HVec3 scaleTest(	g_SuperShieldScale.Value().vx * extents.X() * (pXtVehicle->m_ShieldSizeModifier.vx ? pXtVehicle->m_ShieldSizeModifier.vx : 1.f), 
								g_SuperShieldScale.Value().vy * extents.Y() * (pXtVehicle->m_ShieldSizeModifier.vy ? pXtVehicle->m_ShieldSizeModifier.vy : 1.f),
								g_SuperShieldScale.Value().vz * extents.Z() * (pXtVehicle->m_ShieldSizeModifier.vz ? pXtVehicle->m_ShieldSizeModifier.vz : 1.f) );

		Maths::HVec3 offset;
		if(VehicleIsLocal(mFiringIdx))
		{
			offset = Maths::HVec3((F32)g_SuperShieldBumperTranslation.Value().vx, (F32)g_SuperShieldBumperTranslation.Value().vy, (F32)g_SuperShieldBumperTranslation.Value().vz);
		}


		//Start the special effect, with the potentially modified shield duration. 
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(*sfx, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Shield, pLogicActor->mShieldTime, scaleTest, offset);

		//send end msg to audio
		//set fx duration envelope to audio
		C_VehicleLogicMsg_PowerUpFXDurations pMsg(mFiringIdx,mUniqueId,E_PICKUP_TYPE_SHIELD,Game::E_BodyUserDataType_Actor);

		for( int i = 0; i < sfx->count; i++ )
		{
			pMsg.mDurationIntro = (*sfx)[i].mDurationIntro.mEnd - (*sfx)[i].mDurationIntro.mStart;			
			pMsg.mDurationOutro = (*sfx)[i].mDurationOutro.mEnd - (*sfx)[i].mDurationOutro.mStart;

			pMsg.mDurationLoop = (pLogicActor->mShieldTime - (pMsg.mDurationIntro + pMsg.mDurationOutro));
		}

		//send shield durations to vehicle audio component
		Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		if(pCore && pCore->GetVehicleActor(mFiringIdx)->QueryInterface(Components::E_AudioEntity_AdditionalSpawned))
			pCore->GetVehicleActor(mFiringIdx)->SendMessageToComponent(Components::E_AudioEntity_AdditionalSpawned,&pMsg);
		
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_SetFXDurations, 0, &pMsg);
	}

	// --------------------------------------------------------------------

	void C_PickupSuperShield::PostDelayInitialise( Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx )
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);
		C_PickupBase::SetPostDelayMatrix(matrix);
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::StoreOriginalMass(const Gravity::C_Body* pBody)
	{
		mOriginalMass = pBody->Mass();
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::IncreaseMass(Gravity::C_Body* pBody, const float newMass) const
	{
		BIZ_ASSERT_MSG(mOriginalMass, "C_PickupSuperShield::IncreaseMass() > Trying to change the body's mass without having stored the original mass...bad? Probably, talk to CA");
		pBody->SetMass(newMass);
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::RevertMass(Gravity::C_Body* pBody) const
	{
		//Make sure we're not trying to set the mass incorrectly, I consider 0.f to be incorrect
		BIZ_ASSERT_MSG(mOriginalMass, "C_PickupSuperShield::RevertMass() > Trying to set the mass to 0.f, I, at least, think this would be a bad thing! Talk to CA");
		pBody->SetMass(mOriginalMass);
	}


	// --------------------------------------------------------------------------------

	bool C_PickupSuperShield::Update()
	{
		if(!mActivated)
			return false;

		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pLogicActor	= pLayer->GetLogicVehicleActor(mFiringIdx);
		Actors::C_VehicleActor* pGraphicalActor		= pLogicActor->GetGraphicalActor(mFiringIdx);
		Gravity::C_Body* pBody						= pGraphicalActor->GetCollisionBody();
		const bool localPlayer						= VehicleIsLocal(mFiringIdx);

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		
		//Apply our various boost (accel/brake, steering, downforce)
		if(localPlayer)
		{
			ApplyAssortedBoosts(pGraphicalActor);
		}

		if(g_SuperShieldFinishNowPlease)
		{
			pLogicActor->mShieldTime = 0.f;
			g_SuperShieldFinishNowPlease = false;
			g_InfiniShield = false;
		}
		pLogicActor->mShieldTime -= frameTime;

		if(pLogicActor->mShieldTime <= 0.f)
		{
			if(g_InfiniShield)
			{
				Restart(pBody);
			}
			else
			{
				RevertMass(pBody);
				pLogicActor->mSuperShield = false;
			}
			return true;
		}

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::ApplyAssortedBoosts(const Actors::C_VehicleActor* pGraphicalActor) const
	{
		if(!m_pBalancingData->mBoostDuringUse)
		{
			return;
		}

		SharedPhysics::C_RigidBody& rigidBody	= pGraphicalActor->GetCollisionBody()->GetBody();

		float steeringInput = 0.f;	
		float throttleInput = 0.f;  //Range -1 -> 1; -1 == full braking
		GetSteeringAndThrottleInputs(steeringInput, throttleInput);


		//Apply the super duper steering (TM)
		ApplySteeringBoost(pGraphicalActor, rigidBody, steeringInput);

		//Apply the acceleration/deceleration boost
		ApplyAccelerationOrDecelerationBoost(pGraphicalActor, rigidBody, throttleInput);
		
		//Apply the down force
		ApplyDownForceBoost(pGraphicalActor, rigidBody);
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::GetSteeringAndThrottleInputs(float& steeringInput, float& throttleInput) const
	{
		Game::C_LevelState& levelState			= C_Game::GetInstance()->GetLevelState();
		int controllerIndex						= levelState.mCars[mFiringIdx].mController;

		steeringInput = Maths::Clamp(Input::GetGameInput(controllerIndex)->GetAxisSignedValue(Game::E_Axis_Steering) + Input::GetGameInput(controllerIndex)->GetAxisSignedValue(Game::E_Axis_DigitalSteering), -1.f,1.f);
		
		float acceleratorInput = Maths::Clamp(Input::GetGameInput(controllerIndex)->GetButtonValue(Game::E_Button_Throttle), 0.f, 1.f);
		acceleratorInput = NormaliseInput(acceleratorInput);

		float brakeInput = Maths::Clamp(Input::GetGameInput(controllerIndex)->GetButtonValue(Game::E_Button_Brake), 0.f, 1.f);
		brakeInput = NormaliseInput(brakeInput);

		if(acceleratorInput)
		{
			throttleInput = acceleratorInput;
		}
		else
		{
			throttleInput = -brakeInput;
		}
	}

	// --------------------------------------------------------------------------------

	const float C_PickupSuperShield::NormaliseInput(const float input) const
	{
		float normalisedInput = 0.f;
		float minTriggerValue = m_pBalancingData->mMinimumTriggerValue;
		if(input > minTriggerValue)
		{
			//Normalise
			if(minTriggerValue < 1.f)
			{
				normalisedInput = (input - minTriggerValue) / (1.f - minTriggerValue);
			}
			else
			{
				normalisedInput = input;
			}
		}

		return normalisedInput;
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::ApplySteeringBoost(const Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody& rigidBody, const float steeringInput) const
	{
		Maths::HVec3 position = pGraphicalActor->GetMatrix().AW();
		const Maths::HVec3 left = pGraphicalActor->GetMatrix().AX();
		const Maths::HVec3 forward = pGraphicalActor->GetMatrix().AZ();

		position += (forward * m_pBalancingData->mSteeringForwardOffset);
		
		const Maths::HVec3 impulse = left * steeringInput * m_pBalancingData->mSteeringBoost;

		rigidBody.ApplyImpulseWorldSpace( impulse, position);
	}

	// --------------------------------------------------------------------------------
	
	void C_PickupSuperShield::ApplyAccelerationOrDecelerationBoost(const Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody& rigidBody, const float throttleInput) const
	{
		const Maths::HVec3 position = pGraphicalActor->GetMatrix().AW();
		const Maths::HVec3 forward = pGraphicalActor->GetMatrix().AZ();

		const Maths::HVec3 impulse = forward * m_pBalancingData->mAccelerationBoost * throttleInput;

		rigidBody.ApplyImpulseWorldSpace( impulse, position);
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::ApplyDownForceBoost(const Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody& rigidBody) const
	{
		const Maths::HVec3 position = pGraphicalActor->GetMatrix().AW();
		const Maths::HVec3 down = - pGraphicalActor->GetMatrix().AY();
		const Maths::HVec3 impulse = Maths::HVec3( down * m_pBalancingData->mDownForceBoost );

		//TODO Make sure we're vaguely the right way up!
		rigidBody.ApplyImpulseWorldSpace( impulse, position );
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::DebugDraw()
	{
	}

	// --------------------------------------------------------------------------------

	void C_PickupSuperShield::Restart(Gravity::C_Body* pBody)
	{
		//Kill the pickup
		KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_RESET);

		//Revert the mass again (or we'll just store the increase mass when the new pickup gets spawned)
		RevertMass(pBody);

		//Create a new pickup just like this one
		PickupCreationStruct pickupInfo(E_PICKUP_TYPE_SUPER_SHIELD, mPickupSubType, mFiringIdx);
		pickupInfo.mDisableVehicleFX	= true;
		pickupInfo.mParentPickupId		= mUniqueId;

		C_PickupsManager::Instance().CreatePickup(pickupInfo);
	}

	// --------------------------------------------------------------------------------
}