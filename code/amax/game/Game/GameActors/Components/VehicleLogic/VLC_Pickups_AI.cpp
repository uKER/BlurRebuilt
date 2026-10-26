#include "stdafx.h"
#include "VLC_Pickups_AI.h"
#include "System/misc/Random.h"

#include <misc_shared/Messaging/GameMessaging.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <Game/Game.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/Messaging/VehicleActorMessages.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/RaceLogicCore/Data/RLVehicleData.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Structures/LevelState.h>

#include <AI\AI_Amax.h>

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
	#if defined(CFG_HACKED_MASTER)
Debug::Bool g_AIVehiclesBehaveCrazy("Multiplayer.AIPlayersBehaveCrazy", true);
	#else
Debug::Bool g_AIVehiclesBehaveCrazy("Multiplayer.AIPlayersBehaveCrazy", false);
	#endif
#endif
Debug::Bool	gDebugEnableAIPerks("Perks.AI.EnableAIPerkUse", true, Debug::Persist());
Debug::Bool gDebugMustBeInRangeOfHuman("Perks.AI.MustBeInRangeOfHuman", true, Debug::Persist());
Debug::Bool gDebugDropWhenOutOfRangeofHuman("Perks.AI.DropWhenOutOfRangeOfHuman", true, Debug::Persist());

Debug::Bool	gDebugEnableAIShieldUse("Perks.AI.EnableAIShieldUse", true, Debug::Persist());

Debug::Float gBargeProbabilityModifier("Perks.AI.BargeProbabilityModifier", 2, Debug::Persist());
Debug::Float g_DebugInDangerHangoverTime("Perks.AI.InDangerHangoverTime", 0.3, Debug::Persist());
Debug::Float g_DebugLevel2InRangeHelper("Perks.AI.Level2InRangeHelper", 2.0f, Debug::Persist());

Debug::Float g_DebugShuffleAheadDistance("Perks.AI.ShuffleAheadDistance", 40.0f, Debug::Persist());

extern Debug::Float g_debugCheckCutOffLength;

Debug::Logger &g_PickupAILogger( Debug::GetLogger("Pickups.AI") );

const float vehicleAheadDot = 0.7;
const float vehicleBehindDot = -0.7;

const float inRangeSwitchValue = 1.2f;

extern Debug::Float g_debugPerkStartDelay;

namespace Components
{

	bool C_VehicleComponent_Pickups_AI::CreateImpl()
	{
		const I_VehicleComponent_Pickup_AI_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Pickup_AI_CreateInfo *>(GetCreator().GetData());
		
		if(creation_data)
		{
			mActorHandle=creation_data->mParentActorHandle;
			mVehicleIndex=creation_data->mVehicleIndex;
			mpRaceLogicLayer=creation_data->mpRaceLogicLayer;
		}

		Initialise();

		return true;
	}


	void C_VehicleComponent_Pickups_AI::Initialise()
	{
		//AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_GameflowMessage_RaceFinished );
		AddSubscriber( E_VehicleLogicMsg_FiredPerk );
		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
		AddSubscriber( Game::E_GameLogicMessage_Failed_Rule );
		AddSubscriber( Game::E_VehicleMessage_FatallyDamaged );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_SetVehicleLogic );
		AddSubscriber( Game::E_VehicleMessage_Eliminated );

		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

		int maxLevel = levelState.mCars[mVehicleIndex].maxPerkLevel;

		if(maxLevel == 0)
			maxLevel = 3;

		float rval = GetRandom(1.0f, (float)maxLevel);
		mGrantTime = GetRandom(levelState.mCars[mVehicleIndex].mGrantTimeMin, levelState.mCars[mVehicleIndex].mGrantTimeMax);

		AIPickupControllerSetup* aiContoller = Game::C_Game::GetInstance()->GetLevelState().mAIController;

		mEvaluateTime = aiContoller->mSharedEvalTime;
		mShieldEvaluateTime = aiContoller->mSharedShieldEvalTime;

		mTimer_HitWithPerkDisableTime = mInDangerTime = mRecentlyInDangerTime = mTimer_PerkAIDisable = mTimer_PickupSwitching = 0.0f;

		mEnabled = false;

		mCurrCurveIdx = 0;

		mRandGenerator.Seed((U32)Time::GetCurrentTime());

		for(int i = 0; i < mAccuracyCurves.Size(); i++)
			mAccuracyCurves[i].Clear();

		mAccuracyCurves.Clear();
		RLAIPickupHermiteKeyFrame keyFrame;

		//Build the accuracy curves
		for(int i = 0; i < aiContoller->mAIPickupAccuracyGraphList.count; i++)
		{
			mAccuracyCurves.PushBack();

			for(int j = 0; j < aiContoller->mAIPickupAccuracyGraphList[i].mAIPickupAccuracyHermiteCurve.count; j++)
			{
				keyFrame = aiContoller->mAIPickupAccuracyGraphList[i].mAIPickupAccuracyHermiteCurve[j];
				mAccuracyCurves[i].AddKeyFrame(keyFrame.mDistance, keyFrame.mProbability, keyFrame.mInTangent,  keyFrame.mOutTangent);
			}

			mAccuracyCurves[i].AddKeyFrame(100, 0, 0, 0);
		}

		mShieldedCurve.Clear();
		for(int i = 0; i < aiContoller->mAIShieldedHermiteCurve.count; i++)
		{
			keyFrame = aiContoller->mAIShieldedHermiteCurve[i];
			mShieldedCurve.AddKeyFrame(keyFrame.mDistance, keyFrame.mProbability, keyFrame.mInTangent,  keyFrame.mOutTangent);
		}
		mShieldedCurve.AddKeyFrame(100, 0, 0, 0);

		if(!aiContoller->mAIPickupAccuracyGraphList.count)
			LOGWARN(g_PickupAILogger, "No accuracy curve data for AI - pickup use disabled.");

		float timeMin = Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedOutOfRangePickupDropTimeMin;
		float timeMax = Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedOutOfRangePickupDropTimeMax;
		mTimer_DropPickup = GetRandom(timeMin, timeMax);
	}

	void C_VehicleComponent_Pickups_AI::Update()
	{

		return;

#if 0
		PERF_EVENT("VLC.Pickups_AI.Update");

		if(!mEnabled)
			return;

		if( gDebugEnableAIPerks && mAccuracyCurves.Size() )
		{
			airacingtech::AIEventSegmentData* balancingData = AIAmax::AmaxAIR::Instance().GetEventBalancingSegmentData(mVehicleIndex);

			if(balancingData)
				mCurrCurveIdx = balancingData->m_PerkTemplateIndex;

			if(mCurrCurveIdx < 0 || mCurrCurveIdx >= mAccuracyCurves.Size() )
			{
				mCurrCurveIdx = 0;
				LOGWARN(g_PickupAILogger, "Failed to handle balancing data index - resetting to zero.");
			}

			DecrementTimers();
			GiftPickups();
			CyclePickups();
			DropPickups();
			EvaluateShieldUse();
			EvaluatePickupUse();
			SetTimers();
		}


#endif
	}

	// -------------------------------------------------------------------------------------------------

	void C_VehicleComponent_Pickups_AI::SetTimers()
	{
		PERF_EVENT("VLC.Pickups_AI.SetTimers");

		if(mEvaluateTime < 0)
		{
			// Rand offset, to help stop all vehicles evaluating in the same frame.
			float randOffset = mRandGenerator.FRand(-0.1, 0.1);
			mEvaluateTime = Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedEvalTime + randOffset;
		}

		if(mShieldEvaluateTime < 0)
		{
			// Rand offset, to help stop all vehicles evaluating in the same frame.
			float randOffset = mRandGenerator.FRand(-0.05, 0.05);
			mShieldEvaluateTime = Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedShieldEvalTime + randOffset;
		}
	}

	void C_VehicleComponent_Pickups_AI::DecrementTimers()
	{
		PERF_EVENT("VLC.Pickups_AI.DecrementTimers");
		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
		float frameTime = C_GameApp::GetInstance()->GetTimeStep();

		if( mTimer_HitWithPerkDisableTime > 0 )
			mTimer_HitWithPerkDisableTime -= frameTime;

		if( mGrantTime > 0 )
			mGrantTime -= frameTime;

		if( mEvaluateTime > 0 )
			mEvaluateTime -= frameTime;

		if( mShieldEvaluateTime > 0 )
			mShieldEvaluateTime -= frameTime;

		if( mTimer_PerkAIDisable > 0 )
			mTimer_PerkAIDisable -= frameTime;

		if( mTimer_PickupSwitching > 0 )
			mTimer_PickupSwitching -= frameTime;

		for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
		{
			Game::Actors::C_VehicleLogicActor* otherVehActor = (Game::Actors::C_VehicleLogicActor*)GetActorList()->GetActorPtr(mpRaceLogicLayer->GetLogicVehicleHandle(i));

			if( mVehicleIndex == i || !otherVehActor)
				continue;

			// Warning system has changed!
			//if(pActor->mTargetInDanger[otherVehActor->mVehicleIndex] && otherVehActor->mPickupInfo.mCharging)
			//{
			//	mInDangerTime += frameTime;
			//	mRecentlyInDangerTime = g_DebugInDangerHangoverTime;
			//	break;
			//}
			//else
			//{
			//	if(mRecentlyInDangerTime > 0.0f)
			//		mRecentlyInDangerTime -= frameTime;

			//	mInDangerTime = 0.0f;
			//}
		}

	}

	float C_VehicleComponent_Pickups_AI::GetRandom(float a, float b)
	{
		return mRandGenerator.FRand(a, b);
	}

	// -------------------------------------------------------------------------------------------------

	bool C_VehicleComponent_Pickups_AI::EvaluatePickupChange(E_PICKUP_TYPE pickupType)
	{
		PERF_EVENT("VLC.Pickups_AI.EvaluatePickupChange");

		Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();
		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

		// Don't drop anything when what I have will hurt my mates when dropped.
		for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
		{
			if( i == mVehicleIndex)
				continue;

			//if( pActor->mTargetInRange[i] <= 1.0f )
			//{
			//	// Check this person is not a member of my group
			//	airacingtech::AIEventSegmentData* hisBalancingData = AIAmax::AmaxAIR::Instance().GetEventBalancingSegmentData(i);
			//	airacingtech::AIEventSegmentData* myBalancingData = AIAmax::AmaxAIR::Instance().GetEventBalancingSegmentData(mVehicleIndex); // Can't cache, dunno when AI is loaded.

			//	if( myBalancingData && hisBalancingData && myBalancingData->m_Group == hisBalancingData->m_Group && myBalancingData->m_Group != 0 )
			//		return false;

			//	// Pete H's magic number for people we never want to hit.
			//	if(hisBalancingData && hisBalancingData->m_Group == 999)
			//		return false;
			//}
		}

		// We still have slots, so don't bother.
		if( pActor->mPickupInfo.mPickupStore.Size() < levelState.mCars[mVehicleIndex].mUpgrades.NumPickupSlots() || !pActor->mPickupInfo.mPickupStore.Size() )
			return false;

		E_PICKUP_TYPE currPickup = pActor->mPickupInfo.mPickupStore[pActor->mPickupInfo.mPickupStoreIndex].mType;

		// We don't pick these ones up
		if(currPickup == pickupType)
			return false;

		// If I need health, drop what we have, because we'd sure like this.
		if( pickupType == E_PICKUP_TYPE_REPAIR && pActor->mCurrentHealth < pActor->GetMaxHealth() )
			return true;

		// If I have a repair but don't need health, let's chuck it for something more awesome.
		if( currPickup == E_PICKUP_TYPE_REPAIR && pActor->mCurrentHealth == pActor->GetMaxHealth() )
			return true;

		// ------------------------
		// Do I prefer this pickup?
		// ------------------------
		for( int i = 0; i < levelState.mCars[mVehicleIndex].mPreferredPickups.Size(); i++ )
		{
			if( levelState.mCars[mVehicleIndex].mPreferredPickups[i] == pickupType )
				return true;
		}

		// ------------------------------
		// Do I prefer the pickup I have?
		// ------------------------------
		for( int i = 0; i < levelState.mCars[mVehicleIndex].mPreferredPickups.Size(); i++ )
		{
			if( currPickup == levelState.mCars[mVehicleIndex].mPreferredPickups[i] )
				return true;
		}

		// ------------------------------------------------------
		// If we have a mine, this is an awesome place to drop it
		// ------------------------------------------------------
		if( currPickup == E_PICKUP_TYPE_BOMB )
			return true;

		// --------------------------------------------------------------------
		// Check if this weapon is something useful for hitting the nearest guy
		// --------------------------------------------------------------------
		float shortestDist = -1;
		int idx = -1;
		for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
		{
			if( i != mVehicleIndex )
			{
				float distance	= pActor->mVehicleProximities[i].mDistanceSquared;

				if( distance < shortestDist || shortestDist == -1 )
				{
					idx = i;
					shortestDist = distance;
				}
			}
		}

		if(idx == -1)
			return false;

		if(pActor->mVehicleProximities[idx].mRelativeHeading > vehicleAheadDot) // Ahead
		{
			if( currPickup != E_PICKUP_TYPE_SHUNT && ( pickupType == E_PICKUP_TYPE_BARGE || pickupType == E_PICKUP_TYPE_SHOCK ) )
				return true;

			if( pickupType == E_PICKUP_TYPE_SHUNT )
				return true;
		}
		else if( pActor->mVehicleProximities[idx].mRelativeHeading < vehicleBehindDot ) // Behind
		{
			if( currPickup != E_PICKUP_TYPE_BOMB && ( pickupType == E_PICKUP_TYPE_BARGE || pickupType == E_PICKUP_TYPE_SHOCK ) )
				return true;

			if( pickupType == E_PICKUP_TYPE_BOMB )
				return true;
		}
		else // To the side
		{
			if( pickupType == E_PICKUP_TYPE_BARGE )
				return true;
		}

		return false;		
	}

	// -------------------------------------------------------------------------------------------------

	bool C_VehicleComponent_Pickups_AI::InRangeOfHumanCheck()
	{
		PERF_EVENT("VLC.Pickups_AI.InRangeOfHumanCheck");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if(pActor->mPositionInfo.mInRangeOfHuman || !gDebugMustBeInRangeOfHuman)
			return true;

		return false;
	}

	bool C_VehicleComponent_Pickups_AI::PlayerHitRecentlyCheck()
	{
		PERF_EVENT("VLC.Pickups_AI.PlayerHitRecentlyCheck");

		Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		// The human has been hit recently, so drop out.
		float rval			= GetRandom(0, Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedHumanAvoidRatio);
		int humanAvoidRand	= (int)(fmod(rval, 1) >= 0.5 ? ceil(rval) : floor(rval));			

		if( mpRaceLogicLayer->mTimer_PerkPlayerHitAIDisable > 0 || humanAvoidRand )
			return true;

		return false;
	}

	void C_VehicleComponent_Pickups_AI::EvaluatePickedUpPickup(E_PICKUP_TYPE type)
	{	
		if( EvaluatePickupChange(type) )
		{
			Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

			// Fire my pickup and pick up the new one.
			Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(mVehicleIndex);
			pickupFireMsg.mPickupOverride = pActor->mPickupInfo.mPickupStore[pActor->mPickupInfo.mPickupStoreIndex].mType;
			GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(),E_IF_VehicleComponent_Pickups,&pickupFireMsg);
		}
	}

	// -------------------------------------------------------------------------------------------------

	bool C_VehicleComponent_Pickups_AI::CheckPickupCanFireOnTarget(int idx, E_PICKUP_TYPE type)
	{
		PERF_EVENT("VLC.Pickups_AI.CheckPickupCanFireOnTarget");

		Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		// Check out of range
		float checkDist  = vehActor->mVehicleProximities[idx].mDistanceSquared;

		if(checkDist > g_debugCheckCutOffLength * g_debugCheckCutOffLength || !InRangeOfHumanCheck())
		{
			bool shockInRangeOfHuman = (type == E_PICKUP_TYPE_SHOCK && ShockInRangeOfHuman());
			if( !shockInRangeOfHuman )
				return false;
		}

		if( type != E_PICKUP_TYPE_SHOCK && checkDist > g_debugCheckCutOffLength * g_debugCheckCutOffLength )
			return false;

		// Check the player in SP
		//if( !idx && vehActor->mTargetInRange[idx] <= 1 && PlayerHitRecentlyCheck() )
		//	return false;

		// Check this person is not a member of my group
		airacingtech::AIEventSegmentData* hisBalancingData = AIAmax::AmaxAIR::Instance().GetEventBalancingSegmentData(idx);
		airacingtech::AIEventSegmentData* myBalancingData = AIAmax::AmaxAIR::Instance().GetEventBalancingSegmentData(mVehicleIndex); // Can't cache, dunno when AI is loaded.

		if( myBalancingData && hisBalancingData && myBalancingData->m_Group == hisBalancingData->m_Group && myBalancingData->m_Group != 0 )
			return false;

		// Pete H's magic number for people we never want to hit.
		if(hisBalancingData && hisBalancingData->m_Group == 999)
			return false;

		// Check the shield
		Game::Actors::C_VehicleLogicActor* pActor	 = ((Game::Actors::C_VehicleLogicActor*)GetActor());
		float rval									= GetRandom(0, 1);
		float noticeShield							= mShieldedCurve.Evaluate(pActor->mShieldTime);

		if(rval < noticeShield)
			return false;

		return true;
	}

	bool C_VehicleComponent_Pickups_AI::CheckCanDropMine(int idx)
	{
		PERF_EVENT("VLC.Pickups_AI.CheckCanDropMine");

		Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		float checkAngle = vehActor->mVehicleProximities[idx].mRelativeHeading;
		float checkDist  = vehActor->mVehicleProximities[idx].mDistanceSquared;

		// This is not pre-calculated as only pickup AI uses it.
		float mineDistanceMax = 300;
		float mineAngle = 0.8;

		float angleCheck = ( 1 + checkAngle ) / ( 1 - mineAngle );
		float distCheck  = checkDist / mineDistanceMax;

		float finalCheck = angleCheck + distCheck;

		if( finalCheck <= 1 && (angleCheck > 1 || distCheck > 1) )
			finalCheck += 1;

		float rval = GetRandom(0, 1);
		float probability = mAccuracyCurves[mCurrCurveIdx].Evaluate(finalCheck);

		if( rval < probability )
			return true;

		return false;
	}

	// -------------------------------------------------------------------------------------------------

	bool C_VehicleComponent_Pickups_AI::CheckLevel2PickupTriggers(E_PICKUP_TYPE type)
	{
		return false;

		//Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		//if(vehActor->mPickupInfo.mPickupStore.Size() <= 1 || vehActor->mPickupInfo.mPickupStore[1].mType != type)
		//	return false;

		//Btl::Vector<float> targetsInRange;
		//for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
		//	targetsInRange.PushBack(500.f);

		//vehActor->CalculateVehiclesInRangeOfPickups(GetLevel2Perk(type), &targetsInRange);

		//for( int i = 0; i < targetsInRange.Size(); i++ )
		//{
		//	if( i == mVehicleIndex || !CheckPickupCanFireOnTarget(i) )
		//		continue;

		//	if ( type == E_PICKUP_TYPE_BOMB && CheckCanDropMine(i))
		//	{
		//		return true;
		//	}
		//	else if( type == E_PICKUP_TYPE_GHOST && vehActor->mPickupInfo.mCanFireNitro )
		//	{
		//		return true;
		//	}
		//	else
		//	{
		//		if( type == E_PICKUP_TYPE_RIFT && !vehActor->mPickupInfo.mCanFireNitro)
		//			return false;

		//		float rval = GetRandom(0, 1);
		//		rval /= g_DebugLevel2InRangeHelper;
		//		float probability = mAccuracyCurves[mCurrCurveIdx].Evaluate(targetsInRange[i]);

		//		if( rval < probability )
		//			return true;
		//	}
		//}

		//return false;
	}

	bool C_VehicleComponent_Pickups_AI::CheckPickupTriggers(E_PICKUP_TYPE pickupType)
	{
		PERF_EVENT("VLC.Pickups_AI.CyclePickupTriggers");
		Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	
		for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
		{
			if( i == mVehicleIndex || !CheckPickupCanFireOnTarget(i, pickupType) )
				continue;

			if ( pickupType == E_PICKUP_TYPE_BOMB && CheckCanDropMine(i))
			{
				return true;
			}
			//else if( pickupType == E_PICKUP_TYPE_NITROUS && vehActor->mPickupInfo.mCanFireNitro )
			//{
			//	return true;
			//}
			else
			{
				//float rval = GetRandom(0, 1);
				//float inRangeVal = vehActor->mTargetInRange[i];
				//float probability = mAccuracyCurves[mCurrCurveIdx].Evaluate(inRangeVal);

				//if( pickupType == E_PICKUP_TYPE_BARGE )
				//	probability *= gBargeProbabilityModifier;

				//if( rval < probability )
				//{
				//	if(probability > 1)
				//	{
				//		C_VehicleLogicMsg_MissedPerkMsg missedMsg(pickupType, mVehicleIndex, i, vehActor->GetWorldMatrix(), -1);

				//		if(Game::C_InGameDialogueManager::Instance().IsEnabled())
				//			Game::C_InGameDialogueManager::Instance().MessageHandler(&missedMsg);

				//	}

				//	return true;
				//}
			}
		}

		return false;
	}

	void C_VehicleComponent_Pickups_AI::EvaluateShieldUse()
	{
		if(!gDebugEnableAIShieldUse)
			return;

		PERF_EVENT("VLC.Pickups_AI.EvaluateShieldUse");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		float randShieldTest = mRandGenerator.FRand();

		if( mShieldEvaluateTime <= 0 && pActor->mFanPower >= 1.0f && pActor->mShieldTime == 0.f)
		{
			bool inDanger = false;

			if( mInDangerTime > 0.0f )
				inDanger = mShieldedCurve.Evaluate(mInDangerTime) > randShieldTest;
			else if( mRecentlyInDangerTime > 0.0f )
				inDanger = mShieldedCurve.Evaluate(g_debugPerkStartDelay + ( g_DebugInDangerHangoverTime - mRecentlyInDangerTime) ) > randShieldTest;

			if(inDanger)
			{
				RLPickupType* pPickup = (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_SHIELD);
				float shield_time = (float)((RLShieldBalancingData *)pPickup->mNormal->mBalancingData)->mLifetime;

				//C_VehicleLogicShieldTimeMsg shieldMsg(mVehicleIndex, shield_time);
				//GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(),E_IF_VehicleComponent_Fan_Power,&shieldMsg);
			}
		}
	}

	void C_VehicleComponent_Pickups_AI::EvaluatePickupUse()
	{
		PERF_EVENT("VLC.Pickups_AI.EvaluatePickupUse");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::RaceLogicCore* pRLC			= pActor->GetRLC();

		if( mTimer_HitWithPerkDisableTime > 0.f || mTimer_PickupSwitching > 0.f || !pActor->mPickupInfo.mPickupStore.Size())
			return;

		E_PICKUP_TYPE currentType =  pActor->mPickupInfo.mPickupStore[pActor->mPickupInfo.mPickupStoreIndex].mType;		
#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
		if( !g_AIVehiclesBehaveCrazy && (currentType != E_PICKUP_TYPE_SHOCK && !InRangeOfHumanCheck()) )
#else
		if( currentType != E_PICKUP_TYPE_SHOCK && !InRangeOfHumanCheck() )
#endif
		return;


		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(mVehicleIndex);
		pickupFireMsg.mPickupOverride = currentType;
		bool firePickup = false;

		// -----------------------------------
		// Check if now is a good time to fire
		// -----------------------------------
		if( mEvaluateTime <= 0 && mTimer_PerkAIDisable <= 0 )
		{
			if(false)//CheckLevel2PickupTriggers(pickupFireMsg.mPickupOverride))
			{
				pickupFireMsg.mPickupOverride = GetLevel2Perk(pickupFireMsg.mPickupOverride);				
				GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(),E_IF_VehicleComponent_Pickups,&pickupFireMsg);
			}
#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
			else if(g_AIVehiclesBehaveCrazy || CheckPickupTriggers(pickupFireMsg.mPickupOverride))
#else
			else if(CheckPickupTriggers(pickupFireMsg.mPickupOverride))
#endif
			{
				GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(),E_IF_VehicleComponent_Pickups,&pickupFireMsg);
			}
		}
	}

	// -------------------------------------------------------------------------------------------------

	E_PICKUP_TYPE C_VehicleComponent_Pickups_AI::GetLevel2Perk(E_PICKUP_TYPE type)
	{
		return type;
		//switch(type)
		//{
		//case E_PICKUP_TYPE_NITROUS:
		//	return E_PICKUP_TYPE_GHOST;

		//case E_PICKUP_TYPE_SHOCK:
		//	return E_PICKUP_TYPE_PULSE;

		//case E_PICKUP_TYPE_SHUNT:
		//	return E_PICKUP_TYPE_GRAB;

		//case E_PICKUP_TYPE_FLASH:
		//	return E_PICKUP_TYPE_BOMB;

		//case E_PICKUP_TYPE_BARGE:
		//	return E_PICKUP_TYPE_RIFT;
		//}

		//return E_PICKUP_TYPE_NONE;
	}

	// -------------------------------------------------------------------------------------------------

	void C_VehicleComponent_Pickups_AI::GiftPickups()
	{
		PERF_EVENT("VLC.Pickups_AI.GiftPickups");

		Game::C_LevelState& levelState				= Game::C_Game::GetInstance()->GetLevelState();
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if( levelState.mCars[mVehicleIndex].mGrantTimeMax > 0 && mGrantTime <= 0 )
		{
			Game::Actors::C_GrantPickupMsg powerMsg(mVehicleIndex,levelState.mCars[mVehicleIndex].mPerkToGrantAI);
			GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(),E_IF_VehicleComponent_Pickups,&powerMsg);

			mGrantTime = GetRandom(levelState.mCars[mVehicleIndex].mGrantTimeMin, levelState.mCars[mVehicleIndex].mGrantTimeMax);
		}

		if( levelState.mCars[mVehicleIndex].maxPerkLevel >= 1 && pActor->mPickupInfo.mPickupStore.Size() >= levelState.mCars[mVehicleIndex].maxPerkLevel )
		{
			Game::Actors::PickupRemovePowerMsg removePowerMsg(mVehicleIndex, 1);
			GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(),E_IF_VehicleComponent_Pickups,&removePowerMsg);
		}

	}

	// -------------------------------------------------------------------------------------------------

	void C_VehicleComponent_Pickups_AI::DropPickups()
	{
		PERF_EVENT("VLC.Pickups_AI.DropPickups");	

		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if( !InRangeOfHumanCheck() && gDebugDropWhenOutOfRangeofHuman && pActor->mPickupInfo.mPickupStoreIndex != -1)
		{
			if( mTimer_DropPickup <= 0)
			{
				E_PICKUP_TYPE currentType = pActor->mPickupInfo.mPickupStore[pActor->mPickupInfo.mPickupStoreIndex].mType;

				bool inRangeOfShock = (currentType == E_PICKUP_TYPE_SHOCK && ShockInRangeOfHuman());
				if( !inRangeOfShock )
				{
					if( pActor->mPickupInfo.mPickupStore.Size() && !pActor->mPickupInfo.mCharging )
					{
						pActor->mPickupInfo.mPickupStore.EraseByIndex(pActor->mPickupInfo.mPickupStoreIndex);
						
						if(pActor->mPickupInfo.mPickupStore.Size())
						{
							pActor->mPickupInfo.mPickupStoreIndex = 0;
							pActor->mPickupInfo.mCurrentlySelectedType = pActor->mPickupInfo.mPickupStore[pActor->mPickupInfo.mPickupStoreIndex].mType;
						}
						else
						{
							pActor->mPickupInfo.mPickupStoreIndex = -1;
							pActor->mPickupInfo.mCurrentlySelectedType = E_PICKUP_TYPE_NONE;
						}
					}

					float timeMin = Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedOutOfRangePickupDropTimeMin;
					float timeMax = Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedOutOfRangePickupDropTimeMax;
					mTimer_DropPickup = GetRandom(timeMin, timeMax);
				}
			}
			else
			{
				float frameTime = C_GameApp::GetInstance()->GetTimeStep();
				mTimer_DropPickup -= frameTime;
			}
		}
	}

	bool C_VehicleComponent_Pickups_AI::ShockInRangeOfHuman()
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::RaceLogicCore* pRLC			= pActor->GetRLC();

		for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
		{
			//if( !pRLC->IsVehicleAIControlled(i) && pActor->mTargetInRange[i] <= 1.0f )
			//	return true;
		}

		return false;
	}

	// -------------------------------------------------------------------------------------------------

	void C_VehicleComponent_Pickups_AI::CyclePickups()
	{
		PERF_EVENT("VLC.Pickups_AI.CyclePickups");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		// Nothing to cycle...
		if( pActor->mPickupInfo.mPickupStore.Size() <= 1 || mTimer_PickupSwitching > 0.0f )
			return;

		// -----------------------------------------
		// See if I can hit him with anything I have
		// -----------------------------------------
		Btl::Vector<float> targetsInRange;
		{
			PERF_EVENT("VLC.Pickups_AI.SetupVector");

			for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
				targetsInRange.PushBack(500.f);
		}

		{
			PERF_EVENT("VLC.Pickups_AI.CheckInRange");

			Btl::Set<E_PICKUP_TYPE> pickupsChecked;
			for( int i = 0; i < pActor->mPickupInfo.mPickupStore.Size(); i++ )
			{
				Btl::Set<E_PICKUP_TYPE>::InsertionReturnType result = pickupsChecked.Insert(pActor->mPickupInfo.mPickupStore[i].mType);

				if(!result.inserted)
					continue;

				//pActor->CalculateVehiclesInRangeOfPickups(pActor->mPickupInfo.mPickupStore[i].mType, &targetsInRange);

				//for( int j = 0; j < targetsInRange.Size(); j++ )
				//{
				//	if( targetsInRange[j] <= inRangeSwitchValue )
				//	{
				//		SwitchToPickup(i);
				//		return;
				//	}
				//}
			}
		}

		// ----------------------------
		// Is now a good time to nitro?
		// ----------------------------
		{
			PERF_EVENT("VLC.Pickups_AI.CheckNitro");

			//if(pActor->mPickupInfo.mCanFireNitro)
			//{
			//	for( int i = 0; i < pActor->mPickupInfo.mPickupStore.Size(); i++ )
			//	{
			//		if( i == pActor->mPickupInfo.mPickupStoreIndex)
			//			continue;

			//		if(pActor->mPickupInfo.mPickupStore[i].mType == E_PICKUP_TYPE_NITROUS)
			//		{
			//			SwitchToPickup(i);
			//			return;
			//		}
			//	}
			//}
		}

		// --------------------------------------
		// If all else fails, aim for the nearest
		// --------------------------------------
		int idx = -1;
		{
			PERF_EVENT("VLC.Pickups_AI.GetNearest");

			float shortestDist = -1;
			for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
			{
				if( i != mVehicleIndex )
				{
					float distance	= pActor->mVehicleProximities[i].mDistanceSquared;

					if( distance < shortestDist || shortestDist == -1 )
					{
						idx = i;
						shortestDist = distance;
					}
				}
			}
		}

		// Find our most appropriate weapon
		{
			PERF_EVENT("VLC.Pickups_AI.GetBestWeapon");

			float aheadDistanceSq = g_DebugShuffleAheadDistance * g_DebugShuffleAheadDistance;
			Btl::Vector<E_PICKUP_TYPE> preferenceOrder;
			if( mVehicleIndex == 1 && idx != -1 )
			{
				if(pActor->mVehicleProximities[idx].mRelativeHeading > vehicleAheadDot) // Ahead
				{
					if(pActor->mVehicleProximities[idx].mDistanceSquared > aheadDistanceSq) // A fair distance
					{
						preferenceOrder.PushBack(E_PICKUP_TYPE_SHOCK);
						preferenceOrder.PushBack(E_PICKUP_TYPE_SHUNT);
						preferenceOrder.PushBack(E_PICKUP_TYPE_BARGE);
					}
					else // Not so far
					{
						preferenceOrder.PushBack(E_PICKUP_TYPE_SHUNT);
						preferenceOrder.PushBack(E_PICKUP_TYPE_SHOCK);
						preferenceOrder.PushBack(E_PICKUP_TYPE_BARGE);
					}
				}
				else if( pActor->mVehicleProximities[idx].mRelativeHeading < -vehicleBehindDot ) // Behind
				{
					preferenceOrder.PushBack(E_PICKUP_TYPE_BOMB);
					preferenceOrder.PushBack(E_PICKUP_TYPE_BARGE);
				}
				else // To the side
				{
					preferenceOrder.PushBack(E_PICKUP_TYPE_BARGE);
					preferenceOrder.PushBack(E_PICKUP_TYPE_BOMB);
					preferenceOrder.PushBack(E_PICKUP_TYPE_SHUNT);
				}
			}

			// Have we got this weapon?
			for( int i = 0; i < preferenceOrder.Size(); i++ )
			{
				for( int j = 0; j < pActor->mPickupInfo.mPickupStore.Size(); j++ )
				{
					if( pActor->mPickupInfo.mPickupStore[j].mType == preferenceOrder[i] )
					{
						SwitchToPickup(j);
						return;
					}
				}
			}
		}
	}

	// -------------------------------------------------------------------------------------------------

	void C_VehicleComponent_Pickups_AI::SwitchToPickup(int idx)
	{
		PERF_EVENT("VLC.Pickups_AI.SwitchToPickup(int)");

		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if(idx)
		{
			Game::Actors::PickupShuffleMsg shuffleMsg(mVehicleIndex, true, idx);
			GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(), E_IF_VehicleComponent_Pickups, &shuffleMsg);

			mTimer_PickupSwitching = Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedPickupSwitchTime;
		}
	}

	void C_VehicleComponent_Pickups_AI::SwitchToPickup(E_PICKUP_TYPE type)
	{
		PERF_EVENT("VLC.Pickups_AI.SwitchToPickup(E_PICKUP_TYPE)");

		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if(type != E_PICKUP_TYPE_NONE)
		{
			for(int i = 1; i < pActor->mPickupInfo.mPickupStore.Size(); i++)
			{
				if(pActor->mPickupInfo.mPickupStore[i].mType == type)
				{
					Game::Actors::PickupShuffleMsg shuffleMsg(mVehicleIndex, true, i);
					GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(), E_IF_VehicleComponent_Pickups, &shuffleMsg);
					break;
				}
			}
		}
	}

	// -------------------------------------------------------------------------------------------------

	bool C_VehicleComponent_Pickups_AI::OnMessage(const Messaging::C_MessageBase *pMessage) 
	{
		PERF_EVENT("VLC.Pickups_AI.MessageHandler");
		switch(pMessage->Type())
		{
		case Game::E_GameflowMessage_RaceStarted:
			{
				mEnabled = true;
				return true;
			}
			break;

		case Game::E_GameflowMessage_RaceFinished:
			{
				mEnabled = false;
				return true;
			}
			break;

		case Game::E_VehicleMessage_Eliminated:
			{
				const Game::Actors::C_VehicleEliminationMessage* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleEliminationMessage>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
					mEnabled = false;
				
				return true;
			}
			break;

		//case GameMessaging::E_GameMsg_Update:
		//	{
		//		if(mEnabled)
		//			Update();
		//		return true;
		//	}
		//	break;

		case Game::E_GameLogicMessage_Failed_Rule:
			{
				const Game::C_FailedRuleMsg* pMsg = Messaging::MessageCast<Game::C_FailedRuleMsg>(pMessage);

				if( pMsg->mVehicleIndex == mVehicleIndex )
					mEnabled = false;

				return true;
			}
			break;

		case E_VehicleLogicMsg_FiredPerk:
			{
				const C_VehicleLogicMsg_FiredPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_FiredPerkMsg >(pMessage);

				if( msg->mActorHandle == mActorHandle )
					mTimer_PerkAIDisable += Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedFiredPerkDisableTime;

				return true;
			}
			break;

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);

				if( msg->mVehicleIndex == mVehicleIndex )
					mTimer_HitWithPerkDisableTime += Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedBeenHitWithPerkDisableTime;

				return true;
			}
			break;

		case ActorMsg::E_DynamicObjMsg_SetVehicleLogic:
			{
				const ActorMsg::C_SetVehicleLogicMsg* pMsg = Messaging::MessageCast<ActorMsg::C_SetVehicleLogicMsg>(pMessage);
				mEnabled = pMsg->m_Pickups;			
				return true;
			}
		}

		return I_GameActorComponent::OnMessage(pMessage);
	}

} // namespace Components
