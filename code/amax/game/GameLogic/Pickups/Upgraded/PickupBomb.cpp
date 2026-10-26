#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <GameLogic/Pickups/Upgraded/PickupBomb.h>
#include <GameLogic/Pickups/PickupClasses.h>
#include <GameLogic\MultiplayerUtilities.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>

#include "GameAudio/GameAudioLayer.h"

#include "GameAudio/PickupSubsystem.h"

#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <GameLogic\StickerManager.h>
#include <SystemProfile\SystemProfileManager.h>
#include <AI/AI_Amax.h>

Debug::Bool g_debugMineDraw("Perks.DebugDraw.Mine", false);
Debug::Bool g_ForceMasterMineOn("Perks.MasterMineBomb.ForceMasterMineOn", false);

Debug::Bool g_ForcePrime("Perks.MasterMineBomb.ForcePrime", false);
Debug::Float g_UnPrimeableTimer("Perks.MasterMineBomb.UnPrimeableTimer", 1.f, Debug::Range(0.f, 5.f));
extern Debug::Float g_MasterMinePrimeTimer;

namespace Game
{	
	C_PickupBomb::C_PickupBomb(S32 classType, U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, float timerOverride, const bool useMasterMineVersionOfTheMineSFX) 
		: C_PickupMine(classType, id, firingIdx, subType, noFiringVehicleFX)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BOMB);
		mpBombData							= (RLBombBalancingData*)pPickup->mNormal->mBalancingData;
		mpBombSFX							= (RLBombSpecialFX*)pPickup->mNormal->mSpecialFX;;

		BIZ_ASSERT(mpBombData);
		BIZ_ASSERT(mpBombSFX);

		mInnerCollisionRadius	= mpBombData->mRadius;
		mOuterCollisionRadius	= mpBombData->mOuterRadius;
		
		mMasterMineTimer = 0.f;
		mUnPrimeableTimer = g_UnPrimeableTimer;	//To help make sure the firing vehicle doesn't prime the mine
		mPrimed = false;
		mMasterMineEquipped = useMasterMineVersionOfTheMineSFX;
		mVehicleIndexWhoPrimedTheMine = -1;

		mBehindDistance = mpBombData->mPlaceBehindDistance;

		timerOverride == -1.f ? mTimer = mpBombData->mLifespan : mTimer = timerOverride;

		RLPickupLevel* pLevel = GetRLSubTypeLevel();
		if(pLevel)
		{
			if(!mNoFiringVehicleFX && pLevel->mSpecialFX)
			{
				C_InGameSpecialFXManager::Instance().CreateSpecialFX(pLevel->mSpecialFX->mChargingSpecialFX, mMatrix, firingIdx, -1, id, E_InGameSpecialFX_Trigger_Charging);
			}
		}

		mAITrackableData.mRadius = mInnerCollisionRadius;

		mTimeConsideredRecentlyComeToRest	= C_AchievementData::Instance().GetMiscStickerData().mTimeLobbedConsideredRecentlyComeToRest; 
		mRecentlyComeToRestTimer			= 0.0f;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		if( HasAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist, mFiringIdx) && AbilityActiveOnPickupType(UI::E_MultiplayerVehicleAbilities_Iron_Fist) )
		{
			if(mpBombSFX)
			{
				C_PickupBase::PostDelayInitialise(matrix, &mpBombSFX->mIronFistFiredSpecialFX);
			}
		}
		else
		{
			C_PickupBase::PostDelayInitialise(matrix, sfx);
		}

		if(VehicleIsLocal(mFiringIdx))
		{
			ApplyMatrixModifiers(matrix);
			SetPostDelayMatrix(matrix);
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, matrix);
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::SetPostDelayMatrix( Maths::HMat43 matrix, float fVal )
	{
		C_PickupBase::SetPostDelayMatrix(matrix, fVal);
		CreateCollisionBodies();

		XtArray<RLPickupSpecialFX> mineSFX;
		if(!mMasterMineEquipped && !g_ForceMasterMineOn)
		{
			mineSFX = mpBombSFX->mMineSpecialFX;
		}
		else
		{
			mineSFX = mpBombSFX->mMasterMineEquipedSpecialFX;
		}
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mineSFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Firing);
	
		// get and store the track data for the mine...
		airacingtech::ITrackDataAI *pTrackDataSystem = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
		airacingtech::ISplineTracker *pFiringSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(mFiringIdx);
		if (pTrackDataSystem && pFiringSplineTracker)
		{
			const airacingtech::RouteInfo *pLogicRoute = pFiringSplineTracker->GetLogicRoute();
			if (pLogicRoute)
			{
				mAITrackableData.mAILogicIndex = pLogicRoute->m_index;

				// this is the starting point for the search....
				int startIndex = pLogicRoute->m_pRoute->GetIndexOffset(mAITrackableData.mAILogicIndex, -10);
				int endIndex = pLogicRoute->m_pRoute->GetIndexOffset(mAITrackableData.mAILogicIndex, 50);				
				float paramDist = 0.0f;
				Maths::HVec3 positionBomb(mMatrix.AW());
				mAITrackableData.mAILogicIndex =  pLogicRoute->m_pRoute->FindNearest( startIndex, endIndex, positionBomb, paramDist, pLogicRoute->m_pRoute->GetStepSize());
				if (mAITrackableData.mAILogicIndex == -1)
				{
					mAITrackableData.mAILogicIndex = pLogicRoute->m_index;
				}
			
			}
		}
	}

	// --------------------------------------------------------------------------------
	void C_PickupBomb::FlagAsLobbedMineRecentlyComeToRest()
	{
		mRecentlyComeToRestTimer = mTimeConsideredRecentlyComeToRest;
	}

	bool C_PickupBomb::GetHasBeenRecentlyLobbed() const
	{
		return mRecentlyComeToRestTimer > 0.0f;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupBomb::Update()
	{
		if(!mActivated)
			return false;

		if(g_debugMineDraw)
			DebugDraw();

		if(g_ForcePrime)
		{
			if(!mPrimed)
			{
				Prime(-1);
			}	
		}

		float frameTime				= C_GameApp::GetInstance()->GetTimeStep();

		if(GetHasBeenRecentlyLobbed())
		{
			mRecentlyComeToRestTimer -= frameTime;
			if(mRecentlyComeToRestTimer < 0.0f)
			{
				mRecentlyComeToRestTimer = 0.0f;
			}
		}

		UpdateCollisionBodies();

		mTimer -= frameTime;
		bool priming = (mPrimed && mMasterMineTimer > 0.f);
		if(!priming && (mTimer <= 0.f || mKillPickup))
		{
			if(mIsBeingReplaced)
			{
				mAutopsyReport.mCauseOfDeath = E_PICKUP_CAUSE_OF_DEATH_REPLACED;
				C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);
			}
			else
			{
				Explode();
			}

			//kill audio
			C_VehicleLogicMsg_PerkEnded pMsg(mFiringIdx,mUniqueId,E_PICKUP_TYPE_BOMB,Game::E_BodyUserDataType_Actor,mPickupSubType);
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);

			return true;
		}	

		//Update the unprimeable timer
		if(mUnPrimeableTimer > 0.f)
		{
			UpdateUnPrimeableTimer(frameTime);
		}

		//See if we're converting to a set of mini mines
		UpdateMasterMineTimer(frameTime);
		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::UpdateUnPrimeableTimer(const float elapsedTime)
	{
		if( (mUnPrimeableTimer - elapsedTime) <= 0.f)
		{
			mUnPrimeableTimer = 0.f;
		}
		else
		{
			mUnPrimeableTimer -= elapsedTime;
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::UpdateMasterMineTimer(const float elapsedTime)
	{
		if(!mPrimed)
			return;

		mMasterMineTimer -= elapsedTime;
		if(mMasterMineTimer <= 0.f)
		{
			S8 vehicleIndexToUse = -1;
			if(mVehicleIndexWhoPrimedTheMine >= 0)
			{
				vehicleIndexToUse = mVehicleIndexWhoPrimedTheMine;
			}
			else
			{
				vehicleIndexToUse = mFiringIdx;
			}
			

			//Kill the old bomb
			C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);
			C_VehicleLogicMsg_PerkEnded pMsg(vehicleIndexToUse,mUniqueId,E_PICKUP_TYPE_BOMB,Game::E_BodyUserDataType_Actor,E_PICKUP_SUB_TYPE_MOD);
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);
			KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_REPLACED);
			
			//Create a new bomb
			PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BOMB, E_PICKUP_SUB_TYPE_MOD, vehicleIndexToUse);
			pickupInfo.mTargetIdx			= -1;
			pickupInfo.mMatrix				= mMatrix;
			pickupInfo.mDisableVehicleFX	= true;
			pickupInfo.mOverrideMatrix		= true;
			pickupInfo.mVal3				= (U32)Time::GetCurrentTime();

			C_PickupsManager::Instance().CreatePickup(pickupInfo);
			mMasterMineTimer = 0.f;

			if(mVehicleIndexWhoPrimedTheMine >= 0)
			{
				//Turn off the ability of the firing vehicle
				SetMasterMineFiring(mVehicleIndexWhoPrimedTheMine, false);
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mMatrix.AW(),1.f,Maths::RGBA(255,0,0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mInnerCollisionRadius,Maths::RGBA(255,255,0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mOuterCollisionRadius,Maths::RGBA(100,100,0));
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::Trigger(int idx)
	{
		if(CheckCanHit(idx))
		{
			C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			if (pLayer)
			{
				Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(idx);
				if (pActor)
				{
					Maths::HMat43 mat = pActor->GetGraphicalActor(idx)->GetMatrix();
					mHitInfo.mPosition = mMatrix.AW();
					mHitInfo.mDirection = mat.AW() - mPrevPosition;
					mHitInfo.mDirection.Normalise();
					mHitInfo.mPosition = mHitInfo.mPosition + mHitInfo.mDirection * mInnerCollisionRadius;
					mHitInfo.mDirection = mHitInfo.mDirection * mat.Rotation().Inverse();
				}
			}
			RegisterHit(idx);

			// Store any data specific to this type of death
			mAutopsyReport.mVehicleIdHit	   =  idx;

			// Do the killing
			KillPickup(-1,E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE);

			// if the firing vehicle is local... and didnt hit own mine
			if( VehicleIsLocal(mFiringIdx) && idx != mFiringIdx &&
				SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(mFiringIdx))
			{
				if(GetHasBeenRecentlyLobbed())
				{
					// If this mine has recently come to rest... tell the stickers about it!
					Game::C_Sticker* pSticker = Game::C_StickerManager::Instance().GetSticker(BLUR_STICKER_MINE_WITH_LOBBED);
					pSticker->AddProgress();
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	int C_PickupBomb::GetControllerIndex(const int idx)
	{
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

		if(levelState.mNumCars > (U32)idx && levelState.mCars[idx].mHuman && levelState.mCars[idx].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
			return levelState.mCars[idx].mController;

		return -1;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::Prime(int vehicleIndex)
	{
		if(!VehicleIsLocal(vehicleIndex))
			return;

		//Make sure we're not primed already
		if(mPrimed)
			return;
		//Make sure that the person who fired the bomb isn't also trying to prime it within an initial time limit(bug# 31665) 
		else if(mUnPrimeableTimer > 0.f && vehicleIndex == mFiringIdx)
			return;

		if(vehicleIndex < 0)
		{
			//Set timer here 
			mPrimed = true;
			mMasterMineTimer = mpBombData->mMasterMinePrimeTimer;
			return;
		}
		else
		{
			//Check to see if the potential victim has Mastermine mod
			Game::C_LevelState& levelState								= C_Game::GetInstance()->GetLevelState();
			Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities	= levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
			BIZ_ASSERT_MSG(pAbilities, "C_PickupBomb::Prime() > Couldn't get a pointer to the MultiplayerAbilities :-( KAPPPPOW!!!");

			Game::C_MultiplayerVehicleAbility* pVictimsMasterMineMod	= pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Mastermine);
			BIZ_ASSERT_MSG(pVictimsMasterMineMod, "C_PickupBomb::Prime() > Couldn't get a pointer to the Mastermine :-( KAPPPPOW!!!");

			//See if the victim's got the master mine mod loaded
			if( (pVictimsMasterMineMod && pVictimsMasterMineMod->GetActive()) || g_ForceMasterMineOn)
			{
				//Set timer here 
				mPrimed = true;
				mMasterMineTimer = mpBombData->mMasterMinePrimeTimer;
				mVehicleIndexWhoPrimedTheMine = vehicleIndex;

				//Funky parameter for the set ability firing message
				const bool isNotLocal = !VehicleIsLocal(mVehicleIndexWhoPrimedTheMine);

				pAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Mastermine, true, isNotLocal, mVehicleIndexWhoPrimedTheMine);

				//Now set off the SFX
				if(mVehicleIndexWhoPrimedTheMine >= 0)
				{
					Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(	mpBombSFX->mMasterMinePrimedSpecialFX, 
																				Maths::HMat43(Maths::HMat33(), mMatrix.AW()), 
																				mVehicleIndexWhoPrimedTheMine, 
																				-1, 
																				-1, 
																				Game::E_InGameSpecialFX_Trigger_PickedUp);
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::Explode()
	{
		// Remove the mine and play the boom effect
		C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpBombSFX->mExplodeSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Firing);
	}

	// --------------------------------------------------------------------------------

	void C_PickupBomb::SetMasterMineFiring(const U8 vehicleIndex, const bool firing)
	{
		//Funky parameter for the set ability firing message
		const bool isNotLocal = !VehicleIsLocal(vehicleIndex);

		Game::C_LevelState& levelState								= C_Game::GetInstance()->GetLevelState();
		Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities	= levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
		BIZ_ASSERT_MSG(pAbilities, "C_PickupBomb::SetMasterMineFiring() > Couldn't get a pointer to the MultiplayerAbilities :-( KAPPPPOW!!!");

		Game::C_MultiplayerVehicleAbility* pMasterMineMod	= pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Mastermine);
		BIZ_ASSERT_MSG(pMasterMineMod, "C_PickupBomb::SetMasterMineFiring() > Couldn't get a pointer to the Mastermine :-( KAPPPPOW!!!");

		//See if the vehicle's got the master mine mod loaded
		if( (pMasterMineMod && pMasterMineMod->GetActive()) || g_ForceMasterMineOn)
		{
			pAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Mastermine, firing, isNotLocal, vehicleIndex);
		}
	}

	// --------------------------------------------------------------------------------
}
