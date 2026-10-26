#include "stdafx.h"
#include "VLC_Damage.h"

#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <shared_actors/SharedActorUtils.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <GameLogic/AchievementsManager.h>
#include <GameLogic/ActorStudioActors/DynamicObjectActor.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

#include <GameLogic\MultiplayerUtilities.h>

#include <GameLogic/pickups/PickupsManager.h>
#include <GameLogic/StickerManager.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic\Profile\GameProfile.h>
#include <GameLogic\Profile\GameProfileManager.h>

#include "Ui/SoundEffects.h"
#include "Ui/UiHelpers.h"
#include <ui/Game/Gadgets/ButtonMessages.h>

#include "Automation\GameDataLogging.h"

#include "GameAudio/GameAudioLayer.h"
#include "GameAudio\SystemSubsystem.h"
#include "game/AmaxCam/VehicleFeed.h"
#include "Game/AmaxCam/CameraSystem.h"

//#pragma optimize("", off)					// TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP


extern Debug::Logger &g_MultiplayerLog;
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;

Debug::Logger &g_DamageLogger( Debug::GetLogger("Amax.VLC.Damage") );
Debug::Bool  g_debugDisableGameplayDamage("Vehicle.Components.DisableVehicleGameplayDamage", false);
Debug::Bool  g_debugFatallyDamageCar("Vehicle.Components.Damage.FatallyDamage", false);
Debug::Float g_debugFrontalCollisionDot("Vehicle.Components.Damage.FrontalCollisionAngle", 0.85f, Debug::Persist());
Debug::Float g_debugFrontalCollisionUprightDot("Vehicle.Components.Damage.FrontalCollisionUprightDot", 0.707f, Debug::Persist());
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;
extern Debug::Logger &g_MultiplayerLog;
extern Debug::Logger &g_VehicleActorEffectsLog;

Debug::Bool  g_AllowDamageMultiplier("Vehicle.Components.Damage.AllowDamageMultiplier", true);
Debug::Bool  g_BitchAboutInvalidHealthPointMultipliers("Vehicle.Components.Damage.BitchAboutInvalidHealthPointMultipliers", true);

#ifndef CFG_GOLD
	Debug::Bool g_debugFakeFatalDamage				("Vehicle.Damage.Fatal.FakeFatalDamage", false);
	Debug::Int g_debugVehicleToFatallyDamage		("Vehicle.Damage.Fatal.VehicleToFatallyDamage", 0, Debug::Range(0, 19), Debug::Step(1));
	Debug::Bool g_debugFrontImpactTriggerEffect		("Vehicle.Effects.Impact.TriggerFrontEffect",		false);
	Debug::Bool g_debugSideImpactTriggerEffect		("Vehicle.Effects.Impact.TriggerSideEffect",		false);
	Debug::Bool g_debugRollImpactTriggerEffect		("Vehicle.Effects.Impact.TriggerRollEffect",		false);
	Debug::Float g_debugImpactImpulseValue			("Vehicle.Effects.Impact.ImpulseValue",		1.0f, Debug::Range(0.0f, 1.0f));

#endif

#ifndef CFG_GOLD
	Debug::Float g_LoggingMinimumImpulse			("System.DataLogging.MinimumCollisionImpulseToLog", 0.1f, Debug::Range(0.0f, 0.5f), Debug::Step(0.01f), Debug::Persist());
#endif

extern Debug::Logger &g_MultiplayerLog;
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;

Debug::Bool g_BatteringRamHeavyHit("Multiplayer.Upgrades.BatteringRam.HeavyHit", false);
Debug::Bool g_BatteringRamApplyFakePhysics("Multiplayer.Upgrades.BatteringRam.ApplyFakePhysics", false);
Debug::Int g_BatteringRamApplyFakePhysicsToVehicle("Multiplayer.Upgrades.BatteringRam.ApplyFakePhysicsToVehicle", 0, Debug::Range(0,19));

Debug::Bool g_BatteringRamHairTrigger("Multiplayer.Upgrades.BatteringRam.HairTrigger", false);
Debug::Bool g_BatteringRamReCacheConsequences("Multiplayer.Upgrades.BatteringRam.ReCacheConsequences", false);

DEBUG_ENUM(E_TARGETTING_DIRECTION)
	DE_OPTION(E_TARGETTING_DIRECTION_AHEAD,						"Ahead")
	DE_OPTION(E_TARGETTING_DIRECTION_BEHIND,					"Behind")
	DE_OPTION(E_TARGETTING_DIRECTION_LEFT,						"Left")
	DE_OPTION(E_TARGETTING_DIRECTION_RIGHT,						"Right")
	DE_OPTION(E_TARGETTING_DIRECTION_NONE,						"None")
END_DEBUG_ENUM
Debug::Enum<E_TARGETTING_DIRECTION> g_BatteringRamTargettingDirection("Multiplayer.Upgrades.BatteringRam.TargettingDirection", E_TARGETTING_DIRECTION_LEFT);

extern Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkImpulseScale;
extern Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkKickForce;
extern Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkConstantKickForce;

extern Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkImpulseScale;
extern Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkKickForce;
extern Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkConstantKickForce;

namespace
{
	float MphToMps(float mph) { return mph * (1609.0f / 3600.0f); }
	float MpsToMph(float mps) { return mps * (3600.0f / 1609.0f); }
}

namespace Game
{
	C_CachedMPAbility::C_CachedMPAbility(	Game::C_LevelStateMultiplayerVehicleAbilities* p_pAbilities, UI::E_MultiplayerVehicleAbilities p_Type, 
											bool p_IsFiring, bool p_NotLocal, U8 p_VehicleIndex, U8 p_VictimIndex)
	{
		m_pAbilities	= p_pAbilities;
		m_Type			= p_Type;
		m_IsFiring		= p_IsFiring;
		m_NotLocal		= p_NotLocal;
		m_VehicleIndex	= p_VehicleIndex;
		m_VictimIndex	= p_VictimIndex;
	}

	void C_CachedMPAbility::SetFiring()
	{
		m_pAbilities->SetAbilityFiring(m_Type, m_IsFiring, m_NotLocal, m_VehicleIndex, m_VictimIndex);
	}
}

namespace Components
{
	bool C_VehicleComponent_Damage::CreateImpl()
	{
		const I_VehicleComponent_Damage_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Damage_CreateInfo *>(GetCreator().GetData());

		if(creation_data)
		{
			mActorHandle=creation_data->mParentActorHandle;
			mVehicleIndex=creation_data->mVehicleIndex;
		}

		//AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( E_VehicleLogicMsg_ForceDamage );
		AddSubscriber( E_VehicleLogicMsg_Repair );
		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
		AddSubscriber( Game::E_GameLogicMessage_Impact );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_SetVehicleLogic );
		AddSubscriber( Game::E_VehicleMessage_CancelWreckedTimer );
		AddSubscriber( E_VehicleLogicMsg_Wrecked );
		AddSubscriber( E_VehicleLogicMsg_HOGReset );

		Initialise();

		return true;
	}

	void C_VehicleComponent_Damage::Initialise()
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::C_GameProfile* prof					= Game::C_GameProfileManager::Instance().GetPrimaryProfile();

		mpRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		mpRLC				= pActor->GetRLC();
		mpLevelState		= &Game::C_Game::GetInstance()->GetLevelState();
		BIZ_ASSERT(mpRaceLogicLayer);
		BIZ_ASSERT(mpRLC);
		BIZ_ASSERT(mpLevelState);

		const RLDamageSystem* pRLDamageSystem		= mpRLC->GetRLSystem()->mDamageSystem;

		mEnabled = false;
		mPrevLinearVelocity = Maths::Constants<Maths::HVec3>::Zero();
		
		mBlindFireChainLength = 0; 
		mBlindFireChainTimer = 0.0f;
		mTotalBlindFireImpacts = 0;
		mCooldown = 0.f;
		mBatteringRamCoolDownTimer = 0.f;
		mResetInProgress = false;
		mNumBlindFireRequiredToCauseScrambler = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings.m_ScramblerNumBoltRequiredInChain;

		MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
		BIZ_ASSERT_MSG(pConfig," C_VehicleComponent_Damage::Initialise() - error pConfig invalid");
		mBlindFireChainTimerIncrement = pConfig->m_MiscModSettings.m_ScramblerModBoltChainTimerIncrement;

		mpMultiplayerAbilities = mpLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
		BIZ_ASSERT_MSG(mpMultiplayerAbilities,"Error.. VLC_Damage CreateImpl() unable to obtain pointer to Multiplayer Abilities");

		mScramblerPowerupBayDisableTime = pConfig->m_MiscModSettings.m_ScramblerPowerupBayDisableTime;
		mScramblerModBoltsStack = pConfig->m_MiscModSettings.m_ScramblerBoltsStack;
		mScramblerBoltDisableTime = pConfig->m_MiscModSettings.m_ScramblerBoltDisableTime;

		// Init last gasp data
		mLastGaspModData.Initialise(mpLevelState, mVehicleIndex); 
		mLastDamageWasInflictedLastGasp = false;

		//Init battering ram data
		mBatteringRamModData.Initialise(pConfig);
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::Update()
	{
		PERF_EVENT("C_VehicleComponent.Damage.Update");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* vehActor		= pActor->GetGraphicalActor(mVehicleIndex);
		float frameTime								= C_GameApp::GetInstance()->GetTimeStep();

		//[CWright] Bug: #37896, #38078, #34610 - process mp ability SFX that would previously have been triggered on a different thread
		for (S32 iAbility = 0; iAbility < m_CachedAbilities.Size(); iAbility++)
			m_CachedAbilities[iAbility].SetFiring();
		m_CachedAbilities.Clear();

		// Update time since last damaged
		mLastGaspModData.mTimeSinceLastDamaged += frameTime; 

		if(pActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED)
		{
			mWreckedTimer -= frameTime;

			if(mWreckedTimer <= 0.f)
			{
				// if reset process already initiated.. then already in middle of reset.. so dont send message to reset again.
				if(!mResetInProgress)
				{
					mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Scrambler, false);

					if (pActor->mRemoveWhenWrecked)
					{
						pActor->RemoveFromRace();
					}
					else if(mpLevelState->mWreckedBehaviour == Game::E_AMAXWRECKED_RESPAWN)
					{
						Game::Actors::C_VehicleActor* pVehActor = pActor->GetGraphicalActor(mVehicleIndex);
						Game::Actors::C_VehicleControlsEnableMsg enableControlsMsg(pVehActor->GetHandle(), pActor->GetVehicleIndex());
						pVehActor->SendMessageToActor(pVehActor->GetHandle(), &enableControlsMsg);


						//Now reset the vehicle
						Game::Actors::C_VehicleResetAfterWreckedMsg resetAfterWreckedMsg(mVehicleIndex);

						pActor->SendMessageToActor(pActor->GetHandle(), &resetAfterWreckedMsg);
						// Tell the UI
						_UI_MESSAGE1( Game::E_GameflowMessage_Respawn, UI::GetLocalPlayerIndex(pActor->GetVehicleIndex()) );
					}
					else if(mpLevelState->mWreckedBehaviour == Game::E_AMAXWRECKED_ELIMINATED)
					{
						Game::Actors::C_VehicleEliminationMessage elimMsg(pActor->GetHandle(), mVehicleIndex);
						pActor->SendMessageToActor(pActor->GetHandle(), &elimMsg);
						Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&elimMsg);
					}

					if( mpLevelState->mWreckedBehaviour !=  Game::E_AMAXWRECKED_DO_NOTHING)
					{
						// mark as reset process initiated
						mResetInProgress = true;
					}

					// ALWAYS turn off the last gasp icon when 'respawn behaviour' has finished (wether wrecked, eliminated, evaporated, disintegrated, whatever)
					if(mLastGaspModData.mHasLastGaspModEquipped)
					{
						Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = mpLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
						Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);
						pAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Last_Gasp, false, !Game::VehicleIsLocal(mVehicleIndex), mVehicleIndex);
						// Reset the last gasp timers so they don't force 
						mLastGaspModData.Initialise(mpLevelState, mVehicleIndex); 
					}
				}
			}
		}

		if(mCooldown > 0.f)
			mCooldown -= frameTime;

		// Cache the linear velocity as we need to 
		// use this when we receive and impact message
		mPrevLinearVelocity = vehActor->GetCollisionBody()->LinearVelocity();

#ifndef CFG_GOLD
		if (vehActor->IsFocusVehicle())
		{			
			const RLImpactType* pImpactType = mpRaceLogicLayer->CalculateImpactType(g_debugImpactImpulseValue, false);
			if(pImpactType)
			{
				if (g_debugFrontImpactTriggerEffect)
				{
					g_debugFrontImpactTriggerEffect = false;
					vehActor->SetImpactEffect(pImpactType->mFrontScreenEffect, Maths::Constants<Maths::HMat33>::Identity());
				}
				if (g_debugSideImpactTriggerEffect)
				{
					g_debugSideImpactTriggerEffect = false;
					vehActor->SetImpactEffect(pImpactType->mSideScreenEffect, Maths::Constants<Maths::HMat33>::Identity());
				}
				if (g_debugRollImpactTriggerEffect)
				{
					g_debugRollImpactTriggerEffect = false;
					vehActor->SetImpactEffect(pImpactType->mRollScreenEffect, Maths::Constants<Maths::HMat33>::Identity());
				}
			}
		}	
#endif 

		// If vehicle is currently recording being hit by a chain of blindFires, then decrement the chain timer
		if(mBlindFireChainLength > 0)
		{
			if(mBlindFireChainTimer <= 0.0f)
			{
				// Out of time.. reset!
				mBlindFireChainLength = 0; 
				mBlindFireChainTimer = 0.0f; 
			}
			else
			{
				// Decrement the timer....
				mBlindFireChainTimer -= C_GameApp::GetInstance()->GetTimeStep();
			}
		}

		// Update timers (and possibly handle a delayed respawn / force fire if required)
		bool respawn = false;
		bool forceFireLastGasp = false;

		mLastGaspModData.UpdateTimers( respawn, forceFireLastGasp, mVehicleIndex );

		if( respawn )
		{
			HandleDelayedWreckRespawn();
		}
		else if( forceFireLastGasp )
		{
			FireLastGasp(); 
		}

		if(mBatteringRamCoolDownTimer > 0.f)
		{
			mBatteringRamCoolDownTimer -= frameTime;
		}

#ifndef CFG_GOLD
		UpdateDebugFunctionality();
#endif
	}

#ifndef CFG_GOLD
	void C_VehicleComponent_Damage::UpdateDebugFunctionality()
	{
		MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
		if(g_BatteringRamApplyFakePhysics && g_BatteringRamApplyFakePhysicsToVehicle == mVehicleIndex)
		{
			StartBatteringRamPhysicsEffect( ((Game::Actors::C_VehicleLogicActor *)GetActor())->GetGraphicalActor(mVehicleIndex), 
											(enum E_TARGETTING_DIRECTION)g_BatteringRamTargettingDirection,
											g_BatteringRamHeavyHit);
			g_BatteringRamApplyFakePhysics = false;
		}

		if(g_BatteringRamReCacheConsequences)
		{
			mBatteringRamModData.Initialise(pConfig);

			g_BatteringRamReCacheConsequences = false;
		}

	}
#endif
	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::HandleDelayedWreckRespawn()
	{
		// if so, lets get on with the show!
		mLastGaspModData.mWreckedSequenceDelayTimer = 0.0f;
		mLastGaspModData.mWreckedSequenceDelayed = false;
		Game::Actors::C_VehicleLogicActor* pLogicActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if(pLogicActor)
		{
			// Before we wreck the player, lets remove any camera manipulation (e.g. forcing chase cam, locking camera choices). 
			RemoveLastGaspCameraManipulation(); 

			pLogicActor->mScoringInfo.mMostRecentVehicleIdxHitBy = mLastGaspModData.mVehicleCausingLastGaspID;
			LastGaspModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_LastGaspModSettings;
			pLogicActor->mScoringInfo.mWreckedTimer = 0.1f + modSettings.m_wreckedSequenceDelay; //needs to still be classed as a wreck , when wrecked sequence transitioned to.
			Components::C_VehicleComponent_NewScoring* pScoring = (Components::C_VehicleComponent_NewScoring*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring);
			U8 vehIdx = mLastGaspModData.mVehicleCausingLastGaspID;
			if(pScoring && vehIdx != -1 && vehIdx != mVehicleIndex && vehIdx != INVALID_VEHICLE_IDX)
			{
				pScoring->ResetHitByPerkRecentlyTimer(vehIdx);
			}

			Game::Actors::C_VehicleFatallyDamagedMsg vehicleFatalDamageMsg(pLogicActor->GetHandle(), mVehicleIndex);
			pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), &vehicleFatalDamageMsg);
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&vehicleFatalDamageMsg);

			_UI_MESSAGE1( Game::E_GameflowMessage_Wrecked, UI::GetLocalPlayerIndex(pLogicActor->GetVehicleIndex()) );

			// Make sure the correct wrecked screen effect is now shown again (e.g  broken glass ) if it was delayed
			bool delayWreckedBrokenGlassScreenEffect = modSettings.m_delayWreckedBrokenGlassScreenEffect;
			if( delayWreckedBrokenGlassScreenEffect )
			{
				// If it was delayed then we may need to switch it on
				// But only if we dont also have the JTG mod equipped..
				if(!mLastGaspModData.mHasJumpTheGunModEquipped)
				{
					SetCurrentHealthLevel(false,true);
				}
			}
		}
	}


	void C_VehicleComponent_Damage::FireLastGasp(bool damagePlayer)
	{
		Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
		Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* pLastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);
		if(pLastGaspAbility && pLastGaspAbility->GetActive())
		{

			// If we are still in the delay period where firing is disabled... stop now!
			if(mLastGaspModData.mInitialDisableTimer > 0.0f)
			{
				return;
			}

			// Since we are now firing last gasp.. lets stop counting down for a force fire.
			mLastGaspModData.mRemainingTimeTillForceFire = 0.0f;
			mLastGaspModData.mExplosionFiredThisLife = true;
			mLastGaspModData.mNextSoundEffectIn		 = 0.0f;

			// Flag as not in last gasp mode any longer
			Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
			if(vehActor)
			{
				vehActor->SetIsInLastGaspMode(false);
			}

			// Whatever the reason it was triggerd... the UI should know about this
			_UI_MESSAGE( UI::E_UiGameMsg_DisarmLastGasp );

			// Wreck player and fire last gasp
			const enum E_PICKUP_TYPE ePickupType = (const enum E_PICKUP_TYPE)((int)pLastGaspAbility->GetValue());
			Game::PickupCreationStruct pickupInfo(ePickupType, E_PICKUP_SUB_TYPE_MOD, mVehicleIndex);
			Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
			
			Game::Actors::C_VehicleLogicActor* pLogicActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
			Game::Actors::C_VehicleActor* pVehicleActor =  (Game::Actors::C_VehicleActor*)pLogicActor->GetGraphicalActor(mVehicleIndex);
			if(pVehicleActor)
			{
				// Apply the impulse (using any last gasp multiplier)
				LastGaspModSettings& settings = C_MultiplayerData::Instance().m_pConfig->m_LastGaspModSettings;
				float lastGaspWreckedImpulseScale = settings.m_wreckImpulseGlobalScale;
				pVehicleActor->ApplyWreckedImpulse(lastGaspWreckedImpulseScale);

				// Lets record the fact we have taken responsibility for applying the wrecked impulse for last gasp.. so nobody else tries to!
				mLastGaspModData.mHasAppliedWreckedImpulse = true;
			}

			// Before trying to kill ourselves.. lets make sure we can take damage...
			mLastGaspModData.mDamageCooldownTimer = 0.0f; 

			// Let the player see the carnage they have caused
			mLastGaspModData.mWreckedSequenceDelayed = true;
			mLastGaspModData.mWreckedSequenceDelayTimer = mLastGaspModData.mWreckedSequenceDelay;

			// Now kill self BUT we may want the last person who damaged us to get 'wrecked' points
			if(pLogicActor)
			{
				U8 vehIdx = mLastGaspModData.mVehicleCausingLastGaspID;
				if(vehIdx != -1 && vehIdx != INVALID_VEHICLE_IDX && vehIdx != mVehicleIndex)
				{
					// We need to award the player that wrecked us appropriately.. this involves some horribleness to get around checks
					pLogicActor->mScoringInfo.mMostRecentVehicleIdxHitBy = vehIdx;
					LastGaspModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_LastGaspModSettings;
					pLogicActor->mScoringInfo.mWreckedTimer = 0.1f + modSettings.m_wreckedSequenceDelay; //needs to still be classed as a wreck , when wrecked sequence transitioned to.
					Components::C_VehicleComponent_NewScoring* pScoring = (Components::C_VehicleComponent_NewScoring*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring);
					if(pScoring)
					{
						pScoring->ResetHitByPerkRecentlyTimer(vehIdx);
					}

					if(damagePlayer)
					{
						// Have some points you crafty player you..
						ApplyDamage(pLogicActor->GetMaxHealth(), vehIdx);
					}
				}
				else
				{
					if(damagePlayer)
					{
						// We did this ourselves!
						ApplyDamage(pLogicActor->GetMaxHealth(), mVehicleIndex);
					}
				}
			}

			// We are done with any visual fx... turn them off!
			if(mpMultiplayerAbilities)
			{
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Last_Gasp, false);
			}
		}
	}

	bool C_VehicleComponent_Damage::GetHasLastGaspAppliedWreckedImpulse() const
	{
		return mLastGaspModData.mHasAppliedWreckedImpulse; 
	}

	void C_VehicleComponent_Damage::AdjustHealth(int healthPoints)
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		// Bail out if we are wrecked and trying to apply more damage.
		if(pActor->mCurrentHealthLevel == E_HEALTH_LEVEL_WRECKED && healthPoints <= 0)
			return;

		int previousHealth = pActor->mCurrentHealth;
		pActor->mCurrentHealth += healthPoints;

		// Did we take damage?
		if(pActor->mCurrentHealth < previousHealth)
		{
			UpdateDamageStats(); 
		}

		if(healthPoints != 0 && pActor->mCurrentHealth < 0)
		{
			pActor->mCurrentHealth = 0;
		}
		else if( pActor->mCurrentHealth >= pActor->GetMaxHealth() )
		{
			pActor->mCurrentHealth = pActor->GetMaxHealth() - 1;
		}

		E_HEALTH_LEVEL prevHealthlevel = pActor->mCurrentHealthLevel;

		// If we are delaying the wrecked sequence due to last gasp..
		if(pActor->mCurrentHealth <= 0 &&
		   mLastGaspModData.mWreckedSequenceDelayed )
		{
			// we may update screen effects (broken glass etc) manually later
			LastGaspModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_LastGaspModSettings;
			bool delayWreckedBrokenGlassScreenEffect = modSettings.m_delayWreckedBrokenGlassScreenEffect;
			SetCurrentHealthLevel(delayWreckedBrokenGlassScreenEffect,false);
		}
		else
		{
			// Else update as normal
			SetCurrentHealthLevel();
		}

		if(pActor->mCurrentHealthLevel == E_HEALTH_LEVEL_WRECKED)
		{
#ifndef CFG_GOLD
			AmaxDataLogging::LogWrecked(mVehicleIndex, pActor->GetPositionInfo().mWorldPosition, pActor->GetRLC()->GetRaceTime(), pActor->mScoringInfo.mMostRecentVehicleIdxHitBy);
#endif
			// If we have the last gasp mod, activate it to give an extra lease of life (around half a pip!)
			if(mLastGaspModData.mHasLastGaspModEquipped && !mLastGaspModData.mHealthReplenishedThisLife)
			{		
				ActivateLastGasp();
			}	
			else
			{
				// Standard Wrecked logic...

				Game::Actors::C_VehicleLogicActor* pLogicActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
				if(mLastGaspModData.mHasLastGaspModEquipped)
				{
					// Now we are wrecked.. 
					
					// Fire off last gasp (force it) if didnt fire it off in time limit. 
					if(!mLastGaspModData.mExplosionFiredThisLife)
					{
						// Since we have just been wrecked... make sure we update our records of who hit us last
						// Store index of player (if any.. could be -1) that damaged us enough to enter last gasp mode. They should get a wreck when we detonate!
						if(pActor && pActor->mScoringInfo.mWreckedTimer > 0.f && pActor->mScoringInfo.mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX)
						{
							mLastGaspModData.mVehicleCausingLastGaspID =  pActor->mScoringInfo.mMostRecentVehicleIdxHitBy;
						}
						else
						{
							mLastGaspModData.mVehicleCausingLastGaspID = -1; // We must have killed ourselves *facepalm*
						}

						// Now fire it...

						// However... as we are already wrecked.. we dont need to apply damage to ourselves...
						// And we can let the normal wrecked logic take over for asssigning points to our wrecker! 
						FireLastGasp(false); 

						// Make sure broke glass effect hidden
						// we may update screen effects (broken glass etc) manually later
						LastGaspModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_LastGaspModSettings;
						bool delayWreckedBrokenGlassScreenEffect = modSettings.m_delayWreckedBrokenGlassScreenEffect;
						SetCurrentHealthLevel(delayWreckedBrokenGlassScreenEffect,false);
						
					}

					// reactivate next last gasp saviour 'health pip' 
					mLastGaspModData.mHealthReplenishedThisLife = false;

					// Re-enable powerup slots 
					if(pLogicActor)
					{
						C_VehicleLogicMsg_EnableDisablePickupSystem disablePickupsMsg(mVehicleIndex,0.0f, 0.0f, false);
						pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &disablePickupsMsg);
					}
				}

				Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = mpLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
				Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);

				Game::Actors::C_VehicleActor* pVehActor = pActor->GetGraphicalActor(mVehicleIndex);
				Game::Actors::C_VehicleControlsDisableMsg disableControlsMsg(pActor->GetVehicleIndex());
				pVehActor->SendMessageToActor(pVehActor->GetHandle(), &disableControlsMsg);

				// Get the wrecked sequence duration taking into account any mods that affect it. 
				mWreckedTimer = pLogicActor->GetWreckedSequenceDuration();

				// Transition to wrecked sequence.. unless last gasp has caused it to be delayed..
				if(!mLastGaspModData.mWreckedSequenceDelayed)
				{
					// If we fired last gasp.. then the camera settings will have been played with.. restore original
					if( mLastGaspModData.mExplosionFiredThisLife )
					{
						RemoveLastGaspCameraManipulation();
					}

					// Tell the UI
					_UI_MESSAGE1( Game::E_GameflowMessage_Wrecked, UI::GetLocalPlayerIndex(pActor->GetVehicleIndex()) );
		
					Game::Actors::C_VehicleFatallyDamagedMsg vehicleFatalDamageMsg(pActor->GetHandle(), mVehicleIndex);
					pActor->SendMessageToActor(pActor->GetHandle(), &vehicleFatalDamageMsg);
					Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&vehicleFatalDamageMsg);


					if (mVehicleIndex != 0)					// If in destruction and not the player then this car might fire off a last gasp mine/shunt
					{
						Game::C_RaceModeBase* raceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();
						if (raceMode && ((raceMode->GetID() == Game::E_RaceMode_Destruction) || (raceMode->GetID() == Game::E_RaceMode_FriendDemand_Destruction)))
						{
							SP_DestructionEvent* destruct_event = (SP_DestructionEvent*)(SP_GetEventById( SP_GetCurrentSaveGame(), mpLevelState->mSpEventId ));
							AmaxRaceModeDestruction* destruct_data = (AmaxRaceModeDestruction*)destruct_event->event->raceMode;


							AmaxRaceModeDestructionVars* dvars;
							switch(mpLevelState->m_difficulty)
							{	
								case Game::GAME_DIFFICULTY_VERY_EASY:
								case Game::GAME_DIFFICULTY_EASY:
									dvars = &destruct_data->easy;
									break;

								case Game::GAME_DIFFICULTY_NORMAL:
									dvars = &destruct_data->medium;
									break;

								default:
									dvars = &destruct_data->hard;
									break;
							}


							U32 vehicleID = pVehActor->GetXtIndex();
							if (vehicleID == destruct_data->vehicleEasy->vehicle->ID)			// Easy - Green - Bolt
							{
								if (dvars->EasyCarsUsePerks)
								{
									Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(mVehicleIndex);
									pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_BLINDFIRE;
									pickupFireMsg.mBackwards = true;
									pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &pickupFireMsg);
								}
							}
							else if (vehicleID == destruct_data->vehicleMedium->vehicle->ID)		// Medium - Yellow - Mine
							{
								if (dvars->MediumCarsUsePerks)
								{
									RLGameMode_Destruction& destructionParams = RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction;

									Game::PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BOMB, E_PICKUP_SUB_TYPE_NORMAL, mVehicleIndex);
									pickupInfo.mDelay	= 0.f;
									pickupInfo.mVal2	= destructionParams.mBombLifetime;
									Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
								}
							}
							else if (vehicleID == destruct_data->vehicleHard->vehicle->ID)		// Hard - Red - Shunt
							{
								if (dvars->HardCarsUsePerks)
								{
									Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(mVehicleIndex);
									pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_SHUNT;
									pickupFireMsg.mBackwards = true;
									pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &pickupFireMsg);	
								}
							}
							else if (vehicleID == destruct_data->vehicleVHard->vehicle->ID)		// Very Hard - Blue - Barge
							{
								if (dvars->VHardCarsUsePerks)
								{
									Game::PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BARGE, E_PICKUP_SUB_TYPE_NORMAL, mVehicleIndex);
									pickupInfo.mDelay = 0.f;
									Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
									
									//Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(mVehicleIndex);
									//pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_BARGE;
									//pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &pickupFireMsg);	
								}
							}
						}
					}
				}
				else
				{
					// Delay the wrecked sequence, and ghost a bit longer
					mWreckedTimer += mLastGaspModData.mWreckedSequenceDelay;
				}

				// Ghost until the after wrecked sequence..
				pActor->Ghost(mWreckedTimer + 2);	//sorry, there is a 2 second gap between the wrecked and the HoG, need to get SteveB to fix this up on Monday

				if( mpLevelState->mWreckedBehaviour != Game::E_AMAXWRECKED_RESPAWN)
					return;

				CheckJumpTheGun();
			}
		}
#ifndef CFG_GOLD
		if(previousHealth != pActor->mCurrentHealth)
			AmaxDataLogging::LogHealthChanged(mVehicleIndex, previousHealth, pActor->mCurrentHealth, pActor->mPositionInfo.mWorldPosition, pActor->GetRLC()->GetRaceTime());
#endif
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::UpdateDamageStats()
	{
		// Update time since last damaged
		Game::Actors::C_VehicleLogicActor* pLogicActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if(pLogicActor)
		{
			// If time since last damaged > previousbest, update
			if(mLastGaspModData.mTimeSinceLastDamaged > pLogicActor->mStatistics.mLongestTimeNotDamaged)
			{
				pLogicActor->mStatistics.mLongestTimeNotDamaged = mLastGaspModData.mTimeSinceLastDamaged;
			}

		}

		// Reset time since last damaged
		mLastGaspModData.mTimeSinceLastDamaged = 0.0f;
	}

	// -------------------------------------------------------------------------------------------

	//	Instructs another vehicle that it has dealt some damage
	void C_VehicleComponent_Damage::VictimDamaged(int damage, int aggressiveVehicleIdx )
	{
		if(aggressiveVehicleIdx == Game::SYSTEM_FIRING_INDEX)
			return;

		Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
		pLogicActor->mStatistics.mDualVehicleStats[aggressiveVehicleIdx].mNumTimesHitBy++;

		if(Game::VehicleIsLocal(aggressiveVehicleIdx))
		{
			Game::Actors::C_VehicleLogicActor* pAggressiveLogicActor = mpRaceLogicLayer->GetLogicVehicleActor(aggressiveVehicleIdx);
			pAggressiveLogicActor->mStatistics.mDamageDealt += damage;
			pAggressiveLogicActor->mStatistics.mDualVehicleStats[mVehicleIndex].mNumTimesHit++;
		}

		//	If we're playing a network game we need to send this to the aggressor (but probably let everyone have it so we can use it for cool UI stuff?)
		if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
		{
			const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
			netGameVars.m_EventManager->Add_DamageDealt(mpLevelState->mCars[aggressiveVehicleIdx].mNetJoinId, mpLevelState->mCars[mVehicleIndex].mNetJoinId, (S16)damage);
		}
	}

	void C_VehicleComponent_Damage::ApplyDamage(int damagePoints, int aggressiveVehicleIdx)
	{
		// Check if we can actually take damage..
		if(mLastGaspModData.mDamageCooldownTimer > 0.0f)
		{
			return;
		}

		//Apply a damage modifier if needed
		ApplyDamageMultiplier(damagePoints);

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if(g_debugDisableGameplayDamage || !mEnabled || mpLevelState->mDamageMode == E_DAMAGE_MODE_INVULNERABLE || !pActor->mStillInRace)
			return;

		AdjustHealth(-damagePoints);

		// Update damage in the statistics
		pActor->mStatistics.mDamageTaken += damagePoints;
		if(aggressiveVehicleIdx != INVALID_VEHICLE_IDX && aggressiveVehicleIdx != mVehicleIndex)
		{
			pActor->mStatistics.mNumTimesHit++;
			VictimDamaged(damagePoints, aggressiveVehicleIdx);
		}

		pActor->mStatistics.mTimesCrashed++;
	}

	// -------------------------------------------------------------------------------------------

	Debug::Float g_debugPickupShockHitImpulse("Perks.ShockHitDamageSize", 1.0f);
	Debug::Float g_debugPickupShuntHitImpulse("Perks.ShuntHitDamageSize", 0.5f);
	Debug::Float g_debugPickupBargeHitImpulse("Perks.BargeHitDamageSize", 0.5f);
	Debug::Float g_debugPickupBoltHitImpulse("Perks.BoltHitDamageSize", 0.25f);
	Debug::Float g_debugPickupMineHitImpulse("Perks.MineHitDamageSize", 0.5f);

	void C_VehicleComponent_Damage::HandlePickupDamage(E_PICKUP_HIT_TYPE p_PickupHitType, E_PICKUP_TYPE p_PickupType)
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* vehActor		= pActor->GetGraphicalActor(mVehicleIndex);
		
		Game::C_ImpactMsg impactMessage;

		impactMessage.mMatrix = vehActor->GetMatrix();

		impactMessage.mCollisionInfo.m_DamageImpulse = 0.0f;

		switch (p_PickupType)
		{
		case E_PICKUP_TYPE_SHOCK:
			{
				impactMessage.mCollisionInfo.m_DamageImpulse = g_debugPickupShockHitImpulse;

				impactMessage.mCollisionInfo.m_ImpactRadiusOverride = 12.0f;
				impactMessage.mCollisionInfo.m_ContactNormal = Maths::HVec3(0.0f, -1.0f, 0.0f);
				impactMessage.mCollisionInfo.m_ContactPoints.PushBack();
				impactMessage.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint = Maths::HVec3(0.0f, vehActor->GetExtents().Y(), 0.0f) * impactMessage.mMatrix;
			}
			break;
		case E_PICKUP_TYPE_BARGE:
			{
				impactMessage.mCollisionInfo.m_DamageImpulse = g_debugPickupBargeHitImpulse;

				impactMessage.mCollisionInfo.m_ContactPoints.PushBack();
				impactMessage.mCollisionInfo.m_ImpactRadiusOverride = 1.0f;

				float fDotX = pActor->mPickupHitVector | Maths::HVec3(1.0f, 0.0f, 0.0f);
				if (fDotX >= 0.5f)
					impactMessage.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint = Maths::HVec3(-vehActor->GetExtents().X(), 0.0f, 0.0f) * impactMessage.mMatrix;
				else if (fDotX <= -0.5f)
					impactMessage.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint = Maths::HVec3(vehActor->GetExtents().X(), 0.0f, 0.0f) * impactMessage.mMatrix;

				float fDotZ = pActor->mPickupHitVector | Maths::HVec3(0.0f, 0.0f, 1.0f);
				if (fDotZ >= 0.5f)
					impactMessage.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint = Maths::HVec3(0.0f, 0.0f, -vehActor->GetExtents().Z()) * impactMessage.mMatrix;					
				else if (fDotZ <= -0.5f)
					impactMessage.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint = Maths::HVec3(0.0f, 0.0f, vehActor->GetExtents().Z()) * impactMessage.mMatrix;

				impactMessage.mCollisionInfo.m_ContactNormal = pActor->mPickupHitVector * impactMessage.mMatrix;				
			}
			break;
		case E_PICKUP_TYPE_SHUNT:
		case E_PICKUP_TYPE_BLINDFIRE:		
		case E_PICKUP_TYPE_BOMB:
			{
				if (p_PickupType == E_PICKUP_TYPE_SHUNT)
					impactMessage.mCollisionInfo.m_DamageImpulse = g_debugPickupShuntHitImpulse;
				else if (p_PickupType == E_PICKUP_TYPE_BLINDFIRE)
					impactMessage.mCollisionInfo.m_DamageImpulse = g_debugPickupBoltHitImpulse;
				else if (p_PickupType == E_PICKUP_TYPE_BOMB)
					impactMessage.mCollisionInfo.m_DamageImpulse = g_debugPickupMineHitImpulse;

				impactMessage.mCollisionInfo.m_ContactPoints.PushBack();
				impactMessage.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint = pActor->mPickupHitPosition;					
				impactMessage.mCollisionInfo.m_ContactNormal = pActor->mPickupHitVector * impactMessage.mMatrix;				
			}
			break;
		default:
			return;
		}
			
		impactMessage.mCollisionInfo.m_GameplayImpulse = impactMessage.mCollisionInfo.m_DamageImpulse;

		PopulateDamageFilter(impactMessage.mDamageFilter, impactMessage.mCollisionInfo.m_GameplayImpulse);
		
		vehActor->GetActorList()->SendMessageToActor(vehActor->GetHandle(), &impactMessage);
	}

	void C_VehicleComponent_Damage::PopulateDamageFilter(Vehicle::C_DamageFilter& filter, Maths::HF32 impulse)
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		const E_IMPACT_LEVEL impactLevel			= mpRaceLogicLayer->CalculateImpactLevel(impulse, false);
		const RLHealthLevelZone* pHealthZone		= GetCurrentHealthZone();

		filter.Reset();

		if (!pHealthZone)
			return;

		filter.m_ScratchDamage = (pHealthZone->mDamageFilter.mScratchDamage.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mScratchDamage.mImpactRequired);
		filter.m_DeformParts = (pHealthZone->mDamageFilter.mDeformParts.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mDeformParts.mImpactRequired);
		filter.m_DislodgeParts = (pHealthZone->mDamageFilter.mDislodgeParts.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mDislodgeParts.mImpactRequired);
		filter.m_DetachParts = (pHealthZone->mDamageFilter.mDetachParts.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mDetachParts.mImpactRequired);
		filter.m_BurstTyres = (pHealthZone->mDamageFilter.mBurstTyres.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mBurstTyres.mImpactRequired);
		filter.m_DetachWheels = (pHealthZone->mDamageFilter.mDetachWheels.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mDetachWheels.mImpactRequired);
		filter.m_LightBreak = (pHealthZone->mDamageFilter.mLightBreak.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mLightBreak.mImpactRequired);
		filter.m_LightKnockOut = (pHealthZone->mDamageFilter.mLightKnockOut.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mLightKnockOut.mImpactRequired);
		filter.m_RadiatorSmoke = (pHealthZone->mDamageFilter.mRadiatorSmoke.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mRadiatorSmoke.mImpactRequired);
		filter.m_EngineSmokeLight = (pHealthZone->mDamageFilter.mEngineSmokeLight.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mEngineSmokeLight.mImpactRequired);
		filter.m_EngineSmokeHeavy = (pHealthZone->mDamageFilter.mEngineSmokeHeavy.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mEngineSmokeHeavy.mImpactRequired);
		filter.m_EngineFire = (pHealthZone->mDamageFilter.mEngineFire.mAllowable && impactLevel >= pHealthZone->mDamageFilter.mEngineFire.mImpactRequired);
		filter.m_FatalDamage = (pActor->mCurrentHealth <= 0);
	}

	// -------------------------------------------------------------------------------------------

	const RLHealthLevelZone* C_VehicleComponent_Damage::GetCurrentHealthZone()
	{
		const RLDamageSystem* pRLDamageSystem		= ((Game::Actors::C_VehicleLogicActor*)GetActor())->GetRLC()->GetRLSystem()->mDamageSystem;
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		const RLHealthLevelZone* healthZone			= NULL;

		for( int i = 0; i < pRLDamageSystem->mHealthZones.count; i++ )
		{
			if( pRLDamageSystem->mHealthZones[i]->mHealthZone == pActor->mCurrentHealthLevel )
			{
				healthZone = pRLDamageSystem->mHealthZones[i].Object();
				break;
			}
		}

		if(!healthZone)
				LOGWARN(g_DamageLogger, "Health tracking broken - ignoring damage!\n");

		return healthZone;
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::SetCurrentHealthLevel(const bool ignoreWreckedScreenEffects,const bool forceReApply)
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		const RLDamageSystem* pRLDamageSystem		= pActor->GetRLC()->GetRLSystem()->mDamageSystem;

		for( int i = 0; i < pRLDamageSystem->mHealthZones.count; i++ )
		{
			if(pActor->mCurrentHealth > pRLDamageSystem->mHealthZones[i]->mHealthMin && 
				pActor->mCurrentHealth <= pRLDamageSystem->mHealthZones[i]->mHealthMax )
			{
					if (pActor->mCurrentHealthZone.mHealthZone != pRLDamageSystem->mHealthZones[i]->mHealthZone || 
						forceReApply)
					{
						// Tell the world if our health level has changed
						Game::Actors::C_VehicleHealthLevelChangedMsg vehicleHealthMsg(mVehicleIndex, pActor->mCurrentHealthLevel);
						Game::C_InGameDialogueManager::Instance().MessageHandler(&vehicleHealthMsg);

						// Set our new health level
						pActor->mCurrentHealthLevel = pRLDamageSystem->mHealthZones[i]->mHealthZone;
						pActor->mCurrentHealthZone = pRLDamageSystem->mHealthZones[i];

						// Apply screen effects
						bool applyScreenEffects = !(ignoreWreckedScreenEffects && pActor->mCurrentHealthZone.mHealthZone == E_HEALTH_LEVEL_WRECKED);
						// Logic above is the same as....
						// applyScreenEffects = ignoreWreckedScreenEffects == false || pActor->mCurrentHealthZone.mHealthZone != E_HEALTH_LEVEL_WRECKED
						if(applyScreenEffects)
						{
							Game::Actors::C_VehicleActor* pVehicleActor = pActor->GetGraphicalActor(mVehicleIndex);
							if (pVehicleActor && pVehicleActor->IsFocusVehicle())
								pVehicleActor->SetHealthEffect(pRLDamageSystem->mHealthZones[i]->mScreenEffect);
						}
						
					}
					break;
				}
			}
		}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::RepairVehicle(int repairAmount)
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		RLPickupType* pPickup						= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_REPAIR);
		const RLDamageSystem* pRLDamageSystem		= pActor->GetRLC()->GetRLSystem()->mDamageSystem;

		//Game::C_AchievementsManager::Instance().Repaired(mVehicleIndex, pActor->mCurrentHealthLevel);

		int previousHealth				= pActor->mCurrentHealth; 
		int amountToRepair				= pActor->GetMaxHealth() - previousHealth;
		float previousHealthPercent		= (float)pActor->mCurrentHealth / (float)pActor->GetMaxHealth();
		float previousDamagePercent		= 1 - previousHealthPercent;

		if(repairAmount)
		{
			AdjustHealth(repairAmount);
			pActor->mStatistics.mDamageRepaired += repairAmount;
		}
		else
		{
			pActor->mStatistics.mDamageRepaired += (pActor->GetMaxHealth() - previousHealth);
			AdjustHealth(pActor->GetMaxHealth());
		}

		float newHealthPercent	= (float)pActor->mCurrentHealth / (float)pActor->GetMaxHealth();
		float repairPercent		= newHealthPercent - previousHealthPercent;

		SetCurrentHealthLevel();

		//Tell the vehicle to repair itself visually
		Actor::C_ActorHandle handle = pActor->GetGraphicalActor(mVehicleIndex)->GetHandle();
		Game::Actors::C_VehicleRepairDamageMsg repairMsg(handle, mVehicleIndex, repairPercent, newHealthPercent, 1.f, (float)amountToRepair);
		pActor->SendMessageToActor(handle, &repairMsg);

		// Tell the MP challenge system
		I_ActorComponent* pComponent = (I_ActorComponent*)pActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges);
		if(pComponent)
			pComponent->OnMessage(&repairMsg);
		}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::GetCollisionActorName(const Actor::C_Actor* pActor, Util::C_String& string) const
	{
		if( pActor )
		{
			switch( pActor->GetActorTypeId() )
			{
			case Game::E_ActorTypeId_Vehicle:
				string.Printf("Vehicle %i", ((Game::Actors::C_VehicleActor*)pActor)->GetVehicleIndex());
				break;

			case Game::E_ActorTypeId_PhysicalObject:
				string.Printf("Physical");
				break;

			case Game::E_ActorTypeId_DynamicObject:
				string.Printf("Dynamic %s", ((Actors::DynamicObjectActor*)pActor)->GetActorName());
				break;

			default:
				string.Printf("Unknown Object");
			}
		}
		else
			string.Printf("Environment");
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::GetDamagePointsFromImpact(Game::C_GameplayImpactMsg& gameplayImpactMsg)
	{
		Game::Actors::C_VehicleLogicActor* pActor		= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		const RLDamageParameters* pRLDamageParameters	= pActor->GetRLC()->GetRLSystem()->mDamageSystem->mParameters;
		
		gameplayImpactMsg.mDamage = GetDamagePointsFromImpactLevel(pRLDamageParameters, gameplayImpactMsg.mImpactLevel);

		//Now check any modifiers for damage caused by impacts (car vs car or car vs environment)
		CheckImpactDamageModifiers(gameplayImpactMsg.mOtherVehicleIdx, gameplayImpactMsg.mAggressiveVehicleIdx, gameplayImpactMsg.mDamage, gameplayImpactMsg.mImpactLevel);

		//Now double check we've not changed our impact type, if so update the message
		//Bug# - Removed this so the impact level and damage points are more independent, for design purposes
		//gameplayImpactMsg.mImpactLevel = GetImpactLevelFromDamagePoints(pRLDamageParameters, gameplayImpactMsg.mDamage);
	}

	// -------------------------------------------------------------------------------------------

	const int C_VehicleComponent_Damage::GetDamagePointsFromImpactLevel(const RLDamageParameters* pRLDamageParameters, const E_IMPACT_LEVEL eImpactLevel) const
	{
		int damagePoints = 0;
		
		switch(eImpactLevel)
		{
		case E_IMPACT_LEVEL_LIGHT:
			damagePoints = pRLDamageParameters->mLightImpactDamage;
			break;

		case E_IMPACT_LEVEL_MEDIUM:
			damagePoints = pRLDamageParameters->mMediumImpactDamage;
			break;

		case E_IMPACT_LEVEL_HEAVY:
			damagePoints = pRLDamageParameters->mHeavyImpactDamage;
			break;
		}

		return damagePoints;
	}

	// -------------------------------------------------------------------------------------------

	const E_IMPACT_LEVEL C_VehicleComponent_Damage::GetImpactLevelFromDamagePoints(const RLDamageParameters* pRLDamageParameters, const int damagePoints) const
	{
		E_IMPACT_LEVEL eImpactLevel = E_IMPACT_LEVEL_NONE;
		
		if(damagePoints <= pRLDamageParameters->mLightImpactDamage && damagePoints > 0)
		{
			eImpactLevel = E_IMPACT_LEVEL_LIGHT;
		}
		else if(damagePoints > pRLDamageParameters->mLightImpactDamage && damagePoints <= pRLDamageParameters->mMediumImpactDamage)
		{
			eImpactLevel = E_IMPACT_LEVEL_MEDIUM;
		}	
		else if(damagePoints > pRLDamageParameters->mMediumImpactDamage)
		{
			eImpactLevel = E_IMPACT_LEVEL_HEAVY;
		}

		return eImpactLevel;
	}

	// -------------------------------------------------------------------------------------------

	int C_VehicleComponent_Damage::DetermineOtherVehicle(Game::Actors::C_VehicleActor* pActorA, Game::Actors::C_VehicleActor* pActorB)
	{
		if(pActorA && pActorA->GetActorTypeId() == Game::E_ActorTypeId_Vehicle && pActorA->GetVehicleIndex() != mVehicleIndex)
			return pActorA->GetVehicleIndex();

		if(pActorB && pActorB->GetActorTypeId() == Game::E_ActorTypeId_Vehicle && pActorB->GetVehicleIndex() != mVehicleIndex)
			return pActorB->GetVehicleIndex();

		return -1;
	}

	// -------------------------------------------------------------------------------------------

	Debug::Float gVcd_SpeedRatio("Vehicle.Components.Damage.SpeedRatio", 0.5f);
	Debug::Float gVcd_MinSpeed("Vehicle.Components.Damage.MinSpeed", 25.0f);
	Debug::Float gVcd_MaxSpeed("Vehicle.Components.Damage.MaxSpeed", 45.0f);
	Debug::Float gVcd_SwipeThreshold("Vehicle.Components.Damage.SwipeThreshold", 120.0f);

	int C_VehicleComponent_Damage::DetermineAggressiveVehicle(Game::Actors::C_VehicleActor* pActorA, Game::Actors::C_VehicleActor* pActorB, Game::C_ImpactMsg* pMsg)
	{
		Maths::HVec3 delta	= pMsg->mMatrixB.AW() - pMsg->mMatrixA.AW();
		Maths::HVec3 fwdA	= pMsg->mMatrixA.AZ();
		Maths::HVec3 fwdB	= pMsg->mMatrixB.AZ();

		delta.Y(0.0f);
		fwdA.Y(0.0f);
		fwdB.Y(0.0f);

		delta.Normalise();
		fwdA.Normalise();
		fwdB.Normalise();

		bool facingA = fabsf(Maths::DotProduct(fwdA, delta)) >= 0.707f;
		bool facingB = fabsf(Maths::DotProduct(fwdB, delta)) >= 0.707f;

		if (facingA && facingB)
		{
			float mphA = MpsToMph(pMsg->mVehicleVelocityA.Length());
			float mphB = MpsToMph(pMsg->mVehicleVelocityB.Length());

			float	low				= Maths::Min(mphA, mphB);
			float	high			= Maths::Max(mphA, mphB);
			int		potentialVictor	= ((mphA > mphB) ? pActorA : pActorB)->GetVehicleIndex();

			low += Maths::Clamp(low * gVcd_SpeedRatio, (float)gVcd_MinSpeed, (float)gVcd_MaxSpeed);

			if (high > low)
				return potentialVictor;
		}
		else if (facingA && !facingB)
		{
			return pActorA->GetVehicleIndex();
		}
		else if (!facingA && facingB)
		{
			return pActorB->GetVehicleIndex();
		}
		else if (!facingA && !facingB)
		{
			bool winA = false;
			bool winB = false;

			for(int i = 0; i < pMsg->mCollisionInfo.m_ContactPoints.Size(); i++)
			{
				Vehicle::CollisionInfo::ContactPoint* cp = &pMsg->mCollisionInfo.m_ContactPoints[i];

				float da = Maths::DotProduct((Maths::HVec3)(cp->m_Impulse / cp->m_MassA), cp->m_PtVelA);
				float db = Maths::DotProduct((Maths::HVec3)(-cp->m_Impulse / cp->m_MassB), cp->m_PtVelB);

				if (da > db)
				{
					float threshold = (da - db);

					if (threshold > gVcd_SwipeThreshold)
						winA = true;
				}
				else
				{
					float threshold = (db - da);

					if (threshold > gVcd_SwipeThreshold)
						winB = true;
				}
			}

			if (winA && !winB)
				return pActorA->GetVehicleIndex();

			if (!winA && winB)
				return pActorB->GetVehicleIndex();
		}

		return -1;
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::HandleCancelWreckedTimer()
	{
		// Slightly long winded method of doing this but...made me smile in an otherwise barren landscape of business
		
		// RED ALERT! // *WOOP WOOP*
			// If we were already wrecked at this point and were delaying a wrecked sequence... we need to make sure we prioritise this HandleCancelWreckedTimer  
			// request over our existing plan to delay the wrecked sequence (e.g. this may have been requested for going over a cliff!!) .
			// If we don't.. the game will get very confused HOG will possibly fail, and the camera will likely do some craaaazy stuff (like stick to the ground and watch the player drive off..)
			// or... we could ignore the request... :S
		if(!mLastGaspModData.mWreckedSequenceDelayed)
		{
			mWreckedTimer = 0.f;
		}

	}

	// -------------------------------------------------------------------------------------------

	//Apply a damage multiplier to the damage amount, this function WILL ROUND DOWN TO THE NEAREST HALF PIP
	void C_VehicleComponent_Damage::ApplyDamageMultiplier(int& damage) const
	{
		if(!ShouldWeBeApplyingDamageMultipliers())
		{
			return;
		}
	
		//Make sure the damage is always either a whole number of pips (heathIncrement) or a half
		//Work out the minimum amount of damage we can do
		int halfAHealthPip = (mpRLC->GetRLSystem()->mDamageSystem->mParameters->mHealthIncrement / 2);

		//Get the multiplier from sorcery
		MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
		int multiplier = pConfig->m_MotorMashDamageMultiplier;

		damage = (damage * multiplier);
		int adjustment = damage % halfAHealthPip;

		if(adjustment > 0)
		{
			damage -= adjustment;
		}
	}

	// -------------------------------------------------------------------------------------------

	const bool C_VehicleComponent_Damage::ShouldWeBeApplyingDamageMultipliers() const
	{
		return (g_AllowDamageMultiplier 
				|| 
				mpLevelState->mRaceMode == Game::E_RaceMode_Multiplayer_Destruction 
				|| 
				mpLevelState->mRaceMode == Game::E_RaceMode_Multiplayer_Racing);
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::CheckPowerUpDamageModifiers(const int firingVehicleIndex, int& damage, const E_PICKUP_TYPE pickupType)
	{
		if(firingVehicleIndex != Game::SYSTEM_FIRING_INDEX)
		{
			// Check for Iron Fist mod here
			Game::C_LevelStateMultiplayerVehicleAbilities* pOpponentMultiplayerAbilities = mpLevelState->mCars[firingVehicleIndex].mUpgrades.GetMultiplayerAbilities();

			BIZ_ASSERT_MSG(mpMultiplayerAbilities,"C_VehicleComponent_Damage::CheckDamageModifiers() > Error.. VLC_Damage OnMessage(E_VehicleLogicMsg_HitByPerk) unable to obtain pointer to opponent Multiplayer Abilities");
			Game::C_MultiplayerVehicleAbility* opponentIronFist = pOpponentMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist);

			if(opponentIronFist->GetActive()) // Don't require GameVehicleIsLocal() check here as opponent is never local
			{
				int ironFistAdditionalDamagePoints = Game::C_Game::GetInstance()->GetMultiplayerAbilities().IronFistActivatedOnPickupType(pickupType);
				if(ironFistAdditionalDamagePoints > 0)
				{
					pOpponentMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Iron_Fist, true, true, firingVehicleIndex, mVehicleIndex);
					if(g_DebugShowMpAbilityActivateSpam)
					{
						LOGDEBUG(g_MultiplayerLog, "C_VehicleComponent_Damage::CheckDamageModifiers() > Vehicle %d's Iron fist ability has been activated, increasing damage by %d!", firingVehicleIndex, ironFistAdditionalDamagePoints);
					}

					damage += ironFistAdditionalDamagePoints; 
				}
			}

			Game::C_MultiplayerVehicleAbility* opponentShuntDamage = pOpponentMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_ShuntDamage);
			if(opponentShuntDamage && opponentShuntDamage->GetActive() && pickupType == E_PICKUP_TYPE_SHUNT)
			{
				pOpponentMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_ShuntDamage, true, false, firingVehicleIndex, mVehicleIndex);
				damage += GetAFractionOfAHealthPipInDamagePoints(opponentShuntDamage->GetValue());
			}
		}

		// Check for Titanium armour mod here
		Game::C_MultiplayerVehicleAbility* myTitaniumArmour = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Titanium_Armour);

		int titaniumArmourReductionDamagePoints = GetAFractionOfAHealthPipInDamagePoints(myTitaniumArmour->GetValue());
		if( myTitaniumArmour->GetActive())
		{
			bool activated = Game::C_Game::GetInstance()->GetMultiplayerAbilities().TitaniumArmourActivatedOnPickupType(pickupType);

			if(activated)
			{
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Titanium_Armour, true, true, mVehicleIndex);
				if(g_DebugShowMpAbilityActivateSpam)
				{
					LOGDEBUG(g_MultiplayerLog, "C_VehicleComponent_Damage::CheckDamageModifiers() > Vehicle %d's Titanium Armour ability has been activated, reducing damage by %d!", mVehicleIndex, titaniumArmourReductionDamagePoints);
				}

				if(Game::VehicleIsLocal(mVehicleIndex))
				{
					damage -= titaniumArmourReductionDamagePoints;
				}
			}
		}

		Game::C_MultiplayerVehicleAbility* pRepairBonusAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_RepairBonus);
		if(pRepairBonusAbility->GetActive())
		{
			if(pickupType == E_PICKUP_TYPE_BARGE || pickupType == E_PICKUP_TYPE_SHUNT || pickupType == E_PICKUP_TYPE_BOMB)
			{
				damage -= GetAFractionOfAHealthPipInDamagePoints(pRepairBonusAbility->GetValue());
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_RepairBonus, true, false, mVehicleIndex);
			}
		}
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::CheckActivateBatteringRam()
	{
		if(mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Battering_Ram)->GetActive())
		{
			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Battering_Ram, true, false, mVehicleIndex);

			if(g_DebugShowMpAbilityActivateSpam)
			{
				LOGDEBUG(g_MultiplayerLog, "C_VehicleComponent_Damage::CheckDamageModifiers() > Vehicle %d's Battering Ram ability has been activated", mVehicleIndex);
			}
		}
	}

	void C_VehicleComponent_Damage::CheckImpactDamageModifiers(const int otherVehicleIndex, const int aggressorVehicleIndex, int& damage, const E_IMPACT_LEVEL impactLevel)
	{
		const bool areWeActuallyDoingAnyDamage = (damage > 0);

		// Check for Stable Frame mod here
		Game::C_LevelStateMultiplayerVehicleAbilities* pOurAbilities = mpLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* ourStableFrame = pOurAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Stable_Frame);
		
		if(ourStableFrame->GetActive() && areWeActuallyDoingAnyDamage)
		{
			int reductionInDamagePoints = GetAFractionOfAHealthPipInDamagePoints(ourStableFrame->GetValue());

			pOurAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Stable_Frame, true, false, mVehicleIndex);

			// network vehicles have a bit of trouble firing this one off at the correct time.. lets be nice and give them a helping hand.
			const Game::C_NetGameVars& netGameVars	= C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
			netGameVars.m_EventManager->Add_AbilityActivated(mpLevelState->mCars[mVehicleIndex].mNetJoinId, -1, UI::E_MultiplayerVehicleAbilities_Stable_Frame, false);

			if(g_DebugShowMpAbilityActivateSpam)
			{
				LOGDEBUG(g_MultiplayerLog, "C_VehicleComponent_Damage::CheckDamageModifiers() > Vehicle %d's Stable Frame ability has been activated, reduced damage by %d!", mVehicleIndex, reductionInDamagePoints);
			}

			damage -= reductionInDamagePoints;
			if(damage < 0)
			{
				damage = 0;
			}
		}

		//Apply race damage multiplier
		if(mpLevelState->mDamageMode == E_DAMAGE_MODE_MULTIPLIER)
		{
			damage *= mpLevelState->mDamageMultiplier;
		}
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::ApplyVehicleToVehicleBatteringRamPhysicsEffect(const U8 victimVehicleIndex,const U8 aggressorVehicleIndex)
	{
		Game::Actors::C_VehicleLogicActor* pAggressorLogicActor =  (Game::Actors::C_VehicleLogicActor*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(aggressorVehicleIndex);// Note: is it safe to assume thie network veh's position is correct?
		Game::Actors::C_VehicleLogicActor* pVictimLogicActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

		BIZ_ASSERT_MSG(pAggressorLogicActor , "_VehicleComponent_Damage::ApplyVehicleToVehicleBatteringRamPhysicsEffect() < aggressor logic actor could not be obtained");
		BIZ_ASSERT_MSG(pVictimLogicActor , "_VehicleComponent_Damage::ApplyVehicleToVehicleBatteringRamPhysicsEffect() < victim logic actor could not be obtained");

		Game::Actors::C_VehicleActor* pVictimGraphicalActor = pVictimLogicActor->GetGraphicalActor(victimVehicleIndex);
		Game::Actors::C_VehicleActor* pAggressorGraphicalActor = pAggressorLogicActor->GetGraphicalActor(aggressorVehicleIndex);

		BIZ_ASSERT_MSG(pAggressorGraphicalActor , "_VehicleComponent_Damage::ApplyVehicleToVehicleBatteringRamPhysicsEffect() < aggressor graphical actor could not be obtained");
		BIZ_ASSERT_MSG(pVictimGraphicalActor , "_VehicleComponent_Damage::ApplyVehicleToVehicleBatteringRamPhysicsEffect() < victim graphical actor could not be obtained");

		// Tip slightly based on dir they were hit from
		E_TARGETTING_DIRECTION hitDir = GetBatteringRamHitDirection(pVictimLogicActor,pAggressorLogicActor);
		const bool heavyHit = (pAggressorGraphicalActor->GetWeightClass() == AmaxVehicleWeight_Heavy) || g_BatteringRamHeavyHit;

		StartBatteringRamPhysicsEffect(pVictimGraphicalActor, hitDir, heavyHit);
	}

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::StartBatteringRamPhysicsEffect(	Game::Actors::C_VehicleActor* pVictimGraphicalActor, 
																	const enum E_TARGETTING_DIRECTION eHitDir, 
																	const bool heavyHit) const
	{
		BIZ_ASSERT_MSG(eHitDir != E_TARGETTING_DIRECTION_NONE, "C_VehicleComponent_Damage::StartBatteringRamPhysicsEffect() < Hit dir calculated as E_TARGETTING_DIRECTION_NONE");

		enum Game::Actors::E_FakeVehiclePhysicsEffect eFakePhysicsEffect = Game::Actors::E_FAKE_PHYSICS_NONE;
		if(heavyHit)
		{
			//HEAVY
			switch(eHitDir)
			{
			case E_TARGETTING_DIRECTION_LEFT:
				eFakePhysicsEffect = Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_RIGHT;
				break;

			case E_TARGETTING_DIRECTION_RIGHT:
				eFakePhysicsEffect = Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_LEFT;
				break;
			}
		}
		else
		{
			//LIGHT
			switch(eHitDir)
			{
			case E_TARGETTING_DIRECTION_LEFT:
				eFakePhysicsEffect = Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_RIGHT;
				break;

			case E_TARGETTING_DIRECTION_RIGHT:
				eFakePhysicsEffect = Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_LEFT;
				break;
			}
		}

		pVictimGraphicalActor->GetFakePhysics()->Start(eFakePhysicsEffect);
	}

	// -------------------------------------------------------------------------------------------

	E_TARGETTING_DIRECTION C_VehicleComponent_Damage::GetBatteringRamHitDirection(Game::Actors::C_VehicleLogicActor* victimActor, Game::Actors::C_VehicleLogicActor* aggressiveActor)
	{
		E_TARGETTING_DIRECTION hitDir = E_TARGETTING_DIRECTION_NONE;
		Maths::HVec3 dir = Maths::Normalise(aggressiveActor->GetWorldMatrix().AW() - victimActor->GetWorldMatrix().AW());
		float dotProdX = Maths::DotProduct(dir, victimActor->GetWorldMatrix().AX());
		
		//Always treat impacts as though they were left or right
		if(dotProdX >= 0.f)
		{
			hitDir = E_TARGETTING_DIRECTION_RIGHT;
		}
		else
		{
			hitDir = E_TARGETTING_DIRECTION_LEFT;
		}

		return hitDir;
	}

	// -------------------------------------------------------------------------------------------

	const bool C_VehicleComponent_Damage::ValidVehicleToVehicleCollision(	const Game::Actors::C_VehicleActor *pVehicleActorA,
																			const Game::Actors::C_VehicleActor *pVehicleActorB) const
	{
		if(!pVehicleActorA || !pVehicleActorB || pVehicleActorA->GetActorTypeId() != Game::E_ActorTypeId_Vehicle || pVehicleActorB->GetActorTypeId() != Game::E_ActorTypeId_Vehicle)
			return false;
		else
			return true;
	}


	// -------------------------------------------------------------------------------------------

	const bool C_VehicleComponent_Damage::ShouldBatteringRamFire(const Game::Actors::C_VehicleActor *pMyVehicleActor,
																 const Game::Actors::C_VehicleActor *pOtherVehicleActor) const
	{
		bool shouldFire = false;

		const int myVehicleIndex = pMyVehicleActor->GetVehicleIndex();

		Game::C_LevelStateMultiplayerVehicleAbilities* pMyMPAbilities = mpLevelState->mCars[myVehicleIndex].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* pMyBatteringRam = pMyMPAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Battering_Ram);

		//Both vehicles should have one of these even if its not turned on
		if(!pMyBatteringRam)
			return shouldFire;
			
		//First work out if we have battering ram equipped
		if( pMyBatteringRam->GetActive() )
		{
			//Now see if we hit the other car with the front of our car
			const Maths::HMat43 &myMatrix = pMyVehicleActor->GetMatrix();
			const Maths::HMat43 &otherMatrix = pOtherVehicleActor->GetMatrix();

			Maths::HVec3 delta	= otherMatrix.AW() - myMatrix.AW();
			Maths::HVec3 myForward = myMatrix.AZ();

			Maths::HVec3 myVel = pMyVehicleActor->GetVehicleInput()->m_vVelocity;
			Maths::HVec3 otherVel = pOtherVehicleActor->GetVehicleInput()->m_vVelocity;
			Maths::HVec3 closingVel = otherVel - myVel;

			const float closingSpeedInMPH = fabs( MpsToMph(Maths::Magnitude(closingVel)) );

			delta.Normalise();
			myForward.Normalise();

			//Check the angle, is it hitting the front of the car?
			const float myDotForward = Maths::DotProduct(myForward, delta);
			if( myDotForward >= mBatteringRamModData.mTargettingCone)
			{
				//Check the closing speed
				if(closingSpeedInMPH >= mBatteringRamModData.mMinimumImpactSpeedInMPH || g_BatteringRamHairTrigger)
				{
					shouldFire = true;
				}
			}
		}

		return shouldFire;
	}



	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::CheckGameplayModifiers(const enum E_PICKUP_TYPE ePickupType, const enum E_PICKUP_SUB_TYPE ePickupSubType, const int firingVehicleIndex)
	{
		if(firingVehicleIndex != Game::SYSTEM_FIRING_INDEX)
		{
			// Never check for scrambler if this vehicle has the shielded bay mod
			Game::C_LevelStateMultiplayerVehicleAbilities* pOpponentMultiplayerAbilities = mpLevelState->mCars[firingVehicleIndex].mUpgrades.GetMultiplayerAbilities();
			Game::C_MultiplayerVehicleAbility* myShieldedBay = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Shielded_Bay);

			// If hit by shunt, or blindfire, then opponents scrambler mod may take effect and cause us to drop a pickup
			// Only check this stuff if this vehicle is local

			if(Game::VehicleIsLocal(mVehicleIndex))
			{
				bool checkForScrambler = false;
				Game::C_MultiplayerVehicleAbility* theirScramblerMod = pOpponentMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Scrambler);

				if(CanPickupTypeCauseScrambler(ePickupType, ePickupSubType))
				{
					// Only special case is Blindfire, MAY require multiple successive hits to activate scrambler
					if(ePickupType == E_PICKUP_TYPE_BLINDFIRE)
					{
						// Increment chain
						mBlindFireChainLength++; 

						// reset the chain timer
						mBlindFireChainTimer = mBlindFireChainTimerIncrement; 

						// If hit by X or more blind fires in quick succession.. then scramble their powerups. 
						if(mBlindFireChainLength >= mNumBlindFireRequiredToCauseScrambler)
						{
							// If this last hit came from a vehicle with the scrambler.. scramble!
							checkForScrambler = true;
						}
					}
					else
					{
						// All non-blindfire types require a single hit to activate scrambler
						checkForScrambler = true;
					}
				}

				if(checkForScrambler)
				{
					if(theirScramblerMod->GetActive())
					{
						Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

						if(!myShieldedBay->GetActive())
						{
							//We have been scrambled so we shall handle the consequences
							mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Scrambler, true, true, firingVehicleIndex, mVehicleIndex, ePickupType == E_PICKUP_TYPE_BLINDFIRE);

							if(g_DebugShowMpAbilityActivateSpam)
							{
								LOGDEBUG(g_MultiplayerLog, "C_VehicleComponent_Damage::CheckGameplayModifiers() > Vehicle %d's Scrambler ability has been activated, disabling vehicle %d 's powerup bay for %d seconds!", firingVehicleIndex, mVehicleIndex,  mScramblerPowerupBayDisableTime);
							}

							// [ J BUNNER ] : BUG: 30220 - Scrambler redesign.. no longer drops pickups, 
							// instead disables powerup bay for certain time period. 

							// Disable powerup bays for certain time period
							if(ePickupType == E_PICKUP_TYPE_BLINDFIRE)
							{
								C_VehicleLogicMsg_EnableDisablePickupSystem disablePickupsMsg(mVehicleIndex,mScramblerBoltDisableTime, 0.0f, mScramblerModBoltsStack);
								vehActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &disablePickupsMsg);

								Game::Actors::C_VehicleActor* pVehActor = vehActor->GetGraphicalActor(mVehicleIndex);
								if(pVehActor && pVehActor->QueryInterface(Components::E_AudioEntity_AdditionalSpawned))
									pVehActor->SendMessageToComponent(Components::E_AudioEntity_AdditionalSpawned, &disablePickupsMsg);

								if(pVehActor->IsFocusVehicle())
									UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_ScramblerActivatedActionIn );
							
							}
							else
							{
								C_VehicleLogicMsg_EnableDisablePickupSystem disablePickupsMsg(mVehicleIndex,mScramblerPowerupBayDisableTime, 0.0f, false);
								vehActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &disablePickupsMsg);

								Game::Actors::C_VehicleActor* pVehActor = vehActor->GetGraphicalActor(mVehicleIndex);
								if(pVehActor && pVehActor->QueryInterface(Components::E_AudioEntity_AdditionalSpawned))
									pVehActor->SendMessageToComponent(Components::E_AudioEntity_AdditionalSpawned, &disablePickupsMsg);

								if(pVehActor->IsFocusVehicle())
									UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_ScramblerActivatedActionIn );
							}

							// TODO: Play some kind of effect? (user needs to know that they hav been scrambled!)
						}
						else
						{
							mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Shielded_Bay, true, false, mVehicleIndex);
							if(g_DebugShowMpAbilityActivateSpam)
							{
								LOGDEBUG(g_MultiplayerLog, "C_VehicleComponent_Damage::CheckGameplayModifiers() > Vehicle %d's Shielded bay ability has been activated, negating vehicle %d 's Scrambler Mod!", mVehicleIndex, firingVehicleIndex );
							}
						}

						// Reset the chain info
						mBlindFireChainLength = 0; 
						mBlindFireChainTimer = 0.0f; 
					}
				}
			}// end if(check for scrambler)
		}
	}

	// -------------------------------------------------------------------------------------------

	bool C_VehicleComponent_Damage::CanPickupTypeCauseScrambler(const enum E_PICKUP_TYPE ePickupType, const enum E_PICKUP_SUB_TYPE ePickupSubType) const
	{
		// Check against sorcery data to see if the designers want this pickup type to cause scrambler
		MiscModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings;

		switch(ePickupType)
		{
			case E_PICKUP_TYPE_SHOCK:
			{
				return modSettings.m_ScramblerEnabledShock;
			}
			case E_PICKUP_TYPE_SHUNT:
			{
				if( ePickupSubType == E_PICKUP_SUB_TYPE_REVERSED )
				{
					return modSettings.m_ScramblerEnabledShuntReverse;
				}
				else
				{
					return modSettings.m_ScramblerEnabledShunt;
				}
			}
			case E_PICKUP_TYPE_BLINDFIRE:
			{
				if( ePickupSubType == E_PICKUP_SUB_TYPE_REVERSED )
				{
					return modSettings.m_ScramblerEnabledBoltReverse;
				}
				else
				{
					return modSettings.m_ScramblerEnabledBolt;
				}
			}
			case E_PICKUP_TYPE_BARGE:
			{
				return modSettings.m_ScramblerEnabledBarge;
			}
			case E_PICKUP_TYPE_BOMB:
			{
				if( ePickupSubType == E_PICKUP_SUB_TYPE_REVERSED )
				{
					return modSettings.m_ScramblerEnabledBombReverse;
				}
				else
				{
					return modSettings.m_ScramblerEnabledBomb;
				}
			}
			case E_PICKUP_TYPE_NITROUS:
			{
				if( ePickupSubType == E_PICKUP_SUB_TYPE_REVERSED )
				{
					return modSettings.m_ScramblerEnabledNitroRift;
				}
			}
			default:
			{
				break;
			}
		}

	// If reach here then cant absorb / unknown type
	return false;
		


	}

	// -------------------------------------------------------------------------------------------

	Debug::Bool g_debugNormalseImpactImpulse("Vehicle.Collision.NormaliseImpactImpulse", true);
	Debug::Float g_debugNormaliseMassValue("Vehicle.Collision.NormalisedMassValue", 1500.0f, Debug::Range(0.0f, 4000.0f));
	Debug::Bool g_debugDisplayGameplayImpulses("Vehicle.Collision.DisplayGamePlayImpulses", false);

	void C_VehicleComponent_Damage::HandleImpact(Game::C_ImpactMsg *pImpactMsg)
	{
		PERF_EVENT("C_VehicleComponent.Damage.HandleImpact");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* vehActor		= pActor->GetGraphicalActor(mVehicleIndex);

		vehActor->GetActorList()->SendMessageToActor(vehActor->GetHandle(), pImpactMsg);

		//	Bail out if vehicles are invulnerable
		if(mpLevelState->mDamageMode == E_DAMAGE_MODE_INVULNERABLE )
			return;

		if(mpLevelState->mDamageMode == E_DAMAGE_MODE_POWERUPS_ONLY)
		{
			HandleImpactCameraEffects(pImpactMsg, vehActor);
			return;
		}

		if( pActor->mShieldTime == 0.f )
		{
			F32 newImpulse = pImpactMsg->mCollisionInfo.m_GameplayImpulse;

			if (g_debugNormalseImpactImpulse)
				newImpulse = pImpactMsg->mCollisionInfo.m_GameplayImpulse * ((float)g_debugNormaliseMassValue / vehActor->GetCollisionBody()->Mass());

			bool isSideImpact = !IsFrontalImpact(pImpactMsg, vehActor);

			E_IMPACT_LEVEL impactLevel = mpRaceLogicLayer->CalculateImpactLevel(newImpulse, isSideImpact);

			if (g_debugDisplayGameplayImpulses && impactLevel != E_IMPACT_LEVEL_NONE)
			{
				DebugLn("IMPULSE car %i  pre %4.2f  post %4.2f  lvl %i  mph %4.2f  side %i", vehActor->GetVehicleIndex(), pImpactMsg->mCollisionInfo.m_GameplayImpulse, newImpulse, impactLevel,
						MpsToMph(pImpactMsg->mCollisionInfo.m_MaxImpulse / vehActor->GetCollisionBody()->Mass()),
					isSideImpact);
			}

			//Do some camera shizzle
			HandleImpactCameraEffects(pImpactMsg, vehActor);
			
			const Actor::C_Actor* actorA = GetActorList()->GetActorPtr(pImpactMsg->mActorHandleA);
			const Actor::C_Actor* actorB = GetActorList()->GetActorPtr(pImpactMsg->mActorHandleB);

			if(pImpactMsg->mCollisionInfo.m_ContactPoints.Size())
			{
				Game::Actors::C_VehicleActor* vehActorA = (Game::Actors::C_VehicleActor*)actorA;
				Game::Actors::C_VehicleActor* vehActorB = (Game::Actors::C_VehicleActor*)actorB;

				Game::C_GameplayImpactMsg gameplayImpactMsg(mVehicleIndex, 0, impactLevel);

				gameplayImpactMsg.mOtherVehicleIdx = DetermineOtherVehicle(vehActorA, vehActorB);

				if(gameplayImpactMsg.mOtherVehicleIdx != INVALID_VEHICLE_IDX)
				{
					gameplayImpactMsg.mAggressiveVehicleIdx = DetermineAggressiveVehicle(vehActorA, vehActorB, pImpactMsg);
				}

				if(ValidVehicleToVehicleCollision(vehActorA, vehActorB))
				{
					CheckBatteringRam(gameplayImpactMsg, vehActorA, vehActorB);
				}

				if(impactLevel > E_IMPACT_LEVEL_NONE)
				{
					//Check that A's a vehicle and we're A's damage component
					if(actorA && actorA->GetActorTypeId() == Game::E_ActorTypeId_Vehicle && vehActorA && vehActorA->GetVehicleIndex() == mVehicleIndex)
					{
						//Check that we should be getting damaged
						if(!actorB || actorB->GetActorTypeId() != Game::E_ActorTypeId_Vehicle || !vehActorB || mVehicleIndex != gameplayImpactMsg.mAggressiveVehicleIdx )
						{
							GetDamagePointsFromImpact(gameplayImpactMsg);
						}
					}
					//Check that B's a vehicle and we're B's damage component
					else if(actorB && actorB->GetActorTypeId() == Game::E_ActorTypeId_Vehicle && vehActorB && vehActorB->GetVehicleIndex() == mVehicleIndex)
					{	
						//Check that we should be getting damaged
						if(!actorA || actorA->GetActorTypeId() != Game::E_ActorTypeId_Vehicle || !vehActorA || mVehicleIndex != gameplayImpactMsg.mAggressiveVehicleIdx )
						{
							GetDamagePointsFromImpact(gameplayImpactMsg);
						}
					}

	#ifndef CFG_GOLD
					LogImpact(actorA, actorB, pImpactMsg, gameplayImpactMsg.mDamage, pActor->GetRLC()->GetRaceTime());			
	#endif
				}

				// Send impact message to components that want it.
				SendGameplayImpactMessageToComponents(gameplayImpactMsg, pActor);

				if(impactLevel > E_IMPACT_LEVEL_NONE)
				{
					//also send to the sticker manager for use in various sticker criteria
					Game::C_StickerManager::Instance().MessageHandler(&gameplayImpactMsg);

					if(gameplayImpactMsg.mImpactLevel >= E_IMPACT_LEVEL_LIGHT)
					{
						Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
						Game::Actors::C_VehicleActor* pVehActor = pActor->GetGraphicalActor(mVehicleIndex);
						Game::Actors::FakeVehiclePhysicsState* pFakePhysics = pVehActor->GetFakePhysics();

						//bool isFlip = pFakePhysics->_effect == Game::Actors::E_FAKE_PHYSICS_FLIP_FORWARD || pFakePhysics->_effect == Game::Actors::E_FAKE_PHYSICS_FLIP_BACKWARD;
						bool isBargeJink	= pFakePhysics->_effect == Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_LEFT || pFakePhysics->_effect == Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_RIGHT;
						bool isShieldJink	= pFakePhysics->_effect == Game::Actors::E_FAKE_PHYSICS_JINK_SHIELD_LEFT || pFakePhysics->_effect == Game::Actors::E_FAKE_PHYSICS_JINK_SHIELD_RIGHT;
						bool isNitrousJink	= pFakePhysics->_effect == Game::Actors::E_FAKE_PHYSICS_JINK_NITROUS_LEFT || pFakePhysics->_effect == Game::Actors::E_FAKE_PHYSICS_JINK_NITROUS_RIGHT;

						if(isBargeJink || isShieldJink || isNitrousJink)
						{
							pFakePhysics->Stop();
						}
						//else if(isFlip)
						//{
						//	float dot = Maths::DotProduct(pImpactMsg->mCollisionInfo.m_ContactNormal, pVehActor->GetMatrix().AZ());
						//	if(dot > g_debugFlipKillCollisionDot)
						//		pFakePhysics->Stop();
						//}
					}

					//Actually apply the damage to the vehicle
					if(mCooldown <= 0.f)
					{
						if(gameplayImpactMsg.mDamage > 0)
						{
							// Then we were rammed!
							mLastDamageWasInflictedLastGasp = false;
						}

						ApplyDamage(gameplayImpactMsg.mDamage, gameplayImpactMsg.mAggressiveVehicleIdx);

						Game::Actors::C_VehicleLogicActor* pActor		= ((Game::Actors::C_VehicleLogicActor*)GetActor());
						const RLDamageParameters* pRLDamageParameters	= pActor->GetRLC()->GetRLSystem()->mDamageSystem->mParameters;
						mCooldown = pRLDamageParameters->mCooldownCollision;
					}

					if(Game::C_InGameDialogueManager::Instance().IsEnabled())
						Game::C_InGameDialogueManager::Instance().MessageHandler(&gameplayImpactMsg);

	#ifndef CFG_GOLD
					if(Game::C_ReplayDirector::Instance().IsEnabled())
						Game::C_ReplayDirector::Instance().MessageHandler(&gameplayImpactMsg);
	#endif // CFG_GOLD

					Game::C_AchievementsManager::Instance().MessageHandler(&gameplayImpactMsg);
					PopulateDamageFilter(pImpactMsg->mDamageFilter, pImpactMsg->mCollisionInfo.m_GameplayImpulse);
				}
			}
		}

	}

	// -------------------------------------------------------------------------------------------

	const int C_VehicleComponent_Damage::GetAFractionOfAHealthPipInDamagePoints(const float healthPipFraction) const
	{
		//Make sure the damage is always either a whole number of pips (heathIncrement) or a half
		int healthPip = mpRLC->GetRLSystem()->mDamageSystem->mParameters->mHealthIncrement;
		int halfAHealthPip = (healthPip / 2);
		
		int damagePoints = (int)( ((float)healthPip) * healthPipFraction);

		bool valid = true;
		if( (damagePoints % halfAHealthPip) > 0)
		{
			//Some n00b has borked the value...should we assert or just mutter quietly to ourselves?
			valid = false;
		}
		if(g_BitchAboutInvalidHealthPointMultipliers)
		{
			BIZ_ASSERT_MSG(valid, "C_VehicleComponent_Damage::GetAFractionOfAHealthPipInDamagePoints() > The fraction (%3.02f)\n applied means the number of damage points is not a multiple of half a pip (%d)...\n %d points", healthPipFraction, halfAHealthPip, damagePoints);
		}

		//Work out the minimum amount of damage we can do
		return damagePoints;
	}

#ifndef CFG_GOLD

	void C_VehicleComponent_Damage::LogImpact(const Actor::C_Actor* pActorA, const Actor::C_Actor* pActorB, const Game::C_ImpactMsg* pImpactMsg, const int damagePoints, const F32 raceTime) const
	{
		// Data logging
		Util::C_String nameActorA, nameActorB;

		GetCollisionActorName(pActorA, nameActorA);
		GetCollisionActorName(pActorB, nameActorB);

		Maths::HVec3 collPoint(pImpactMsg->mCollisionInfo.m_ContactPoints.Back().m_ContactPoint);

		if (pImpactMsg->mCollisionInfo.m_GameplayImpulse >= g_LoggingMinimumImpulse)
		{
			AmaxDataLogging::LogCollision(nameActorA.Chars(), nameActorB.Chars(), collPoint, pImpactMsg->mCollisionInfo.m_GameplayImpulse, damagePoints, raceTime);
		}
	}

#endif

	// -------------------------------------------------------------------------------------------

	void C_VehicleComponent_Damage::HandleImpactCameraEffects(const Game::C_ImpactMsg *pImpactMsg, Game::Actors::C_VehicleActor *pGraphicalActor)
	{
		const float gameplayImpulse				= pImpactMsg->mCollisionInfo.m_GameplayImpulse;
		const RLImpactType* pImpactType			= mpRaceLogicLayer->CalculateImpactType(gameplayImpulse, !IsFrontalImpact(pImpactMsg, pGraphicalActor));

		if (pImpactType)
		{
			const E_IMPACT_LEVEL impactLevel = pImpactType->meImpactLevel;
			if (pGraphicalActor->IsFocusVehicle())
			{
				// We need to determine which effect to play. 
				// We have three choices. 
				// 1) A forwards impact. Eg. The vehicle has driven head-on into a wall, Has rammed another vehicle or has itself been rammed.
				// 2) A side impact. Eg. We have clipped a wall, been side shunted or side shunted another vehicle. 
				// 3) A roll impact. Eg. We have lost control of the vehicle and are currently tumbling
				if(pGraphicalActor->IsTumbling())
				{
					LOGDEBUG( g_VehicleActorEffectsLog, "Triggered Vehicle Rolling FX: Level - %d  [%f]\n", impactLevel, gameplayImpulse);
					pGraphicalActor->SetImpactEffect(pImpactType->mRollScreenEffect, Maths::Constants<Maths::HMat33>::Identity());
				}
				else
				{
					// So that we don't get landing impacts and crash impacts occurring at the same time we 
					// filter out collisions with surfaces that we do not consider vertical.
					static const F32 verticalSurfaceDot = 0.707f;
					F32 collisionNormalUpDot = pImpactMsg->mCollisionInfo.m_ContactNormal | Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
					if( !pGraphicalActor->IsAirbourne() || fabs(collisionNormalUpDot) < verticalSurfaceDot )
					{
						
						Maths::HMat33 shakeTransform = CalculateImpactShakeTransform(pGraphicalActor);
		
						// Now work out if this was a front or a side impact
						if(IsFrontalImpact(pImpactMsg, pGraphicalActor))
						{
							LOGDEBUG( g_VehicleActorEffectsLog, "Triggered Vehicle Front Impact FX: Level - %d  [%f]\n", impactLevel, gameplayImpulse);
							pGraphicalActor->SetImpactEffect(pImpactType->mFrontScreenEffect,shakeTransform);

							//ssend audio messages
							switch(impactLevel)
							{
								case E_IMPACT_LEVEL_MEDIUM:
									{
										Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eRGBMedium);
									}
									break;

								case E_IMPACT_LEVEL_HEAVY:
									{
										Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eRGBHard);
									}
									break;


							}
						}
						else
						{
							LOGDEBUG( g_VehicleActorEffectsLog, "Triggered Vehicle Side Impact FX: Level - %d  [%f]\n", impactLevel, gameplayImpulse);
							// If this is a side impact we need to rotate the shakeTransform as the zaxis represents the direction
							// of the shake. This direction represents the side in this case. This corrects this so that the data 
							// in sorcery is interpreted correctly.
							shakeTransform = Maths::HMat33(Maths::C_MatrixRotatePHR(Maths::HVec3(0.0f,Maths::PiBy2,0.0f))) * shakeTransform;
							pGraphicalActor->SetImpactEffect(pImpactType->mSideScreenEffect,shakeTransform);

							//ssend audio messages
							switch(impactLevel)
							{
								case E_IMPACT_LEVEL_MEDIUM:
									{
										Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eRGBMedium);
									}
									break;

								case E_IMPACT_LEVEL_HEAVY:
									{
										Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eRGBHard);
									}
									break;


							}
						}
					}
				}
			}
		}
	}

	void C_VehicleComponent_Damage::CheckJumpTheGun()
	{
		//Jump The Gun mod - restart quicker after being wrecked
		Game::C_MultiplayerVehicleAbility* jumpTheGunAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Jump_The_Gun);
		if(jumpTheGunAbility->GetActive())
		{
			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Jump_The_Gun, true);

			if(g_DebugShowMpAbilityActivateSpam)
			{
				LOGDEBUG(g_MultiplayerLog, "Vehicle %d's Jump The Gun ability has been activated!", mVehicleIndex);
			}

			//Apply multiplier
			mWreckedTimer *= jumpTheGunAbility->GetValue();
		}
	}

	void C_VehicleComponent_Damage::CheckVampiricWreck()
	{
		// BUG: 37151
		// If we wrecked somebody in the same frame we oursleves got wrecked...  dont receive any health as it confuses lots of systems.
		// The camera will break off thinking we are wrecked, the player then gets healed so that the rest of the wrecked sequence does not continue...
		// and the player will then be stuck at the start of the wrecked sequence....   forever!. (Undead ftw)
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if(pActor && pActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED)
		{
			return;
		}

		// Vampiric Wreck mod, receive health for wrecking another player
		Game::C_MultiplayerVehicleAbility* vWreck = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Vampiric_Wreck);
		if(vWreck->GetActive())
		{
			RepairVehicle(GetAFractionOfAHealthPipInDamagePoints(vWreck->GetValue()));

			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Vampiric_Wreck, true, false, mVehicleIndex);
#ifndef CFG_GOLD
			if(g_DebugShowMpAbilityActivateSpam)
			{
				LOGDEBUG(g_MultiplayerLog, "Vehicle %d's Vampiric Wreck ability has been activated!", mVehicleIndex);
			}
#endif //CFG_GOLD
		}
	}


	void C_VehicleComponent_Damage::CheckBatteringRam(	Game::C_GameplayImpactMsg& gameplayImpactMsg,
														const Game::Actors::C_VehicleActor *pVehicleActorA, 
														const Game::Actors::C_VehicleActor *pVehicleActorB)
	{
		if(!ValidVehicleToVehicleCollision(	pVehicleActorA, pVehicleActorB))
			return;

		const Game::Actors::C_VehicleActor *pMyVehicleActor = NULL;
		const Game::Actors::C_VehicleActor *pOtherVehicleActor = NULL;

		if(pVehicleActorA->GetVehicleIndex() == mVehicleIndex)
		{
			pMyVehicleActor = pVehicleActorA;
			pOtherVehicleActor = pVehicleActorB;
		}
		else
		{
			pMyVehicleActor = pVehicleActorB;
			pOtherVehicleActor = pVehicleActorA;
		}
		const int otherVehicleIndex = pOtherVehicleActor->GetVehicleIndex();


		Game::C_LevelStateMultiplayerVehicleAbilities* pMyMPAbilities = mpLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();

		Game::C_LevelStateMultiplayerVehicleAbilities* pOtherMPAbilities = mpLevelState->mCars[otherVehicleIndex].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* pOtherBatteringRam = pOtherMPAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Battering_Ram);

		if( ShouldBatteringRamFire(pMyVehicleActor, pOtherVehicleActor) )
		{
			//Batter that ram!
			Game::C_CachedMPAbility ability(pMyMPAbilities, UI::E_MultiplayerVehicleAbilities_Battering_Ram, true, false, mVehicleIndex, pOtherVehicleActor->GetVehicleIndex());
			m_CachedAbilities.PushBack(ability);
		}
		//If we're the victim, usually hit by a remote vehicle, work out locally whether we think we've been hit (rather than sync across the network which is flaky at best)
		else if( ShouldBatteringRamFire(pOtherVehicleActor, pMyVehicleActor) )
		{
			Game::C_CachedMPAbility ability(pOtherMPAbilities, UI::E_MultiplayerVehicleAbilities_Battering_Ram, true, false, otherVehicleIndex, mVehicleIndex);
			m_CachedAbilities.PushBack(ability);

			//See if we've got stable frame on or shield
			// Check for Stable Frame mod here
			Game::C_MultiplayerVehicleAbility* pMyStableFrame = pMyMPAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Stable_Frame);
			if(pMyStableFrame->GetActive())
			{
				//No punishment or damage
				Game::C_CachedMPAbility ability(pMyMPAbilities, UI::E_MultiplayerVehicleAbilities_Stable_Frame, true, false, mVehicleIndex, -1);
				m_CachedAbilities.PushBack(ability);

				if(g_DebugShowMpAbilityActivateSpam)
				{
					LOGDEBUG(g_MultiplayerLog, "C_VehicleComponent_Damage::CheckDamageModifiers() > Vehicle %d's Stable Frame ability has been activated, reducing damage!", mVehicleIndex);
				}
			}
			else
			{
				//Do some punishment here
				ApplyVehicleToVehicleBatteringRamPhysicsEffect(mVehicleIndex, otherVehicleIndex);

				if(mBatteringRamCoolDownTimer <= 0.f)
				{
					//Do some damage
					const int damageDoneByBatteringRam = GetAFractionOfAHealthPipInDamagePoints(pOtherBatteringRam->GetValue());
					ApplyDamage( damageDoneByBatteringRam, otherVehicleIndex);

					mBatteringRamCoolDownTimer = mBatteringRamModData.mCoolDownTimer;
				}
			}
		}
	}



	void C_VehicleComponent_Damage::ActivateLastGasp()
	{
		Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = mpRaceLogicLayer->GetLogicVehicleActor(mVehicleIndex);

		Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = mpLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();

		BIZ_ASSERT_MSG(pVehicleLogicActor, "Could not get the vehicle logic actor in Last Gasp Mod!");	

		Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);
			
		pAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Last_Gasp, true, !Game::VehicleIsLocal(mVehicleIndex), mVehicleIndex);

		// network vehicles have a bit of trouble firing this one off and looping the vehicle effects.. lets be nice and give them a helping hand.
		const Game::C_NetGameVars& netGameVars	= C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		netGameVars.m_EventManager->Add_AbilityActivated(mpLevelState->mCars[mVehicleIndex].mNetJoinId, -1, UI::E_MultiplayerVehicleAbilities_Last_Gasp, false);

		// Repair vehicle half a pip here
		RepairVehicle(GetAFractionOfAHealthPipInDamagePoints(0.5f)); //repairing half a pip so will ALWAYS DIE in one more hit. 

		// Send some magic 'last gasp mode active' message to HUD.lua to show timer if necessary
		_UI_MESSAGE( UI::E_UiGameMsg_PrimeLastGasp );

		// Disable players powerup bay so they cant fire any shunts/bargest/mines etc
		Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if(vehActor)
		{
			const float E_PERMANANT_DISABLE = 999999.0f;
			C_VehicleLogicMsg_EnableDisablePickupSystem disablePickupsMsg(mVehicleIndex,E_PERMANANT_DISABLE, 0.0f, false);
			vehActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &disablePickupsMsg);

			// Flag actor as being in last gasp mode
			vehActor->SetIsInLastGaspMode(true);
		}

		// set flag so we dont use it again
		mLastGaspModData.mHealthReplenishedThisLife = true;

		// Setup timers
		mLastGaspModData.SetupTimers(); 

		// Reset the 'have exploded' flag
		mLastGaspModData.mExplosionFiredThisLife = false;

		// Store index of player (if any.. could be -1) that damaged us enough to enter last gasp mode. They should get a wreck when we detonate!
		if(pVehicleLogicActor && pVehicleLogicActor->mScoringInfo.mWreckedTimer > 0.f && pVehicleLogicActor->mScoringInfo.mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX)
		{
			mLastGaspModData.mVehicleCausingLastGaspID = vehActor->mScoringInfo.mMostRecentVehicleIdxHitBy;
		}
		else
		{
			mLastGaspModData.mVehicleCausingLastGaspID = -1; // We must have killed ourselves *facepalm*
		}

		// The last gasp wrecked impulse hasn't been applied this life (yet)
		mLastGaspModData.mHasAppliedWreckedImpulse = false;

		// Apply any camera manipulation (e.g. locking selection, forcing chase etc)
		ApplyLastGaspCameraManipulation(); 

		// Make sure we remove the 'low health' flashing red screen effect so we can see the awesome countdown fx!
		HideLowHealthScreenEffect(); 


		Game::Actors::C_VehicleActor* pVehicleActor = pVehicleLogicActor->GetGraphicalActor(mVehicleIndex);
		if(pVehicleActor && pVehicleActor->IsFocusVehicle())
			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_LastGaspInitiatedActionIn );

	}

	void C_VehicleComponent_Damage::HideLowHealthScreenEffect()
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if(pActor)
		{
			Game::Actors::C_VehicleActor* pVehicleActor = pActor->GetGraphicalActor(mVehicleIndex);
			const RLDamageSystem* pRLDamageSystem		 = pActor->GetRLC()->GetRLSystem()->mDamageSystem;
			if (pVehicleActor && pVehicleActor->IsFocusVehicle())
			{
				pVehicleActor->SetHealthEffect(pRLDamageSystem->mHealthZones[pRLDamageSystem->mHealthZones.count-1]->mScreenEffect);
			}
		}
	}

	void C_VehicleComponent_Damage::ApplyLastGaspCameraManipulation()
	{
		// Lock player to bumpercam	
		mLastGaspModData.mStartingCamType = AmaxCam::E_VFCamera_Hollywood; //default

		Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if(vehActor)
		{
			mLastGaspModData.mStartingCamType = vehActor->GetGraphicalActor(mVehicleIndex)->GetCameraType( mVehicleIndex );
		}

		// Lets force the player into chase cam if they are not already
		bool isBumper( mLastGaspModData.mStartingCamType == AmaxCam::E_VFCamera_Bumper );
		if(isBumper)
		{
			SwitchLastGaspCameraType(AmaxCam::E_VFCamera_Hollywood);		
		}
		SetPlayerCameraLock(true);				    // and prevent them selecting anything till we are done (remember to unlock when we are done!)
	}

	void C_VehicleComponent_Damage::RemoveLastGaspCameraManipulation()
	{
		// Re-enable the palyer's ability to select camera.
		AmaxCam::CameraSystem().VehicleFeed(0)->SetChangeViewAllowed(true);

		// If player was in bumper cam before, and we forced them to chase, restore their previous choice
		AmaxCam::E_VFCamera prevCamType = mLastGaspModData.mStartingCamType; 
		if( prevCamType == AmaxCam::E_VFCamera_Bumper)
		{
			AmaxCam::CameraSystem().VehicleFeed(0)->ChangeCamera(prevCamType); 
		}
	}

	void C_VehicleComponent_Damage::SwitchLastGaspCameraType(const AmaxCam::E_VFCamera toType)
	{
		AmaxCam::CameraSystem().VehicleFeed(0)->ChangeCamera(toType,0.5f); // 0.5 ICK .. seems to be a magic number in vehiclefeed.cpp :S used for all camera transitions
	}

	void C_VehicleComponent_Damage::SetPlayerCameraLock(const bool isLocked)
	{
		 // Get the vehicle feed for primary player's Viewport and lock it. 
		 AmaxCam::CameraSystem().VehicleFeed(0)->SetChangeViewAllowed(!isLocked);
	}

	void C_VehicleComponent_Damage::GetDamageInflicted(const C_VehicleLogicMsg_HitByPerkMsg* pHitByPerkMessage, int& damage)
	{			
		BIZ_ASSERT_MSG(pHitByPerkMessage, "C_VehicleComponent_Damage::GetDamageInflicted() > pHitByPerkMessage is empty...this is BAD!!! Naughty function call, naughty!");
		
		const E_PICKUP_TYPE ePickupType = pHitByPerkMessage->mPickupType;
		const E_PICKUP_SUB_TYPE ePickupSubType = pHitByPerkMessage->mPickupSubType;
		const E_PICKUP_HIT_TYPE ePickupHitType = pHitByPerkMessage->mPickupHitType;
		const bool pickupTypeIsHarmful = !(ePickupType == E_PICKUP_TYPE_NITROUS || ePickupType == E_PICKUP_TYPE_REPAIR || ePickupType == E_PICKUP_TYPE_SHIELD);

		const RLPickupType* pPickup = (const RLPickupType*)::RLDatabase::Instance().GetPickupType(ePickupType);
		BIZ_ASSERT_MSG(pPickup, "C_VehicleComponent_Damage::GetDamageInflicted() > Couldn't get a pointer to the pickup from the RL Db");
		
		// reset our flag
		mLastDamageWasInflictedLastGasp = false;

		//TODO Refactor this so that we automatically get the correct balancing database from the RL Db
		//HACK - Special case for mini
		if(ePickupType == E_PICKUP_TYPE_BOMB && ePickupSubType == E_PICKUP_SUB_TYPE_MINI)
		{
			damage = pPickup->mMini->mBalancingData->GetDamageInflicted();
		}
		else if(ePickupSubType == E_PICKUP_SUB_TYPE_DECOY)
		{
			Game::C_LevelStateMultiplayerVehicleAbilities* pFirerMultiplayerAbilities = mpLevelState->mCars[pHitByPerkMessage->mFiringVehicleIndex].mUpgrades.GetMultiplayerAbilities();
			if(pFirerMultiplayerAbilities)
			{
				damage = (int)(pFirerMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Decoy_Drop)->GetValue());
			}
		}
		else if(ePickupType == E_PICKUP_TYPE_NITROUS && ePickupSubType == E_PICKUP_SUB_TYPE_REVERSED && pHitByPerkMessage->mPickupHitType == E_PICKUP_HIT_TYPE_MINOR)
		{
			damage = pPickup->mReversed->mBalancingData->GetDamageInflicted();
		}
		else
		{
			damage = pPickup->mNormal->mBalancingData->GetDamageInflicted();
		}

		if(pHitByPerkMessage->mPickupHitType == E_PICKUP_HIT_TYPE_MINOR && (ePickupType == E_PICKUP_TYPE_SHUNT || ePickupType == E_PICKUP_TYPE_SHOCK) )
		{
			damage /= 2;
		}

		//If the pickup that hit us is harmful (don't want to add damage to helpful ones!)
		MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
		if( pickupTypeIsHarmful )
		{
			if( ePickupType == E_PICKUP_TYPE_BLINDFIRE && pConfig)
			{
				//For blindfire, we only wish to damage someone extra on the third hit they receive (from any source)
				mTotalBlindFireImpacts++;
				
				if( pConfig->m_MiscModSettings.m_IronFistNumBoltsRequired > 0 &&
					mTotalBlindFireImpacts % pConfig->m_MiscModSettings.m_IronFistNumBoltsRequired == 0)
				{
					CheckPowerUpDamageModifiers(pHitByPerkMessage->mFiringVehicleIndex, damage, ePickupType);
				}
			}
			else
			{
				//Now check to see if any mods we've got installed will change the value
				CheckPowerUpDamageModifiers(pHitByPerkMessage->mFiringVehicleIndex, damage, ePickupType);
			}

			if(mpLevelState->mDamageMode == E_DAMAGE_MODE_MULTIPLIER)
			{
				damage *= mpLevelState->mDamageMultiplier;
			}
		}

		//Special case for the last gasp mod
		if( ePickupType == E_PICKUP_TYPE_BARGE && ePickupSubType == E_PICKUP_SUB_TYPE_MOD )
		{
			MultiplayerConfig* pMultiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
			BIZ_ASSERT_MSG(pMultiplayerConfig, "Could Not Get Multiplayer Config for Last Gasp Mod");
			LastGaspModSettings& modSettings = pMultiplayerConfig->m_LastGaspModSettings;

			damage = modSettings.m_bargeDamage;

			// Make sure that if this wrecks us.. we know it was a last gasp!
			mLastDamageWasInflictedLastGasp = true;
		}
	}


	// -------------------------------------------------------------------------------------------

	bool C_VehicleComponent_Damage::OnMessage(const Messaging::C_MessageBase *pMessage) 
	{
		PERF_EVENT("C_VehicleComponent.Damage.MessageHandler");

		switch(pMessage->Type())
		{

		case Game::E_GameflowMessage_RaceStarted:
			{
				mEnabled = true;
				return true;
			}
			break;

		case E_VehicleLogicMsg_ForceDamage:
			{
				const C_VehicleLogicForceDamageMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicForceDamageMsg>(pMessage);

				ApplyDamage(pMsg->mDamage, INVALID_VEHICLE_IDX);
				return true;
			}

		case E_VehicleLogicMsg_Repair:
			{
				const C_VehicleLogicRepairMsg* pMsg		= Messaging::MessageCast<C_VehicleLogicRepairMsg>(pMessage);
				
				Game::C_StickerManager::Instance().MessageHandler(pMsg); // sticker manager wants to know about this, before the repairing!
				RepairVehicle(pMsg->mHealAmount);
				return true;
			}

		case Game::E_GameLogicMessage_Impact:
			{
				Game::C_ImpactMsg* pMsg = const_cast<Game::C_ImpactMsg*>(Messaging::MessageCast<Game::C_ImpactMsg>(pMessage));

				HandleImpact(pMsg);
				return true;
			}

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);

				if( mVehicleIndex == msg->mVehicleIndex )
				{
					int damage = 0;
					if(mpLevelState->mDamageMode == E_DAMAGE_MODE_ONE_SHOT_KILL)
						damage = ((Game::Actors::C_VehicleLogicActor*)GetActor())->GetMaxHealth();
					else
					{
						GetDamageInflicted(msg, damage);

						//seems to be the nicest way to let the sticker manager know about this stuff!
						Game::C_StickerManager::Instance().HandleDamageByPickup( msg->mVehicleIndex, msg->mFiringVehicleIndex, damage );
					}

					if(damage >= 0)
					{
						ApplyDamage(damage, msg->mFiringVehicleIndex);

						HandlePickupDamage(msg->mPickupHitType, msg->mPickupType);

						Game::Actors::C_VehicleLogicActor* pActor		= ((Game::Actors::C_VehicleLogicActor*)GetActor());
						Game::Actors::C_VehicleActor* pGraphicalActor	= pActor->GetGraphicalActor(mVehicleIndex);
						const RLDamageParameters* pRLDamageParameters	= pActor->GetRLC()->GetRLSystem()->mDamageSystem->mParameters;
						mCooldown = pRLDamageParameters->mCooldownPerk;

						// Apply the camera effects
						if(pGraphicalActor->IsFocusVehicle())
						{
							const RLDamageSystem* pRLDamageSystem	= mpRLC->GetRLSystem()->mDamageSystem;

							if(pRLDamageSystem->mImpactTypes.count > E_IMPACT_LEVEL_HEAVY)
							{
								const RLImpactType* pImpactType	= pRLDamageSystem->mImpactTypes[E_IMPACT_LEVEL_HEAVY];

								if(damage < pRLDamageParameters->mHealthIncrement)
									pImpactType	= pRLDamageSystem->mImpactTypes[E_IMPACT_LEVEL_MEDIUM];

								if(pImpactType)
								{
									Maths::HMat33 shakeTransform = CalculateImpactShakeTransform(pGraphicalActor);
									pGraphicalActor->SetImpactEffect(pImpactType->mFrontScreenEffect,shakeTransform, true);
								}
							}
						}
					}

					//Check to see if we've got any modifiers to apply 
					CheckGameplayModifiers(msg->mPickupType, msg->mPickupSubType, msg->mFiringVehicleIndex);
				}

				return true;
			}

		case ActorMsg::E_DynamicObjMsg_SetVehicleLogic:
			{
				const ActorMsg::C_SetVehicleLogicMsg* pMsg = Messaging::MessageCast<ActorMsg::C_SetVehicleLogicMsg>(pMessage);
				mEnabled = pMsg->m_Damage;			
				return true;
			}
		case Game::E_VehicleMessage_CancelWreckedTimer:
			{
				const Game::Actors::C_CancelWreckedTimerMsg* pMsg = Messaging::MessageCast<Game::Actors::C_CancelWreckedTimerMsg>(pMessage);
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					HandleCancelWreckedTimer();
				}
				return true;
			}
		case E_VehicleLogicMsg_HOGReset:
			{
				// Reset process is complete
				mResetInProgress = false;
				return true;
			}

		case E_VehicleLogicMsg_Wrecked:
			{
				const C_VehicleLogicMsg_WreckedMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_WreckedMsg>(pMessage);
				if(pMsg && pMsg->mWreckerIdx == mVehicleIndex)
				{
					CheckVampiricWreck();
				}
				return true;
			}
		}

		return I_GameActorComponent::OnMessage(pMessage);
	}

	// -------------------------------------------------------------------------------------------

	Maths::HMat33 C_VehicleComponent_Damage::CalculateImpactShakeTransform(Game::Actors::C_VehicleActor* vehActor) const
	{
		// We orient the impact shake in the direction of lost velocity.
		// So for example, if you drive head on into a wall. You lose all of your
		// zaxis velocity (in the local space of the car)
		const Maths::HMat43& vehicleMatrix = vehActor->GetMatrix();
		Maths::HVec3 linVelChange = vehActor->GetCollisionBody()->LinearVelocity() - mPrevLinearVelocity;
		
		// We are only interested in the velocity change in the world XZ plane.
		// We save movement in the Y axis for the landing effect.
		linVelChange.Y(0.0f);

		// Camera shakes in the FX system are specified in local space so 
		// transform everything relative to this.
		Maths::HVec3 linVelChangeLS = linVelChange * Maths::HMat33(vehicleMatrix.InverseRotTrans());
		Maths::HMat33 shakeTransform;
		if(linVelChangeLS.LengthSquared() > 0.01f)
		{

			Maths::HVec3 toz = linVelChangeLS * -1.0f;
			toz.Normalise();
			Maths::HVec3 tox = Maths::Constants<Maths::HVec3>::UnitAxisPositiveY() ^ toz;
			if(tox.LengthSquared() > 0.1f)
			{
				tox.Normalise();
			}
			else
			{
				tox = Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ();
			}
			Maths::HVec3 toy = toz ^ tox;
			toy.Normalise();

			shakeTransform.AX(tox);
			shakeTransform.AY(toy);
			shakeTransform.AZ(toz);
		}
		else
		{
			shakeTransform.SetIdentity();
		}
		return shakeTransform;
	}

	bool C_VehicleComponent_Damage::IsFrontalImpact(const Game::C_ImpactMsg* pMsg, const Game::Actors::C_VehicleActor* vehActor) const
	{


		// Logic for determining if we are a frontal impact. (As opposed to a side impact)
		// In the XZ plane, if the angle between the two vectors...
		// a) The vehicles z-axis
		// b) The direction representing the change in velocity as a result of the impact
		// ... is greater than the threshold set by g_debugFrontalCollisionAngle
		// then we are considered to be colliding frontally rather than the side.  

		// Of course this only holds up when the car is reasonably upright.
		// We have an initial test which checks this and returns false if
		// the vehicle is not what we consider to be 'upright'
		const Maths::HMat43& vehicleMatrix = vehActor->GetMatrix();

		if( DotProduct( vehicleMatrix.AY(), Maths::Constants<Maths::HVec3>::UnitAxisPositiveY()) > g_debugFrontalCollisionUprightDot)
		{
			Maths::HVec3 currLinearVelocity = vehActor->GetCollisionBody()->LinearVelocity();
				
			Maths::HVec3 linVelChange = currLinearVelocity - mPrevLinearVelocity;
			Maths::HVec3 linVelChangeLSXZ = linVelChange * Maths::HMat33(vehicleMatrix.InverseRotTrans());
			linVelChangeLSXZ.Y(0);
			
			if(linVelChangeLSXZ.LengthSquared() > 0.01f)
			{
				linVelChangeLSXZ.Normalise();
				if( fabs(linVelChangeLSXZ.Z()) > g_debugFrontalCollisionDot )
				{
					return true;
				}
			}
		}
		return false;
	}

	void C_VehicleComponent_Damage::SendGameplayImpactMessageToComponents(const Game::C_GameplayImpactMsg& gameplayImpactMsg,Game::Actors::C_VehicleLogicActor* pActor)
	{
		Actor::C_ActorList* pActorList = pActor->GetActorList();
		Actor::C_ActorHandle actorHandle = pActor->GetHandle();

		//Send the impact message to the pickups component even for a insignificant impacts (for super shield)
		if( pActor->QueryInterface(E_IF_VehicleComponent_Pickups) )
		{
			pActorList->SendMessageToActorComponent(pActor->GetHandle(),E_IF_VehicleComponent_Pickups,&gameplayImpactMsg);
		}

		//We only want to send these one if its a significant impact
		if(gameplayImpactMsg.mImpactLevel > E_IMPACT_LEVEL_NONE)
		{
			if( pActor->QueryInterface(E_IF_VehicleComponent_Statistics) )
			{
				pActorList->SendMessageToActorComponent(actorHandle,E_IF_VehicleComponent_Statistics,&gameplayImpactMsg);
			}

			if( pActor->QueryInterface(E_IF_VehicleComponent_Rumble) )
			{
				pActorList->SendMessageToActorComponent(actorHandle,E_IF_VehicleComponent_Rumble,&gameplayImpactMsg);
			}

			if( pActor->QueryInterface(E_IF_VehicleComponent_Points) )
			{
				pActorList->SendMessageToActorComponent(actorHandle,E_IF_VehicleComponent_Points,&gameplayImpactMsg);
			}

			if( pActor->QueryInterface(E_IF_VehicleComponent_Challenges) )
			{
				pActorList->SendMessageToActorComponent(actorHandle,E_IF_VehicleComponent_Challenges,&gameplayImpactMsg);
			}

			if( pActor->QueryInterface(E_IF_VehicleComponent_NewScoring) )
			{
				pActorList->SendMessageToActorComponent(actorHandle,E_IF_VehicleComponent_NewScoring,&gameplayImpactMsg);
			}
			
			if( pActor->QueryInterface(E_IF_VehicleComponent_Abilities) )
			{
				pActorList->SendMessageToActorComponent(actorHandle,E_IF_VehicleComponent_Abilities,&gameplayImpactMsg);
			}
		}
	}

	void C_VehicleComponent_Damage::CacheMultiplayerAbilities(Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities)
	{
		mpMultiplayerAbilities = pAbilities;
	}	

// --------------------
// Last Gasp Mod Data
// --------------------
	void C_VehicleComponent_Damage::LastGaspModData::Initialise(Game::C_LevelState*	pLevelState, const U8 vehicleIndex)
	{
		//Initialise our variables here
		// Cache wether last gasp mod is in player's loadout
		mHasLastGaspModEquipped = 	false;
		mHasJumpTheGunModEquipped	=   false;
		Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = pLevelState->mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);		
		if(LastGaspAbility && LastGaspAbility->GetActive())
		{
			mHasLastGaspModEquipped = true;
		}
		Game::C_MultiplayerVehicleAbility* pJumpTheGunAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Jump_The_Gun);		
		if(pJumpTheGunAbility && pJumpTheGunAbility->GetActive())
		{
			mHasJumpTheGunModEquipped = true;
		}

		mHealthReplenishedThisLife = false;

		// Store any last gasp timer data
		MultiplayerConfig* pMultiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
		if(pMultiplayerConfig)
		{
			LastGaspModSettings& modSettings = pMultiplayerConfig->m_LastGaspModSettings;
			mForceFireTimer = false;
			if(modSettings.m_timerEnabled)
			{
				mForceFireTimer = true;
			}

			mWreckedSequenceDelay = modSettings.m_wreckedSequenceDelay;
		}
		else
		{
			mForceFireTimer = false;
			mWreckedSequenceDelay = 0.0f;
		}

		mExplosionFiredThisLife = false;
		mVehicleCausingLastGaspID = -1;
		mRemainingTimeTillForceFire = 0.0f;
		mNextSoundEffectIn			= 0.0f;
		mTimeSinceLastDamaged  = 0.0f;
		mInitialDisableTimer = 0.0f;
		mDamageCooldownTimer = 0.0f;

		mWreckedSequenceDelayed = false;
		mWreckedSequenceDelayTimer = 0.0f;
		mHasAppliedWreckedImpulse = false;

		BIZ_ASSERT_MSG(pMultiplayerConfig->m_MiscModSettings.m_IronFistNumBoltsRequired > 0,"C_VehicleComponent_Damage::GetDamageInflicted() < MPConfig m_IronFistNumBoltsRequired has been set to 0 in sorcery... this makes no sense!");
	}

	void C_VehicleComponent_Damage::LastGaspModData::SetupTimers()
	{
		// Set the force fire timer
		MultiplayerConfig* pMultiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
		if(pMultiplayerConfig)
		{
			// Get hold of the mod settings
			LastGaspModSettings& modSettings = pMultiplayerConfig->m_LastGaspModSettings;

			// Set the force fire timer if necessary
			if(mForceFireTimer)
			{
				LastGaspModSettings& modSettings = pMultiplayerConfig->m_LastGaspModSettings;
				mRemainingTimeTillForceFire = modSettings.m_timeTillForceFire;	
			}

			// Set the disable detonation timer
			mInitialDisableTimer = modSettings.m_detonateAllowedDelay;

			// Set the damage cooldown timer
			mDamageCooldownTimer = modSettings.m_damageCooldown;
		}
		else
		{
			mRemainingTimeTillForceFire	= 0.0f;
			mInitialDisableTimer		= 0.0f;
			mDamageCooldownTimer		= 0.0f;
		}
	}

	void C_VehicleComponent_Damage::LastGaspModData::UpdateTimers( bool& respawn, bool& fireLastGasp, const U8 vehicleIndex )
	{
		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		respawn = false;
		fireLastGasp = false;

		if( Game::VehicleIsLocal(vehicleIndex) )
		{
			UpdateTimers_Local(respawn, fireLastGasp, vehicleIndex, frameTime);
		}
		else
		{
			UpdateTimers_Remote(vehicleIndex, frameTime);
		}
	}

	void C_VehicleComponent_Damage::LastGaspModData::UpdateTimers_Local( bool& respawn, bool& fireLastGasp, const U8 vehicleIndex, const float frameTime )
	{
		// If we have fired last gasp recently and are delaying the wrecked sequence
		if(mWreckedSequenceDelayed)
		{
			// Is the delay over?
			mWreckedSequenceDelayTimer -= frameTime;
			if(mWreckedSequenceDelayTimer <= 0.0f)
			{
				// flag that we want to handle a respawn
				respawn = true;
			}
		}
		else
		{
			// Decrement time till last gasp force fire
			if(mForceFireTimer && mRemainingTimeTillForceFire > 0.0f)
			{
				// Decrement disable timer
				if(mInitialDisableTimer >= 0.0f)
				{
					mInitialDisableTimer -= frameTime;
				}

				if(mDamageCooldownTimer >= 0.0f)
				{
					mDamageCooldownTimer -= frameTime;
				}

				mRemainingTimeTillForceFire -= frameTime;
				if(mRemainingTimeTillForceFire < 0.0f)
				{
					// If hit zero.. then force fire
					fireLastGasp = true;
					
				}
				else
				{
					// make sure that the sfx are still playing!
					Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = Game::C_Game::GetInstance()->GetLevelState().mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
					Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);	
					pAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Last_Gasp, true, !Game::VehicleIsLocal(vehicleIndex), vehicleIndex, -1, false,1.0f, frameTime);
				}

				CheckSoundEffects(frameTime);
			}
		}
	}

	void C_VehicleComponent_Damage::LastGaspModData::UpdateTimers_Remote( const U8 vehicleIndex, const float frameTime )
	{
		if(mRemainingTimeTillForceFire > 0.0f)
		{
			Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = Game::C_Game::GetInstance()->GetLevelState().mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
			Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);	
			pAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Last_Gasp, true, false, vehicleIndex, -1, false,1.0f, frameTime );
		}

	}

	void C_VehicleComponent_Damage::LastGaspModData::CheckSoundEffects(const float frameTime)
	{
		mNextSoundEffectIn += frameTime;
   		if ( mNextSoundEffectIn >= 1.0f && mRemainingTimeTillForceFire >  1.0f)
		{
			if ( mRemainingTimeTillForceFire <= 4.0f)
				UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_CountDown03ActionIn );// play on 3,2,1 but not '0'
			else
				UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_CountDownActionIn );// play on 9->4 

			mNextSoundEffectIn -= 1.0f;
		}
	}

	void C_VehicleComponent_Damage::BatteringRamModData::Initialise(const MultiplayerConfig* pConfig)
	{
		BIZ_ASSERT_MSG(pConfig, "C_VehicleComponent_Damage::BatteringRamModData::Initialise() > Invalid pointer to the multiplayer config, talk to CA");

		//Get hold of the mod data
		const BatteringRamCollisionConsequences settings = pConfig->m_BatteringRamCollisionConsequences;

		mTargettingCone = settings.m_TargettingCone; 
		mMinimumImpactSpeedInMPH = settings.m_MinimumImpactSpeedInMPH;
		mCoolDownTimer = settings.m_CoolDownTimer;

		mLightHitSettings.mImpulseScale = Maths::HVec3(	settings.m_LightHitSettings.m_ImpulseScale.vx, 
														settings.m_LightHitSettings.m_ImpulseScale.vy, 
														settings.m_LightHitSettings.m_ImpulseScale.vz);
		mLightHitSettings.mKickForce = Maths::HVec3(settings.m_LightHitSettings.m_KickForce.vx, 
													settings.m_LightHitSettings.m_KickForce.vy, 
													settings.m_LightHitSettings.m_KickForce.vz);
		mLightHitSettings.mConstantKickForce = Maths::HVec3(settings.m_LightHitSettings.m_ConstantKickForce.vx, 
															settings.m_LightHitSettings.m_ConstantKickForce.vy, 
															settings.m_LightHitSettings.m_ConstantKickForce.vz);

		mHeavyHitSettings.mImpulseScale = Maths::HVec3(	settings.m_HeavyHitSettings.m_ImpulseScale.vx, 
														settings.m_HeavyHitSettings.m_ImpulseScale.vy, 
														settings.m_HeavyHitSettings.m_ImpulseScale.vz);
		mHeavyHitSettings.mKickForce = Maths::HVec3(settings.m_HeavyHitSettings.m_KickForce.vx, 
													settings.m_HeavyHitSettings.m_KickForce.vy, 
													settings.m_HeavyHitSettings.m_KickForce.vz);
		mHeavyHitSettings.mConstantKickForce = Maths::HVec3(settings.m_HeavyHitSettings.m_ConstantKickForce.vx, 
															settings.m_HeavyHitSettings.m_ConstantKickForce.vy, 
															settings.m_HeavyHitSettings.m_ConstantKickForce.vz);

		CopyToFakePhysics();
	}

	void C_VehicleComponent_Damage::BatteringRamModData::CopyToFakePhysics()
	{
		g_Perks_FakePhysics_HeavyBatteringRamJinkImpulseScale.SetValue(mHeavyHitSettings.mImpulseScale.X(), mHeavyHitSettings.mImpulseScale.Y(), mHeavyHitSettings.mImpulseScale.Z());
		g_Perks_FakePhysics_HeavyBatteringRamJinkKickForce.SetValue(mHeavyHitSettings.mKickForce.X(), mHeavyHitSettings.mKickForce.Y(), mHeavyHitSettings.mKickForce.Z());
		g_Perks_FakePhysics_HeavyBatteringRamJinkConstantKickForce.SetValue(mHeavyHitSettings.mConstantKickForce.X(), mHeavyHitSettings.mConstantKickForce.Y(), mHeavyHitSettings.mConstantKickForce.Z());

		g_Perks_FakePhysics_LightBatteringRamJinkImpulseScale.SetValue(mLightHitSettings.mImpulseScale.X(), mLightHitSettings.mImpulseScale.Y(), mLightHitSettings.mImpulseScale.Z());
		g_Perks_FakePhysics_LightBatteringRamJinkKickForce.SetValue(mLightHitSettings.mKickForce.X(), mLightHitSettings.mKickForce.Y(), mLightHitSettings.mKickForce.Z());
		g_Perks_FakePhysics_LightBatteringRamJinkConstantKickForce.SetValue(mLightHitSettings.mConstantKickForce.X(), mLightHitSettings.mConstantKickForce.Y(), mLightHitSettings.mConstantKickForce.Z());
	}


} // end namespace components

// -----
// DEBUG
// -----

Debug::Int	g_GameplayDamage("Vehicle.Components.Damage.DamageToApply", 1, Debug::Range(1,60));
Debug::Int	g_GameplayDamageVehicle("Vehicle.Components.Damage.VehicleIdx", 1, Debug::Range(1,20));

static void ApplyDamageDebug(void *)
{
	const Game::Actors::C_VehicleLogicActor* actor = (Game::Actors::C_VehicleLogicActor*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(g_GameplayDamageVehicle - 1);
	if(actor)
	{
		C_VehicleLogicForceDamageMsg forceDamageMsg(actor->GetVehicleIndex(), g_GameplayDamage);
		actor->GetActorList()->SendMessageToActorComponent(actor->GetHandle(),Components::E_IF_VehicleComponent_Damage,&forceDamageMsg);	
	}
}
static void RepairDamageDebug(void *)
{
	const Game::Actors::C_VehicleLogicActor* actor = (Game::Actors::C_VehicleLogicActor*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(g_GameplayDamageVehicle - 1);
	if(actor)
	{
		C_VehicleLogicRepairMsg repairMsg(g_GameplayDamageVehicle-1, g_GameplayDamage);
		actor->GetActorList()->SendMessageToActorComponent(actor->GetHandle(),Components::E_IF_VehicleComponent_Damage,&repairMsg);	
	}
}

Debug::Function				g_ApplyGameplayDamage("Vehicle.Components.Damage.ApplyDamage", ApplyDamageDebug, 0);
Debug::Function				g_RepairGameplayDamage("Vehicle.Components.Damage.RepairDamage", RepairDamageDebug, 0);
