/////////////////////////////////////////////////////////////
// MissileActor
/////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <gravity/sdk/Body.h>

#include "MissileActor.h"


#include <Game/ActorDef/GameActor.h>

#include "../Vehicles/VehicleActor.h"

#include "../../Messaging/GameMessages.h"
#include "../../Messaging/VehicleActorMessages.h"
#include "../../Messaging/PickupActorMessages.h"

#include "../../../GameLogic/RaceLogicCore/Core/RaceLogicCore.h"

#include <gravity/narrowphase/Ray.h>
#include <gravity/sdk/BodyHelper.h>
#include <gravity/sdk/ShapeDescriptors.h>
#include <shared_actors/SharedActorUtils.h>
#include <SpecialFX/SpecialFX.h>
#include <game/GameDef/PolyFlags.h>


Debug::Logger &g_MissileActorLog = Debug::GetLogger("amax.actors.missile");



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//From RLDebugLineController.cpp
extern Debug::Bool g_RenderMissilePositions;

Debug::Float g_UpdateTargetTime("RLC.Pickups.Missile.TargetUpdateTimer", 1.f);

Debug::Int g_TargetVehicleIndex("RLC.Pickups.Missile.TargetVehicleIndex", -1);
Debug::Int g_OutputDebugInfo("RLC.Pickups.Missile.OutputDebugInfo", false);

Debug::Bool g_OverrideUpdateTime("RLC.Pickups.Missile.UpdateAt30FPS", false, Debug::Persist());


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
namespace Game
{
namespace Actors
{
bool MissileActor::InitialiseImpl()
{
	return true;
}

bool MissileActor::CreateImpl()
{
	const MissileActorCreate *pMissileCreate = (const MissileActorCreate *)(GetCreationData().GetData());

	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_VehicleMessage_VehicleWorldPosition );
	AddSubscriber( Game::E_PickupMessage_MissileHitWorld );
	AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );

	mUpdateTargetTimer = 0.f;
	mTargetPosition = Maths::HVec3();
	mActive = true;
	mCollisionRadius = pMissileCreate->mTriggerRadius;

	mWorldMatrix = pMissileCreate->mWorldMatrix;
	if(pMissileCreate->mInitialTargetVehicleIndex > -1)
		mTargetedVehicleIndex = pMissileCreate->mInitialTargetVehicleIndex;
	else
		mTargetedVehicleIndex = g_TargetVehicleIndex;

	mFiredByVehicleIndex = pMissileCreate->mFiredByVehicleIndex;

	mPickupPod = pMissileCreate->mPickupPod;
	mLifeTime = pMissileCreate->mLifeTimeInSeconds;
	mAge = 0.f;

	mSpeedInMetresPerSecond = pMissileCreate->mInitialSpeedInMetresPerSecond;
	mMaximumSpeedInMetresPerSecond = pMissileCreate->mMaximumSpeedInMetresPerSecond;

	mVelocity = mWorldMatrix.AZ() * mSpeedInMetresPerSecond;
	mSeekerConeAngle = pMissileCreate->mSeekerConeAngleInDegrees;

	//By default choose a place a few metres ahead of the firer until we get the target's position
	mTargetPosition = mWorldMatrix.AW() + mWorldMatrix.AZ() * 1000000.f;

	m_pRLC = pMissileCreate->m_pRLC;
	m_pWeaponSpecialFX = NULL; //pMissileCreate->mWeaponSFX;

	//Create our specialFX if we've got one
	if (pMissileCreate->mWeaponSFX.Object())
	{
		m_pWeaponSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pMissileCreate->mWeaponSFX, GetActorList()->GetDefaultContextProvider());
		m_pWeaponSpecialFX->Start();
		m_pWeaponSpecialFX->UpdateWorldMatrix(mWorldMatrix);
		m_pWeaponSpecialFX->Update( 0 );
	}


	m_pCollisionBody = GetPhysicsScene()->CreateBody();
	Gravity::C_SphereDesc gravitySphereJobby;
	gravitySphereJobby.m_Radius = 0.25f;

	gravitySphereJobby.m_Mass = 0.f;
	Gravity::C_BodyHelper helper(&gravitySphereJobby);

	Collision::C_CollisionFilter filter(Game::PolyFlags::kCameraSolid, Game::PolyFlags::kCameraSolid);
	filter.m_GroupId = pMissileCreate->m_pFiredByVehicleActor->GetCollisionBody()->GetShape()->m_Filter.m_GroupFlags;
	filter.m_GroupFlags = 0;
	filter.m_GroupMask = 0;
	Maths::HMat33 inertiaTensor(Maths::ZERO_MATRIX);
	
	helper.CreateBody(m_pCollisionBody, &filter); 
	m_pCollisionBody->ElementMatrix(mWorldMatrix);
	m_ExternalConstants.m_Gravity = Maths::HVec3(0.f, 0.f, 0.f);
	m_pCollisionBody->SetExternalConstants(&m_ExternalConstants);
	SharedPhysics::C_Deactivator deactivator;
	deactivator.m_ActivateThresholdSq = -1.f;
	deactivator.m_SleepThresholdSq = -1.f;
	deactivator.m_sSleepCountdown = 15;
	m_pCollisionBody->SetDeactivator(deactivator);
	
	m_UserData.m_hActorHandle = GetHandle();
	m_pCollisionBody->SetUserData((void*)&m_UserData);
	GetPhysicsScene()->AddBody(m_pCollisionBody);

	m_Controller = Gravity::C_KinematicController(m_pCollisionBody);
	GetPhysicsScene()->AddKinematicController(&m_Controller);


	return true;
}

void MissileActor::Update(float frameTime)
{
	//What we're trying to achieve here is the red shell effect from Mario kart. The missiles there 
	//act as though they were on  a flat plane but they follow the undulations in the track smoothly. 
	//So what we'll do is only consider the missile's movement in 2d (XZ) and then manually force the 
	//missile to fly a predetermined height above the track surface

	//DEBUG DEBUG DEBUG
	if(g_OverrideUpdateTime)
		frameTime = 1.f/30.f;
	//DEBUG DEBUG DEBUG

	const MissileActorCreate *pMissileCreate = (const MissileActorCreate *)(GetCreationData().GetData());

	Maths::Quaternion rotation;

	Maths::HVec3 newDirection = Maths::HVec3();
	Maths::HVec3 targetDirection = Maths::HVec3();

	Maths::HVec3 courseCorrectionDirection = Maths::HVec3();

	Maths::HMat43 newMatrix = Maths::HMat43();
	Maths::HVec3 up = Maths::HVec3(0.f, 1.f, 0.f);
	
	float	distanceToTarget = 100000.f,
			flatDistanceToTarget = 10000.f,
			heightError = 0.f;

	float cosDesiredTurnAngle = 0.f;
	float maximumTurnAngleThisFrame = Maths::DegToRad((float)pMissileCreate->mTurnRatePerSecondInDegrees) * frameTime;
	float cosMaximumTurnAngleThisFrame = Maths::Cos(maximumTurnAngleThisFrame);
	float turnError = 0.f;
	float desiredHeightChange = 0.f;
	float speedToAddThisFrame = 0.f;

	mHeightAboveGround = 0.f;

	speedToAddThisFrame = pMissileCreate->mAccelerationInMetresPerSecondPerSecond * frameTime;
	if( (mSpeedInMetresPerSecond + speedToAddThisFrame) < mMaximumSpeedInMetresPerSecond)
		mSpeedInMetresPerSecond += speedToAddThisFrame;
	else 
		mSpeedInMetresPerSecond = mMaximumSpeedInMetresPerSecond;

	//Update the age of the missile so it doesn't run on forever
	if( (mAge + frameTime) >= mLifeTime)
	{
		if( mPickupPod.mTimeLeft > 0.f)
			Explode();
		else
		{
			if(m_pCollisionBody)
			{
				GetPhysicsScene()->RemoveKinematicController(&m_Controller);
				GetPhysicsScene()->RemoveBody(m_pCollisionBody);
			}

			if (m_pWeaponSpecialFX)
			{
				m_pWeaponSpecialFX->Stop();
				SAFE_DELETE_PTR(m_pWeaponSpecialFX);
			}
			//Kill the actor
			Kill();
		}
		return;
	}
	else
		mAge += frameTime;

	//Every few seconds check to see if we have a better target
	if( (mUpdateTargetTimer - frameTime) <= 0.f)
	{
		UpdateTargeting();
		mUpdateTargetTimer = pMissileCreate->mUpdateTargetingTime;
	}
	else
		mUpdateTargetTimer -= frameTime;

	//Work out how far we are from the target in 2D (y flattened)
	Maths::HVec3 flatTargetPosition = mTargetPosition;
	flatTargetPosition.Y( mWorldMatrix.AW().Y() );
	
	//Work out the direction target in our plane
	Maths::HVec3 flatDistance = mTargetPosition - mWorldMatrix.AW();
	flatDistanceToTarget = flatDistance.Length();
	targetDirection = Maths::Normalise(flatDistance);

	courseCorrectionDirection = targetDirection - mWorldMatrix.AZ();

	//Now we need to work out if the missile can turn that sharply
	cosDesiredTurnAngle = Maths::Clamped( (float)Maths::DotProduct( targetDirection, mWorldMatrix.AZ() ), 0.f, 1.f);

	//As near as damnit to cos zero at least
	float cosZero = 0.999999f;
	
	//Make the missile fly straight if we've got nothing better to do
	newDirection = mWorldMatrix.AZ();
	if(cosMaximumTurnAngleThisFrame < cosZero)
	{
		//See if we are going to be able to make the turn
		if(cosDesiredTurnAngle < cosMaximumTurnAngleThisFrame && cosMaximumTurnAngleThisFrame < cosZero)
		{
			//We're trying to turn a bigger angle than we can manage, so we're not going to make the turn
			if(cosDesiredTurnAngle < cosZero)
			{
				turnError = (1.f - abs(cosMaximumTurnAngleThisFrame)) / (1.f - abs(cosDesiredTurnAngle));

				//Work out the new direction
				newDirection = mWorldMatrix.AZ() + (courseCorrectionDirection * turnError);
				newDirection = Maths::Normalise(newDirection);
			}
		}
		else
			newDirection = targetDirection;
	}


	//See if we're going to get to our target this frame
	if( (mSpeedInMetresPerSecond * frameTime) < distanceToTarget)
		//We're not
		mVelocity = Maths::Normalise(newDirection) * mSpeedInMetresPerSecond;
	else
		//We don't want to over shoot so just add move the distance we need
		mVelocity = Maths::Normalise(newDirection) * distanceToTarget;

	//Move our missile into its new position on the flat plane
	mWorldMatrix.Translation(mWorldMatrix.AW() + (mVelocity * frameTime));

	//From the new position work out how far above the ground we are
	mFoundGround = GetHeightAboveTrack(mHeightAboveGround);
	if(mFoundGround)
		desiredHeightChange = pMissileCreate->mCruiseHeightAboveTrack - mHeightAboveGround;
	else
		//If indoubt fly down
		desiredHeightChange = -1.f;

	//Force the missile to skim the ground exactly
	Maths::HVec3 newPosition = mWorldMatrix.AW();
	newPosition.Y( newPosition.Y() + desiredHeightChange );

	//Now we've put the missile on the ground (or just above it...)
	//newDirection = flatTargetPosition - newPosition;
	newDirection = Maths::Normalise(newDirection);
	
	newMatrix.AZ( newDirection );
	newMatrix.AX( Maths::HVec3( newDirection.Z(), newDirection.Y(), -newDirection.X() ));
	newMatrix.AY( newMatrix.AZ() ^ newMatrix.AX() );
	newMatrix.AW( newPosition );
	
	mWorldMatrix = newMatrix;
	mPickupPod.mWorldMatrix = mWorldMatrix;

	if(m_pWeaponSpecialFX)
	{
		m_pWeaponSpecialFX->UpdateWorldMatrix(mWorldMatrix);
		m_pWeaponSpecialFX->Update( frameTime );
	}
	
	//Update the collision body
	m_Controller.SetTargetTransformation(mWorldMatrix, frameTime);

	CheckCollision(mTargetPosition);
}

const bool MissileActor::GetHeightAboveTrack(float& height)
{
	Maths::HVec3 start = mWorldMatrix.AW();
	Maths::C_Normal down = Maths::C_Normal(0.f, -1.f, 0.f);

	U8 data[1024 * sizeof(void*)];
	SharedPhysics::C_SafeStack<true> stack(data, 1024);
	Collision::C_Ray ray;
	Collision::C_CollisionFilter rayCollisionFilter(0xffffffff, 0xffffffff);				// Probably need to check ONLY for static scene geometry

	ray.InitialiseRay(start, down, 1000.f, &rayCollisionFilter, 0.1f);
	GetPhysicsScene()->DoRayTrace(ray, stack);

	Collision::C_RayTracingResult& result = ray.m_result;
	if (result.Hit())																		// If we've got a new position then adjust the actor
	{
		//height = start.Y() - result.HitPoint().Y();
		//height = result.HitPoint().Y();
		height = (float)result.Distance();
		return true;
	}

	return false;
}

Gravity::C_Scene* MissileActor::GetPhysicsScene()
{
	return GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();
}

void MissileActor::UpdateTargeting()
{
	
	float cosSeekerConeAngle = Maths::Cos(Maths::DegToRad(mSeekerConeAngle/2.f));
	
	Maths::HVec3 targetPosition, relativePosition;
	float	currentTargetDistanceSquared = 100000.0f, 
			cosCurrentTargetAngle = -1.f,

			potentialTargetDistanceSquared = 0.f, 
			cosPotentialTargetAngle = 0.f;

	//Make sure we've actually already got a target
	if(mTargetedVehicleIndex > -1)
	{
		//targetPosition = m_pRLC->GetVehicleWorldPosition(mTargetedVehicleIndex);
		relativePosition = mWorldMatrix.AW() - targetPosition;
	
		currentTargetDistanceSquared = relativePosition.LengthSquared();
		relativePosition = Maths::Normalise(relativePosition);
		
		cosCurrentTargetAngle = Maths::DotProduct(mWorldMatrix.AZ(), relativePosition);
	}

	//Look through the other vehicles for a target that's closer with a and more central
	for(int potentialTargetVehicleIndex = 0; potentialTargetVehicleIndex < m_pRLC->GetNumVehicles(); potentialTargetVehicleIndex++)
	{
		if(potentialTargetVehicleIndex == mTargetedVehicleIndex)
			continue;
		//Try not to target the firer's vehicle
		else if(potentialTargetVehicleIndex == mFiredByVehicleIndex)
			continue;

		//targetPosition = m_pRLC->GetVehicleWorldPosition(potentialTargetVehicleIndex);
		relativePosition = targetPosition - mWorldMatrix.AW();
		
		potentialTargetDistanceSquared = relativePosition.LengthSquared();
		relativePosition = Maths::Normalise(relativePosition);

		cosPotentialTargetAngle = Maths::DotProduct(mWorldMatrix.AZ(), relativePosition);

		//When the target is straight ahead the cosPotentialTargetAngle will be 1.f (cos 0 = 1.f)
		if(cosPotentialTargetAngle >= cosSeekerConeAngle)		//Within the seeker cone
		{
			if( (cosPotentialTargetAngle > cosCurrentTargetAngle)
				&& 
				(potentialTargetDistanceSquared < currentTargetDistanceSquared) )	//More ahead than the current target
			{
				//Closer and more directly ahead than the current target
				mTargetedVehicleIndex = potentialTargetVehicleIndex;
				mTargetPosition = targetPosition;
			}
		}
	}
}


void MissileActor::CheckCollision(const Maths::HVec3 worldPosition)
{
	if(!mActive)
		return;

	float radiusSquared = mCollisionRadius * mCollisionRadius;
	Maths::HVec3 toTarget = worldPosition - mWorldMatrix.AW();

	if(toTarget.LengthSquared() <= radiusSquared)
	{
		Explode();
	}
}

void MissileActor::Explode()
{
	if(mActive)
	{
		mActive = false;
		mAge = mLifeTime;

		MissileExplodedMsg bangMsg;
		bangMsg.mMissileActorHandle = GetCreationData()->GetActorHandle();
		bangMsg.mWorldPosition = mWorldMatrix.AW();

		BroadcastMessage(&bangMsg);

		
	}
}

void MissileActor::Render()
{
	//DEBUG DEBUG DEBUG
	if(g_RenderMissilePositions)
	{
		Maths::HVec3 a, b;

		a = b = mWorldMatrix.AW();

		a += mWorldMatrix.AZ() * 2.f;
		b -= mWorldMatrix.AZ() * 0.5f;

		Maths::RGBA colour = Maths::RGBA(0xff, 0x00, 0x00, 0xff);

		if(mActive)
		{
			if(mFoundGround)
				colour = Maths::RGBA(0xff, 0xff, 0xff, 0xff);
			else
				colour = Maths::RGBA(0x00, 0x00, 0xff, 0xff);
		}

		
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mWorldMatrix.AW(), 0.5f, colour);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, colour);

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCross(mWorldMatrix.AW() - (mWorldMatrix.AY() * mHeightAboveGround), 0.2f, colour);
	}
	//DEBUG DEBUG DEBUG
}

bool MissileActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	PERF_EVENT("MissileActor::MessageHandler");
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_Update:
		{
			const GameMessaging::C_UpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage);

			Update(pMsg->GetValue());
			Render();
			return true;
			break;
		}
	case Game::E_VehicleMessage_VehicleWorldPosition:
		{
			const Game::Actors::C_VehicleWorldPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleWorldPositionMsg>(pMessage);

			//See if we've bumped into another vehicle
			CheckCollision(pMsg->mWorldPosition);
			if(pMsg->mVehicleIndex == mTargetedVehicleIndex)
			{
				mTargetPosition = pMsg->mWorldPosition;
			}
			return true;
		}
	case Game::E_PickupMessage_MissileHitWorld:
		{
			Explode();
			return true;
		}
	case Game::E_GameflowMessage_ClearStoredActorPtrs:
		{
			m_pRLC = NULL;
			return true;
		}

	}

	return Actor::C_Actor::OnMessage(pMessage);
}


}; //namespace Actors
}; //namespace Game

