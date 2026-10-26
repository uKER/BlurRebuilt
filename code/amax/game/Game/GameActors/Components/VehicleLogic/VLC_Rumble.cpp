#include "stdafx.h"

#include "VLC_Rumble.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <Game\GameWorld\GameWorld.h>
#include <Game\Messaging\GameflowMessages.h>

#include <GameLogic/Structures/LevelState.h>
#include <game/gamecontrols/GameControls.h>

#include <input/Device.h>
#include <Vehicle/Input.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game\Messaging\PickupActorMessages.h>
#include <GameLogic\Messaging\GameLogicMessages.h>

#include <specialFX\RumbleSpecialFX.h>

Debug::Bool g_debugEnableRumble("Rumble.Enable",true, Debug::Persist());

Debug::Bool g_debugDebugRumble("Rumble.DebugRumble",false);
Debug::Float g_debugRumbleLowMotor("Rumble.LowFrequency", 0.0f, Debug::Range(0.f, 1.f));
Debug::Float g_debugRumbleHighMotor("Rumble.HighFrequency", 0.0f, Debug::Range(0.f, 1.f));

Debug::Bool g_debugRumbleUseNewSlipModifier("Rumble.UseNewSlipModifier",true);
Debug::Float g_debugRumbleSlipModifierMin("Rumble.SlipModifierMin", 0.05f, Debug::Range(0.f, 1.f));
Debug::Float g_debugRumbleSlipModifierLight("Rumble.SlipModifierLight", 0.25f, Debug::Range(0.f, 1.f));
Debug::Float g_debugRumbleSlipModifierHeavy("Rumble.SlipModifierHeavy", 0.75f, Debug::Range(0.f, 1.f));

Debug::Float g_debugRumbleLandingModifier("Rumble.LandingModifier", 0.05f);

Debug::Float g_debugRumbleRevsPower("Rumble.RevsPower", 0.0f);

Debug::Float g_debugRumbleBuildUpPower("Rumble.BuildUpPower", 0.6f);

Debug::Float g_debugRumbleFirePowerTime("Rumble.FirePowerTime", 0.25f);
Debug::Float g_debugRumbleFirePowerRumblePower("Rumble.FirePowerRumblePower", 0.75f);

Debug::Float g_debugRumbleHitByPowerTime("Rumble.HitByPowerTime", 0.5f);
Debug::Float g_debugRumbleHitByPowerRumblePower("Rumble.HitByPowerRumblePower", 0.75f);

Debug::Float g_debugRumbleCollisionPower("Rumble.CollisionPower", 0.0075f);

Debug::Float g_debugRumbleHandbrakeTimerSpeed("Rumble.HandBrakeTimerSpeed", 1.0f);
Debug::Float g_debugRumbleHandbrakeTimerMax("Rumble.HandBrakeTimerMax", 0.2f);
Debug::Float g_debugRumbleHandbrakeStrength("Rumble.HandBrakeStrength", 1.0f);
Debug::Float g_debugRumbleHandbrakeThreshold("Rumble.HandBrakeThreshold", 1.0f);

Debug::Float g_debugRumblePS3HighFrequencyThreshold("Rumble.PS3HighFrequencyThreshold", 0.75f);

Debug::Float g_debugRumbleWaterThreshold("Rumble.WaterThreshold", 5.0f);
Debug::Float g_debugRumbleWaterEntryExit("Rumble.WaterEntryExit", 3.0f);
Debug::Float g_debugRumbleInWaterRumble("Rumble.InWaterRumble", 0.025f);

namespace Components
{
	C_VehicleComponent_Rumble::~C_VehicleComponent_Rumble() 
	{ 
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		int controller=levelState.mCars[mVehicleIndex].mController;

		// We can get here with automation and not have a real pad attached
		if (controller == Game::INVALID_CONTROLLER)
			return;

		Input::C_Device *pad=Framework::C_ApplicationBase::GetInstance()->GetPad( controller );
		pad->SetRumbleMotorSpeed(0,0);
		pad->SetRumbleMotorSpeed(1,0);

		mShouldUpdate = false;
	}

	bool C_VehicleComponent_Rumble::CreateImpl()
	{
		const I_VehicleComponent_Rumble_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Rumble_CreateInfo *>(GetCreator().GetData());
		Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

		if(creation_data)
		{
			mVehicleIndex=creation_data->mVehicleIndex;
			mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
		}

		//AddSubscriber( GameMessaging::E_GameMsg_Update );

		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_GameflowMessage_RaceFinished );
		AddSubscriber( Game::E_GameflowMessage_VehicleFinished );

		AddSubscriber( E_VehicleLogicMsg_StartDelayToPerk );
		AddSubscriber( E_VehicleLogicMsg_FiredPerk );
		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
		AddSubscriber( Game::E_PickupMessage_PickupPickedUp );
		AddSubscriber( Game::E_GameLogicMessage_GameplayImpact );

		mDeltaTime = 0.f;
		mShouldUpdate = false;

		mPerkBuildUpTimer=0.f;
		mPerkBuildUpLimit=0.f;
		
		mPerkFiredTimer=0.f;

		mPerkHitByTimer=0.f;

		mHeavyLandingTimer=0.f;

		mNitrousTimer=0.f;
		mCollisionTimer=0.f;

		mHandbrakeTimer=0.f;

		mWheelsOnGround.Destroy();
		mWheelSurface.Destroy();

		return true;
	}

	void C_VehicleComponent_Rumble::Update()
	{
		if(!mShouldUpdate)
			return;

		PERF_EVENT("VLC.Rumble.Update");

		mDeltaTime = C_GameApp::GetInstance()->GetTimeStep();

		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		int controller=levelState.mCars[mVehicleIndex].mController;

		// We can get here with automation and not have a real pad attached
		if (controller == Game::INVALID_CONTROLLER)
			return;

		Input::C_Device *pad=Framework::C_ApplicationBase::GetInstance()->GetPad( controller );

		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		float low_rumble=0;
		float high_rumble=0;


		if(mNitrousTimer)
		{
			mNitrousTimer-=mDeltaTime;
			
			if(mNitrousTimer < 0.f)
				mNitrousTimer=0.f;
		}

		//work out the base rev level of the cars
		float revs=0;
		float redLine=0;
		const Vehicle::C_ModelInput *pInput = pRLC->GetVehicleModelInput(mVehicleIndex);
		if(pInput)
		{
			revs = pInput->m_fRevs;
			redLine = pInput->m_fRedLine;

			if(mNitrousTimer == 0)
			{
				high_rumble+=(Maths::Clamp(revs/redLine,0.f,1.f)) * g_debugRumbleRevsPower;
			}

			//work out the slip rumble
			float slip_amount=0;
			int landed_wheels=0;
			int wheel_on_ground=0;
			bool do_landing_check=true;

			//resize the first time - as we don't know how many wheels we have
			if(mWheelsOnGround.Size() == 0)
			{
				mWheelsOnGround.Resize(pInput->m_Wheels.Size());
				do_landing_check=false;
			}

			if(mWheelSurface.Size() == 0)
			{
				mWheelSurface.Resize(pInput->m_Wheels.Size());
			}

			for(int iWheel = 0; iWheel < pInput->m_Wheels.Size(); iWheel++)
			{
				U32 surface_mod=pInput->m_Wheels[iWheel].m_iSurfaceModifier;

				if(do_landing_check)
				{
					if(pInput->m_Wheels[iWheel].m_bOnGround == true && mWheelsOnGround[iWheel] == false)
					{
						landed_wheels++;
					}
				}

				mWheelsOnGround[iWheel]=pInput->m_Wheels[iWheel].m_bOnGround;
				slip_amount += abs(pInput->m_Wheels[iWheel].m_fSlipAngle);

				if(mWheelsOnGround[iWheel])
				{
					wheel_on_ground++;

					if((surface_mod != 2 && surface_mod != 10 && surface_mod != 22) && (mWheelSurface[iWheel] == 2 || mWheelSurface[iWheel] == 10 || mWheelSurface[iWheel] == 22))
					{
						low_rumble += g_debugRumbleWaterEntryExit;
					}

					if((mWheelSurface[iWheel] != 2 && mWheelSurface[iWheel] != 10 && mWheelSurface[iWheel] != 22) && (surface_mod == 2 || surface_mod == 10 || surface_mod == 22))
					{
						low_rumble += g_debugRumbleWaterEntryExit;
					}

					float mph=pInput->m_fSpeed * 3600.f / 1609.f;
					if(mph > g_debugRumbleWaterThreshold)
					{
						if(mWheelSurface[iWheel] == 2 || mWheelSurface[iWheel] == 10)
						{
							high_rumble += g_debugRumbleInWaterRumble;
						}
					}
				}

				mWheelSurface[iWheel]=surface_mod;
			}

			if(slip_amount)
			{
				if(mNitrousTimer == 0)
				{
					if (!g_debugRumbleUseNewSlipModifier)
						high_rumble+=(slip_amount * g_debugRumbleSlipModifierLight);
					else
					{
						slip_amount /= 4.f;

						float high_slip_mod = 0.f;
						float low_slip_mod = 0.f;

						if (slip_amount > g_debugRumbleSlipModifierHeavy)
							low_slip_mod = 1.f;
						else if (slip_amount > g_debugRumbleSlipModifierLight)
						{
							//	blend between light and heavy
							low_slip_mod = (slip_amount - g_debugRumbleSlipModifierLight) / (g_debugRumbleSlipModifierHeavy - g_debugRumbleSlipModifierLight);
							high_slip_mod = 1.f - low_slip_mod;
						}
						else if (slip_amount > g_debugRumbleSlipModifierMin)
							high_slip_mod = (slip_amount - g_debugRumbleSlipModifierMin) / (g_debugRumbleSlipModifierLight - g_debugRumbleSlipModifierMin);

						high_rumble += high_slip_mod;
						low_rumble += low_slip_mod;
					}
				}
			}

			if(landed_wheels)
			{
				float landing_amount=abs(pInput->m_vVelocity.Y());
				
				landing_amount=Maths::Clamp(landing_amount,0.f,0.25f);

				mHeavyLandingTimer+=Maths::Clamp(landing_amount * g_debugRumbleLandingModifier,0.f,0.5f);
			}
			
			if(pInput->m_bHandbrake)
			{
				mHandbrakeTimer+=mDeltaTime * g_debugRumbleHandbrakeTimerSpeed;

				if(mHandbrakeTimer > g_debugRumbleHandbrakeTimerMax)
				{
					mHandbrakeTimer = g_debugRumbleHandbrakeTimerMax;
				}

				if(wheel_on_ground == 4)
				{
					float mph=pInput->m_fSpeed * 3600.f / 1609.f;
					if(mph > g_debugRumbleHandbrakeThreshold)
					{
						low_rumble += (mHandbrakeTimer * g_debugRumbleHandbrakeStrength);
					}
				}
			}
		}

		if(mHeavyLandingTimer)
		{
			mHeavyLandingTimer-=mDeltaTime;

			if(mHeavyLandingTimer < 0.f)
				mHeavyLandingTimer=0.f;
				
			low_rumble+=mHeavyLandingTimer;
		}


		//work out the perk build up
		if(mPerkBuildUpTimer)
		{
			mPerkBuildUpTimer-=mDeltaTime;

			if(mPerkBuildUpTimer < 0.f)
				mPerkBuildUpTimer=0.f;

			high_rumble+=Maths::Clamp(mPerkBuildUpLimit-mPerkBuildUpTimer,0.f,mPerkBuildUpLimit) * g_debugRumbleBuildUpPower;
		}


		//work out perk fire
		if(mPerkFiredTimer)
		{
			mPerkFiredTimer-=mDeltaTime;
			
			if(mPerkFiredTimer < 0.f)
				mPerkFiredTimer=0.f;

			low_rumble+=g_debugRumbleFirePowerRumblePower;
		}

		//work out perk hit by
		if(mPerkHitByTimer)
		{
			mPerkHitByTimer-=mDeltaTime;
			
			if(mPerkHitByTimer < 0.f)
				mPerkHitByTimer=0.f;

			low_rumble+=g_debugRumbleHitByPowerRumblePower;
		}

		//work out the collision
		if(mCollisionTimer)
		{
			mCollisionTimer-=mDeltaTime;

			if(mCollisionTimer < 0.f)
				mCollisionTimer=0.f;

			high_rumble+=mCollisionTimer;
		}

		S32 viewportIndex = pRLC->GetViewportIndex(mVehicleIndex);

		// Query the SFX system for any rumbles
		const SpecialFX::C_RumbleSpecialFX::ActiveRumblesList& RumbleList = SpecialFX::C_RumbleSpecialFX::GetActiveRumblesList();
		for(SpecialFX::C_RumbleSpecialFX::ActiveRumblesList::ConstIterator pIterator = RumbleList.Begin(); pIterator != RumbleList.End(); ++pIterator)
		{
			if( pIterator->m_ViewportID == viewportIndex )
			{
				high_rumble += pIterator->m_HighFreqSpeed;
				low_rumble += pIterator->m_LowFreqSpeed;
			}
		}

		//set the rumble once everything is done
#ifdef PLATFORM_PS3
		pad->SetRumbleMotorSpeed(0,Maths::Clamp(low_rumble/4.f,0.f,0.25f));

		if(high_rumble > g_debugRumblePS3HighFrequencyThreshold)
			pad->SetRumbleMotorSpeed(1,1.f);
		else
			pad->SetRumbleMotorSpeed(1,0.f);
#else
		pad->SetRumbleMotorSpeed(0,Maths::Clamp(low_rumble,0.f,1.f));
		pad->SetRumbleMotorSpeed(1,Maths::Clamp(high_rumble,0.f,1.f));
#endif

		//over-write everything with the debug stuff, if enabled
		if(g_debugDebugRumble)
		{
			pad->EnableRumble();
			pad->SetRumbleMotorSpeed(0,g_debugRumbleLowMotor);
			pad->SetRumbleMotorSpeed(1,g_debugRumbleHighMotor);
		}

		if(!g_debugEnableRumble)
		{
			pad->SetRumbleMotorSpeed(0,0);
			pad->SetRumbleMotorSpeed(1,0);
		}
	}

	bool C_VehicleComponent_Rumble::OnMessage(const Messaging::C_MessageBase *pMessage) 
	{
		PERF_EVENT("C_VehicleComponent.Rumble.MessageHandler");

		switch(pMessage->Type())
		{
			//case GameMessaging::E_GameMsg_Update:
			//	{
			//		mDeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
			//		Update();
			//	}
			//	return true;

			case E_VehicleLogicMsg_EnableRumble:
				{
					mShouldUpdate = true;
				}
				return true;
				
			case Game::E_GameflowMessage_VehicleFinished:
			case Game::E_GameflowMessage_RaceFinished:
			case E_VehicleLogicMsg_DisableRumble:
				{
					Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
					int controller=levelState.mCars[mVehicleIndex].mController;

					// We can get here with automation and not have a real pad attached
					if (controller == Game::INVALID_CONTROLLER)
						return true;

					Input::C_Device *pad=Framework::C_ApplicationBase::GetInstance()->GetPad( controller );
					pad->SetRumbleMotorSpeed(0,0);
					pad->SetRumbleMotorSpeed(1,0);

					mShouldUpdate = false;
				}
				return true;

			case E_VehicleLogicMsg_StartDelayToPerk:
				{
					const C_VehicleLogicMsg_StartDelayToPerk* msg	= Messaging::MessageCast< C_VehicleLogicMsg_StartDelayToPerk >(pMessage);

					if(msg->mVehicleIndex == mVehicleIndex)
					{
						mPerkBuildUpLimit=mPerkBuildUpTimer=msg->mSFXTimeLimit;

						if(msg->mPickupType == E_PICKUP_TYPE_SHOCK)
						{
							mPerkBuildUpLimit=0.f;
							mPerkFiredTimer=g_debugRumbleFirePowerTime;
						}
					}
				}
				return true;

			case E_VehicleLogicMsg_FiredPerk:
				{
					const C_VehicleLogicMsg_FiredPerkMsg* msg	= Messaging::MessageCast< C_VehicleLogicMsg_FiredPerkMsg >(pMessage);

					if(msg->mVehicleIndex == mVehicleIndex)
					{
						mPerkFiredTimer=g_debugRumbleFirePowerTime;

						if(msg->mPickupType == E_PICKUP_TYPE_SHOCK)
						{
							mPerkFiredTimer=0.f;
							mPerkBuildUpLimit=0.2f;
						}

						if(msg->mPickupType == E_PICKUP_TYPE_NITROUS)
						{
							mNitrousTimer=3.f;
						}
					}
				}
				return true;

			case E_VehicleLogicMsg_HitByPerk:
				{
					const C_VehicleLogicMsg_HitByPerkMsg* msg	= Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);
					
					if(msg->mVehicleIndex == mVehicleIndex)
					{
						mPerkHitByTimer=g_debugRumbleHitByPowerTime;
					}
				}
				return true;

			case Game::E_PickupMessage_PickupPickedUp:
				{
					const Game::Actors::PickupPickedUpMsg* msg		= Messaging::MessageCast< Game::Actors::PickupPickedUpMsg >(pMessage);
					

				}
				return true;

			case Game::E_GameLogicMessage_GameplayImpact:
				{
					const Game::C_GameplayImpactMsg* pMsg		= Messaging::MessageCast<Game::C_GameplayImpactMsg>(pMessage);

					if(pMsg->mVehicleIdx == mVehicleIndex || pMsg->mOtherVehicleIdx == mVehicleIndex)
					{
						mCollisionTimer=pMsg->mDamage * g_debugRumbleCollisionPower;
					}
				}
				return true;
			case Game::E_GameflowMessage_RaceStarted:
				{
					mShouldUpdate = true;
					return true;
				}
				break;
		}

		return I_GameActorComponent::OnMessage(pMessage);
	}
}
