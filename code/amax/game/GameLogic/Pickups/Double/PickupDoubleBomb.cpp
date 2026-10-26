#include "stdafx.h"

#include <GameLogic/Pickups/Double/PickupDoubleBomb.h>
#include <GameLogic/Pickups/PickupMineBase.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <GameLogic\Layers\AmaxWorldLayer.h>

#include "GameAudio/GameAudioLayer.h"
#include "GameAudio/PickupSubsystem.h"

Debug::Int g_debugDoubleBombNumMines("Perks.DoubleBomb.NumMines", 3);
Debug::Float g_debugDoubleBombSpreadDistance("Perks.DoubleBomb.SpreadDistance", 15.f);
extern Debug::Float g_debugMineBehindDistance;

namespace Game
{
	C_PickupDoubleBomb::C_PickupDoubleBomb(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX)
		: C_PickupBase(E_PickupClass_DoubleBomb, id, firingIdx, E_PICKUP_TYPE_SHOCK, subType, noFiringVehicleFX)
	{
	}

	// --------------------------------------------------------------------------------

	void C_PickupDoubleBomb::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);

		if(VehicleIsLocal(mFiringIdx))
		{
			SetPostDelayMatrix(matrix);

			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, matrix);
			}
		}
	}
	
	// --------------------------------------------------------------------------------

	bool C_PickupDoubleBomb::Update()
	{
		if(!mActivated)
			return false;

		float dist = g_debugMineBehindDistance;
		mMatrix.AW(mMatrix.AW() - mMatrix.AZ() * dist);

		float spreadPerMine			= g_debugDoubleBombSpreadDistance / g_debugDoubleBombNumMines;
		float totalSpreadPerSide	= spreadPerMine * (g_debugDoubleBombNumMines / 2);

		for( float i = -totalSpreadPerSide; i <= totalSpreadPerSide; i += spreadPerMine )
		{
			Maths::HMat43 matrix = mMatrix;
			matrix.AW(matrix.AW() + matrix.AX() * i);	

			// Try tracing up a short way, we might have ended up underground.
			U8 data[1024 * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);
			Collision::C_Ray ray;
			Collision::C_CollisionFilter rayCollisionFilter;
			rayCollisionFilter.SetBodyMask(PolyFlags::kbWorld);

			Maths::HVec3 up(0.f,1.f,0.f);
			float dist = 3.f;
			ray.InitialiseRay(matrix.AW() + up * dist, matrix.AW(), &rayCollisionFilter);

			Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
			pRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
			Collision::C_RayTracingResult& result = ray.m_result;

			if(result.Hit())
				matrix.AW(result.HitPoint() + up * 0.5f);

			if(CanPlaceMine(matrix))
			{
				PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BOMB, E_PICKUP_SUB_TYPE_NORMAL, mFiringIdx);
				pickupInfo.mTargetIdx			= mFiringIdx;
				pickupInfo.mMatrix				= matrix;
				pickupInfo.mDisableVehicleFX	= true;
				pickupInfo.mOverrideMatrix		= true;
				pickupInfo.mVal					= 1; // Force the mine to raise up based on size.

				C_PickupsManager::Instance().CreatePickup(pickupInfo);
			}
		}

		return true;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupDoubleBomb::CanPlaceMine(Maths::HMat43& matrix)
	{
		Actors::RaceLogicCore* pRLC			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringIdx);

		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		Collision::C_Ray ray;
		Collision::C_CollisionFilter rayCollisionFilter;
		rayCollisionFilter.SetBodyMask(PolyFlags::kbWorld);

		ray.InitialiseRay(pActor->GetWorldMatrix().AW(), matrix.AW(), &rayCollisionFilter);

		pRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
		Collision::C_RayTracingResult& result = ray.m_result;

		if(result.Hit())
			return false;

		return true;
	}

	// --------------------------------------------------------------------------------

	// OLD DOUBLE MINE
	// DROPS A SINGLE, MASSIVE MINE

	/*
	Debug::Bool		g_debugDoubleMineDraw("Perks.DebugDraw.DoubleMine", true);
	Debug::Float	g_debugDoubleMineTimer("Perks.DoubleMine.Timer", 20.0f);
	Debug::Float	g_debugDoubleMineBlastRadius("Perks.DoubleMine.BlastRadius", 15.f);

	C_PickupDoubleBomb::C_PickupDoubleBomb(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) : 
		C_PickupBase(id, firingIdx, E_PICKUP_TYPE_BOMB, subType, noFiringVehicleFX)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BOMB);
		mpBombSFX							= (RLBombSpecialFX*)pPickup->mNormal->mSpecialFX;

		mTimer = g_debugDoubleMineTimer;
	}

	// --------------------------------------------------------------------------------

	void C_PickupDoubleBomb::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		if( HasAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist, mFiringIdx) )
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
			SetPostDelayMatrix(matrix);
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, matrix);
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupDoubleBomb::SetPostDelayMatrix( Maths::HMat43 matrix, float fVal )
	{
		C_PickupBase::SetPostDelayMatrix(matrix, fVal);
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpBombSFX->mMineSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Firing);

		//let the audio know the shock has moved to its landing position
		C_VehicleLogicMsg_PerkUpdateMatrixMsg updateMatrixMsg(mUniqueId, mMatrix, mFiringIdx,Maths::HVec3(0.f,0.f,0.f));
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_UpdateMatrix, 0, &updateMatrixMsg);
	}

	// --------------------------------------------------------------------------------

	bool C_PickupDoubleBomb::Update()
	{
		if(!mActivated)
			return false;

		if(g_debugDoubleMineDraw)
			DebugDraw();

		float frameTime				= C_GameApp::GetInstance()->GetTimeStep();

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		bool firerInRange = true;
		bool targetInRange = false;

		for( int i = 0; i < pLayer->GetNumVehicles(); i++ )
		{
			Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(i);
			float distance						= Maths::Distance(mMatrix.AW(), pActor->GetPositionInfo().mWorldPosition);

			if(distance > g_debugDoubleMineBlastRadius)
			{	
				if(i == mFiringIdx)
					firerInRange = false;
			}
			else if(VehicleIsLocal(i))
			{
				targetInRange = true;
			}

			if(!firerInRange && targetInRange)
				break;
		}

		if(mTimer > 0.f)
			mTimer -= frameTime;
		else
			mKillPickup = true;

		if((!firerInRange && targetInRange) || mKillPickup)
		{
			Explode();
			return true;
		}	

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupDoubleBomb::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mMatrix.AW(),g_debugDoubleMineBlastRadius,Maths::RGBA(255,0,0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),g_debugDoubleMineBlastRadius,Maths::RGBA(255,255,0));
	}

	// --------------------------------------------------------------------------------

	void C_PickupDoubleBomb::Explode()
	{
		// Remove the mine and play the boom effect
		C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpBombSFX->mExplodeSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Firing);

		C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		for( int i = 0; i < pLayer->GetNumVehicles(); i++ )
		{
			if( mFiringIdx != i && CheckCanHit(i))
			{
				Actors::C_VehicleLogicActor* pTargetActor = pLayer->GetLogicVehicleActor(i);
				float distance = Maths::Distance(mMatrix.AW(), pTargetActor->GetPositionInfo().mWorldPosition);

				if(distance <= g_debugDoubleMineBlastRadius)
				{
					RegisterHit(i);	
				}
			}
		}
	}*/
}