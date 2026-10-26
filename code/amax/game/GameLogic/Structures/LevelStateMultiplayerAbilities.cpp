#include "stdafx.h"
#include "GameLogic/Structures/LevelStateMultiplayerAbilities.h"

// Includes
#include <UI/Game/Multiplayer/MultiplayerDefines.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game/GameActors/Components/GameComponents.h>
#include "GameApp.h"
#include <Game/Game.h>
#include <Ui/UiHelpers.h>
#include <Kanzen/Camera/Utility.h>
#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>

extern Debug::Float g_debugShieldScaleX;
extern Debug::Float g_debugShieldScaleY;
extern Debug::Float g_debugShieldScaleZ;

Debug::Float g_debugLasersightSmoothing("Multiplayer.Upgrades.LaserSight.Smoothing", 0.2f);

namespace Game
{

///////////////////////////////////////////
// LevelStateMultiplayerVehicleAbilities //
///////////////////////////////////////////

C_LevelStateMultiplayerVehicleAbilities::C_LevelStateMultiplayerVehicleAbilities()
{
	// Ensure each entry has its type set (would be nice if could do this on construction..)
	BIZ_ASSERT_MSG(!mAbilities.Empty(),"LevelStateMultiplayerVehicleAbilities constructor < error.. mAbilities is empty!");
	for(int i = 0; i < mAbilities.Size(); i++)
	{
		mAbilities[i].SetType((UI::E_MultiplayerVehicleAbilities)(i));
	}
	m_LaserSightSpecialFX = NULL;
	mLasersightUpdateTime = 0.f;

	m_LastGaspCountdownSpecialFX = NULL;
	mLastGaspUpdateTime = 0.0f;
	mLaserSightCurrentRotation = 0;
	mLaserSightVelocity = 0;

	m_ScramblerSpecialFX = NULL;
	mScrambleCount = 0;
}

C_MultiplayerVehicleAbility* C_LevelStateMultiplayerVehicleAbilities::GetAbility(const UI::E_MultiplayerVehicleAbilities type)
{
	BIZ_ASSERT_MSG(!mAbilities.Empty(),"LevelStateMultiplayerVehicleAbilities::GetAbility(type) < error.. mAbilities is empty!");
	BIZ_ASSERT_MSG(type < mAbilities.Size(),"LevelStateMultiplayerVehicleAbilities::GetAbility(type) < error.. mAbilities is empty!");
	return &mAbilities[type];
}

void C_LevelStateMultiplayerVehicleAbilities::Clear()
{
	BIZ_ASSERT_MSG(!mAbilities.Empty(),"LevelStateMultiplayerVehicleAbilities::Clear() < error.. mAbilities is empty!");
	for(int i = 0; i < mAbilities.Size(); i++)
	{
		mAbilities[i].Clear(); 
	}

	// no mods are currently activated
	mActivationBitset = 0;
	SetPassiveAbilityFiringBits(); //sets the "always on" bits

	for(int i = 0; i < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; i++)
	{
		mActivationTimes[i] = 0;
	}

	if(m_LaserSightSpecialFX)
	{
		m_LaserSightSpecialFX->Stop();
		SAFE_DELETE_PTR(m_LaserSightSpecialFX)
	}

	if(m_LastGaspCountdownSpecialFX)
	{
		m_LastGaspCountdownSpecialFX->Stop();
		SAFE_DELETE_PTR(m_LastGaspCountdownSpecialFX)
	}

	if(m_ScramblerSpecialFX)
	{
		m_ScramblerSpecialFX = NULL;
	}

	mScrambleCount = 0;
}

// If you want your ability to be "always on", set it in here. Also make sure it is not zeroed in ResetInstantAbilityFiringBits() below.
void C_LevelStateMultiplayerVehicleAbilities::SetPassiveAbilityFiringBits()
{
	// e.g. Advanced radar (not currently used however...)
	//mActivationBitset |= ( (U64)1 << UI::E_MultiplayerVehicleAbilities_Advanced_Radar );
}

// Set an ability to be firing or not firing, so the UI/audio guys can do cool things
void C_LevelStateMultiplayerVehicleAbilities::SetAbilityFiring(const UI::E_MultiplayerVehicleAbilities type, bool isFiring, bool notLocal, U8 vehicleIndex, U8 victimIndex, bool alternateFX, F32 scaleValue, float timeStep, Maths::HVec3 rotation)
{
	if(type != UI::E_MultiplayerVehicleAbilities_None)
	{
		Game::C_LevelState& levelState				= Game::C_Game::GetInstance()->GetLevelState();

		bool isNetwork = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork();
		if(notLocal && vehicleIndex != (U8)-1 && isNetwork && isFiring)
		{
			const Game::C_NetGameVars& netGameVars		= C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
			netGameVars.m_EventManager->Add_AbilityActivated(levelState.mCars[vehicleIndex].mNetJoinId, victimIndex!=(U8)-1 ? levelState.mCars[victimIndex].mNetJoinId : (U8)-1, type, alternateFX);
		}
		else
		{
			Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

			bool canUseBossMod = pRLC && pRLC->GetViewportIndex(vehicleIndex) != -1 && type >= UI::E_MultiplayerVehicleAbilities_Boss_AdditionalShockDomes && type <= UI::E_MultiplayerVehicleAbilities_Boss_ShuntDamage;

			if(isNetwork || canUseBossMod)
			{
				if(isFiring)
				{
					//Scrambler victim handles FX - aggressor needs to turn their light on
					if(type == UI::E_MultiplayerVehicleAbilities_Scrambler)
					{
						levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities()->SwitchOnFiringIcon(vehicleIndex, type);
					}
					else
					{
						SwitchOnFiringIcon(vehicleIndex, type);
					}
				}
				else
					mActivationBitset &= ~( (U64)1 << type );
			}
		}

		// Dont do any special fx magic if no effect is firing!
		if(!isFiring)
		{
			//Turn off lasersight or last gasp
			switch(type)
			{

			case UI::E_MultiplayerVehicleAbilities_Laser_Sight:
				{
					if(m_LaserSightSpecialFX)
					{
						m_LaserSightSpecialFX->Stop();
						SAFE_DELETE_PTR(m_LaserSightSpecialFX)
						mLasersightUpdateTime = 0.f;
					}
				}
				break;

			case UI::E_MultiplayerVehicleAbilities_Last_Gasp:
				{
					if(m_LastGaspCountdownSpecialFX)
					{
						m_LastGaspCountdownSpecialFX->Stop();
						SAFE_DELETE_PTR(m_LastGaspCountdownSpecialFX)
						mLastGaspUpdateTime = 0.0f;
					}
				}
				break;

			case UI::E_MultiplayerVehicleAbilities_Scrambler:
				{
					if(m_ScramblerSpecialFX)
					{
						mScrambleCount = 0; //Get rid of stacked scrambles
						m_ScramblerSpecialFX->mCurrentTime = m_ScramblerSpecialFX->mIntroEnd; //Move the current animation to the end
						m_ScramblerSpecialFX = NULL; //The Special FX manager will delete the object for us
					}
				}
				break;
			}
			return;
		}

		if(vehicleIndex == (U8)(-1))
		{
			return;
		}

		Maths::HVec3 scale(1.0f, 1.0f, 1.0f);
		Maths::HVec3 offset(0.f, 0.f, 0.f);
		Maths::HMat43 matrix;
		bool cooldownProhibitsEffect = false;
		Game::C_InGameSpecialFXManager& specialFXManager = Game::C_InGameSpecialFXManager::Instance();

		if(alternateFX)
		{
			if(type == UI::E_MultiplayerVehicleAbilities_Scrambler)
			{
				const XtArray<RLPickupSpecialFX>& sfx = C_Game::GetInstance()->GetMultiplayerAbilities().GetAbility(UI::E_MultiplayerVehicleAbilities_Scrambler)->GetSpecialFX();

				if(!m_ScramblerSpecialFX)
				{
					C_InGameSpecialFXVehicle* pFXVehicle = specialFXManager.mVehiclePool.AllocateNoConstructor();
					new (pFXVehicle) C_InGameSpecialFXVehicle(sfx[0], victimIndex, 0, -1, 0, victimIndex, Game::E_InGameSpecialFX_Trigger_Shield, scale, offset);
					Maths::HVec3 velocity(Maths::UNDEFINED);
					Maths::HVec3 planePos(Maths::UNDEFINED);
					Maths::C_Normal planeNormal(Maths::UNDEFINED);
					pFXVehicle->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
					specialFXManager.Insert(pFXVehicle, matrix, velocity, planePos, planeNormal);

					m_ScramblerSpecialFX = pFXVehicle;
				}
				else
				{
					mScrambleCount += 1;
				}
			}
			return;
		}
		else
		{
			if(type == UI::E_MultiplayerVehicleAbilities_Scrambler)
			{
				const XtArray<RLPickupSpecialFX>& sfx = C_Game::GetInstance()->GetMultiplayerAbilities().GetAbility(UI::E_MultiplayerVehicleAbilities_Scrambler)->GetSpecialFX();

				if(!m_ScramblerSpecialFX)
				{
					C_InGameSpecialFXVehicle* pFXVehicle = specialFXManager.mVehiclePool.AllocateNoConstructor();
					new (pFXVehicle) C_InGameSpecialFXVehicle(sfx[0], victimIndex, 0, -1, 0, victimIndex, Game::E_InGameSpecialFX_Trigger_Shield, scale, offset);
					Maths::HVec3 velocity(Maths::UNDEFINED);
					Maths::HVec3 planePos(Maths::UNDEFINED);
					Maths::C_Normal planeNormal(Maths::UNDEFINED);
					pFXVehicle->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
					specialFXManager.Insert(pFXVehicle, matrix, velocity, planePos, planeNormal);

					m_ScramblerSpecialFX = pFXVehicle;
					mScrambleCount += 2; //Needs to play 3 times so queue two more once the first is finished
				}
				return;
			}
		}


		switch(type)
		{
		case UI::E_MultiplayerVehicleAbilities_Battering_Ram:
			{
				float raceTime = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetRaceTime();
				if(raceTime - mBatteringRamTimeFired < 1.0f)
				{
					cooldownProhibitsEffect = true;
				}
				else
				{
					mBatteringRamTimeFired = raceTime;
				}
			}
			break;
		case UI::E_MultiplayerVehicleAbilities_Titanium_Armour:
		case UI::E_MultiplayerVehicleAbilities_Stable_Frame:
			{
				C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				Actors::C_VehicleActor* pVehActor	= pLayer->GetLogicVehicleActor(vehicleIndex)->GetGraphicalActor(vehicleIndex);
				AmaxVehicle* pXtVehicle				= C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)pVehActor->GetXtIndex()];

				Maths::HVec3 extents = pVehActor->GetExtents();
				scale.X(g_debugShieldScaleX * extents.X() * (pXtVehicle->m_ShieldSizeModifier.vx ? pXtVehicle->m_ShieldSizeModifier.vx : 1.f));
				scale.Y(g_debugShieldScaleY * extents.Y() * (pXtVehicle->m_ShieldSizeModifier.vy ? pXtVehicle->m_ShieldSizeModifier.vy : 1.f));
				scale.Z(g_debugShieldScaleZ * extents.Z() * (pXtVehicle->m_ShieldSizeModifier.vz ? pXtVehicle->m_ShieldSizeModifier.vz : 1.f));
			}
			break;
		case UI::E_MultiplayerVehicleAbilities_Laser_Sight:
			{
				Maths::HMat43 vehicleMat = UI::GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex)->GetWorldMatrix();
	
				mLaserSightCurrentRotation = Kanzen::SmoothingFunction(mLaserSightCurrentRotation, rotation.X(), mLaserSightVelocity, g_debugLasersightSmoothing, timeStep);

				rotation.X(mLaserSightCurrentRotation);
				Maths::C_MatrixRotatePHR rotationMatrix(rotation);
				Maths::HMat33 rotMat(rotationMatrix);
				matrix.SetScaleRotTrans(Maths::HVec3(1,1,1/100.0f*scaleValue), rotMat*vehicleMat.Rotation(), vehicleMat.Translation());

				if(!m_LaserSightSpecialFX)
				{
					//Get the specialFX for Laser Sight
					const XtArray<RLPickupSpecialFX>& sfx = C_Game::GetInstance()->GetMultiplayerAbilities().GetAbility(UI::E_MultiplayerVehicleAbilities_Laser_Sight)->GetSpecialFX();
					if(sfx[0].mLocalSpecialFX.Object())
					{
						m_LaserSightSpecialFX = 
							SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(sfx[0].mLocalSpecialFX, UI::GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
						m_LaserSightSpecialFX->Start();
						mLasersightUpdateTime = 0.f;
					}
				}
				
				if(m_LaserSightSpecialFX)
				{
					mLasersightUpdateTime += timeStep;
					if(mLasersightUpdateTime >= m_LaserSightSpecialFX->GetDuration())
					{
						mLasersightUpdateTime = 0.f;
						m_LaserSightSpecialFX->Stop();
						m_LaserSightSpecialFX->Start();
					}
					m_LaserSightSpecialFX->UpdateWorldMatrix(matrix);
					m_LaserSightSpecialFX->Update(mLasersightUpdateTime);
				}

				return;
			}
			break;

			case UI::E_MultiplayerVehicleAbilities_Last_Gasp:
			{
				Game::Actors::C_VehicleLogicActor* pLogicActor	= UI::GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
				if(!pLogicActor)
				{
					return;
				}

				Maths::HMat43 vehicleMat = pLogicActor->GetGraphicalActor(vehicleIndex)->GetMatrix();
				matrix.SetScaleRotTrans(Maths::HVec3(1.0f,1.0f,1.0f), vehicleMat.Rotation(), vehicleMat.Translation());

				if(!m_LastGaspCountdownSpecialFX)
				{
					//Get the specialFX for LastGasp 
					const XtArray<RLPickupSpecialFX>& sfx = C_Game::GetInstance()->GetMultiplayerAbilities().GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp)->GetSpecialFX();
					if(sfx.count > 0 && sfx[0].mLocalSpecialFX.Object())
					{
						S32 viewIndex = pLogicActor->GetRLC()->GetViewportIndex(vehicleIndex);

						m_LastGaspCountdownSpecialFX = 
							SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(sfx[0].mLocalSpecialFX, UI::GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
						m_LastGaspCountdownSpecialFX->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void*)viewIndex);
						m_LastGaspCountdownSpecialFX->Start();
						mLastGaspUpdateTime = 0.f;
					}
				}
				
				if(m_LastGaspCountdownSpecialFX)
				{
					mLastGaspUpdateTime += timeStep;
					if(mLastGaspUpdateTime >= m_LastGaspCountdownSpecialFX->GetDuration())
					{
						mLastGaspUpdateTime = 0.0f;
						m_LastGaspCountdownSpecialFX->Stop();
						m_LastGaspCountdownSpecialFX->Start();
					}
					m_LastGaspCountdownSpecialFX->Update(mLastGaspUpdateTime);
					m_LastGaspCountdownSpecialFX->UpdateWorldMatrix(matrix);
				}

				return;
			}
			break;

		default:
			{
				break;
			}
		}

		if(!cooldownProhibitsEffect)
		{
			C_Game::GetInstance()->GetMultiplayerAbilities().FireAbilityVFX(type, matrix, vehicleIndex, victimIndex, 
				Game::E_InGameSpecialFX_Trigger_Shield, -1, scale, offset);
		}
	}
}

void C_LevelStateMultiplayerVehicleAbilities::Update(float timeStep, int vehicleIndex)
{
	if(m_ScramblerSpecialFX)
	{
		bool finished = m_ScramblerSpecialFX->mCurrentTime + timeStep >= m_ScramblerSpecialFX->mIntroEnd;
		if(finished)
		{
			if(mScrambleCount > 0) //Still one or more to go;
			{
				m_ScramblerSpecialFX->mCurrentTime = 0.f;
				mScrambleCount--;
			}
			else
			{
				m_ScramblerSpecialFX = NULL; //Special FX manager will delete the object for us		

				C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				if(pLayer)
				{
					Actors::C_VehicleActor* pVehActor	= pLayer->GetLogicVehicleActor(vehicleIndex)->GetGraphicalActor(vehicleIndex);
					if(pVehActor && pVehActor->IsFocusVehicle())
						UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_ScramblerDeactivatedActionIn );
				}
			}
		}
	}
}

bool C_LevelStateMultiplayerVehicleAbilities::IsVehicleScrambled()
{
	return m_ScramblerSpecialFX != NULL;
}

void  C_LevelStateMultiplayerVehicleAbilities::SendAbilityFiringMessage(const UI::E_MultiplayerVehicleAbilities type,  const U8 vehicleIndex)
{
	// Tell the challenge component about this
	Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
	if(pLogicActor && pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
	{
		C_VehicleLogicMsg_ModActivated modActivatedMessage(vehicleIndex,type);
		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &modActivatedMessage);
		UpdateModStats(type, pLogicActor);
	}
}

void C_LevelStateMultiplayerVehicleAbilities::UpdateModStats(const UI::E_MultiplayerVehicleAbilities type,  Game::Actors::C_VehicleLogicActor* pLogicActor )
{
	if(pLogicActor)
	{
		VehicleRaceStatistics& statistics = pLogicActor->mStatistics;
		switch(type)
		{
		case UI::E_MultiplayerVehicleAbilities_Iron_Fist:
			{
				statistics.mPlayersHitWithIronFist++;
			}
			break;
		case UI::E_MultiplayerVehicleAbilities_Stable_Frame:
			{
				statistics.mTimesProtectedByStableFrame++;
			}
			break;
		case UI::E_MultiplayerVehicleAbilities_Scrambler:
			{
				statistics.mPlayersScrambled++;
			}
			break;
		case UI::E_MultiplayerVehicleAbilities_Battering_Ram:
			{
				statistics.mCarsBatteringRammed++;
			}
			break;

			default:
			break;
		}
	}
}

// Has the specified ability been triggered by an in-game event?
bool C_LevelStateMultiplayerVehicleAbilities::GetAbilityFiring(const UI::E_MultiplayerVehicleAbilities type) const
{
	return !!( mActivationBitset & ( (U64)1 << type ) );
}

bool C_LevelStateMultiplayerVehicleAbilities::ActivateAbilityFiringIcon(const UI::E_MultiplayerVehicleAbilities type, bool activate)
{
	C_MultiplayerVehicleAbility* Ability = GetAbility(type);

	if(Ability->GetActive())
	{
		//Turn off display in hud
		SetAbilityFiring(type, activate);

		return true;
	}

	return false;
}
bool C_LevelStateMultiplayerVehicleAbilities::IsAbilityIconInstantOff(const UI::E_MultiplayerVehicleAbilities type)
{
	//add your mod into here if you wish the icon to turn off immediately when you SetAbilityFiring to false
	return ( type == UI::E_MultiplayerVehicleAbilities_Laser_Sight );
}

void C_LevelStateMultiplayerVehicleAbilities::SwitchOnFiringIcon(U8 vehicleIndex, const UI::E_MultiplayerVehicleAbilities type)
{
	mActivationBitset |= (U64)1 << type;

	// Tell the challenges component about this
	SendAbilityFiringMessage(type, vehicleIndex);

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	//Record a fired ability for data purposes
	for(U8 i = 0; i < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; i++)
	{
		if(levelState.mCars[0].mLoadOuts.Size() > i && levelState.mCars[0].mLoadOuts[i] == type)
		{
			mActivationTimes[i]++;
		}
	}
}

void C_LevelStateMultiplayerVehicleAbilities::SetPickupSlotGiftedInto(const int pickupSlot)
{
	mPickupSlotBitset |= 1 << pickupSlot;
}

bool C_LevelStateMultiplayerVehicleAbilities::HasPickupSlotBeenGiftedInto(const int pickupSlot)
{
	return !!( mPickupSlotBitset & ( 1 << pickupSlot ) );
}

void C_LevelStateMultiplayerVehicleAbilities::Reset()
{
	mBatteringRamTimeFired = 0.0f;
	if(m_LaserSightSpecialFX)
	{
		m_LaserSightSpecialFX->Stop();
		SAFE_DELETE_PTR(m_LaserSightSpecialFX)
	}
	if(m_LastGaspCountdownSpecialFX)
	{
		m_LastGaspCountdownSpecialFX->Stop();
		SAFE_DELETE_PTR(m_LastGaspCountdownSpecialFX)
	}
	if(m_ScramblerSpecialFX)
	{
		m_ScramblerSpecialFX = NULL;

		mScrambleCount = 0;
	}
}

// This is called every frame so we do not need to worry about resetting the bits elsewhere on
// abilities that go on and then instantly off again
void C_LevelStateMultiplayerVehicleAbilities::ResetInstantAbilityFiringBits()
{
	mActivationBitset &= ~(
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Iron_Fist ) + 
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Jump_The_Gun ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Front_Runner ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Drifter ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Titanium_Armour ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Showy_Flourish ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Stable_Frame ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Battering_Ram ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Decoy_Drop ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Scrambler ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Splash_Damage ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Safety_Net ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Shielded_Bay ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Vampiric_Wreck ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Bribe ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Fan_Favourite ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Mastermine ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Shielded_Boosters ) +
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Road_Sweeper ) + 
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Boss_FansNitroGift ) + 
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Boss_NitroRift ) + 
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Boss_RepairBonus ) + 
							( (U64)1 << UI::E_MultiplayerVehicleAbilities_Boss_ShuntDamage ) 
						);

	//Conveniently, since all pickup-gifting mods are contained here, we may also reset the "gifted slots"
	mPickupSlotBitset = 0;
}

const U16 C_LevelStateMultiplayerVehicleAbilities::GetActivationTimes(UI::E_MULTIPLAYER_LOADOUT_SLOT slot) const
{
	if(slot < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX)
	{
		return mActivationTimes[slot];
	}

	return -1;
}

///////////////////////////////
// MultiplayerVehicleAbility //
///////////////////////////////

C_MultiplayerVehicleAbility::C_MultiplayerVehicleAbility():mType(UI::E_MultiplayerVehicleAbilities_None)
{
	Clear(); 
}

void C_MultiplayerVehicleAbility::Clear()
{
	 mValue = 0.0f; 
	 mActive = false;
}

void C_MultiplayerVehicleAbility::SetActive(bool val)
{
	mActive = val;
}

bool C_MultiplayerVehicleAbility::GetActive() const
{
	return mActive;
}

void C_MultiplayerVehicleAbility::SetValue(float val)
{
	mValue = val;
}

float C_MultiplayerVehicleAbility::GetValue() const
{
	return mValue;
}

UI::E_MultiplayerVehicleAbilities C_MultiplayerVehicleAbility::GetType() const
{
	return mType;
}

void C_MultiplayerVehicleAbility::SetType(UI::E_MultiplayerVehicleAbilities type)
{
	mType = type;
}

}// end namespace Game
