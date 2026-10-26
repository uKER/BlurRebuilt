#ifndef SHOCK_MANAGER_H
#define SHOCK_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "System/misc/Random.h"

namespace airacingtech
{
	class AITrackDataRoutes;
}

namespace Game
{
	class C_RaceLogicLayer;

	namespace Actors
	{
		class C_VehicleLogicActor;
	}

	struct ShockPoint
	{
		Maths::HVec3	mPosition;
		float			mWidth;
		int				mLogicIdx;
	};

	struct ManagedShock
	{
		ManagedShock() : mUniqueId(0), mHitTarget(false), mMissedATarget(false) {};

		int				mFirerIdx;
		int				mTargetIdx;
		U32				mUniqueId;
		ManagedShock*	mPrev;
		ManagedShock*	mNext;
		bool			mHitTarget;
		bool			mMissedATarget;

		ShockPoint		mPoint;
	};

	struct ShockCachedRoute
	{
		airacingtech::AITrackDataRoutes*	mpRoute;
		int									mIndex;
		float								mRandVal; //Used for random selection
	};

	struct ShockSort
	{
	public:
		ShockSort(Actors::C_VehicleLogicActor* pActor) : pTargetActor(pActor) {};
		bool operator()(ShockCachedRoute& lhs, ShockCachedRoute& rhs);

		Actors::C_VehicleLogicActor* pTargetActor;
	};

	// ------------------------------------------------------------------------------------------

	class C_ShockManager
	{
	public:
		C_ShockManager();
		~C_ShockManager();

		void											Init(C_RaceLogicLayer* pLayer);
		void											Update();

		void											RegisterShocks(int firerIdx, int targetIdx, U32 id);
		void											ClearShocks();
		void											RemoveShock(U32 id);
		void											PlayFX(int firerIdx, int targetIdx, U32 id);

		void											GetRoutes(int targetIdx, int idxOverride = -1);
		void											SetIndexFromOffset(ManagedShock* pShock);
		bool											GenerateShockPosition(ManagedShock* pShock, ShockCachedRoute& route);
		void											TraceToGround(Maths::HVec3& pos);
		void											TraceToObstacles(Maths::HVec3& pos);
		float											CheckVehicleDistance(ManagedShock* pShock, int vehIdx);
		float											GetTargetDistanceAhead(int idx);
		void											RegisterNetworkShockHit(U32 vehHitIndex, U32 perkUID, E_PICKUP_HIT_TYPE hitType );

	private:
		Btl::Vector<ManagedShock*>						mShocksInSystem;
		int												mNextShockIdx;
		float											mTimeToNextShock;
		float											mTimeSinceLastPrimaryDrop;
		Maths::HVec3									mInitialTargetPos;

		Util::C_Random									mRandGenerator;

		float											mCurrentDistanceAhead;
		int												mNumSequentialShocks;

		float											mCentreProb;
		bool											mOffsetLeft;
		int												mShocksSinceLastCentre;

		Btl::Vector<ShockCachedRoute>					mpCurrentRoutes;
		
		// Cache
		C_RaceLogicLayer*								mpRaceLogicLayer;

		friend class C_PickupsManager;
	};
}

#endif
