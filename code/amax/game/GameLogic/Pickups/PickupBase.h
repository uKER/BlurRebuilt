#ifndef PICKUP_BASE_H
#define PICKUP_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceLogicCore/Data/RLEnums.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

namespace Game
{
	struct PickupHitInfo
	{
		Maths::HVec3 mPosition;
		Maths::HVec3 mDirection;
	};

	struct AITrackableData
	{
		bool			mIsTrackable;
		bool			mIsShootable;
		int				mFiringIdx;
		Maths::HVec3	mVelocity;
		float			mRadius;
		int				mAILogicIndex;

		bool CanTrack(int vehIdx) { return (mIsTrackable && (mFiringIdx != vehIdx || mVelocity.Length() == 0.f)); };
		bool CanShoot(int vehIdx) { return (CanTrack(vehIdx) && mIsShootable); };
	};

	// Filled in when pickup is killed so we can determine cause of death
	struct PickupAutopsyReport
	{
		PickupAutopsyReport()
		{
			mCauseOfDeath  = E_PICKUP_CAUSE_OF_DEATH_NONE;
			mVehicleIdHit  = -1;
			mTargettedVehicleId = -1;
		}

		// Main new info needed
		E_PICKUP_CAUSE_OF_DEATH mCauseOfDeath;

		// extra info 
		S8		mVehicleIdHit;  
		S8		mTargettedVehicleId;
	};

	// Used for fired pickups
	class C_PickupBase
	{
	public:
		C_PickupBase(S32 classType, U32 id, int firingIdx, E_PICKUP_TYPE type, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);
		virtual ~C_PickupBase();

		virtual void			FireChargingSpecialFX();

		virtual void			PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		virtual void			SetPostDelayMatrix(Maths::HMat43 matrix, float fVal = 0.f);

		virtual void			PreUpdateOne() {};
		virtual void			PreUpdateTwo() {};
		virtual bool			Update() = 0;

		virtual bool			CheckCanHit(int vehIdx, E_PICKUP_HIT_TYPE hitType = E_PICKUP_HIT_TYPE_MAJOR);
		virtual void			RegisterHit(int vehIdx, E_PICKUP_HIT_TYPE hitType = E_PICKUP_HIT_TYPE_MAJOR);
		void					RecordHit(int vehIdx, E_PICKUP_HIT_TYPE hitType = E_PICKUP_HIT_TYPE_MAJOR);

		virtual bool			IsDefenderCentric() { return true; }
		bool					CheckLegalHit(U32 idx)		{ return ((IsDefenderCentric() &&  VehicleIsLocal(idx)) || (!IsDefenderCentric() && VehicleIsLocal(mFiringIdx))); }
		virtual bool			NinjaKillIfAnyHit() { return false; }
		virtual void			SetParentId(U32 id) { mParentId = id; };

		virtual void			DebugDraw() = 0;
		virtual E_PICKUP_LOCK_ON	CheckVehicleTargetted(int vehIdx) { return E_PICKUP_LOCK_ON_NONE; };

		S32						GetClassType()	const { return mClassType; };
		E_PICKUP_TYPE			GetType()		const { return mPickupType; };
		E_PICKUP_SUB_TYPE		GetSubType()	const { return mPickupSubType; };
		U32						GetUid()		const { return mUniqueId; };
		void					Activate()		{ mActivated = true; };

		void					KillPickup(int killed_by_vehicle_index, E_PICKUP_CAUSE_OF_DEATH causeOfDeath);
		bool					IsSetToDie() { return mKillPickup; };

		void					SetReplacement() { mIsReplacement = true; };
		void					SetToBeReplaced() { mIsBeingReplaced = true; };
		virtual void			NinjaKill(const Game::PickupAutopsyReport& autoReport);
		bool					GetNinjaKill()	{ return mNinjaKill; };

		virtual void			SetHitPlayer() {};

		const Maths::HMat43&	GetMatrix()	const { return mMatrix; };
		int						GetFiringIdx()	const { return mFiringIdx; };
		const PickupAutopsyReport&	GetAutopsyReport() const { return mAutopsyReport; };

		void					RegisterVehicleShielded(int idx) { mVehiclesShielded[idx] = true; }

		bool					GetDidHitTarget( int vehIdx );
		bool					ApplyDamage(float damage, U32 id);
		int						GetDamageToApply();

		RLPickupLevel*			GetRLSubTypeLevel(const E_PICKUP_SUB_TYPE eOverridePickupSubType = E_PICKUP_SUB_TYPE_NONE) const;

		virtual E_TARGETTING_DIRECTION GetHitDirection(Actors::C_VehicleLogicActor* pActor);

		virtual void			PhotoMove(F32 frac) { }

		AITrackableData&		GetAITrackableData() { return mAITrackableData; };

		float					GetCreationTime() const;

		void					PreSetMatrix(Maths::HMat43 matrix) { mMatrix = matrix; mPreSetMatrix = true; };
		bool					IsMatrixSet() { return mPreSetMatrix; };

	protected:

		bool				SetAbilityActive(const UI::E_MultiplayerVehicleAbilities type, int TargetIdx,  bool active = true);
		bool				HasAbility(const UI::E_MultiplayerVehicleAbilities type, int vehicleIdx) const;
		bool				AbilityActiveOnPickupType(const UI::E_MultiplayerVehicleAbilities type) const;

		S32					mClassType;
		E_PICKUP_TYPE		mPickupType;
		E_PICKUP_SUB_TYPE	mPickupSubType;

		bool				mPreSetMatrix;

		U32					mUniqueId;
		U32					mGroupId;
		U32					mParentId;
		int					mFiringIdx;
		bool				mKillPickup;
		bool				mNinjaKill;	//	Signifies that we wish to kill the pickup without telling anyone else about it in a network game
		bool				mIsBeingReplaced;
		bool				mIsReplacement;
		bool				mActivated;
		bool				mNoFiringVehicleFX;
		bool				mNoPhysicalEffect;

		Maths::HMat43		mStartingMatrix;
		Maths::HMat43		mMatrix;
		Maths::HVec3		mPrevPosition;

		PickupHitInfo		mHitInfo;

		float				mHealth;
		Btl::Set<U32>		mPickupCollisionSet;

		AITrackableData		mAITrackableData;
		PickupAutopsyReport mAutopsyReport; 

		Maths::HVec3		mScale;

		Btl::Vector< bool, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> > mVehiclesHitMajor;
		Btl::Vector< bool, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> > mVehiclesHitMinor;
		Btl::Vector< bool, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> > mVehiclesShielded;

		float				mCreationTime;
	};

	// Used for applying perf mods and other effects locally
	class C_PickupLocalEffectsBase
	{
	public:
		C_PickupLocalEffectsBase(E_PICKUP_TYPE type, int vehIdx, float time, E_PICKUP_SUB_TYPE subType = E_PICKUP_SUB_TYPE_NORMAL, U32 id = 0);

		virtual bool Update() = 0;
		virtual void KillEffect() { mTimer = 999999.f; };
		
		E_PICKUP_TYPE		GetType() { return mPickupType; };
		E_PICKUP_SUB_TYPE	GetSubType() { return mPickupSubType; };

		virtual void SetUniqueID(U32 id) { mUniqueId = id;};

	protected:
		E_PICKUP_TYPE		mPickupType;
		E_PICKUP_SUB_TYPE	mPickupSubType;
		float				mTimer;
		int					mVehicleIndex;
		U32					mUniqueId;
	};
}

#endif //PICKUP_BASE_H
