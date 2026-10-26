#include "stdafx.h"

#include <GameLogic/Pickups/PickupMineBase.h>

#include <gravity/sdk/ShapeDescriptors.h>
#include <gravity/sdk/BodyHelper.h>

#include "GameAudio/GameAudioLayer.h"
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>
#include <GameLogic\Structures\LevelStateConstants.h>
#include <GameApp.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game/GameDef/PhysicsConstants.h>

#include "System/misc/Random.h"

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <GameAudio\PickupSubsystem.h>
#include <Game\Game.h>

Debug::Float g_debugMineBehindDistance("Perks.Mine.BehindDistance", 5.0f);
Debug::Float g_MasterMinePrimeTimer("Perks.Mine.MasterMinePrimeTimer", 2.0f, Debug::Range(0.f, 5.f));
Debug::Bool g_CreateMasterMineOuterCollisionBody("Perks.Mine.CreateOuterCollisionBody", true);

Util::C_Random gPickupMineLevel2RandomNumber;

namespace Game
{
	C_PickupMine::C_PickupMine(S32 classType, U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX)
		: C_PickupBase(classType, id, firingIdx, E_PICKUP_TYPE_BOMB, subType, noFiringVehicleFX), 
			mTimer(0.f), 
			mpInnerCollisionBody(NULL), 
			mpOuterCollisionBody(NULL), 
			mInnerCollisionUserData(E_BodyUserDataType_MineInner),
			mOuterCollisionUserData(E_BodyUserDataType_MineOuter)
	{
		// The children should create the collision body, as we know the radius at that point.
		mMovementForce = 0.f;

		mAITrackableData.mIsTrackable = true;

		C_VehicleLogicMsg_FiredPerkMsg fired_msg(E_PICKUP_TYPE_BOMB, 0, firingIdx, id, mMatrix, 1, false, subType, false, 1);
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, ::PickUpSubsystem::E_PickUpSubsystem_Fire_Projectile, 0, &fired_msg);
	}

	// --------------------------------------------------------------------------------

	C_PickupMine::~C_PickupMine()
	{ 
		// A whole heap of checks here, because some of this stuff might have been destroyed before we get here.
		// Apparently, if this is the case, the body will already have been cleaned up anyway. =D
		C_RaceBookLayer* pRBLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if(pRBLayer)
		{
			C_AmaxWorldLayer* pWorldLayer = pRBLayer->GetAmaxWorldLayer();
			if(pWorldLayer)
			{	
				Game::Actors::RaceLogicCore* pRLC	= pWorldLayer->GetRaceLogicCore();
				C_RaceLogicLayer* pLayer			= pRBLayer->GetRaceLogicLayer();

				if(pRLC && pLayer)
				{
					if(mpInnerCollisionBody)
					{
						mpInnerCollisionBody->SetUserData(NULL);
						mpInnerCollisionBody->Deactivate(); 
						pRLC->GetPhysicsScene()->RemoveBody(mpInnerCollisionBody);
					}

					if(mpOuterCollisionBody)
					{
						mpOuterCollisionBody->SetUserData(NULL);
						mpOuterCollisionBody->Deactivate(); 
						pRLC->GetPhysicsScene()->RemoveBody(mpOuterCollisionBody);
					}
				}
			}	
		}
	};

	// --------------------------------------------------------------------------------

	void C_PickupMine::ApplyMatrixModifiers(Maths::HMat43 &matrix, const bool overrideSorcery, const float scale)
	{
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

		F32 mineRadiusMult = 1.f;
		if(!overrideSorcery)
		{
			mineRadiusMult = levelState.GetPickupInfo(mFiringIdx).BombInfo().mRadius;
			mineRadiusMult = mineRadiusMult == 0.0f ? 1.0f : mineRadiusMult;
		}
		else
		{
			mineRadiusMult = scale;
		}

		matrix.AX( Maths::HVec3( matrix.AX() * mineRadiusMult ) );
		matrix.AY( Maths::HVec3( matrix.AY() * mineRadiusMult ) );
		matrix.AZ( Maths::HVec3( matrix.AZ() * mineRadiusMult ) );

		Maths::HVec3 up = matrix.AY();
		up.Normalise();

		Maths::HVec3 back = -matrix.AZ();
		back.Normalise();

		if(!mIsReplacement)
		{
			Maths::HVec3 spawnPos = matrix.AW() + (up * mInnerCollisionRadius * mineRadiusMult / 2.0f) + (back * mBehindDistance);
			matrix.AW(spawnPos);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupMine::CreateCollisionBodies()
	{
		Actors::RaceLogicCore* pRLC						= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		BIZ_ASSERT_MSG(pRLC, "C_PickupMine::CreateCollisionBodies() > Couldn't get a pointer to the RLC :¬(");

		C_RaceLogicLayer* pLayer						= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		BIZ_ASSERT_MSG(pLayer, "C_PickupMine::CreateCollisionBodies() > Couldn't get a pointer to the RaceLogicLayer :¬(");

		Actors::C_VehicleLogicActor* pLogicActor		= pLayer->GetLogicVehicleActor(mFiringIdx);
		BIZ_ASSERT_MSG(pLogicActor, "C_PickupMine::CreateCollisionBodies() > Couldn't get a pointer to the LogicActor :¬(");

		Game::Actors::C_VehicleActor* pGraphicalActor	= pLogicActor->GetGraphicalActor(mFiringIdx);
		BIZ_ASSERT_MSG(pGraphicalActor, "C_PickupMine::CreateCollisionBodies() > Couldn't get a pointer to the GraphicalActor :¬(");

		//Create the inner collision body
		Collision::C_CollisionFilter innerCollisionFilter(0xffffffff, 0xffffffff, 0, 0, 0, PolyFlags::kbPerk, PolyFlags::kbVehicle | PolyFlags::kbPerk | PolyFlags::kbWorld);

		CreateCollisionBody(pRLC, pLayer, pLogicActor, pGraphicalActor, mInnerCollisionRadius, &mpInnerCollisionBody, Game::E_BodyUserDataType_MineInner, &mInnerCollisionUserData, innerCollisionFilter);

		if(g_CreateMasterMineOuterCollisionBody)
		{
			Collision::C_CollisionFilter outerCollisionFilter(0xffffffff, 0xffffffff, 0, 0, 0, PolyFlags::kbOuterMine, PolyFlags::kbVehicle);

			//...and the outer one
			CreateCollisionBody(pRLC, pLayer, pLogicActor, pGraphicalActor, mOuterCollisionRadius, &mpOuterCollisionBody, Game::E_BodyUserDataType_MineOuter, &mOuterCollisionUserData, outerCollisionFilter);
		}
	}

	void C_PickupMine::CreateCollisionBody(	Actors::RaceLogicCore			*pRLC, 
											C_RaceLogicLayer				*pLayer, 
											Actors::C_VehicleLogicActor		*pLogicActor, 
											Game::Actors::C_VehicleActor	*pGraphicalActor, 
											const float						radius,
											Gravity::C_Body					**ppCollisionBody, 
											Game::E_BodyUserDataType		eUserDataType,
											C_BodyUserDataMine				*pUserData,
											Collision::C_CollisionFilter&	collisionFilter)
	{
		//De-reference this sucker, I can't be arsed with dereferencing this every time I need to use it!
		Gravity::C_Body *pCollisionBody = (*ppCollisionBody);

		// Create the physics collision object
		pCollisionBody = pRLC->GetPhysicsScene()->CreateBody();
		
		Maths::HF32 mass(200.f);
		Maths::HMat33 rotation(Maths::Constants<Maths::HMat33>::Identity());
		Gravity::C_BoxDesc desc;
		desc.m_HalfExtents=Maths::HVec3(radius,radius,radius);
		desc.m_Margin = radius * 0.05f;
		desc.m_Epsilon = radius * 0.01f;

		Gravity::C_BodyHelper helper(&desc);
		helper.CreateBody(mass, mMatrix.AW(), rotation, pCollisionBody, &collisionFilter, Game::E_PhysicsMaterial_Vehicle, 0);

		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = -1.f;
		deactivator.m_SleepThresholdSq = -1.f;
		deactivator.m_sSleepCountdown = 120;
		pCollisionBody->SetDeactivator(deactivator);

		pCollisionBody->Baumgarte(Game::PhysicsConstants::kDefaultBaumgarte);
		pCollisionBody->Slop(Game::PhysicsConstants::kDefaultSlop);
		pCollisionBody->SetLinearDampingCoefficient(0.99f);
		pCollisionBody->SetAngularDampingCoefficient(0.98f);

		m_ExternalConstants.m_Gravity		= Maths::HVec3(0.0f, -9.8f, 0.0f);
		m_ExternalConstants.m_AirResistance = Maths::HVec3(50.0f, 0.0f, 50.0f);
		m_ExternalConstants.m_Wind			= Maths::HVec3(0.0f, 0.0f, 0.0f);
		pCollisionBody->SetExternalConstants(&m_ExternalConstants);

		pUserData->m_eType = eUserDataType;
		pUserData->m_pMineObject = this;
		pCollisionBody->SetUserData((void*)pUserData);

		pRLC->GetPhysicsScene()->AddBody(pCollisionBody);
		pCollisionBody->Activate();

		//Copy the address back
		(*ppCollisionBody) = pCollisionBody;
	}

	// --------------------------------------------------------------------------------

	void C_PickupMine::UpdateCollisionBodies()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringIdx);

		if(!mpInnerCollisionBody)
			return;

		mPrevPosition = mMatrix.AW();
		if(mMovementForce)
		{
			// Move around if we need to
			mpInnerCollisionBody->ApplyForce(mMovementDir * mMovementForce, mpInnerCollisionBody->GetBody().m_Position);
			mMovementForce = 0;
		}
		

		Maths::HVec3 newpos = mpInnerCollisionBody->GetBody().m_Position;

		// Keep my pos up to date with physics
		mMatrix.AW(newpos);

		// If I've moved, update.
		if(!(mMatrix.AW().All_EQ(mPrevPosition)))
		{
			//Apply the new position of the inner body to the outer one
			if(mpOuterCollisionBody)
				mpOuterCollisionBody->GetBody().m_Position = mpInnerCollisionBody->GetBody().m_Position;

			C_InGameSpecialFXManager::Instance().UpdatePerkMatrix(mUniqueId, mMatrix);
		}

		//update the bomb position
		C_VehicleLogicMsg_PerkUpdateMatrixMsg updateMatrixMsg(mUniqueId, mMatrix, mFiringIdx,Maths::HVec3(0.f,0.f,0.f));
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_UpdateMatrix, 0, &updateMatrixMsg);
	}

	// --------------------------------------------------------------------------------

	void C_PickupMine::ApplyForce(Maths::HVec3 dir, float force)
	{
		mMovementForce	= force;
		mMovementDir	= dir;
	}
}
