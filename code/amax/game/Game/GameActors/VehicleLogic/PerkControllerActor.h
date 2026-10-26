#ifndef PERKCONTROLLERACTOR_H
#define PERKCONTROLLERACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <shared_actors\XtActorBase.h>

#include <btl/Containers/containers.h>

#include <gamelogic\racelogiccore\data\RLEnums.h>

namespace Game
{
	namespace Actors
	{
		const int SHOCK_RADIUS=13.f;
		const int SHOULDER_BARGE_RADIUS=8.f;
		const int SHUNT_RADIUS=15.f;
		
		const int MINE_RADIUS=2.f;
		const int MINE_TIMER=10.f;
	
		typedef struct PerkUpdater
		{
			float mTimer;
			float mRadius;
			float mTotalRadius;
			Actor::C_ActorHandle mOwner;
			E_PICKUP_TYPE mType;
			int mLevel;
			Btl::Vector<Actor::C_ActorHandle> mAlreadyHit;
			Maths::HVec3 mPosition;
		}PerkUpdater;

		typedef struct PerkCarInfo
		{
			Maths::HVec3 mPosition;
			Maths::HMat43 mMatrix;
		}PerkCarInfo;

		typedef Btl::OpenHashMap<Actor::C_ActorHandle,PerkCarInfo> VehicleWorldPos;
		typedef Btl::OpenHashMap<Actor::C_ActorHandle,bool> VehicleShielded;

		class C_PerkControllerActorCreate : public ::Actors::C_GameActorCreate
		{
		public:
			static Actor::C_ActorTypeId ClassType()			{ return E_ActorTypeId_PerkControllerActor; }
			const Actor::C_ActorTypeId GetType() const		{ return E_ActorTypeId_PerkControllerActor; }

		};

		class C_PerkControllerActor : public C_GameActor
		{
		public:
			C_PerkControllerActor();
			virtual ~C_PerkControllerActor();
			static const Actor::C_ActorTypeId	TypeId()			{ return E_ActorTypeId_PerkControllerActor; }
			static const Btl::String32			TypeName()			{ return "PerkController"; }
			virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);

		protected:
			virtual bool InitialiseImpl();
			virtual bool CreateImpl();

		private:
			float mDeltaTime;
			void Update();

			void ActivatePerk(Actor::C_ActorHandle owner,E_PICKUP_TYPE type,int level);
			
			float VehicleToVehicleAngle(const PerkCarInfo *owner, const PerkCarInfo *other);
			bool CanHit(PerkUpdater *pu,VehicleWorldPos::Iterator it);

			void ExplodeMine(PerkUpdater *pu);

			VehicleWorldPos mVehiclePositions;
			VehicleShielded mVehicleShielded;

			Btl::Vector<PerkUpdater> mShockUpdaters;
			bool UpdateShock(PerkUpdater *pu);

			Btl::Vector<PerkUpdater> mNitrousUpdaters;
			bool UpdateNitrous(PerkUpdater *pu);

			Btl::Vector<PerkUpdater> mShoulderBargeUpdaters;
			bool UpdateShoulderBarge(PerkUpdater *pu);

			Btl::Vector<PerkUpdater> mShuntUpdaters;
			bool UpdateShunt(PerkUpdater *pu);

			Btl::Vector<PerkUpdater> mMineUpdaters;
			bool UpdateMine(PerkUpdater *pu);
		};

	} // namespace Actors


} // namespace Game

#endif //  PERKCONTROLLERACTOR_H
