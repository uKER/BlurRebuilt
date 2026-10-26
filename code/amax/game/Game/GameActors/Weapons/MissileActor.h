/////////////////////////////////////////////////////////////
// MissileActor
/////////////////////////////////////////////////////////////

#ifndef __MISSILE_ACTOR_H__
#define __MISSILE_ACTOR_H__

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <Game/GameDef/UserData.h>
#include <shared_actors/XtActorBase.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include "../../../GameLogic/RaceLogicCore/Data/RLDefinitions.h"
#include "gravity/sdk/KinematicController.h"


namespace SpecialFX
{
	class I_SpecialFXInstance;
};

namespace Game
{

namespace Actors
{
class RaceLogicCore;
class C_VehicleActor;
/* 
===============================================================
					class MissileActorCreate

NOTES: 
===============================================================
*/
class MissileActorCreate : public ::Actors::C_PosActorCreate
{
public:

	MissileActorCreate()
	{
		mInitialTargetVehicleIndex = -1;
		mFiredByVehicleIndex = -1;

		m_pRLC = 0;

		mWorldMatrix = Maths::HMat43();
		mPickupPod.Initialise();

		mInitialSpeedInMetresPerSecond = 0.f;
		
		mCruiseHeightAboveTrack = 0.f;
		mAccelerationInMetresPerSecondPerSecond = 0.f;
		mMaximumSpeedInMetresPerSecond = 0.f;
		mTurnRatePerSecondInDegrees = 0.f;
		mSeekerConeAngleInDegrees = 0.f;
		mLifeTimeInSeconds = 0.f;
		mTriggerRadius = 0.f;

		mUpdateTargetingTime = 0.f;
	}

	static Actor::C_ActorTypeId					ClassType() { return E_ActorTypeId_Missile; }
	const Actor::C_ActorTypeId					GetType() const { return E_ActorTypeId_Missile; }
	const C_VehicleActor						*m_pFiredByVehicleActor;

	Game::Actors::RaceLogicCore					*m_pRLC;
	Maths::HMat43								mWorldMatrix;
	int											mInitialTargetVehicleIndex,		//The target the firer was aiming at
												mFiredByVehicleIndex;			//It might be useful to know who fired this bad boy

	float										mInitialSpeedInMetresPerSecond,
												mCruiseHeightAboveTrack,
												mAccelerationInMetresPerSecondPerSecond,
												mMaximumSpeedInMetresPerSecond,
												mTurnRatePerSecondInDegrees,
												mSeekerConeAngleInDegrees,
												mLifeTimeInSeconds,

												mTriggerRadius,
												mUpdateTargetingTime;

	RLPickupPod									mPickupPod;
	XtHandle<SpecialFX::C_SpecialFX>			mWeaponSFX;
};

/* 
===============================================================
					class MissileActor

NOTES: 
===============================================================
*/
class MissileActor : public C_GameActor
{
public:

	MissileActor(){};
	~MissileActor(){};

	static const Actor::C_ActorTypeId			TypeId()	{ return E_ActorTypeId_Missile; }
	static const Btl::String32					TypeName()	{ return "Missile"; }

	//! Update
	void										Update(float frameTime);

	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);
	bool										Create();

protected:

	virtual bool								InitialiseImpl();
	virtual bool								CreateImpl();

private:
	void										Render();
	void										CheckCollision(const Maths::HVec3 worldPosition);

	void										UpdateTargeting();
	void										UpdateTargetPosition();
	void										Explode();

	const bool									GetHeightAboveTrack(float& height);
	Gravity::C_Scene*							GetPhysicsScene();


//Variables
private:

	Maths::HMat43								mWorldMatrix, 
												mBirthWorldMatrix;
	Gravity::C_ExternalConstants				m_ExternalConstants;
	Gravity::C_KinematicController				m_Controller;

	int											mTargetedVehicleIndex,
												mFiredByVehicleIndex;
	Maths::HVec3								mTargetPosition,

												mVelocity;

	float										mUpdateTargetTimer,
												mSeekerConeAngle,

												mCollisionRadius,
												mHeightAboveGround,

												mTriggerRadius,

												mLifeTime,				//So that the missile doesn't carry on forever
												mAge, 					//The current age of the missile

												mSpeedInMetresPerSecond,
												mMaximumSpeedInMetresPerSecond;
												
	bool										mActive,
												mFoundGround;

	Gravity::C_Body								*m_pCollisionBody;

	Game::Actors::RaceLogicCore					*m_pRLC;
	SpecialFX::I_SpecialFXInstance				*m_pWeaponSpecialFX;
	RLPickupPod									mPickupPod;

	C_BodyUserDataActor							m_UserData;	
};

} //namespace Actors

} //namespace Game

#endif //__MISSILE_ACTOR_H__
