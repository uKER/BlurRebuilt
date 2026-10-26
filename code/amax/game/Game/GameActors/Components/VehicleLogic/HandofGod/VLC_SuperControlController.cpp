#include "stdafx.h"
#include "VLC_SuperControlController.h"

#include <GameLogic\Structures\LevelState.h>
#include <Input/GameInput.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

Debug::Bool g_AddSuperControl("Multiplayer.SuperControlComponent.SuperControlEnable", false);
Debug::Bool g_SuperControlDynamicOverride("Multiplayer.SuperControlComponent.SuperControlDynamicOverride", false);

Debug::Int g_SuperControlVehicleIndex("Multiplayer.SuperControlComponent.VehicleToAddTo", 0, Debug::Range(0, 19));

Debug::Float g_SuperControlSteeringForwardOffset("Multiplayer.SuperControlComponent.SteeringForwardOffset",  2.5f, Debug::Range(-10.f, 10.f));
Debug::Float g_SuperControlSteeringBoost("Multiplayer.SuperControlComponent.SteeringBoost", 100.f, Debug::Range(0.f, 5000.f));
Debug::Float g_SuperControlSteeringBoostMinimumSpeed("Multiplayer.SuperControlComponent.SteeringBoostMinimumSpeed", 2.f, Debug::Range(0.f, 100.f));
Debug::Float g_SuperControlAccelerationOrDecelerationBoost("Multiplayer.SuperControlComponent.AccelerationOrDecelerationBoost", 400.f, Debug::Range(0.f, 5000.f));
Debug::Float g_SuperControlAccelerationOrDecelerationBoostTargetSpeed("Multiplayer.SuperControlComponent.AccelerationOrDecelerationBoostTargetSpeed", 100.f, Debug::Range(0.f, 200.f));
Debug::Float g_SuperControlDownForceBoost("Multiplayer.SuperControlComponent.DownForceBoost", 0.f, Debug::Range(0.f, 5000.f));
Debug::Float g_SuperControlHandbrakeBoost("Multiplayer.SuperControlComponent.HandbrakeBoost", 1.5f, Debug::Range(0.f, 5.f));

Debug::Bool g_SuperControlDebugMode("Multiplayer.SuperControlComponent.DebugMode", false);
Debug::Bool g_SuperControlAccelerationOrDecelerationBoostActive("Multiplayer.SuperControlComponent.AccelerationOrDecelerationBoostActive", false);
Debug::Bool g_SuperControlSteeringBoostActive("Multiplayer.SuperControlComponent.SteeringBoostActive", false);
Debug::Bool g_SuperControlDownforceBoostActive("Multiplayer.SuperControlComponent.DownforceBoostActive", false);

namespace Components
{
	const float C_VehicleComponent_SuperControlController::MINIMUM_CONTROLLER_INPUT = 0.15f;

	C_VehicleComponent_SuperControlController::~C_VehicleComponent_SuperControlController()
	{
		//
	}

	bool C_VehicleComponent_SuperControlController::InitialiseImpl()
	{
		m_pGraphicalActor = 0;
		m_pLogicActor = 0;
		m_pRigidBody = 0;
		mControllerIndex = -1;
		mRaceStarted = false;

		return true;
	}

	bool C_VehicleComponent_SuperControlController::CreateImpl()
	{
		const I_VehicleComponent_SuperControlController_CreateInfo* pCreationData = static_cast<const I_VehicleComponent_SuperControlController_CreateInfo *>(GetCreator().GetData());
		BIZ_ASSERT_MSG(pCreationData, "C_VehicleComponent_SuperControlController::CreateImpl() > *** COULDN'T GET A POINTER TO THE CREATION DATA, THIS IS SUPPOSED TO BE GUARANTEED! *** ");
		
		mActorHandle = pCreationData->mParentActorHandle;
		mVehicleIndex = pCreationData->mVehicleIndex;
		Game::C_RaceLogicLayer* pLayer = pCreationData->m_pRaceLogicLayer;
		
		Game::Actors::C_VehicleLogicActor* pLogicActor	= pLayer->GetLogicVehicleActor(mVehicleIndex);
		m_pGraphicalActor = pCreationData->m_pGraphicalActor;
		m_pLogicActor = pCreationData->m_pLogicActor;
		m_pRigidBody = &m_pGraphicalActor->GetCollisionBody()->GetBody();
 
		mControllerIndex = pCreationData->m_pLevelState->mCars[mVehicleIndex].mController;
		return true;
	}

	void C_VehicleComponent_SuperControlController::Update()
	{
		//Uninitialised.................default for AI
		if(!mRaceStarted || mControllerIndex == -1 || mControllerIndex == 99)
			return;

		PERF_EVENT("VehicleComponent.SuperControlController");

		float speedInMPH = GetSpeedInMPH();

		float steeringInput = 0.f;	
		float throttleInput = 0.f;  //Range -1 -> 1; -1 == full braking
		bool handbrakeInput = false;
		GetSteeringThrottleAndHandbrakeInputs(steeringInput, throttleInput, handbrakeInput);

		//Apply the super duper steering (TM)
		const bool steeringBoostActive = ApplySteeringBoost(m_pGraphicalActor, m_pRigidBody, throttleInput, steeringInput, handbrakeInput, abs(speedInMPH));

		//Apply the acceleration/deceleration boost
		const bool accelerationOrDecelerationBoostActive = ApplyAccelerationOrDecelerationBoost(m_pGraphicalActor, m_pRigidBody, throttleInput, handbrakeInput, speedInMPH);
		
		//Apply the down force
		const bool downforceBoostActive = ApplyDownForceBoost(m_pGraphicalActor, m_pRigidBody);

		if(g_SuperControlDebugMode)
		{
			g_SuperControlDownforceBoostActive = downforceBoostActive;
			g_SuperControlSteeringBoostActive = steeringBoostActive;
			g_SuperControlAccelerationOrDecelerationBoostActive = accelerationOrDecelerationBoostActive;
		}
	}

	//Get the inputs that are being added by the player
	void C_VehicleComponent_SuperControlController::GetSteeringThrottleAndHandbrakeInputs(	float& steeringInput, 
																							float& throttleInput, 
																							bool& handbrakeInput) const
	{
		//Get the input from the pad
		Input::C_GameInput* pInput = Input::GetGameInput(mControllerIndex);

		//Steering
		steeringInput = Maths::Clamp(pInput->GetAxisSignedValue(Game::E_Axis_Steering) + pInput->GetAxisSignedValue(Game::E_Axis_DigitalSteering), -1.f,1.f);
		
		//Accelerator
		float acceleratorInput = Maths::Clamp(pInput->GetButtonValue(Game::E_Button_Throttle), 0.f, 1.f);
		acceleratorInput = NormaliseInput(acceleratorInput);

		//Brake
		float brakeInput = Maths::Clamp(pInput->GetButtonValue(Game::E_Button_Brake), 0.f, 1.f);
		brakeInput = NormaliseInput(brakeInput);

		handbrakeInput = Maths::Clamp(pInput->GetButtonValue(Game::E_Button_Handbrake), 0.f, 1.f) ? true : false;

		//Convert the brake and accelerator inputs into -1 : 1 range, where 1 means full accelerator and -1 full brake
		if(acceleratorInput)
		{
			throttleInput = acceleratorInput;
		}
		else
		{
			throttleInput = -brakeInput;
		}
	}

	
	const bool C_VehicleComponent_SuperControlController::ApplySteeringBoost(	const Game::Actors::C_VehicleActor* pGraphicalActor, 
																				SharedPhysics::C_RigidBody* pRigidBody, 
																				const float throttleInput, 
																				const float steeringInput, 
																				const bool handbrakeInput, 
																				const float absSpeedInMPH) const
	{
		if(!ShouldWeBeApplyingSteeringBoost(throttleInput, absSpeedInMPH))
			return false;

		Maths::HVec3 position = pGraphicalActor->GetMatrix().AW();
		const Maths::HVec3 left = pGraphicalActor->GetMatrix().AX();
		const Maths::HVec3 forward = pGraphicalActor->GetMatrix().AZ();

		position += (forward * g_SuperControlSteeringForwardOffset);

		float handbrakeBoostMultiplier = 1.f;
		if(handbrakeInput && g_SuperControlHandbrakeBoost > 0.f)
		{
			handbrakeBoostMultiplier = g_SuperControlHandbrakeBoost;
		}
		
		const float multiplier = steeringInput * g_SuperControlSteeringBoost * handbrakeBoostMultiplier;
		const Maths::HVec3 impulse = left * multiplier;

		pRigidBody->ApplyImpulseWorldSpace( impulse, position);

		if(abs(multiplier) > 0.f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	const bool C_VehicleComponent_SuperControlController::ApplyAccelerationOrDecelerationBoost(	const Game::Actors::C_VehicleActor* pGraphicalActor, 
																								SharedPhysics::C_RigidBody* pRigidBody, 
																								const float throttleInput, 
																								const bool handbrakeInput, 
																								const float speedInMPH) const
	{
		if(!ShouldWeBeApplyingAccelerationOrDecelerationBoost(handbrakeInput))
			return false;

		const Maths::HVec3 position = pGraphicalActor->GetMatrix().AW();
		const Maths::HVec3 forward = pGraphicalActor->GetMatrix().AZ();

		float boostMultiplier = 0.f;
		GetAccelerationOrDecelerationBoostMultiplier(throttleInput, speedInMPH, boostMultiplier);

		const float multiplier = g_SuperControlAccelerationOrDecelerationBoost * throttleInput * boostMultiplier;
		const Maths::HVec3 impulse = forward * multiplier;

		pRigidBody->ApplyImpulseWorldSpace( impulse, position);
		if(abs(multiplier) > 0.f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	const bool C_VehicleComponent_SuperControlController::ApplyDownForceBoost(	const Game::Actors::C_VehicleActor* pGraphicalActor, 
																				SharedPhysics::C_RigidBody* pRigidBody) const
	{
		const Maths::HVec3 position = pGraphicalActor->GetMatrix().AW();
		const Maths::HVec3 down = - pGraphicalActor->GetMatrix().AY();
		const Maths::HVec3 impulse = Maths::HVec3( down * g_SuperControlDownForceBoost );

		pRigidBody->ApplyImpulseWorldSpace( impulse, position );
		if(abs(g_SuperControlDownForceBoost) > 0.f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	const float C_VehicleComponent_SuperControlController::NormaliseInput(const float input)
	{
		float normalisedInput = 0.f;
		float minTriggerValue = MINIMUM_CONTROLLER_INPUT;
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

	void C_VehicleComponent_SuperControlController::GetAccelerationOrDecelerationBoostMultiplier(	const float throttleInput, 
																									const float speedInMPH, 
																									float& boostMultiplier) const
	{
		float speedDifference = (g_SuperControlAccelerationOrDecelerationBoostTargetSpeed - abs(speedInMPH));
		
		//If we're trying to turn of the feathering just set the multiplier to 1.f
		if( g_SuperControlAccelerationOrDecelerationBoostTargetSpeed < 0.01f)
		{
			boostMultiplier = 1.f;
		}
		else
		{
			if(throttleInput > 0.f)
			{
				//Accelerating forwards
				if(speedDifference > 0.f && g_SuperControlAccelerationOrDecelerationBoostTargetSpeed >= 0.01f)
				{
					//Feather the boost multiplier in so the further away from the target speed the more powerful the boost
					boostMultiplier = speedDifference / g_SuperControlAccelerationOrDecelerationBoostTargetSpeed;
				}
			}
			else if(throttleInput < 0.f && speedInMPH > 0.f)
			{
				//Braking
				boostMultiplier = 1.f;
			}
		}
	}

	//Gets the speed in MPH of the vehicle and also works makes the speed negative if we're going backwards
	const float	C_VehicleComponent_SuperControlController::GetSpeedInMPH() const
	{
		Maths::HVec3 velDir = m_pGraphicalActor->GetVehicleInput()->m_vVelocity;
		Maths::HVec3 forward = m_pGraphicalActor->GetMatrix().AZ();

		float dotForward = forward | velDir;

		float currentSpeedInMPH = m_pLogicActor->GetPositionInfo().mSpeedInMph;
		if(dotForward < 0.f)
		{
			//We're going backwards
			currentSpeedInMPH *= -1.f;
		}

		return currentSpeedInMPH;
	}
	
	const bool C_VehicleComponent_SuperControlController::ShouldWeBeApplyingAccelerationOrDecelerationBoost(const bool handbrakeInput) const
	{
		//Make sure that all the wheels are on the ground
		if(m_pLogicActor->GetScoring().mTimeInAir > 0.f || handbrakeInput)
			return false;

		return true;
	}

	const bool C_VehicleComponent_SuperControlController::ShouldWeBeApplyingSteeringBoost(const float throttleInput, const float absSpeedInMPH) const
	{
		//Make sure that all the wheels are on the ground
		if(m_pLogicActor->GetScoring().mTimeInAir > 0.f || (throttleInput <= 0.f && absSpeedInMPH < g_SuperControlSteeringBoostMinimumSpeed))
			return false;
		
		return true;
	}

	bool C_VehicleComponent_SuperControlController::OnMessage( const Messaging::C_MessageBase *pMessage )
	{
		switch(pMessage->Type())
		{
		case Game::E_GameflowMessage_RaceStarted:
			{
				mRaceStarted=true;
				return true;
			}

		case Game::E_GameflowMessage_RestartRace:		//	Mass fall-through
		case Game::E_GameflowMessage_QuitAndWinRace:
		case Game::E_GameflowMessage_QuitRace:
		case Game::E_GameflowMessage_RaceFinished:
		case Game::E_GameflowMessage_VehicleFinished:
			{
				mRaceStarted=false;
				return true;
			}
		}

		return I_GameActorComponent::OnMessage(pMessage);
	}

} // namespace Components
