#ifndef PICKUP_MINE_BASE_H
#define PICKUP_MINE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <gravity/sdk/Body.h>


class C_RaceLogicLayer;

namespace Actors
{
	class RaceLogicCore;
	class C_VehicleLogicActor;
};

namespace Game
{
	namespace Actors
	{
		class C_VehicleActor;
	}

	class C_PickupMine : public C_PickupBase
	{
	public:
		C_PickupMine(S32 classType, U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual ~C_PickupMine();

		void ApplyMatrixModifiers(Maths::HMat43 &matrix, const bool overrideSorcery = false, const float overrideScale = 1.f);

		void ApplyForce(Maths::HVec3 dir, float force);

		virtual void Trigger(int idx) = 0;
		virtual void Prime(int idx){};
		virtual void Explode() = 0;

	protected:
		virtual void					CreateCollisionBodies();
		void							UpdateCollisionBodies();

		void							CreateCollisionBody(	Actors::RaceLogicCore			*pRLC, 
																C_RaceLogicLayer				*pLayer, 
																Actors::C_VehicleLogicActor		*pLogicActor, 
																Game::Actors::C_VehicleActor	*pGraphicalActor, 
																const float						radius,
																Gravity::C_Body					**ppCollisionBody, 
																Game::E_BodyUserDataType		eUserDataType,
																C_BodyUserDataMine				*pUserData,
																Collision::C_CollisionFilter	&collisionFilter);

	public:
		Gravity::C_ExternalConstants	m_ExternalConstants;
		
	protected:
		Gravity::C_Body*				mpInnerCollisionBody;				//Will cause an explosion
		Gravity::C_Body*				mpOuterCollisionBody;				//Will detect someone's nearby

		C_BodyUserDataMine				mInnerCollisionUserData;
		C_BodyUserDataMine				mOuterCollisionUserData;


		float							mTimer;

		float							mInnerCollisionRadius;
		float							mOuterCollisionRadius;
		float							mBehindDistance;

		float							mMovementForce;
		Maths::HVec3					mMovementDir;
	};
}

#endif //PICKUP_MINE_BASE_H
