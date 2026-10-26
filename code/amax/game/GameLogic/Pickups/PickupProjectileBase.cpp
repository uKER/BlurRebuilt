#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <GameLogic/Pickups/PickupProjectileBase.h>
#include <GameLogic\Pickups\PickupMineBase.h>

#include <gravity/sdk/ShapeDescriptors.h>
#include <gravity/sdk/BodyHelper.h>

#include "GameAudio/GameAudioLayer.h"
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>
#include <GameApp.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <GameLogic\ActorStudioActors\DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/PhysicalObjectActor.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <GameAudio\PickupSubsystem.h>
#include <GameLogic/StickerManager.h>

#include <Game/GameDef/PhysicsConstants.h>
#include <System/scheduler.h>
#include <SchedulerConfig.h>

#include "System/misc/Random.h"

Debug::Float g_debugProjectileExplodeDot("Perks.Projectile.ExplodeDot", 0.9f);
Debug::Float g_debugProjectileSpeedYScale("Perks.Projectile.SpeedYScale", 1.f);
Debug::Float g_debugProjectileAngle("Perks.Projectile.UpAngle", 0.5f);
Debug::Float g_debugProjectileReboundAngle("Perks.Projectile.ReboundAngle", 0.1f);
Debug::Float g_debugProjectileCosAngle("Perks.Projectile.CosAngle", 0.2f);

Debug::Float g_debugProjectileFallRateMin("Perks.Projectile.FallRateMin", 1.f);
Debug::Float g_debugProjectileFallRateDot("Perks.Projectile.FallRateDot", 40.f);

Debug::Float g_debugProjectileTraceLengthUp("Perks.Projectile.TraceLengthUp", 2.0f);
Debug::Float g_debugProjectileTraceLengthDown("Perks.Projectile.TraceLengthDown", 100.0f);
Debug::Float g_debugProjectileTraceSearchIncrement("Perks.Projectile.SearchIncrement", 1.0f);
Debug::Float g_debugProjectileTraceSearchMax("Perks.Projectile.SearchMax", 5.0f);
Debug::Float g_debugProjectileDistancePerTrace("Perks.Projectile.DistancePerTrace", 5.f);

Debug::Float g_debugCollideWithEnvironmentYOffset("Perks.Projectile.CollideWithEnvYOffset", -0.3f);

Debug::Bool g_debugProjectileDrawRay("Perks.DebugDraw.DrawProjectileRay", false);

namespace Game
{
	C_PickupProjectile::C_PickupProjectile(S32 classType, U32 id, int firingIdx, int targetIdx, E_PICKUP_TYPE type, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupBase(classType, id, firingIdx, type, subType, noFiringVehicleFX), mTargetIdx(targetIdx), mpCollisionBody(NULL), mpRLC(NULL)
	{
		mpRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		mAITrackableData.mIsTrackable = true;

		for( int i = 0; i < mDeferredRayList.Size(); i++ )
		{
			Collision::C_TransparentRay& ray = mDeferredRayList[i];
			ray.SetNumLayers(5);
		}

		S32 vehGroupId = mpRaceLogicLayer->GetLogicVehicleActor(firingIdx)->GetGraphicalActor(firingIdx)->GetPhysicalGroupID();

		mSolidRayFilter.SetBodyMask(PolyFlags::kbPerk | PolyFlags::kbVehicle | PolyFlags::kbWorld);
		mSolidRayFilter.SetGroupId(vehGroupId);
		mSolidRayFilter.SetGroupFlags(0);

		mTransparentRayFilter.SetBodyMask(PolyFlags::kbDynamicObject);
		mTransparentRayFilter.SetGroupId(vehGroupId);
		mTransparentRayFilter.SetGroupFlags(0);

		mGroundRayFilter.SetCollisionMask(PolyFlags::kWheelSolid | PolyFlags::kFluid);
		mGroundRayFilter.SetBodyMask(PolyFlags::kbWorld);

		C_VehicleLogicMsg_FiredPerkMsg fired_msg(type, 0, firingIdx, id, mMatrix, 1, false, subType, false, 1);
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, ::PickUpSubsystem::E_PickUpSubsystem_Fire_Projectile, 0, &fired_msg);

		mSpeed = 0.f;
		mFallRate = 1.f;
		mMovementDir = Maths::HVec3(0.f, 0.f, 0.f);
	}; 

	// --------------------------------------------------------------------------------

	C_PickupProjectile::~C_PickupProjectile()
	{
		if(mpCollisionBody)
		{
			mpCollisionBody->SetUserData(NULL);
			mpCollisionBody->Deactivate(); 
			mpRLC->GetPhysicsScene()->RemoveBody(mpCollisionBody);
			mpCollisionBody = NULL;
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::CreateCollisionBody()
	{
		Actors::C_VehicleLogicActor* pActor	= mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringIdx);

		mpRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		mpRLC				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		if(mTargetIdx != -1)
		{
			Maths::HVec3 targetPos = mpRaceLogicLayer->GetLogicVehicleActor(mTargetIdx)->GetWorldMatrix().AW();
			mMovementDir = Maths::Normalise(targetPos - mMatrix.AW());
		}
		else
		{
			mMovementDir = mStartingMatrix.AZ();
		}

		mpCollisionBody		= mpRLC->GetPhysicsScene()->CreateBody();

		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = -1.f;
		deactivator.m_SleepThresholdSq = -1.f;
		deactivator.m_sSleepCountdown = 120;
		mpCollisionBody->SetDeactivator(deactivator);

		Maths::HF32 mass(200.f);
		Maths::HMat33 rotation(Maths::Constants<Maths::HMat33>::Identity());
		Gravity::C_SphereDesc desc;
		desc.m_Radius=mRadius;
		desc.m_Margin = mRadius * 0.05f;
		desc.m_Epsilon = mRadius * 0.01f;

		S32 vehGroupId = mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx)->GetGraphicalActor(mFiringIdx)->GetPhysicalGroupID();

		Collision::C_CollisionFilter worldFilter;
		worldFilter.SetCollisionMask(worldFilter.GetCollisionMask() & (~PolyFlags::kVehicleSolid));
		worldFilter.SetBodyMask(worldFilter.GetBodyMask() & (~PolyFlags::kbVehicle));
		worldFilter.SetBodyFlags(PolyFlags::kbPerk);
		worldFilter.SetGroupId(vehGroupId);
		worldFilter.SetGroupMask(0);

		Gravity::C_BodyHelper helper(&desc);
		helper.CreateBody(mass, mMatrix.AW(), rotation, mpCollisionBody, &worldFilter, Game::E_PhysicsMaterial_Vehicle, 0);

		mUserData.m_pProjectileObject = this;
		mpCollisionBody->SetUserData((void*)&mUserData);

		mpRLC->GetPhysicsScene()->AddBody(mpCollisionBody);
		mpCollisionBody->Activate();
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::CollideWithEnvironment(Maths::HVec3 hitPoint, Maths::C_Normal normal)
	{
		hitPoint.Y(hitPoint.Y() + g_debugCollideWithEnvironmentYOffset);
		Explode(hitPoint, normal);
		KillPickup(-1,E_PICKUP_CAUSE_OF_DEATH_HIT_ENVIRONMENT);
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::CollideWithPickup(int vehIdx)
	{
		 KillPickup(vehIdx, E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP);
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::PreUpdateOne()
	{
		if(!mpRaceLogicLayer || !mpRLC || !mActivated)
			return;

		Actors::C_VehicleLogicActor* pActor	= mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();

		mPrevPos					= mMatrix.AW();
		Maths::HVec3 alignedPos		= mPrevPos + (mMovementDir * mSpeed * frameTime);

		KickOffAlignProjectileToGround(alignedPos);

		mAITrackableData.mVelocity = mMovementDir * mSpeed;
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::PreUpdateTwo()
	{
		if(!mpRaceLogicLayer || !mpRLC || !mActivated)
			return;

		float frameTime				= C_GameApp::GetInstance()->GetTimeStep();

		mPrevPos					= mMatrix.AW();
		Maths::HVec3 alignedPos		= mPrevPos + (mMovementDir * mSpeed * frameTime);

		bool isFalling = AlignProjectileToGround(alignedPos);

		// Fell out of world - just die
		if(isFalling)
		{
			CollideWithEnvironment(mMatrix.AW(), Maths::C_Normal(-mMovementDir));

			//let audio know projectile has hit the world
			C_VehicleLogicMsg_PerkEnded pMsg(mFiringIdx,mUniqueId,mPickupType,Game::E_BodyUserDataType_Unknown);
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Projectile_Hit_World, 0, &pMsg);
		}
		else
			mMatrix.AW(alignedPos);		

		if(!(mMatrix.AW().All_EQ(mPrevPos))) // If I've moved, update.
		{
			C_InGameSpecialFXManager::Instance().UpdatePerkMatrix(mUniqueId, mMatrix);
			mpCollisionBody->Position(mMatrix.AW());

			// Kick off deferred ray traces
			KickOffCollision(mPrevPos, alignedPos, 0);

			Maths::HVec3 straightPos = mPrevPos + (mMovementDir * mSpeed * frameTime * 1.2f);
			KickOffCollision(mPrevPos, straightPos, 1);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::UpdateCollisionBody()
	{		
		if(!(mMatrix.AW().All_EQ(mPrevPos))) // If I've moved, update.
		{			
			float frameTime	= C_GameApp::GetInstance()->GetTimeStep();
			if(!CheckCollisions(mPrevPos, 0))
				CheckCollisions(mPrevPos, 1);

			mMovementDir = Maths::Normalise(mMatrix.AW() - mPrevPos);
			
			float dirY = Maths::Clamp((F32)mMovementDir.Y(), -0.1f, 0.1f);
			mMovementDir.Y(dirY);

			// Seems to be exploding the matrix if these are 0
			if(abs(mMovementDir.X()) > 0.01f || abs(mMovementDir.Z()) > 0.01f)
			{
				// fill in the orientation
				mMatrix.AZ(mMovementDir);
				Maths::HVec3 mySideDirection(mMovementDir.Z(), mMovementDir.Y(), -mMovementDir.X());
				mMatrix.AX(mySideDirection);
				mMatrix.AY(mySideDirection ^ mMovementDir);

				// should really then orthonormalise the matrix to be on the safe side
				Maths::Mat33 newRotation = Maths::OrthoNormaliseZX( mMatrix.Rotation() );
				Maths::HMat33 temp(newRotation);
				mMatrix.AX(temp.AX()); mMatrix.AY(temp.AY()); mMatrix.AZ(temp.AZ());
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::KickOffAlignProjectileToGround(Maths::HVec3& pos)
	{
		const Maths::HVec3 up(0.f, 1.f, 0.f);
		const Maths::HVec3 down(0.f, -1.f, 0.f);
		Maths::HVec3 downTrace = down * g_debugProjectileTraceLengthDown;

		Scheduler::GroupHandle& handle = C_PickupsManager::Instance().GetPickupSchedulerGroupHandle();

		mDeferredGroundRay.InitialiseRay(pos + (up * g_debugProjectileTraceLengthUp), pos + downTrace, &mGroundRayFilter);
		mpRLC->GetPhysicsScene()->DoDeferredRayTrace(&mDeferredGroundRay, NULL, handle, C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy());
	}

	// --------------------------------------------------------------------------------

	bool C_PickupProjectile::AlignProjectileToGroundImmediate(Maths::HVec3& pos)
	{
		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);

		const Maths::HVec3 up(0.f, 1.f, 0.f);
		const Maths::HVec3 down(0.f, -1.f, 0.f);
		Maths::HVec3 downTrace = down * g_debugProjectileTraceLengthDown;

		mDeferredGroundRay.InitialiseRay(pos + (up * g_debugProjectileTraceLengthUp), pos + downTrace, &mGroundRayFilter);
		mpRLC->GetPhysicsScene()->DoRayTrace(mDeferredGroundRay, stack);

		Collision::C_RayTracingResult& result = mDeferredGroundRay.m_result;

		if( result.Hit() )
		{
			pos = result.HitPoint();
			return true;
		}	

		return false;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupProjectile::AlignProjectileToGround(Maths::HVec3& pos)
	{
		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		bool falling	= false;

		const Maths::HVec3 up(0.f, 1.f, 0.f);
		const Maths::HVec3 down(0.f, -1.f, 0.f);

		Collision::C_RayTracingResult& result = mDeferredGroundRay.m_result;

		bool trySecondTrace = true;
		if( result.Hit() )
		{
			CheckFall(pos, result);
			trySecondTrace = false;
		}	

		if(trySecondTrace)
		{
			// We might have ended up under the world with our move, so lets try and save ourselves.
			Maths::HVec3 downTrace = down * g_debugProjectileTraceLengthDown;
			Maths::HVec3 end =  pos + downTrace;
			Maths::HVec3 start;

			U8 data[1024 * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);

			bool secondaryHit = false;
			for(float i = 0; i < g_debugProjectileTraceSearchMax; i += g_debugProjectileTraceSearchIncrement)
			{
				start = pos + (up * i);
				
				mDeferredGroundRay.InitialiseRay(start, end, &mGroundRayFilter);
				mpRLC->GetPhysicsScene()->DoRayTraceMesh(mDeferredGroundRay, stack);
				Collision::C_RayTracingResult& secondRayRes = mDeferredGroundRay.m_result;

				if( secondRayRes.Hit() )
				{
					CheckFall(pos, result);
					secondaryHit = true;
					break;
				}
			}

			if (secondaryHit == false)
			{
				falling = true;
			}
		}

		return falling;
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::CheckFall(Maths::HVec3& pos, const Collision::C_RayTracingResult& rayResult)
	{
		Maths::C_Normal upNormal(0,1,0);
		float dot = Maths::Abs(Maths::DotProduct(rayResult.TriangleNormal(), upNormal));
		float invDot = 1.f - dot;

		Maths::HVec3 newPos = rayResult.HitPoint();
		mGroundPoint = newPos;
		mGroundNormal = rayResult.TriangleNormal();

		float fallDist = (g_debugProjectileFallRateMin + (g_debugProjectileFallRateDot * invDot)) * mFallRate;
		float minY = pos.Y() - fallDist;

		if(newPos.Y() < minY)
			newPos.Y(minY + mRadius + mHeightFromGround);
		else
			newPos.Y(newPos.Y() + mRadius + mHeightFromGround); 

		pos = newPos;
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::ReportMinedDestroyedEvent(C_PickupMine* pMine)
	{
		// Send the message to all cars in the world, across the network. 
		const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		netGameVars.m_EventManager->Add_MineDestroyedByProjectile(GetType(), GetSubType(), GetJoinIdFromLocalIndex(mFiringIdx), GetUid(), 
			E_PICKUP_TYPE_BOMB, pMine->GetSubType(), Game::GetJoinIdFromLocalIndex(pMine->GetFiringIdx()), pMine->GetUid());

		if(VehicleIsLocal(mFiringIdx) || VehicleIsLocal(pMine->GetFiringIdx()))
		{
			//also send the message to our challenge component in case we're interested!
			C_VehicleLogicMsg_DestroyedPickupWithPickup mineDestroyedMsg( GetType(), GetSubType(), mFiringIdx, GetUid(), E_PICKUP_TYPE_BOMB,
																						pMine->GetSubType(), pMine->GetFiringIdx(), pMine->GetUid());
			if(VehicleIsLocal(mFiringIdx))
			{
				const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(mFiringIdx);
				if(pLogicActor && pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
				{
					//Tell the multiplayer challenges component 
					pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &mineDestroyedMsg);
				}
			}
			// if these vehicles are distinct local vehicles, tell both
			if(mFiringIdx != pMine->GetFiringIdx() && VehicleIsLocal(pMine->GetFiringIdx()))
			{
				const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(pMine->GetFiringIdx());
				if(pLogicActor && pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
				{
					//Tell the multiplayer challenges component 
					pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &mineDestroyedMsg);
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::ReportProjectileDestroyedEvent(C_PickupProjectile* pProjectile)
	{
		// If we destroyed a projectile, we want some credit. Dont care about others. 
		if(Game::VehicleIsLocal(mFiringIdx))
		{
			C_VehicleLogicMsg_DestroyedPickupWithPickup projectileDestroyedMsg( GetType(), GetSubType(), mFiringIdx, GetUid(),
									pProjectile->GetType(), pProjectile->GetSubType(), pProjectile->GetFiringIdx(), pProjectile->GetUid() );

			Actors::C_VehicleLogicActor* pActor	= mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx);
			if(pActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
			{
				//Tell the multiplayer challenges component 
				pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &projectileDestroyedMsg);
			}

			Game::C_StickerManager::Instance().MessageHandler(&projectileDestroyedMsg);
		}

		// NOTE: could add a net event here if others would like this information in future. 
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::KickOffCollision(Maths::HVec3 oldPos, Maths::HVec3 newPos, int rayIdx)
	{
		if(rayIdx >= 0 && rayIdx < mDeferredRayList.Size())
		{
			float dist = Maths::Distance(oldPos, newPos);
			Maths::HVec3 dir = Maths::Normalise(newPos - oldPos);
			Maths::C_Normal normal(dir);

			mDeferredRayList[rayIdx].InitialiseRay(oldPos, normal, dist, &mSolidRayFilter, &mTransparentRayFilter, (F32)mRadius);
			//ray.SetMaxCosAngle(-0.707f);
			//ray.SetMaxCosAngle((F32)-g_debugProjectileCosAngle);
			Scheduler::GroupHandle& handle = C_PickupsManager::Instance().GetPickupSchedulerGroupHandle();
			mpRLC->GetPhysicsScene()->DoDeferredTransparentRayTrace(&mDeferredRayList[rayIdx], handle, C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy());	
			
			mDeferredThinRayList[rayIdx].InitialiseRay(oldPos, newPos, &mSolidRayFilter);
			mpRLC->GetPhysicsScene()->DoDeferredRayTrace(&mDeferredThinRayList[rayIdx], NULL, handle, C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy());	

			if(g_debugProjectileDrawRay)
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(oldPos, oldPos + (normal * dist), Maths::RGBA(255,255,0));
		}
	}

	// --------------------------------------------------------------------------------

	bool C_PickupProjectile::CheckCollisions(Maths::HVec3 oldPos, int rayIdx)
	{
		if(rayIdx >= mDeferredRayList.Size() || rayIdx < 0)
			return false;

		bool collided = false;
		Maths::HVec3 down(0.f, -1.f, 0.f);

		Collision::C_TransparentRay& ray		= mDeferredRayList[rayIdx];
		Collision::C_RayTracingResult* result	= ray.GetResult();

		Actors::C_VehicleLogicActor* pFiringActor = mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx);
		Maths::HVec3 explodePoint;
		Maths::HVec3 explodeNormal;
		int numExplodePoints = 0;

		if(ray.m_SolidResult.Hit())
		{
			const Gravity::C_Body* pBody(ray.m_SolidResult.GetBody());

			if(pBody && pBody != mpCollisionBody)
			{
				const Actor::C_Actor* pActor = NULL;

				C_BodyUserDataBase* pUserData = (C_BodyUserDataBase*)pBody->GetUserData<void>();
				if (pUserData)
				{
					if(pUserData->m_eType == E_BodyUserDataType_Actor)
					{
						Actor::C_ActorHandle handle = ((C_BodyUserDataActor*)pUserData)->m_hActorHandle;
						pActor = mpRLC->GetActorList()->GetActorPtr(handle);

						if(pActor->GetActorTypeId() == E_ActorTypeId_Vehicle)
						{
							Game::Actors::C_VehicleActor* pVehActor = (Game::Actors::C_VehicleActor*)pActor;
							if(mFiringIdx != pVehActor->GetVehicleIndex())
							{
								mHitPoint = ray.m_SolidResult.HitPoint();
								Trigger(pVehActor->GetVehicleIndex());
							}
						}
					}
					else if(pUserData->m_eType == E_BodyUserDataType_MineInner)
					{
						C_PickupMine* pMine = ((C_BodyUserDataMine*)pUserData)->m_pMineObject;

						//If we are newer we killed them
						bool iKilledThem = GetCreationTime() >= pMine->GetCreationTime();

						if(pMine->ApplyDamage((float)GetDamageToApply(), GetUid()))
						{
							pMine->KillPickup(iKilledThem ? mFiringIdx : -1,E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP);

							// Lets inform interested parties about this (but only if we killed it! blindfire...). 
							if(iKilledThem)
							{
								ReportMinedDestroyedEvent(pMine);
							}
						}

						if(ApplyDamage((float)pMine->GetDamageToApply(), pMine->GetUid()))
						{
							CollideWithPickup(iKilledThem ? -1 : pMine->GetFiringIdx());
						}
					}
					else if(pUserData->m_eType == E_BodyUserDataType_Projectile)
					{
						C_PickupProjectile* pProjectile = ((C_BodyUserDataProjectile*)pUserData)->m_pProjectileObject;

						if(pProjectile->GetUid() != mUniqueId)
						{
							//If we are newer we killed them
							bool iKilledThem = GetCreationTime() >= pProjectile->GetCreationTime();

							if(pProjectile->ApplyDamage((float)GetDamageToApply(), GetUid()))
							{
								// We only want the special FX to play once when two shunts explode...
								if(!(GetType() == E_PICKUP_TYPE_SHUNT && pProjectile->GetType() == E_PICKUP_TYPE_SHUNT))
									pProjectile->CollideWithPickup(iKilledThem ? mFiringIdx : -1);
								else
									pProjectile->KillPickup(iKilledThem ? mFiringIdx : -1,E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP);

								if(iKilledThem)
								{
									// Lets inform interested parties about this.
									ReportProjectileDestroyedEvent(pProjectile);
								}
							}

							if(ApplyDamage((float)pProjectile->GetDamageToApply(), pProjectile->GetUid()))
							{
								CollideWithPickup(iKilledThem ? -1 : pProjectile->GetFiringIdx());
								//They are newer - they killed us
								if(!iKilledThem)
								{
									pProjectile->ReportProjectileDestroyedEvent(this);
								}
							}
						}
					}

					//let audio know projectile has hit an object
					C_VehicleLogicMsg_PerkEnded pMsg(mFiringIdx,mUniqueId,mPickupType,pUserData->m_eType);
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Projectile_Hit_World, 0, &pMsg);

					collided = true;
				}
				else
				{
					if(CheckIsValidWallCollision(ray.m_SolidResult))
					{
						if(CheckRebound(ray.m_SolidResult))
						{
							Collision::C_Ray& thinRay						= mDeferredThinRayList[rayIdx];
							const Collision::C_RayTracingResult& thinResult	= thinRay.GetResult();

							if(thinResult.Hit() && CheckIsValidWallCollision(thinResult))
							{
								CollideWithEnvironment(ray.m_SolidResult.HitPoint(), ray.m_SolidResult.Normal());

								//let audio know projectile has hit the world
								C_VehicleLogicMsg_PerkEnded pMsg(mFiringIdx,mUniqueId,mPickupType,Game::E_BodyUserDataType_Unknown);
								Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Projectile_Hit_World, 0, &pMsg);
							}
						}

						collided = true;
					}
				}		
			}
		}

		for(U32 i = 0; i < ray.m_numLayers; i++)
		{
			const Gravity::C_Body* pBody(result[i].GetBody());

			if(!pBody || pBody == mpCollisionBody)
				continue;

			const Actor::C_Actor* pActor = NULL;

			C_BodyUserDataBase* pUserData = (C_BodyUserDataBase*)pBody->GetUserData<void>();
			if (pUserData)
			{
				if(pUserData->m_eType == E_BodyUserDataType_Actor)
				{
					Actor::C_ActorHandle handle = ((C_BodyUserDataActor*)pUserData)->m_hActorHandle;
					pActor = mpRLC->GetActorList()->GetActorPtr(handle);

					if(pActor)
					{
						if(pActor->GetActorTypeId() == E_ActorTypeId_DynamicObject || pActor->GetActorTypeId() == E_ActorTypeId_PhysicalObject)
						{
							RLPickupForceEffects* forces = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetDatabase()->mRLSystems[0]->mPickupForceEffects;
							Maths::HVec3 dir = Maths::Normalise(pActor->GetMatrix().AW() - oldPos);

							ActorMsg::C_DynamicObjectApplyImpulseMsg msg;

							switch(mPickupType)
							{
							case E_PICKUP_TYPE_SHUNT:
								{
									msg.m_Force = dir * forces->m_ShuntFire.m_MaxForce;
									msg.m_ElectricalEffect = forces->m_ShuntFire.m_ElectricalEffect;
									msg.m_ElectricalEffectColour = forces->m_ShuntFire.m_ElectricalEffectColour;
									msg.m_DamageScale = forces->m_ShuntFire.m_DamageScale;
									break;
								}

							case E_PICKUP_TYPE_BOMB:
								{
									msg.m_Force = dir * forces->m_MineProjectile.m_MaxForce;
									msg.m_ElectricalEffect = forces->m_MineProjectile.m_ElectricalEffect;
									msg.m_ElectricalEffectColour = forces->m_MineProjectile.m_ElectricalEffectColour;
									msg.m_DamageScale = forces->m_MineProjectile.m_DamageScale;
									break;
								}

							case E_PICKUP_TYPE_BLINDFIRE:
								{
									msg.m_Force = dir * forces->m_Blindfire.m_MaxForce;
									msg.m_ElectricalEffect = forces->m_Blindfire.m_ElectricalEffect;
									msg.m_ElectricalEffectColour = forces->m_Blindfire.m_ElectricalEffectColour;
									msg.m_DamageScale = forces->m_Blindfire.m_DamageScale;
									break;
								}
							}

							pActor->GetActorList()->SendMessageToActor(pActor->GetHandle(), &msg);

							if (forces->m_ShuntFire.m_ElectricalEffect)
								pActor->GetActorList()->DelayMessageToActorBySeconds(1.0f + ::Actors::C_ActorUtils::Instance().GetRandomNum(), pActor->GetHandle(), ActorMsg::E_DynamicObjMsg_DynamicObjectDisableElectricalEffect);	// End electrical effect message
						}
					}
				}
			}	
		}

		return collided;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupProjectile::CheckIsValidWallCollision(const Collision::C_RayTracingResult& rayResult)
	{
		const Maths::HVec3 down(0.f, -1.f, 0.f);
		Maths::C_Normal upNormal(-down);
		float dot = Maths::Abs(Maths::DotProduct(rayResult.TriangleNormal(), upNormal));
		if(dot < g_debugProjectileAngle)
			return true;

		return false;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupProjectile::CheckRebound(const Collision::C_RayTracingResult& rayResult)
	{
		Maths::C_Normal zNormal(mMovementDir);
		float reboundDot = Maths::Abs(Maths::DotProduct(rayResult.TriangleNormal(), zNormal));
		bool canRebound = (reboundDot < g_debugProjectileReboundAngle); 

		if(!canRebound || !Rebound(rayResult.TriangleNormal(), rayResult.HitPoint()))
			return true;

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupProjectile::KillPickupAudio(E_PICKUP_SUB_TYPE subType)
	{
			//kill audio
			C_VehicleLogicMsg_PerkEnded pMsg(mFiringIdx,mUniqueId,mPickupType,Game::E_BodyUserDataType_Actor,subType);
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);
	}
}
