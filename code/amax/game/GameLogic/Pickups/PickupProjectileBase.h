#ifndef PICKUP_PROJECTILE_BASE_H
#define PICKUP_PROJECTILE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <gravity/sdk/Body.h>
#include <gravity/sdk/KinematicController.h>

// Forward decl
namespace Game
{
	class C_PickupMine; 
}

namespace Game
{
	class C_PickupProjectile : public C_PickupBase
	{
	public:
		C_PickupProjectile(S32 classType, U32 id, int firingIdx, int targetIdx, E_PICKUP_TYPE type, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);
		~C_PickupProjectile();

		void CreateCollisionBody();
		void UpdateCollisionBody();

		void KickOffAlignProjectileToGround(Maths::HVec3& pos);
		bool AlignProjectileToGround(Maths::HVec3& pos);
		bool AlignProjectileToGroundImmediate(Maths::HVec3& pos);

		void KickOffCollision(Maths::HVec3 oldPos, Maths::HVec3 newPos, int rayIdx);
		bool CheckCollisions(Maths::HVec3 oldPos, int rayIdx);

		virtual void PreUpdateOne();
		virtual void PreUpdateTwo();

		virtual void Trigger(int idx) = 0;
		virtual void Explode(Maths::HVec3 pos, Maths::C_Normal normal) = 0;

		virtual void CollideWithEnvironment(Maths::HVec3 hitPoint, Maths::C_Normal normal);
		virtual void CollideWithPickup(int vehIdx);

		virtual bool Rebound(Maths::C_Normal normal, Maths::HVec3 pos) { return false; };

		virtual void KillPickupAudio(E_PICKUP_SUB_TYPE subType = E_PICKUP_SUB_TYPE_NORMAL);

	protected:
		int								mTargetIdx;
		Maths::HVec3					mMovementDir;

		float							mRadius;
		float							mHeightFromGround;
		float							mSpeed;
		float							mFallRate;
		Maths::HVec3					mHitPoint;
		Maths::HVec3					mPrevPos;

		Maths::HVec3					mGroundPoint;
		Maths::C_Normal					mGroundNormal;

		C_BodyUserDataProjectile		mUserData;
		Gravity::C_Body*				mpCollisionBody;

		Game::Actors::RaceLogicCore*	mpRLC;
		C_RaceLogicLayer*				mpRaceLogicLayer;
	private:
		void							ReportMinedDestroyedEvent(C_PickupMine* pMine); 
		void							ReportProjectileDestroyedEvent(C_PickupProjectile* pProjectile);

		void							CheckFall(Maths::HVec3& pos, const Collision::C_RayTracingResult& rayResult);
		bool							CheckIsValidWallCollision(const Collision::C_RayTracingResult& rayResult);
		bool							CheckRebound(const Collision::C_RayTracingResult& rayResult);

		Btl::Vector< Collision::C_TransparentRay, Btl::VectorParameters_Array<2> >	mDeferredRayList;
		Btl::Vector< Collision::C_Ray, Btl::VectorParameters_Array<2> >				mDeferredThinRayList;
		Collision::C_Ray mDeferredGroundRay;

		Collision::C_CollisionFilter mTransparentRayFilter;
		Collision::C_CollisionFilter mSolidRayFilter;
		Collision::C_CollisionFilter mGroundRayFilter;
	};
}

#endif //PICKUP_PROJECTILE_BASE_H
