#include "stdafx.h"

#include <GameLogic/Pickups/Reverse/PickupShuntReverse.h>
#include <GameLogic/Pickups/PickupClasses.h>

#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameLogic/AchievementsManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>

#include <Ui\UiMessages.h>

#include "GameAudio/GameAudioLayer.h"
#include "GameAudio/PickupSubsystem.h"

Debug::Float g_debugShuntReverseLiveTime("Perks.ShuntReverse.LiveTime", 20.0f);


namespace Game
{
	C_PickupShuntProjectileReverse::C_PickupShuntProjectileReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupProjectile(E_PickupClass_ShuntProjectileReverse, id, firingIdx, -1, E_PICKUP_TYPE_SHUNT, subType, noFiringVehicleFX)
	{
		RLPickupType* pPickup			= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHUNT);
		mpShuntData						= (RLShuntReverseBalancingData*)pPickup->mReversed->mBalancingData;
		mpShuntSFX						= (RLShuntSpecialFX*)pPickup->mReversed->mSpecialFX;

		BIZ_ASSERT(mpShuntData);
		BIZ_ASSERT(mpShuntSFX);

		mRadius				= mpShuntData->mRadius;
		mSpeed				= mpShuntData->mSpeed;
		mHeightFromGround	= mpShuntData->mHeightFromGround;
		mDropFXTimer		= mpShuntData->mFXDropTime;
		mAliveTime			= 0.f;
		mFallRate			= mpShuntData->mFallRate;
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectileReverse::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		Maths::HMat43 reverseMatrix = matrix;
		reverseMatrix.AZ(-matrix.AZ());

		C_PickupBase::PostDelayInitialise(reverseMatrix, sfx);
		if(VehicleIsLocal(mFiringIdx))
		{
			SetPostDelayMatrix(reverseMatrix);
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, reverseMatrix);
			}
		}
	}

	// --------------------------------------------------------------------

	void C_PickupShuntProjectileReverse::SetPostDelayMatrix( Maths::HMat43 targetMatrix, float fVal )
	{
		C_PickupBase::SetPostDelayMatrix(targetMatrix, fVal);

		Maths::HVec3 up(0.f, 1.f, 0.f);
		mMatrix.AW(mMatrix.AW() + (up * mRadius));

		CreateCollisionBody();
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mProjectileSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Projectile);
	}

	// --------------------------------------------------------------------------------

	bool C_PickupShuntProjectileReverse::Update()
	{
		PERF_EVENT("PickupShunt.Update");

		if(!mActivated)
			return false;

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();

		mAliveTime += frameTime;
		if(mAliveTime > g_debugShuntReverseLiveTime)
			KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_OLD_AGE);

		mDropFXTimer -= frameTime;

		if(mDropFXTimer <= 0.0f)
		{
			mDropFXTimer = mpShuntData->mFXDropTime;
			C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mProjectileDroppedSpecialFX, mMatrix, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Firing);
		}

		if(mKillPickup)
		{
			C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);

			KillPickupAudio();

			return true;
		}

		UpdateCollisionBody();

		Maths::HVec3 velocity = mpShuntData->mSpeed * mMatrix.AZ();

		C_VehicleLogicMsg_PerkUpdateMatrixMsg updateMatrixMsg(mUniqueId, mMatrix, mFiringIdx, velocity);
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_UpdateMatrix, 0, &updateMatrixMsg);

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectileReverse::Trigger(int idx)
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

	void C_PickupShuntProjectileReverse::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(mMatrix.AW(), mRadius, Maths::RGBA(255,255,0));
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectileReverse::Explode(Maths::HVec3 pos, Maths::C_Normal normal)
	{
		Maths::AxisY axisY(normal);
		Maths::HMat33 mat33(axisY);
		Maths::HMat43 mat(mat33, pos);
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mHitEnvironmentSpecialFX, mat, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Firing);
	}
}
