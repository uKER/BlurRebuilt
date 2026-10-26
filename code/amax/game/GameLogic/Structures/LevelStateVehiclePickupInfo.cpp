#include "LevelStateVehiclePickupInfo.h"

#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

extern Debug::Float g_debugPerkStartDelay;
extern Debug::Float g_debugPerkStartDelayLong;
extern Debug::Float g_debugShockTargetMinRange;
extern Debug::Float g_debugShockTargetRange;
extern Debug::Float g_DebugDraftingBoost;


namespace Game
{
	void PickupEffectBase::GetBaseValues( RLBalancingDataBase* pPickup )
	{
	}


	void NitrousEffectInfo::InitNormalValues()
	{
		RLNitrousBalancingData* pPickup = NULL;
		if(RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS)->mNormal)
		{
			pPickup = (RLNitrousBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS)->mNormal->mBalancingData;
		}

		BIZ_ASSERT_MSG(pPickup, "Failed to find Nitrous balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);

		mClassAModifier			= pPickup->mClassAModifier;
		mClassBModifier			= pPickup->mClassBModifier;
		mClassCModifier			= pPickup->mClassCModifier;
		mClassDModifier			= pPickup->mClassDModifier;

		mDuration				= pPickup->mDuration;
		mSlowDownVal			= pPickup->mSlowDownVal;

		mLifetime				= 1.0f;	//	Why can't we get this from Sorcery?		
	}

	void NitrousEffectInfo::InitCheckpointValues()
	{
		RLNitrousBalancingData* pPickup = NULL;
		if(RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS)->mNormal)
		{
			pPickup = (RLNitrousBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS)->mNormal->mBalancingData;
		}

		BIZ_ASSERT_MSG(pPickup, "Failed to find Nitrous balancing data - see GavinY/SteveB");

		mDuration				= pPickup->mDurationCheckpoint;
		mSlowDownVal			= pPickup->mSlowDownValCheckpoint;	
	}

	void NitrousDoubleTapEffectInfo::InitNormalValues()
	{
		RLNitrousDoubleTapBalancingData* pPickup = NULL;
		if(RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS)->mDoubleTap)
		{
			pPickup = (RLNitrousDoubleTapBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS)->mDoubleTap->mBalancingData;
		}

		BIZ_ASSERT_MSG(pPickup, "Failed to find Nitrous balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);

		mLowSpeedBoost			= pPickup->mLowSpeedBoost;
		mDraftingBoost			= pPickup->mDraftingBoost;
		mDuration				= pPickup->mDuration;

		mRiftTime				= pPickup->mRiftTime;
		mRiftRadius				= pPickup->mRiftRadius;
		mRiftMinForce			= pPickup->mRiftMinForce;
		mRiftMaxForce			= pPickup->mRiftMaxForce;
		mRiftAngleRange			= pPickup->mRiftAngleRange;

		mAirBrakeCutOff			= pPickup->mAirBrakeCutOff;

		mLifetime				= 1.0f;	//	Why can't we get this from Sorcery?
	}

	void ShockEffectInfo::InitNormalValues()
	{
		RLShockBalancingData* pPickup = (RLShockBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHOCK)->mNormal->mBalancingData;
		
		BIZ_ASSERT_MSG(pPickup, "Failed to find Shock balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);
	}

	void BlindfireEffectInfo::InitNormalValues()
	{
		RLShuntBalancingData* pPickup = (RLShuntBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHUNT)->mNormal->mBalancingData;

		BIZ_ASSERT_MSG(pPickup, "Failed to find Shock balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);

		mRadius					= pPickup->mRadius;
		mDamageInflicted		= pPickup->mDamageInflicted;
		mFXDropTime				= pPickup->mFXDropTime;
		mChargeDelay			= g_debugPerkStartDelay;
	}

	void ShuntEffectInfo::InitNormalValues()
	{
		RLShuntBalancingData* pPickup = (RLShuntBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHUNT)->mNormal->mBalancingData;

		BIZ_ASSERT_MSG(pPickup, "Failed to find Shock balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);

		mRadius					= pPickup->mRadius;
		mDamageInflicted		= pPickup->mDamageInflicted;
		mFXDropTime				= pPickup->mFXDropTime;
	}

	void BargeEffectInfo::InitNormalValues()
	{
		RLBargeBalancingData* pPickup = (RLBargeBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BARGE)->mNormal->mBalancingData;

		BIZ_ASSERT_MSG(pPickup, "Failed to find Barge balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);

		mRadius					= pPickup->mRadius;
		mAngleRange				= pPickup->mAngleRange;
		mDamageInflicted		= pPickup->mDamageInflicted;
		mFixedPosition			= false;
	}

	void BargeEffectInfo::InitDestructionValues()
	{
		RLBargeBalancingData* pPickup = (RLBargeBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BARGE)->mNormal->mBalancingData;

		BIZ_ASSERT_MSG(pPickup, "Failed to find Barge balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);

		mRadius					= pPickup->mRadius;
		mAngleRange				= pPickup->mAngleRange;
		mDamageInflicted		= pPickup->mDamageInflicted;
		mFixedPosition			= true;
	}

	void BombEffectInfo::InitNormalValues()
	{
		RLBombBalancingData* pPickup = NULL;
		if(RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BOMB)->mNormal)
		{
			pPickup = (RLBombBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BOMB)->mNormal->mBalancingData;
		}

		BIZ_ASSERT_MSG(pPickup, "Failed to find Bomb balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);

		mRadius						= pPickup->mRadius;
		mLifespan					= pPickup->mLifespan;
		mDamageInflicted			= pPickup->mDamageInflicted;
	}

	void BombDoubleTapEffectInfo::InitNormalValues()
	{
		RLBombDoubleTapBalancingData* pPickup = NULL;
		if(RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BOMB)->mDoubleTap)
		{
			pPickup = (RLBombDoubleTapBalancingData*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BOMB)->mDoubleTap->mBalancingData;
		}

		BIZ_ASSERT_MSG(pPickup, "Failed to find Bomb balancing data - see GavinY/SteveB");

		GetBaseValues(pPickup);

		mRadius						= pPickup->mRadius;
		mLifespan					= pPickup->mLifespan;
		mDamageInflicted			= pPickup->mDamageInflicted;
	}

	void LevelStateVehiclePickupInfo::Init()
	{
		mNitroInfo.InitNormalValues();
		mShockInfo.InitNormalValues();
		mShuntInfo.InitNormalValues();
		mBlindfireInfo.InitNormalValues();
		mBargeInfo.InitNormalValues();
		mBombInfo.InitNormalValues();

		mNitroDoubleTapInfo.InitNormalValues();
		mBombDoubleTapInfo.InitNormalValues();
	}

	void LevelStateVehiclePickupInfo::InitCheckpoint()
	{
		mNitroInfo.InitCheckpointValues();
	}

	void LevelStateVehiclePickupInfo::InitDestruction()
	{
		mBargeInfo.InitDestructionValues();
	}
}