#include "stdafx.h"

#include <GameLogic/Pickups/Normal/PickupBlindfire.h>
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


Debug::Float g_debugBlindfireLiveTime("Perks.Blindfire.LiveTime", 10.0f);
Debug::Bool g_debugBlindfireDraw("Perks.DebugDraw.Blindfire", false);

namespace Game
{
	C_PickupBlindfireProjectile::C_PickupBlindfireProjectile(S32 classType, U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, int group_id) 
		: C_PickupProjectile(classType, id, firingIdx, -1, E_PICKUP_TYPE_BLINDFIRE, subType, noFiringVehicleFX)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BLINDFIRE);
		mpBlindfireData						= (RLBlindfireBalancingData *)pPickup->mNormal->mBalancingData;
		mpBlindfireSFX						= NULL;

		RLPickupLevel* pLevel = GetRLSubTypeLevel();
		if(pLevel && pLevel->mSpecialFX)
		{
			mpBlindfireSFX = (RLBlindfireSpecialFX*)pLevel->mSpecialFX;
		}

		BIZ_ASSERT(mpBlindfireData);

		mRadius				= mpBlindfireData->mRadius;
		mSpeed				= mpBlindfireData->mSpeed;
		mHeightFromGround	= mpBlindfireData->mHeightFromGround;
		mAliveTime			= 0.f;
		mFallRate			= mpBlindfireData->mFallRate;

		mGroupId			= group_id;

		BIZ_ASSERT(mpBlindfireSFX);

		mAITrackableData.mIsShootable = false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireProjectile::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		if( HasAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist, mFiringIdx) && AbilityActiveOnPickupType(UI::E_MultiplayerVehicleAbilities_Iron_Fist) )
		{
			RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BLINDFIRE);
			RLBlindfireSpecialFX* pBlindfireFX	= (RLBlindfireSpecialFX*)pPickup->mNormal->mSpecialFX;

			if(pBlindfireFX)
			{
				C_PickupBase::PostDelayInitialise(matrix, &pBlindfireFX->mIronFistFiredSpecialFX);
			}
		}
		else
		{
			C_PickupBase::PostDelayInitialise(matrix, sfx);
		}
		if(VehicleIsLocal(mFiringIdx))
		{
			SetPostDelayMatrix(matrix);
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, matrix);
			}
		}
	}

	// --------------------------------------------------------------------

	void C_PickupBlindfireProjectile::SetPostDelayMatrix( Maths::HMat43 targetMatrix, float fVal )
	{
		C_PickupBase::SetPostDelayMatrix(targetMatrix, fVal);

		Maths::HVec3 up(0.f, 1.f, 0.f);
		mMatrix.AW(mMatrix.AW() + (up * mRadius));

		CreateCollisionBody();
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpBlindfireSFX->mProjectileSpecialFX, mMatrix, 50.f * mMatrix.AZ(), mGroundPoint, mGroundNormal, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Projectile);
	}

	// --------------------------------------------------------------------------------

	bool C_PickupBlindfireProjectile::Update()
	{
		PERF_EVENT("PickupProjectile.Update");

		if(!mActivated)
			return false;
		m_PrevPos = mMatrix.AW();

		mAliveTime += C_GameApp::GetInstance()->GetTimeStep();

		if(mAliveTime > g_debugBlindfireLiveTime)
			KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_OLD_AGE);

		if(mKillPickup)
		{
			C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);

			KillPickupAudio();

			return true;
		}

		if(g_debugBlindfireDraw)
			DebugDraw();

		UpdateCollisionBody();

		C_VehicleLogicMsg_PerkUpdateMatrixMsg updateMatrixMsg(mUniqueId, mMatrix, mFiringIdx,mSpeed * mMatrix.AZ());

		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_UpdateMatrix, 0, &updateMatrixMsg);

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireProjectile::Trigger(int idx)
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
					mHitInfo.mDirection = mHitPoint - m_PrevPos;
					mHitInfo.mDirection.Normalise();
					mHitInfo.mDirection = mHitInfo.mDirection * mat.Rotation().Inverse();
				}
			}
			RegisterHit(idx);
			if(!CheckLegalHit(idx))
				NinjaKill(mAutopsyReport);		//NOTE : is this is special case gavin was talking about?

			// Store any data specific to this type of death
			mAutopsyReport.mTargettedVehicleId =  -1;
			mAutopsyReport.mVehicleIdHit	   =  idx;

			// Do the killing
			KillPickup(-1,E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE);
			
		}
	}

	// --------------------------------------------------------------------------------

	E_TARGETTING_DIRECTION C_PickupBlindfireProjectile::GetHitDirection(Actors::C_VehicleLogicActor* pActor)
	{
		E_TARGETTING_DIRECTION hitDir = E_TARGETTING_DIRECTION_NONE;

		if(pActor)
		{
			Maths::HVec3 dir = Maths::Normalise(mHitPoint - pActor->GetWorldMatrix().AW());
			float dotProdX = Maths::DotProduct(dir, pActor->GetWorldMatrix().AX());

			if( dotProdX < 0 )
			{
				hitDir = E_TARGETTING_DIRECTION_LEFT;
			}
			else
			{
				hitDir = E_TARGETTING_DIRECTION_RIGHT;
			}
		}

		return hitDir;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireProjectile::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(mMatrix.AW(), mRadius, Maths::RGBA(255,255,0));
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireProjectile::Explode(Maths::HVec3 pos, Maths::C_Normal normal)
	{
		Maths::AxisY axisY(normal);
		Maths::HMat33 mat33(axisY);
		Maths::HMat43 mat(mat33, pos);
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpBlindfireSFX->mHitEnvironmentSpecialFX, mat, 50.f * mMatrix.AZ(), mGroundPoint, mGroundNormal, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Missed);
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireProjectile::PhotoMove(F32 frac)
	{
		using namespace Maths;
		
		HVec3 movement = mMatrix.AW() - m_PrevPos;

		HMat43 tweakMatrix(mMatrix);
		tweakMatrix.AW(tweakMatrix.AW() + movement * frac);

		C_InGameSpecialFXManager::Instance().UpdatePerkMatrix(mUniqueId, tweakMatrix, true);
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireProjectile::NinjaKill(const Game::PickupAutopsyReport& autoReport) 
	{ 
		switch(autoReport.mCauseOfDeath)
		{
		case E_PICKUP_CAUSE_OF_DEATH_HIT_STATIC_OBJECT:
		case E_PICKUP_CAUSE_OF_DEATH_HIT_DYNAMIC_OBJECT:
		case E_PICKUP_CAUSE_OF_DEATH_HIT_ENVIRONMENT:
		case E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP:
			{
				C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpBlindfireSFX->mHitEnvironmentSpecialFX, mMatrix, 50.f * mMatrix.AZ(), mGroundPoint, mGroundNormal, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Missed);
				break;
			}
		}

		C_PickupProjectile::NinjaKill(autoReport); 
	};
}
