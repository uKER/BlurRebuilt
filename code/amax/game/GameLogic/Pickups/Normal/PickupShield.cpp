#include "stdafx.h"

#include <GameLogic/Pickups/Normal/PickupShield.h>

#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameLogic/Pickups/PickupsManager.h>
#include <GameLogic/Pickups/PickupClasses.h>
#include <GameLogic/StickerManager.h>

Debug::Float g_debugShieldScaleX("Perks.Shield.ScaleTestX", 0.9f);
Debug::Float g_debugShieldScaleY("Perks.Shield.ScaleTestY", 1.0f);
Debug::Float g_debugShieldScaleZ("Perks.Shield.ScaleTestZ", 0.45f);

Debug::Float g_debugBumperTranslationX("Perks.Shield.BumperTranslationX", 0.f);
Debug::Float g_debugBumperTranslationY("Perks.Shield.BumperTranslationY", -0.65f);
Debug::Float g_debugBumperTranslationZ("Perks.Shield.BumperTranslationZ", 1.f);

Debug::Float g_debugShieldDamageTimeScale("Perks.Shield.DamageTimeScale", 0.02f);

#include "GameAudio/GameAudioLayer.h"
#include "GameAudio/PickupSubsystem.h"
#include "Layers/AmaxWorldLayer.h"
extern Debug::Logger &g_MultiplayerLog;
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;


namespace Game
{
	C_PickupShield::C_PickupShield(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupBase(E_PickupClass_Shield, id, firingIdx, E_PICKUP_TYPE_SHIELD, subType, noFiringVehicleFX)
	{
		RLPickupType* pPickup	= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHIELD);
		mpShieldData			= (RLShieldBalancingData*)pPickup->mNormal->mBalancingData;

		if(subType == E_PICKUP_SUB_TYPE_DOUBLE_TAP)
			mpShieldData			= (RLShieldBalancingData*)pPickup->mDoubleTap->mBalancingData;

		BIZ_ASSERT(mpShieldData);

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringIdx);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringIdx);
		AmaxVehicle* pXtVehicle				= C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)pVehActor->GetXtIndex()];

		Maths::HVec3 extents = pVehActor->GetExtents();
		Maths::HVec3 scaleTest(	g_debugShieldScaleX * extents.X() * (pXtVehicle->m_ShieldSizeModifier.vx ? pXtVehicle->m_ShieldSizeModifier.vx : 1.f), 
								g_debugShieldScaleY * extents.Y() * (pXtVehicle->m_ShieldSizeModifier.vy ? pXtVehicle->m_ShieldSizeModifier.vy : 1.f),
								g_debugShieldScaleZ * extents.Z() * (pXtVehicle->m_ShieldSizeModifier.vz ? pXtVehicle->m_ShieldSizeModifier.vz : 1.f) );

		Maths::HVec3 offset;

		if(VehicleIsLocal(mFiringIdx))
		{
			offset = Maths::HVec3((F32)g_debugBumperTranslationX, (F32)g_debugBumperTranslationY, (F32)g_debugBumperTranslationZ);
		}
		
		mShieldModActive = mBossShieldModActive = false; 

		pActor->mShieldTime		= mpShieldData->mLifetime;
		pActor->mShieldHealth	= mpShieldData->mHealth;
		pActor->mDoubleShield	= false;

		ApplyShieldModMultipliers(pActor->mShieldTime, pActor->mShieldHealth);

		XtArray<RLPickupSpecialFX>* sfx = &((RLShieldSpecialFX*)pPickup->mNormal->mSpecialFX)->mShieldDomeSpecialFX;

		if(subType == E_PICKUP_SUB_TYPE_DOUBLE_TAP)
		{
			pActor->mDoubleShield = true;
			sfx = &((RLShieldSpecialFX*)pPickup->mDoubleTap->mSpecialFX)->mShieldDomeSpecialFX;
		}

		// Start the special effect, with the potentially modified shield duration. 
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(*sfx, mMatrix, firingIdx, -1, id, E_InGameSpecialFX_Trigger_Shield, pActor->mShieldTime, scaleTest, offset);

		//send end msg to audio
		//set fx duration envelope to audio
		C_VehicleLogicMsg_PowerUpFXDurations pMsg(mFiringIdx,mUniqueId,E_PICKUP_TYPE_SHIELD,Game::E_BodyUserDataType_Actor);

		for( int i = 0; i < sfx->count; i++ )
		{
			pMsg.mDurationIntro = (*sfx)[i].mDurationIntro.mEnd - (*sfx)[i].mDurationIntro.mStart;			
			pMsg.mDurationOutro = (*sfx)[i].mDurationOutro.mEnd - (*sfx)[i].mDurationOutro.mStart;

			pMsg.mDurationLoop = (pActor->mShieldTime - (pMsg.mDurationIntro + pMsg.mDurationOutro));
		}
		
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_SetFXDurations, 0, &pMsg);

		//send shield durations to vehicle audio component
		Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		if(pCore && pCore->GetVehicleActor(mFiringIdx)->QueryInterface(Components::E_AudioEntity_AdditionalSpawned))
			pCore->GetVehicleActor(mFiringIdx)->SendMessageToComponent(Components::E_AudioEntity_AdditionalSpawned,&pMsg);

		mGenerateNewShield = false;
		pActor->mLastShieldId = id;
	}

	bool C_PickupShield::Update()
	{
		PERF_EVENT("PickupShield.Update");

		if(!mActivated)
			return false;

		C_RaceLogicLayer* pLayer				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor		= pLayer->GetLogicVehicleActor(mFiringIdx);

		if(mKillPickup)
		{
			Game::C_InGameSpecialFXManager::Instance().SetState(mUniqueId, E_InGameSpecialFX_Mode_Outro);
			pActor->mShieldTime = 0.f;
			pActor->mShieldHealth = 0.f;

			//send end msg to audio
			C_VehicleLogicMsg_PerkEnded pMsg(mFiringIdx,mUniqueId,E_PICKUP_TYPE_SHIELD,Game::E_BodyUserDataType_Actor);
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);
			Game::C_StickerManager::Instance().MessageHandler(&pMsg); //sticker system would also like to know about this

			if(VehicleIsLocal(mFiringIdx) && mGenerateNewShield)
			{
				PickupCreationStruct pickupInfo(E_PICKUP_TYPE_SHIELD, mPickupSubType, mFiringIdx);
				pickupInfo.mDisableVehicleFX	= true;
				pickupInfo.mParentPickupId		= mUniqueId;

				C_PickupsManager::Instance().CreatePickup(pickupInfo);
			}

			return true;
		}

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		pActor->mShieldTime -= frameTime;

		if(VehicleIsLocal(mFiringIdx) && ((pActor->mShieldTime <= 0.f) || (pActor->mShieldHealth <= 0.f)))
		{
			KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_OLD_AGE);

			Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
			Game::C_LevelStateMultiplayerVehicleAbilities* pVehicleAbilities = levelState.mCars[mFiringIdx].mUpgrades.GetMultiplayerAbilities();

			if(mShieldModActive)
			{
				// Stop the mod activation
				pVehicleAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Shielding_Efficiency, false);
			}

			if(mBossShieldModActive)
			{
				pVehicleAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_DoubleShield, false, false, mFiringIdx);
			}
		}


		return false;
	}

	void C_PickupShield::Restart()
	{
		KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_RESET);
		mGenerateNewShield = true;
	}

	void C_PickupShield::ApplyShieldModMultipliers(float& shieldTime, float& shieldHits)
	{
		// If mod is activated.. then increase these by mutiplier
		Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
		Game::C_LevelStateMultiplayerVehicleAbilities* pVehicleAbilities = levelState.mCars[mFiringIdx].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* ability = pVehicleAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Shielding_Efficiency);
		if(ability->GetActive())
		{
			MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;

			// Activate the mod...
			shieldTime *= ability->GetValue();
			shieldHits += pConfig->m_MiscModSettings.m_ShieldingEfficiencyExtraHealth;

			// Show a UI effect
			pVehicleAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Shielding_Efficiency, true);

			// Output debug spam
			if(g_DebugShowMpAbilityActivateSpam)
			{
				LOGDEBUG(g_MultiplayerLog, "vehicle %d's Shielding Efficiency MOD was activated increasing Shield Hits/Duration", mFiringIdx);
			}

			mShieldModActive = true; 
		}

		Game::C_MultiplayerVehicleAbility* pBossAbility = pVehicleAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_DoubleShield);
		if(mPickupSubType == E_PICKUP_SUB_TYPE_DOUBLE_TAP && pBossAbility->GetActive()) // Double checking just in case something mental happens
		{
			pVehicleAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_DoubleShield, true, false, mFiringIdx);
			mBossShieldModActive = true;
		}
	}

	void C_PickupShield::PostDelayInitialise( Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx )
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);
		C_PickupBase::SetPostDelayMatrix(matrix);
	}


	void C_PickupShield::DebugDraw()
	{
	}
}
