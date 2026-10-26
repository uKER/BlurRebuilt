#include "stdafx.h"
#include <GameLogic/Pickups/PickupClasses.h>	// [Blur2PC]

#include <GameLogic/Pickups/Reverse/PickupBombReverse.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>
#include <GameLogic\MultiplayerUtilities.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Pickups/PickupsManager.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <GameLogic\Pickups\Upgraded\PickupBomb.h>

#include <GameApp.h>
#include <Game/game.h>
#include "GameAudio/GameAudioLayer.h"
#include "GameAudio/PickupSubsystem.h"

Debug::Float g_debugBombEnvironmentPushBackModifier("Perks.ReverseBomb.EnvironmentPushBack", 2.f);
Debug::Bool g_debugReverseMineDraw("Perks.DebugDraw.ReverseMine", false);

namespace Game
{
	C_PickupBombReverse::C_PickupBombReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupProjectile(E_PickupClass_BombReverse, id, firingIdx, -1, E_PICKUP_TYPE_BOMB, subType, noFiringVehicleFX), mLiveTime(0.f)
	{
		RLPickupType* pPickup	= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BOMB);
		mpBombData				= (RLBombReverseBalancingData*)pPickup->mReversed->mBalancingData;
		mpBombGraph				= &pPickup->mReversed->mBalancingGraph;
		mpBombSFX				= (RLBombSpecialFX*)pPickup->mReversed->mSpecialFX;

		BIZ_ASSERT(mpBombData);
		BIZ_ASSERT(mpBombSFX);
		BIZ_ASSERT(mpBombGraph);

		mRadius				= mpBombData->mRadius;
		mHeightFromGround	= mpBombData->mHeightFromGround;
		mFallRate			= mpBombData->mFallRate;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBombReverse::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);
		if(VehicleIsLocal(mFiringIdx))
		{
			C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);
			float speed = (mpBombData->mScaledSpeed * pActor->GetPositionInfo().mSpeedInMph) + mpBombData->mMinSpeed;	

			SetPostDelayMatrix(matrix, speed);
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, matrix, speed);
			}
		}
	}

	// --------------------------------------------------------------------

	void C_PickupBombReverse::SetPostDelayMatrix( Maths::HMat43 targetMatrix, float fVal )
	{
		mInitialSpeed = mSpeed = fVal;
		C_PickupBase::SetPostDelayMatrix(targetMatrix, fVal);

		Maths::HVec3 up(0.f, 1.f, 0.f);
		mMatrix.AW(mMatrix.AW() + (up * mRadius));

		CreateCollisionBody();
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpBombSFX->mMineSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Firing);

	}

	// --------------------------------------------------------------------------------

	bool C_PickupBombReverse::Update()
	{
		PERF_EVENT("PickupBombReverse.Update");

		if(!mActivated)
			return false;

		if(g_debugReverseMineDraw)
			DebugDraw();

		if(mKillPickup)
		{
			C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);

			KillPickupAudio(E_PICKUP_SUB_TYPE_REVERSED);
			return true;
		}

		if(mSpeed == 0.f)
		{
			if(VehicleIsLocal(mFiringIdx))
			{
				KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_REPLACED);

				PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BOMB, E_PICKUP_SUB_TYPE_NORMAL, mFiringIdx);
				pickupInfo.mMatrix				= mMatrix;
				pickupInfo.mDisableVehicleFX	= true;
				pickupInfo.mOverrideMatrix		= true;
				pickupInfo.mParentPickupId		= mUniqueId;
				pickupInfo.mVal					= 0.f;
				pickupInfo.mVal2				= -1.f;

				U32 newId = C_PickupsManager::Instance().CreatePickup(pickupInfo);
				C_InGameSpecialFXManager::Instance().CopySFXState(newId, mUniqueId);

				// If we fired this bomb
				// Set a flag on the newly created mine to indicate that it was lobbed and has recently come to rest. 
				C_PickupBomb* pBomb = (Game::C_PickupBomb*)(C_PickupsManager::Instance().GetPickup(newId));
				pBomb->FlagAsLobbedMineRecentlyComeToRest();
			}
		}

		float frameTime	= C_GameApp::GetInstance()->GetTimeStep();
		mLiveTime += frameTime;

		mSpeed = mpBombGraph->mEffectCurve.Evaluate(mLiveTime) * mInitialSpeed;

		// Stops jittering at low speeds
		if(mSpeed < 20.f)
		{
			mSpeed = 0.f;
		}
		else
		{
			UpdateCollisionBody();
		}

		C_VehicleLogicMsg_PerkUpdateMatrixMsg updateMatrixMsg(mUniqueId, mMatrix, mFiringIdx,mSpeed * mMatrix.AZ());

		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_UpdateMatrix, 0, &updateMatrixMsg);


		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBombReverse::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mMatrix.AW(),1.f,Maths::RGBA(255,0,0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mRadius,Maths::RGBA(255,255,0));
	}

	// --------------------------------------------------------------------------------

	void C_PickupBombReverse::Trigger(int idx)
	{
		if( idx != mFiringIdx && CheckCanHit(idx) )
		{
			C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			if (pLayer)
			{
				Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(idx);
				if (pActor)
				{
					Maths::HMat43 mat = pActor->GetGraphicalActor(idx)->GetMatrix();
					mHitInfo.mPosition = mHitPoint;
					mHitInfo.mDirection = mHitPoint - mPrevPos;
					mHitInfo.mDirection.Normalise();
					mHitInfo.mDirection = mHitInfo.mDirection * mat.Rotation().Inverse();
				}
			}
			RegisterHit(idx);
			if(!CheckLegalHit(idx))
				NinjaKill(mAutopsyReport);

			// Store any data specific to this type of death
			mAutopsyReport.mVehicleIdHit	   =  idx;

			// Do the killing
			KillPickup(-1,E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupBombReverse::CollideWithEnvironment(Maths::HVec3 hitPoint, Maths::C_Normal normal)
	{
		mSpeed = 0.f;
		Maths::HVec3 up(0.f, 1.f, 0.f);
		Maths::HVec3 pushUp = up * (mRadius + mHeightFromGround);
		Maths::HVec3 pos = hitPoint + (-mMovementDir * mRadius * g_debugBombEnvironmentPushBackModifier);

		if(AlignProjectileToGroundImmediate(pos))
			mMatrix.AW(pos + pushUp);
		else // Probably never going to hit this, I would hope.
			mMatrix.AW( hitPoint + (normal * mRadius * g_debugBombEnvironmentPushBackModifier) + pushUp ); 

		mMovementDir = Maths::HVec3(0.f, 0.f, 0.f);

		C_InGameSpecialFXManager::Instance().UpdatePerkMatrix(mUniqueId, mMatrix);
		mpCollisionBody->Position(mMatrix.AW());
	}

	// --------------------------------------------------------------------------------

	E_TARGETTING_DIRECTION C_PickupBombReverse::GetHitDirection(Actors::C_VehicleLogicActor* pActor)
	{
		E_TARGETTING_DIRECTION hitDir = E_TARGETTING_DIRECTION_NONE;
		return hitDir;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBombReverse::Explode(Maths::HVec3 pos, Maths::C_Normal normal)
	{
		// Remove the mine and play the boom effect
		C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpBombSFX->mExplodeSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Firing);
	}
}
