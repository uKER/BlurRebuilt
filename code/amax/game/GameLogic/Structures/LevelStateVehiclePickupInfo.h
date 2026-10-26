#ifndef LEVEL_STATE_VEHICLE_PICKUP_INFO_H
#define LEVEL_STATE_VEHICLE_PICKUP_INFO_H

#ifdef _MSC_VER
#pragma once
#endif

//	Forward declaration
struct RLBalancingDataBase;

namespace Game
{
	

struct PickupEffectBase
{
	void										GetBaseValues(RLBalancingDataBase* pPickup);

	float										mSpeed;
	float										mSingleTapMultiplier;
	float										mDoubleTapMultiplier;

	float										mChargeDelay;
};

struct NitrousEffectInfo : PickupEffectBase
{
	void										InitNormalValues();
	void										InitCheckpointValues();

	float										mDraftingBonus;
	float										mLifetime;

	float										mDuration;
	float										mSlowDownVal;

	float										mClassAModifier;
	float										mClassBModifier;
	float										mClassCModifier;
	float										mClassDModifier;
};

struct NitrousDoubleTapEffectInfo : PickupEffectBase
{
	void										InitNormalValues();

	float										mDraftingBonus;
	float										mLifetime;

	float										mDuration;
	float										mLowSpeedBoost;
	float										mDraftingBoost;
	float										mRiftTime;

	float										mRiftRadius;
	float										mRiftMinForce;
	float										mRiftMaxForce;
	float										mRiftAngleRange;

	float										mAirBrakeCutOff;
};

struct ShockEffectInfo : PickupEffectBase
{
	void										InitNormalValues();

	float										mRadius;
	float										mTimeShocked;
	int											mDamageInflicted;
	Btl::Vector<float>							mForce;

	float										mUpForce;
	float										mUpForceTime;

	float										mUpForceOffset;

	float										mDownForce;
	float										mDownForceTime;

	float										mMinRange;
	float										mMaxRange;
};

struct BlindfireEffectInfo : PickupEffectBase
{
	void										InitNormalValues();

	float										mRadius;
	float										mAngleRange;
	int											mDamageInflicted;

	float										mWheelLockAngle;
	float										mTimeShunted;
	float										mFXDropTime;
	float										mForceHitRadius;
};

struct ShuntEffectInfo : PickupEffectBase
{
	void										InitNormalValues();

	float										mRadius;
	float										mAngleRange;
	int											mDamageInflicted;

	float										mWheelLockAngle;
	float										mTimeShunted;
	float										mFXDropTime;

	Btl::Vector<float>							mForce;
	Btl::Vector<float>							mUpwardForce;
	float										mForceHitRadius;
};

struct BargeEffectInfo : PickupEffectBase
{
	void										InitNormalValues();
	void										InitDestructionValues();

	float										mRadius;
	float										mAngleRange;
	int											mDamageInflicted;

	bool										mFixedPosition;
};

struct BombEffectInfo : PickupEffectBase
{
	void										InitNormalValues();

	float										mRadius;
	float										mLifespan;
	int											mDamageInflicted;
};

struct BombDoubleTapEffectInfo : PickupEffectBase
{
	void										InitNormalValues();

	float										mRadius;
	float										mLifespan;
	int											mDamageInflicted;
};


struct LevelStateVehiclePickupInfo
{
	void										Init();
	void										InitCheckpoint();
	void										InitDestruction();

	NitrousEffectInfo&							NitrousInfo()					{ return mNitroInfo; }
	ShockEffectInfo&							ShockInfo()						{ return mShockInfo; }
	ShuntEffectInfo&							ShuntInfo()						{ return mShuntInfo; }
	BlindfireEffectInfo&						BlindfireInfo()					{ return mBlindfireInfo; }
	BargeEffectInfo&							BargeInfo()						{ return mBargeInfo; }
	BombEffectInfo&								BombInfo()						{ return mBombInfo;}

	NitrousDoubleTapEffectInfo&					NitrousDoubleTapInfo()			{ return mNitroDoubleTapInfo; }
	BombDoubleTapEffectInfo&					BombDoubleTapInfo()				{ return mBombDoubleTapInfo;}

private:
	NitrousEffectInfo							mNitroInfo;
	ShockEffectInfo								mShockInfo;
	ShuntEffectInfo								mShuntInfo;
	BlindfireEffectInfo							mBlindfireInfo;
	BargeEffectInfo								mBargeInfo;
	BombEffectInfo								mBombInfo;

	NitrousDoubleTapEffectInfo					mNitroDoubleTapInfo;
	BombDoubleTapEffectInfo						mBombDoubleTapInfo;
};

}	//	namespace Game

#endif // LEVEL_STATE_VEHICLE_PICKUP_INFO_H