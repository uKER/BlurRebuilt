#include "stdafx.h"
#include <GameLogic/Pickups/PickupClasses.h>	// [Blur2PC]

#include <GameLogic/Pickups/Misc/PickupMiniBomb.h>
#include <GameLogic\MultiplayerUtilities.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>

#include "GameAudio/GameAudioLayer.h"

#include "GameAudio/PickupSubsystem.h"

#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>

Debug::Bool g_debugMiniMineDraw("Perks.DebugDraw.MiniMine", false);

namespace Game
{	
	C_PickupMiniBomb::C_PickupMiniBomb(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, float dirId, U32 seed, float timerOverride) 
		: C_PickupBomb(E_PickupClass_MiniBomb, id, firingIdx, subType, noFiringVehicleFX, timerOverride)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BOMB);
		mpBombData							= (RLThrownBombBalancingData*)pPickup->mMini->mBalancingData;
		mpBombSFX							= (RLBombSpecialFX*)pPickup->mMini->mSpecialFX;;

		BIZ_ASSERT(mpBombData);
		BIZ_ASSERT(mpBombSFX);

		mInnerCollisionRadius	= mpBombData->mRadius;

		mBehindDistance = mpBombData->mPlaceBehindDistance;

		timerOverride == -1.f ? mTimer = mpBombData->mLifespan : mTimer = timerOverride;

		mMiniMineId = (int)dirId;
		mRandGen.Seed(seed);
	}

	// --------------------------------------------------------------------------------

	void C_PickupMiniBomb::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);

		SetPostDelayMatrix(matrix);
	}

	// --------------------------------------------------------------------------------

	//We only want to create a single collision body this time
	void C_PickupMiniBomb::CreateCollisionBodies()
	{
		Actors::RaceLogicCore* pRLC						= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		BIZ_ASSERT_MSG(pRLC, "C_PickupMiniBomb::CreateCollisionBodies() > Couldn't get a pointer to the RLC :¬(");

		C_RaceLogicLayer* pLayer						= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		BIZ_ASSERT_MSG(pLayer, "C_PickupMiniBomb::CreateCollisionBodies() > Couldn't get a pointer to the RaceLogicLayer :¬(");

		Actors::C_VehicleLogicActor* pLogicActor		= pLayer->GetLogicVehicleActor(mFiringIdx);
		BIZ_ASSERT_MSG(pLogicActor, "C_PickupMiniBomb::CreateCollisionBodies() > Couldn't get a pointer to the LogicActor :¬(");

		Game::Actors::C_VehicleActor* pGraphicalActor	= pLogicActor->GetGraphicalActor(mFiringIdx);
		BIZ_ASSERT_MSG(pGraphicalActor, "C_PickupMiniBomb::CreateCollisionBodies() > Couldn't get a pointer to the GraphicalActor :¬(");

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

		//Create the inner collision body
		Collision::C_CollisionFilter collisionFilter(0xffffffff, 0xffffffff, 0, 0, 0, PolyFlags::kbPerk, PolyFlags::kbVehicle | PolyFlags::kbPerk | PolyFlags::kbWorld);
		CreateCollisionBody(pRLC, pLayer, pLogicActor, pGraphicalActor, mInnerCollisionRadius, &mpInnerCollisionBody, Game::E_BodyUserDataType_MineInner, &mInnerCollisionUserData, collisionFilter);

		float quadrant = Maths::Pi2 / pMasterMineBalancingData->mNoMiniMinesToCreate;
		float halfQuad = quadrant/2;

		float myQuad = (quadrant * mMiniMineId);
		
		float xMin = Maths::Sin(myQuad - halfQuad);
		float xMax = Maths::Sin(myQuad + halfQuad);

		float zMin = Maths::Cos(myQuad - halfQuad);
		float zMax = Maths::Cos(myQuad + halfQuad);

		RLThrownBombBalancingData* pMiniBombBalancingData = (RLThrownBombBalancingData*)mpBombData;

		mMovementDir.X(mRandGen.FRand(xMin, xMax));
		mMovementDir.Y(pMiniBombBalancingData->mThrowForceY);
		mMovementDir.Z(mRandGen.FRand(zMin, zMax));
		mMovementForce = mRandGen.FRand(pMiniBombBalancingData->mThrowForceMin, pMiniBombBalancingData->mThrowForceMax);
	}

	// --------------------------------------------------------------------------------
}
