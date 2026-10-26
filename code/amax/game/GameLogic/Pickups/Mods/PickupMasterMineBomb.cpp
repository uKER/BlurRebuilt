#include "stdafx.h"

#include <GameLogic/Pickups/Mods/PickupMasterMineBomb.h>
#include <GameLogic/Pickups/PickupMineBase.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameLogic/Pickups/PickupClasses.h>
#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <GameLogic\Layers\AmaxWorldLayer.h>

#include "GameAudio/GameAudioLayer.h"
#include "GameAudio/PickupSubsystem.h"

namespace Game
{
	C_PickupMasterMineBomb::C_PickupMasterMineBomb(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, U32 seed)
		: C_PickupBase(E_PickupClass_MasterMineBomb, id, firingIdx, E_PICKUP_TYPE_BOMB, subType, noFiringVehicleFX)
	{
		mRandGen.Seed(seed);

		//Get the pickup level for the modified version of the pickup
		RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(mPickupType);
		BIZ_ASSERT_MSG(pPickup, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the pickup :(");

		RLPickupLevel* pMasterMineData = pPickup->mModified;
		BIZ_ASSERT_MSG(pMasterMineData, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the master mine sublevel :(");

		RLBombModifiedBalancingData *pMasterMineBalancingData = ((RLBombModifiedBalancingData *)pMasterMineData->mBalancingData);
		BIZ_ASSERT_MSG(pMasterMineBalancingData, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the master mine balancing data :(");

		if(VehicleIsLocal(mFiringIdx))
		{
			C_PickupsManager::Instance().IncrementUniqueId(pMasterMineBalancingData->mNoMiniMinesToCreate);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupMasterMineBomb::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
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

	bool C_PickupMasterMineBomb::Update()
	{
		if(!mActivated)
			return false;

		// This is the big momma that exists solely to spawn little evil mine babies. 
		// At this point, there is no going back, mastermine has been successfully activated!

		// Every kit does this individually
		//if(Game::VehicleIsLocal(mFiringIdx))
		//{
			//MinePlacementFixed(); // Let's try a new way!
			MinePlacementRandom();

			// Send to challenges component
			C_RaceLogicLayer* pLayer				 = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Actors::C_VehicleLogicActor* pLogicActor = pLayer->GetLogicVehicleActor(mFiringIdx);
			if(pLogicActor)
			{
				pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, E_GameLogicMessage_MastermineSpawnedMinis);

				// Increment mastermine stats for awards
				pLogicActor->mStatistics.mMinesFragmentedWithMastermine++;
			}
		//}

		return true;
	}

	// --------------------------------------------------------------------------------

	void C_PickupMasterMineBomb::MinePlacementFixed()
	{
		Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		BIZ_ASSERT_MSG(pRLC, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the RLC :-(");

		//Get the pickup level for the modified version of the pickup
		RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(mPickupType);
		BIZ_ASSERT_MSG(pPickup, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the pickup :(");

		RLPickupLevel* pMasterMineData = pPickup->mModified;
		BIZ_ASSERT_MSG(pMasterMineData, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the master mine sublevel :(");

		//Get the pickup level for our new mini bombs
		RLPickupLevel* pMiniMineData = GetRLSubTypeLevel(E_PICKUP_SUB_TYPE_MINI);
		BIZ_ASSERT_MSG(pMiniMineData, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the mini bombs sublevel :(");

		RLBombModifiedBalancingData *pMasterMineBalancingData = ((RLBombModifiedBalancingData *)pMasterMineData->mBalancingData);
		BIZ_ASSERT_MSG(pMasterMineBalancingData, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the master mine balancing data :(");

		float noMines = (float)pMasterMineBalancingData->mNoMiniMinesToCreate;
		float spreadPerMine	= 0.f;
		float totalSpread = (float)pMasterMineBalancingData->mTotalSpreadWidth;
		if(noMines > 0)
			spreadPerMine = totalSpread / noMines;

		float totalSpreadPerSide = spreadPerMine * (noMines / 2.f);

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


			pRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
			Collision::C_RayTracingResult& result = ray.m_result;

			if(result.Hit())
				matrix.AW(result.HitPoint() + up * 0.5f);

			if(CanPlaceMine(matrix))
			{
				PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BOMB, E_PICKUP_SUB_TYPE_MINI, mFiringIdx);
				pickupInfo.mTargetIdx			= mFiringIdx;
				pickupInfo.mMatrix				= mMatrix;
				pickupInfo.mDisableVehicleFX	= true;
				pickupInfo.mOverrideMatrix		= true;
				pickupInfo.mVal					= 1; // Force the mine to raise up based on size.

				C_PickupsManager::Instance().CreatePickup(pickupInfo);
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupMasterMineBomb::MinePlacementRandom()
	{
		//Get the pickup level for the modified version of the pickup
		RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(mPickupType);
		BIZ_ASSERT_MSG(pPickup, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the pickup :(");

		RLPickupLevel* pMasterMineData = pPickup->mModified;
		BIZ_ASSERT_MSG(pMasterMineData, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the master mine sublevel :(");

		RLBombModifiedBalancingData *pMasterMineBalancingData = ((RLBombModifiedBalancingData *)pMasterMineData->mBalancingData);
		BIZ_ASSERT_MSG(pMasterMineBalancingData, "C_PickupMasterMineBomb::Update() > Couldn't get a pointer to the master mine balancing data :(");

		for( int i = 1; i <= pMasterMineBalancingData->mNoMiniMinesToCreate; i++ )
		{
			PickupCreationStruct pickupInfo(E_PICKUP_TYPE_BOMB, E_PICKUP_SUB_TYPE_MINI, mFiringIdx);
			pickupInfo.mTargetIdx			= mFiringIdx;
			pickupInfo.mMatrix				= mMatrix;
			pickupInfo.mDisableVehicleFX	= true;
			pickupInfo.mOverrideMatrix		= true;
			pickupInfo.mVal					= -1;
			pickupInfo.mVal2				= (float)i;
			pickupInfo.mPickupId			= mUniqueId + i;
			pickupInfo.mVal3				= mRandGen.Rand(-1);
			pickupInfo.mDelay				= ((float)i - 1) * pMasterMineBalancingData->mDelay;

			C_PickupsManager::Instance().CreatePickup(pickupInfo);	
		}
	}

	// --------------------------------------------------------------------------------

	bool C_PickupMasterMineBomb::CanPlaceMine(Maths::HMat43& matrix)
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
}
