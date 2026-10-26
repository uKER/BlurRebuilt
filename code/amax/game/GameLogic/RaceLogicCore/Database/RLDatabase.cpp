#include "stdafx.h"

#include "../Core/RaceLogicCore.h"
#include "../../GameData/GameData.h"
#include "RLDatabase.h"
#include "../Data/RLVehicleData.h"
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <System/Misc/Random.h>

RLDatabase RLDatabase::s_instance;

static Debug::Logger &g_RLDatabaseLogger = Debug::GetLogger("Amax.RLC.RLDatabase");

#define RLDATABASE_CALLBACK(Type, Member) \
	class Callback##Type { public: RLData<Type*>* operator()() { return &RLDatabase::Instance().Member; } };


///////////////////////////////////////////////////////////////////////////////
//BASE CLASS
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(RLBaseObj,								"RLBaseObject")
	XT_FIELD(mLoc_string,						"LocString")
XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//BASE CLASS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//HELPERS
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(RLMinMax,								"MinMax")
	XT_FIELD(mMax,								"Max")
	XT_FIELD(mMin,								"Min")
XT_END_TYPE

XT_TYPE(RLAngularVelocity,						"AngularVelocity")
	XT_FIELD(mPitch,							"Pitch")
	XT_FIELD(mYaw,								"Yaw")
	XT_FIELD(mRoll,								"Roll")
XT_END_TYPE


XT_TYPE(RLRuleDebugInfo,						"RuleDebugInfo")
	XT_FIELD(mOutputDebugInfo,					"OutputDebugInfo")
XT_END_TYPE

XT_TYPE(RLRuleConditionalDisplay,				"RuleConditionalDisplay")
	XT_FIELD(mConditionalDisplay,				"ConditionalDisplay")
	XT_FIELD(mDisplayWhenTrue,					"DisplayWhenTrue")
XT_END_TYPE

XT_TYPE(RLHermiteKeyFrame,						"RLHermiteKeyFrame")
	XT_FIELD(mTime,								"Time")
	XT_FIELD(mValue,							"Value")
	XT_FIELD(mInTangent,						"InTangent")
	XT_FIELD(mOutTangent,						"OutTangent")
XT_END_TYPE

XT_TYPE(RLHermiteCurve,							"RLHermiteCurve")
	XT_FIELD(mKeyFrames,						"KeyFrames")
XT_END_TYPE

XT_TYPE(RLAIPickupHermiteKeyFrame,				"RLPickupAccuracyHermiteKeyFrame")
	XT_FIELD(mDistance,							"Distance")
	XT_FIELD(mProbability,						"Probability")
	XT_FIELD(mInTangent,						"InTangent")
	XT_FIELD(mOutTangent,						"OutTangent")
XT_END_TYPE

XT_TYPE(RLAIPickupHermiteCurve,					"RLPickupAccuracyHermiteCurve")
	XT_FIELD(mKeyFrames,						"KeyFrames")
XT_END_TYPE

///////////////////////////////////////////////////////////////////////////////
//HELPERS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//PICKUPS
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(E_PICKUP_TYPE,							"Pickup")
	XT_OPTION(E_PICKUP_TYPE_NITROUS,			"Nitrous")
	XT_OPTION(E_PICKUP_TYPE_SHOCK,				"Shock")
	XT_OPTION(E_PICKUP_TYPE_SHUNT,				"Shunt")
	XT_OPTION(E_PICKUP_TYPE_BLINDFIRE,			"Blindfire")
	XT_OPTION(E_PICKUP_TYPE_BARGE,				"Barge")
	XT_OPTION(E_PICKUP_TYPE_BOMB,				"Bomb")

	XT_OPTION(E_PICKUP_TYPE_REPAIR,				"Repair")
	XT_OPTION(E_PICKUP_TYPE_SHIELD,				"Shield")
	XT_OPTION(E_PICKUP_TYPE_FAN,				"Fan")
	XT_OPTION(E_PICKUP_TYPE_STOPWATCH,			"Stopwatch")
	XT_OPTION(E_PICKUP_TYPE_SUPER_SHIELD,		"SuperShield")
	
	XT_OPTION(E_PICKUP_TYPE_ALL,				"All")
XT_END_TYPE

// Fake version for flags
XT_TYPE( E_POSITION_ACTOR_PICKUP_TYPE,			"PickupTypeList" )
	XT_FLAG_STORAGE(XtU8)
	XT_FLAG( 1 << E_PICKUP_TYPE_NITROUS,		"Nitrous" )
	XT_FLAG( 1 << E_PICKUP_TYPE_SHOCK,			"Shock" )
	XT_FLAG( 1 << E_PICKUP_TYPE_SHUNT,			"Shunt" )
	XT_FLAG( 1 << E_PICKUP_TYPE_BLINDFIRE,		"Bolt" )
	XT_FLAG( 1 << E_PICKUP_TYPE_BARGE,			"Barge" )
	XT_FLAG( 1 << E_PICKUP_TYPE_BOMB,			"Mine" )
	XT_FLAG( 1 << E_PICKUP_TYPE_REPAIR,			"Repair" )
	XT_FLAG( 1 << E_PICKUP_TYPE_SHIELD,			"Shield" )
XT_END_TYPE

XT_TYPE(RLPickupPositionRow,					"PickupPositionRow")
	XT_FIELD(mPickupFlagList,					"TypeList")
XT_END_TYPE

XT_TYPE(E_PICKUP_AI_TYPE,						"AIUsageType")
	XT_OPTION(E_PICKUP_AI_TYPE_AGGRESSIVE,		"Aggressive")	
	XT_OPTION(E_PICKUP_AI_TYPE_PASSIVE,			"Passive")
XT_END_TYPE

XT_TYPE(RLBalancingGraph,						"BalancingGraph")
	XT_FIELD(mEffectGraph,						"EffectGraph")
	XT_FIELD(mMaxBoost,							"MaxBoost")
XT_END_TYPE

XT_TYPE(RLBalancingDataBase,					"BalancingDataBase")
	XT_FIELD(mDelay,							"Delay")
	XT_FIELD(mHealth,							"Health")
XT_END_TYPE

XT_TYPE(RLNitrousBalancingData,					"NitrousBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mDuration,							"Duration")
	XT_FIELD(mSlowDownVal,						"SlowDownGraphValue")

	XT_FIELD(mDurationCheckpoint,				"DurationCheckpoint")
	XT_FIELD(mSlowDownValCheckpoint,			"SlowDownGraphValueCheckpoint")

	XT_FIELD(mClassAModifier,					"ClassAModifier")
	XT_FIELD(mClassBModifier,					"ClassBModifier")
	XT_FIELD(mClassCModifier,					"ClassCModifier")
	XT_FIELD(mClassDModifier,					"ClassDModifier")
XT_END_TYPE

XT_TYPE(RLNitrousDoubleTapBalancingData,		"NitrousDoubleTapBalancingData")
	XT_BASE(RLBalancingDataBase)

	XT_FIELD(mLowSpeedBoost,					"LowSpeedBoost")
	XT_FIELD(mDraftingBoost,					"DraftingBoost")
	XT_FIELD(mDuration,							"Duration")

	XT_FIELD(mRiftTime,							"RiftTime")
	XT_FIELD(mRiftRadius,						"RiftRadius")										 
	XT_FIELD(mRiftMinForce,						"RiftMinForce")
	XT_FIELD(mRiftMaxForce,						"RiftMaxForce")
	XT_FIELD(mRiftAngleRange,					"RiftAngleRange")

	XT_FIELD(mAirBrakeCutOff,					"AirBrakeCutOff")
XT_END_TYPE

XT_TYPE(RLNitrousReverseBalancingData,			"NitrousReverseBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
	XT_FIELD(mDuration,							"Duration")
	XT_FIELD(mForwardDot,						"ForwardDot")
	XT_FIELD(mRiftRangeSq,						"RiftRangeSquared")
XT_END_TYPE

XT_TYPE(RLShockBalancingData,					"ShockBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mInnerRadius,						"InnerRadius")
	XT_FIELD(mOuterRadius,						"OuterRadius")
	XT_FIELD(mLifetime,							"Lifetime")
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
	XT_FIELD(mHeightDiff,						"HeightDiff")
XT_END_TYPE

XT_TYPE(RLBlindfireBalancingData,				"BlindfireBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mRadius,							"Radius")
	XT_FIELD(mFallRate,							"FallRate")
	XT_FIELD(mSpeed,							"Speed")
	XT_FIELD(mHeightFromGround,					"HeightFromGround")
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
	XT_FIELD(mWarningDistance,					"WarningDistance")
	XT_FIELD(mWarningDot,						"WarningDot")
XT_END_TYPE

XT_TYPE(RLShuntBalancingData,					"ShuntBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mRadius,							"Radius")
	XT_FIELD(mFallRate,							"FallRate")
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
	XT_FIELD(mFXDropTime,						"FXDropTime")
	XT_FIELD(mShuntTurnRateScalar,				"ShuntTurnRateScalar")
	XT_FIELD(mECMActivationDist,				"ECMActivationDist")
	XT_FIELD(mECMDeactivationDist,				"ECMDeactivationDist")
	XT_FIELD(mECMWarningDist,					"ECMWarningDist")
	XT_FIELD(mECMWarningEnabled,				"ECMWarningEnabled")

XT_END_TYPE

XT_TYPE(RLShuntDoubleBalancingData,				"ShuntDoubleBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mShotDelay,						"ShotDelay")
XT_END_TYPE

XT_TYPE(RLShuntReverseBalancingData,			"ShuntReverseBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mRadius,							"Radius")
	XT_FIELD(mFallRate,							"FallRate")
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
	XT_FIELD(mFXDropTime,						"FXDropTime")
	XT_FIELD(mSpeed,							"Speed")
	XT_FIELD(mHeightFromGround,					"HeightFromGround")
	XT_FIELD(mWarningDistance,					"WarningDistance")
	XT_FIELD(mWarningDot,						"WarningDot")
XT_END_TYPE

XT_TYPE(RLBargeBalancingData,					"BargeBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mRadius,							"Radius")
	XT_FIELD(mAngleRange,						"AngleRange")
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
	XT_FIELD(mDuration,							"Duration")

	XT_FIELD(mMinMinePushForce,					"MinMinePushForce")
	XT_FIELD(mMinePushForceMultiplier,			"MinePushForceMultiplier")
XT_END_TYPE

XT_TYPE(RLBombBalancingData,					"BombBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mRadius,							"Radius")
	XT_FIELD(mOuterRadius,						"OuterRadius")
	XT_FIELD(mPlaceBehindDistance,				"PlaceBehindDistance")
	XT_FIELD(mLifespan,							"Lifespan")
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
	XT_FIELD(mMasterMinePrimeTimer,				"MasterMinePrimeTimer")
XT_END_TYPE

XT_TYPE(RLThrownBombBalancingData,				"ThrownBombBalancingData")
	XT_BASE(RLBombBalancingData)
	XT_FIELD(mThrowForceY,						"ThrowForceY")
	XT_FIELD(mThrowForceMin,					"ThrowForceMin")
	XT_FIELD(mThrowForceMax,					"ThrowForceMax")
XT_END_TYPE

XT_TYPE(RLBombDoubleTapBalancingData,			"BombDoubleTapBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mRadius,							"Radius")
	XT_FIELD(mPlaceBehindDistance,				"PlaceBehindDistance")
	XT_FIELD(mLifespan,							"Lifespan")
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
XT_END_TYPE

XT_TYPE(RLBombReverseBalancingData,				"BombReverseBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mRadius,							"Radius")
	XT_FIELD(mFallRate,							"FallRate")
	XT_FIELD(mLifetime,							"Lifetime")
	XT_FIELD(mMinSpeed,							"MinSpeed")
	XT_FIELD(mScaledSpeed,						"ScaledSpeed")
	XT_FIELD(mDeceleration,						"Deceleration")
	XT_FIELD(mHeightFromGround,					"HeightFromGround")
	XT_FIELD(mDamageInflicted,					"DamageInflicted")
	XT_FIELD(mWarningDistance,					"WarningDistance")
	XT_FIELD(mWarningDot,						"WarningDot")
XT_END_TYPE

XT_TYPE(RLBombModifiedBalancingData,			"BombModifiedBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mPlaceBehindDistance,				"PlaceBehindDistance")
	XT_FIELD(mNoMiniMinesToCreate,				"NoMiniMinesToCreate")
	XT_FIELD(mTotalSpreadWidth,					"TotalSpreadWidth")
XT_END_TYPE

XT_TYPE(RLRepairBalancingData,					"RepairBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mAmountToRepair,					"AmountToRepair")
XT_END_TYPE

XT_TYPE(RLShieldBalancingData,					"ShieldBalancingData")
	XT_BASE(RLBalancingDataBase)
	XT_FIELD(mLifetime,							"Lifetime")
XT_END_TYPE

XT_TYPE(RLSuperShieldBalancingData,				"SuperShieldBalancingData")
	XT_BASE(RLBalancingDataBase)

	XT_FIELD(mLifetime,							"Lifetime")

	XT_FIELD(mNitroOnStartup,					"NitroOnStartup")
	XT_FIELD(mNitroTargetSpeedInMPH,			"NitroTargetSpeedInMPH")
												 
	XT_FIELD(mBoostDuringUse,					"BoostDuringUse")
	XT_FIELD(mSteeringForwardOffset,			"SteeringForwardOffset")
	XT_FIELD(mSteeringBoost,					"SteeringBoost")
	XT_FIELD(mAccelerationBoost,				"AccelerationBoost")
	XT_FIELD(mBrakingBoost,						"BrakingBoost")
	XT_FIELD(mDownForceBoost,					"DownForceBoost")
												 
	XT_FIELD(mMinimumTriggerValue,				"MinimumTriggerValue")

	XT_FIELD(mIncreaseMassDuringUse,			"IncreaseMassDuringUse")
	XT_FIELD(mIncreasedMass,					"IncreasedMass")
												 
	XT_FIELD(mDamagePointsOnImpact,				"DamagePointsOnImpact")
XT_END_TYPE

XT_TYPE( RLPickupSpecialFXLocation, "PickupSpecialFXLocation" )
	XT_OPTION( E_PICKUP_SPECIALFX_LOCATION_ACTINGCAR,					"ActingCar" )
	XT_OPTION( E_PICKUP_SPECIALFX_LOCATION_TARGETCAR,					"TargetCar" )
	XT_OPTION( E_PICKUP_SPECIALFX_LOCATION_TARGETCAR_FIXEDORIENTATION,	"TargetCarFixedOrientation" )
	XT_OPTION( E_PICKUP_SPECIALFX_LOCATION_WORLDPOS,					"WorldPosition" )
	XT_OPTION( E_PICKUP_SPECIALFX_LOCATION_REARWHEELPOS,				"RearWheelPosition" )
	XT_OPTION( E_PICKUP_SPECIALFX_LOCATION_VEHICLETOMATRIX,				"VehicleToMatrix" )
XT_END_TYPE

XT_TYPE(RLPickupSpecialFXDuration,				"PickupSpecialFXDuration")
	XT_FIELD(mStart,							"StartTime")
	XT_FIELD(mEnd,								"EndTime")
XT_END_TYPE

XT_TYPE(RLPickupSpecialFX,						"PickupSpecialFX")
	XT_FIELD(mSpecialFXLocation,				"SpecialFXLocation")
	XT_FIELD(mLocalSpecialFX,					"LocalSpecialFX")
	XT_FIELD(mExternalSpecialFX,				"ExternalSpecialFX")
	XT_FIELD(mInternalSpecialFX,				"InternalSpecialFX")
	XT_FIELD(mGlobalSpecialFX,					"GlobalSpecialFX")	
	XT_FIELD(mDurationIntro,					"DurationIntro")
	XT_FIELD(mDurationLoop,						"DurationLoop")
	XT_FIELD(mDurationOutro,					"DurationOutro")
XT_END_TYPE

XT_TYPE(RLSpecialFXBase,						"PickupBaseSpecialFX")
	XT_FIELD(mChargingSpecialFX,				"ChargingSpecialFX")
	XT_FIELD(mPickedUpSpecialFX,				"PickedUpSpecialFX")
	XT_FIELD(mRoadsweeperedSpecialFX,			"RoadsweeperedSpecialFX")
	XT_FIELD(mFiredSpecialFX,					"FiredSpecialFX")
	XT_FIELD(mHitVehicleSpecialFX,				"HitVehicleSpecialFX")
	XT_FIELD(mHitVehicleSpecialFXDecoy,			"HitVehicleSpecialFXDecoy")
	XT_FIELD(mShieldedSpecialFX,				"ShieldedSpecialFX")
XT_END_TYPE

XT_TYPE(RLShuntSpecialFX,						"PickupShuntSpecialFX")
	XT_BASE(RLSpecialFXBase)
	XT_FIELD(mProjectileSpecialFX,				"ProjectileSpecialFX")
	XT_FIELD(mProjectileDroppedSpecialFX,		"ProjectileDroppedSpecialFX")
	XT_FIELD(mHitEnvironmentSpecialFX,			"HitEnvironmentSpecialFX")
	XT_FIELD(mExplosionSpecialFX,				"ExplodeSpecialFX")
	XT_FIELD(mSafetyNetShieldedSpecialFX,		"SafetyNetShieldedSpecialFX")
	XT_FIELD(mIronFistFiredSpecialFX,			"IronFistFiredSpecialFX")
XT_END_TYPE

XT_TYPE(PickupShuntDoubleSpecialFX,				"PickupShuntDoubleSpecialFX")
	XT_BASE(RLSpecialFXBase)
XT_END_TYPE

XT_TYPE(RLShockSpecialFX,						"PickupShockSpecialFX")
	XT_BASE(RLSpecialFXBase)
	XT_FIELD(mShockDomeSpecialFX,				"DomeSpecialFX")
	XT_FIELD(mSafetyNetShieldedSpecialFX,		"SafetyNetShieldedSpecialFX")
	XT_FIELD(mIronFistDomeSpecialFX,			"IronFistDomeSpecialFX")
XT_END_TYPE

XT_TYPE(RLNitrousSpecialFX,						"PickupNitrousSpecialFX")
	XT_BASE(RLSpecialFXBase)
	XT_FIELD(mDrifterGiftedNitroSpecialFX,		"DrifterGiftedNitroSpecialFX")
XT_END_TYPE

XT_TYPE(RLNitrousReverseSpecialFX,				"PickupNitrousReverseSpecialFX")
	XT_BASE(RLSpecialFXBase)
	XT_FIELD(mForwardSpecialFX,					"ForwardSpecialFX")
XT_END_TYPE

XT_TYPE(RLStopwatchSpecialFX,					"PickupStopwatchSpecialFX")
	XT_BASE(RLSpecialFXBase)
XT_END_TYPE

XT_TYPE(RLBlindfireSpecialFX,					"PickupBlindfireSpecialFX")
	XT_BASE(RLSpecialFXBase)
	XT_FIELD(mProjectileSpecialFX,				"ProjectileSpecialFX")
	XT_FIELD(mHitEnvironmentSpecialFX,			"HitEnvironmentSpecialFX")
	XT_FIELD(mSafetyNetShieldedSpecialFX,		"SafetyNetShieldedSpecialFX")
	XT_FIELD(mIronFistFiredSpecialFX,			"IronFistFiredSpecialFX")
	XT_FIELD(mScramblerHitVehicleSpecialFX,		"ScramblerHitVehicleSpecialFX")
XT_END_TYPE

XT_TYPE(RLBombSpecialFX,						"PickupBombSpecialFX")
	XT_BASE(RLSpecialFXBase)
	XT_FIELD(mMineSpecialFX,					"MineSpecialFX")
	XT_FIELD(mExplodeSpecialFX,					"ExplodeSpecialFX")
	XT_FIELD(mSafetyNetShieldedSpecialFX,		"SafetyNetShieldedSpecialFX")
	XT_FIELD(mIronFistFiredSpecialFX,			"IronFistFiredSpecialFX")
	XT_FIELD(mMasterMinePrimedSpecialFX,		"MasterMinePrimedSpecialFX")
	XT_FIELD(mMasterMineEquipedSpecialFX,		"MasterMineEquipedSpecialFX")
XT_END_TYPE

XT_TYPE(RLBargeSpecialFX,						"PickupBargeSpecialFX")
	XT_BASE(RLSpecialFXBase)
	XT_FIELD(mFiredFixedPositionSpecialFX,		"FiredFixedPositionSpecialFX")
	XT_FIELD(mSplashDamageChargingSpecialFX,	"SplashDamageChargingSpecialFX")
	XT_FIELD(mSplashDamageFiredSpecialFX,		"SplashDamageFiredSpecialFX")
	XT_FIELD(mSafetyNetShieldedSpecialFX,		"SafetyNetShieldedSpecialFX")
	XT_FIELD(mIronFistFiredSpecialFX,			"IronFistFiredSpecialFX")
	XT_FIELD(mLastGaspChargingSpecialFX,		"LastGaspChargingSpecialFX")
	XT_FIELD(mLastGaspFiredSpecialFX,			"LastGaspFiredSpecialFX")
	XT_FIELD(mSplashDamageVehicleHitSpecialFX,	"SplashDamageHitVehicleSpecialFX")
XT_END_TYPE

XT_TYPE(RLShieldSpecialFX,						"PickupShieldSpecialFX")
	XT_BASE(RLSpecialFXBase)
	XT_FIELD(mShieldDomeSpecialFX,				"ShieldDomeSpecialFX")
	XT_FIELD(mShieldDomeShieldedBoostersSpecialFX,				"ShieldedBoostersSpecialFX")
XT_END_TYPE

XT_TYPE(RLRepairSpecialFX,						"PickupRepairSpecialFX")
	XT_BASE(RLSpecialFXBase)
XT_END_TYPE

void RLBalancingGraphCallback(XtEventType event, XtRecord* pRecord, RLPickupLevel* pRLPickupLevel, XtCallbackContext*)
{
	if(event == kXtEtConstruct && pRLPickupLevel)
	{
		RLHermiteKeyFrame keyFrame;

		for(int j = 0; j < pRLPickupLevel->mBalancingGraph.mEffectGraph.count; j++)
		{
			keyFrame = pRLPickupLevel->mBalancingGraph.mEffectGraph[j];
			pRLPickupLevel->mBalancingGraph.mEffectCurve.AddKeyFrame(keyFrame.mTime, keyFrame.mValue, keyFrame.mInTangent, keyFrame.mOutTangent);
		}
	}
}

XT_TYPE(RLPickupLevel,							"PickupLevel")
	XT_CALLBACK(RLBalancingGraphCallback)
	XT_FIELD(mBalancingGraph,					"BalancingGraph")
	XT_FIELD(mBalancingData,					"BalancingData")
	XT_FIELD(mSpecialFX,						"SpecialFX")
XT_END_TYPE

XT_TYPE(RLPickupPositionModifier,				"PickupPositionModifier")
	XT_FIELD(minPos,							"MinPos")
	XT_FIELD(maxPos,							"MaxPos")
	XT_FIELD(modifier,							"Modifier")
XT_END_TYPE

XT_TYPE(RLPickupType,							"PickupType")
	XT_BASE(RLBaseObj)

	XT_FIELD(meType,							"Type")

	XT_FIELD(mNormal,							"Normal")
	XT_FIELD(mDoubleTap,						"DoubleTap")
	XT_FIELD(mReversed,							"Reversed")
	XT_FIELD(mReversedDoubleTap,				"ReversedDoubleTap")
	XT_FIELD(mMini,								"Mini")
	XT_FIELD(mModified,							"Modified")
	XT_FIELD(mReverseModified,					"ReversedModified") 

	XT_FIELD(meAIType,							"AIUsageType")
	XT_FIELD(mBaseProbability,					"BaseProbability")

	XT_FIELD(mDoNotRandomise,					"DoNotRandomise")
	XT_FIELD(mPositionModifiers,				"PositionModifiers")

	XT_FIELD(mPickupSpecialFX,					"PickupSpecialFX")
	XT_FIELD(mPickupSpecialFXLocal,				"PickupSpecialFXLocal")
	XT_FIELD(mPickupSpecialFXFaded,				"PickupSpecialFXFaded")
	XT_FIELD(mPickupSpecialFXAggressive,		"PickupSpecialFXAggressive")
	XT_FIELD(mPickupSpecialFXAggressiveFaded,	"PickupSpecialFXAggressiveFaded")
	XT_FIELD(mPickupSpecialFXGolden,			"PickupSpecialFXGolden")
	XT_FIELD(mPickupSpecialFXGoldenFaded,		"PickupSpecialFXGoldenFaded")
	XT_FIELD(mPickupSpecialFXDecoy,				"PickupSpecialFXDecoy")
	XT_FIELD(m_PickupSpecialFXBeginCycle,		"PickupSpecialFXBeginCycle")
	XT_FIELD(m_PickupSpecialFXEndCycle,			"PickupSpecialFXEndCycle")

	XT_FIELD(mPickupFiringDelayFX,				"PickupFiringDelayFX")
	XT_FIELD(mPickupAIFiringDelayFX,			"PickupAIFiringDelayFX")
	XT_FIELD(mTargetteeSpecialFX,				"TargetteeSpecialFX")
	XT_FIELD(mTargettedSpecialFX,				"TargettedSpecialFX")
XT_END_TYPE

XT_TYPE(RLPickupParameters,						"PickupParameters")
	XT_FIELD(mPickupRadius,						"PickupRadius")
	XT_FIELD(mPickupDeactivateTimer,			"PickupDeactivateTimer")
	XT_FIELD(mPostPickupAwardCoolDownTimer,		"PostPickupAwardCoolDownTimer")
	XT_FIELD(mDestroyExistingPickupOnNewPickup,	"DestroyExistingPickupOnNewPickup")

	XT_FIELD(mMaximumXPMultiplication,			"MaximumXPMultiplication")

	XT_FIELD(mGarageSpecialFX,					"GarageSpecialFX")
	XT_FIELD(mGarageLockedCarSFX,				"GarageLockedCarSFX")
	
XT_END_TYPE

XT_TYPE(RLPickupParametersStoredOnline, "PickupParametersStoredOnline")
	XT_FIELD(mRecoveryTime, "RecoveryTime")
	XT_FIELD(mRecoveryTimeSpeedMod, "RecoveryTimeSpeedMod")
XT_END_TYPE

XT_TYPE(RLPickupSystem,							"PickupSystem")
	XT_FIELD(mParameters,						"Parameters")
	XT_FIELD(mParametersStoredOnline,			"ParametersStoredOnline")
	XT_FIELD(mTypes,							"Types")
XT_END_TYPE


//================================================================================

XT_TYPE(E_FORCE_EFFECT_TYPE,					"ForceEffectType" )
	XT_OPTION( E_FORCE_EFFECT_TYPE_AREA,		"Area" )
	XT_OPTION( E_FORCE_EFFECT_TYPE_FORWARD,		"Forward" )
	XT_OPTION( E_FORCE_EFFECT_TYPE_SIDE,		"Side" )
XT_END_TYPE

XT_TYPE(RLForceEffect,							"ForceEffect")
	XT_FIELD(m_ForceType,						"ForceType")
	XT_FIELD(m_MaxForce,						"MaxForce")
	XT_FIELD(m_Radius,							"Radius")
	XT_FIELD(m_Time,							"Time")
	XT_FIELD(m_ConeAngle,						"ConeAngle")
	XT_FIELD(m_UpwardForce,						"UpwardForce")
	XT_FIELD(m_DamageScale,						"DamageScale")
	XT_FIELD(m_ElectricalEffect,				"ElectricalEffect")
	XT_FIELD(m_ElectricalEffectColour,			"ElectricalEffectColour")
XT_END_TYPE

XT_TYPE(RLPickupForceEffects,					"PerkForceEffects")
	XT_FIELD(m_LargeExplosion,					"LargeExplosion")
	XT_FIELD(m_SmallExplosion,					"SmallExplosion")
	XT_FIELD(m_Shock,							"Shock")
	XT_FIELD(m_ShuntFire,						"ShuntFire")
	XT_FIELD(m_ShuntExplode,					"ShuntExplode")
	XT_FIELD(m_Barge,							"Barge")
	XT_FIELD(m_Mine,							"Mine")
	XT_FIELD(m_MineProjectile,					"MineProjectile")
	XT_FIELD(m_Blindfire,						"Blindfire")
XT_END_TYPE



///////////////////////////////////////////////////////////////////////////////
//PICKUPS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//AI PICKUP USE
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(Game::GAME_DIFFICULTY,						"GameDifficulty")
	XT_OPTION(Game::GAME_DIFFICULTY_VERY_EASY,		"VeryEasy")	
	XT_OPTION(Game::GAME_DIFFICULTY_EASY,			"Easy")	
	XT_OPTION(Game::GAME_DIFFICULTY_NORMAL,			"Normal")
	XT_OPTION(Game::GAME_DIFFICULTY_HARD,			"Hard")
	XT_OPTION(Game::GAME_DIFFICULTY_HARDCORE,		"HardCore")
XT_END_TYPE

XT_TYPE(RLAIPickupController,							"AIPickupController")
	XT_FIELD( mControllers,								"Controllers" )
XT_END_TYPE

XT_TYPE(AIPickupControllerDifficulty,					"AIPickupControllerDifficulty")
	XT_FIELD( mDifficulty,								"Difficulty" )
	XT_FIELD( mController,								"Controller" )
XT_END_TYPE

XT_TYPE(AIPickupControllerSetup,						"AIPickupControllerSetup")
	XT_FIELD( mSharedEvalTime,							"EvaluateTimeInSeconds" )
	XT_FIELD( mSharedShieldEvalTime,					"EvaluateShieldTimeInSeconds" )
	XT_FIELD( mSharedFiredPerkDisableTime,				"FiredPerkDisableTime" ) 
	XT_FIELD( mSharedPickupSwitchTime,					"PickupSwitchTime" )
	XT_FIELD( mSharedBeenHitWithPerkDisableTime,		"BeenHitWithPerkDisableTime" ) 
	XT_FIELD( mSharedHumanAvoidRatio,					"IgnoreHumanRatio")
	XT_FIELD( mSharedPlayerHitDisableTime,				"PlayerHitWithPerkDisableTime" )
	XT_FIELD( mSharedOutOfRangePickupDropTimeMin,       "OutOfRangePickDropTimeMin" )
	XT_FIELD( mSharedOutOfRangePickupDropTimeMax,       "OutOfRangePickDropTimeMax" )
	XT_FIELD( mTempFireRatio,							"FireProbabilityRatio" )

	XT_FIELD( mAIPickupAccuracyGraphList,				"AIPickupAccuracyGraphs")
	XT_FIELD( mAIShieldedHermiteCurve,					"ShieldedGraph")
XT_END_TYPE

XT_TYPE(AIPickupGraph,									"AIPickupAccuracyGraph")
	XT_FIELD( mAIPickupAccuracyHermiteCurve,			"InRangeAccuracyGraph")
XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//AI PICKUP USE
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//SCORING
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(E_SCORE_RULE,							"ScoreRule")
	XT_OPTION(E_SCORE_RULE_WHEELS_OFF_GROUND,	"WheelsOffGround")
	XT_OPTION(E_SCORE_RULE_WHEELS_ON_GROUND,	"WheelsOnGround")

	XT_OPTION(E_SCORE_RULE_AIR_DISTANCE_TRAVELLED,"AirDistanceTravelled")
	XT_OPTION(E_SCORE_RULE_DRIFT_DISTANCE_TRAVELLED,"DriftDistanceTravelled")

	XT_OPTION(E_SCORE_RULE_BEST_RACE_POSITION,	"BestRacePosition")
	XT_OPTION(E_SCORE_RULE_SPEED_IN_MPH,		"SpeedInMPH")
	XT_OPTION(E_SCORE_RULE_DRIFTING,			"Drifting")

	XT_OPTION(E_SCORE_RULE_LIGHT_INTRAVEHICULAR_IMPACT, "LightIntraVehicularImpact")
	XT_OPTION(E_SCORE_RULE_MEDIUM_INTRAVEHICULAR_IMPACT, "MediumIntraVehicularImpact")
	XT_OPTION(E_SCORE_RULE_HEAVY_INTRAVEHICULAR_IMPACT, "HeavyIntraVehicularImpact")

XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//SCORING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//DAMAGE
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(E_IMPACT_LEVEL,							"ImpactLevel")					
	XT_OPTION(E_IMPACT_LEVEL_NONE,				"NoImpact")

	XT_OPTION(E_IMPACT_LEVEL_LIGHT,				"LightImpact")
	XT_OPTION(E_IMPACT_LEVEL_MEDIUM,			"MediumImpact")
	XT_OPTION(E_IMPACT_LEVEL_HEAVY,				"HeavyImpact")
XT_END_TYPE

//Defines how the gamelogic deals with impacts
XT_TYPE(E_HEALTH_LEVEL,					"HealthLevel")
	XT_OPTION(E_HEALTH_LEVEL_WRECKED,	"Wrecked")
	
	XT_OPTION(E_HEALTH_LEVEL_LOW,		"LowHealth")
	XT_OPTION(E_HEALTH_LEVEL_MEDIUM,	"MediumHealth")
	XT_OPTION(E_HEALTH_LEVEL_HIGH,		"HighHealth")
XT_END_TYPE

XT_TYPE(RLImpactType,							"ImpactType")
	XT_FIELD(meImpactLevel,						"ImpactLevel")
	XT_FIELD(mFrontImpulseBounds,				"ImpulseBounds")
	XT_FIELD(mSideImpulseBounds,				"SideImpulseBounds")
	XT_FIELD(mRollScreenEffect,					"RollScreenEffect")
	XT_FIELD(mFrontScreenEffect,				"FrontScreenEffect")
	XT_FIELD(mSideScreenEffect,					"SideScreenEffect")
XT_END_TYPE

XT_TYPE(RLLandingType,							"LandingType")
	XT_FIELD(meImpactLevel,						"ImpactLevel")
	XT_FIELD(mImpulseBounds,					"ImpulseBounds")
	XT_FIELD(mScreenEffect,						"ScreenEffect")
XT_END_TYPE

XT_TYPE(RLBodyShakeParams,						"BodyShakeParams")
	XT_FIELD(mMinSpeed,							"minSpeed")
	XT_FIELD(mSpeedRange,						"speedRange")
	XT_FIELD(mFrequency,						"frequency")
	XT_FIELD(mShakeRange,						"shakeRange")
XT_END_TYPE

XT_TYPE(RLPartShakeParams,						"PartShakeParams")
	XT_FIELD(mMinSpeed,							"minSpeed")
	XT_FIELD(mSpeedRange,						"speedRange")
	XT_FIELD(mTorque,							"torque")
XT_END_TYPE

XT_TYPE(RLHealthLevelZone,						"HealthLevelZone")
	XT_FIELD(mHealthZone,						"HealthZone")
	XT_FIELD(mHealthMin,						"HealthMin")
	XT_FIELD(mHealthMax,						"HealthMax")

	XT_FIELD(mScreenEffect,						"ScreenEffect")	
	XT_FIELD(mDamageFilter,						"DamageFilter")

	XT_FIELD(mBodyShake,						"BodyShake")
	XT_FIELD(mPartShake,						"PartShake")
XT_END_TYPE

XT_TYPE(RLDamageAllowable,						"DamageAllowable")
	XT_FIELD(mAllowable,						"allowable")
	XT_FIELD(mImpactRequired,					"impactRequired")
XT_END_TYPE

XT_TYPE(RLDamageFilter,							"DamageFilter")
	XT_FIELD(mScratchDamage,					"scratchDamage")
	XT_FIELD(mDeformParts,						"deformParts")
	XT_FIELD(mDislodgeParts,					"dislodgeParts")
	XT_FIELD(mDetachParts,						"detachParts")
	XT_FIELD(mBurstTyres,						"burstTyres")
	XT_FIELD(mDetachWheels,						"detachWheels")
	XT_FIELD(mLightBreak,						"lightBreak")
	XT_FIELD(mLightKnockOut,					"lightKnockOut")
	XT_FIELD(mRadiatorSmoke,					"radiatorSmoke")
	XT_FIELD(mEngineSmokeLight,					"engineSmokeLight")
	XT_FIELD(mEngineSmokeHeavy,					"engineSmokeHeavy")
	XT_FIELD(mEngineFire,						"engineFire")
XT_END_TYPE

XT_TYPE(RLInertiaScale,							"InertiaScale")
	XT_FIELD(mHumanToAiInertiaScaleHuman,		"HumanToAiInertiaScaleHuman")
	XT_FIELD(mHumanToAiInertiaScaleAi,			"HumanToAiInertiaScaleAi")
	XT_FIELD(mHumanToHumanInertiaScale,			"HumanToHumanInertiaScale")
	XT_FIELD(mAiToAiInertiaScale,				"AiToAiInertiaScale")
	XT_FIELD(mCarToObjectInertiaScaleCar,		"CarToObjectInertiaScaleCar")
	XT_FIELD(mCarToObjectInertiaScaleObject,	"CarToObjectInertiaScaleObject")
	XT_FIELD(mHumanToAiMassScaleHuman,			"HumanToAiMassScaleHuman")
	XT_FIELD(mHumanToAiMassScaleAi,				"HumanToAiMassScaleAi")
	XT_FIELD(mCarToObjectMassScaleCar,			"CarToObjectMassScaleCar")
	XT_FIELD(mCarToObjectMassScaleObject,		"CarToObjectMassScaleObject")												

	XT_FIELD(mScaledImpactMaxSpeed,				"ScaledImpactMaxSpeed")
	XT_FIELD(mScaledImpactMinSpeed,				"ScaledImpactMinSpeed")
	XT_FIELD(mMinCarToObjectMassValue,			"MinCarToObjectMassValue")
XT_END_TYPE

XT_TYPE(RLVehicleHealthGroup,					"VehicleHealthGroup")
	XT_FIELD(mHealthGroup,						"HealthGroup")
	XT_FIELD(mBaseHealth,						"BaseHealth")
XT_END_TYPE

XT_TYPE(RLDamageParameters,						"DamageParameters")
	XT_FIELD(mMaxWorldDamageImpulse,			"MaxWorldDamageImpulse")
	XT_FIELD(mMaxWorldEffectImpulse,			"MaxWorldEffectImpulse")
	XT_FIELD(mMaxVehicleDamageImpulse,			"MaxVehicleDamageImpulse")
	XT_FIELD(mMaxVehicleEffectImpulse,			"MaxVehicleEffectImpulse")
	XT_FIELD(mMaxObjectDamageImpulse,			"MaxObjectDamageImpulse")
	XT_FIELD(mMaxObjectEffectImpulse,			"MaxObjectEffectImpulse")											

	XT_FIELD(mMaxStaticImpulse,					"MaxStaticImpulse")
	XT_FIELD(mMaxDynamicImpulse,				"MaxDynamicImpulse")

	XT_FIELD(mResetTimerAfterWrecked,			"ResetTimerAfterWrecked")

	XT_FIELD(mCooldownPerk,						"HitByPerkCooldown")
	XT_FIELD(mCooldownCollision,				"CollidedCooldown")

	XT_FIELD(mHealthIncrement,					"HealthIncrement")
	XT_FIELD(mLightImpactDamage,				"LightImpactDamage")
	XT_FIELD(mMediumImpactDamage,				"MediumImpactDamage")
	XT_FIELD(mHeavyImpactDamage,				"HeavyImpactDamage")
	XT_FIELD(mVehicleHealthGroups,				"VehicleHealthGroups")

	XT_FIELD(mCapGameplayImpulse,				"CapGameplayImpulse")

	XT_FIELD(mIntraVehicularCollisionInertiaScale,"IntraVehicularCollisionInertiaScale")
XT_END_TYPE

XT_TYPE(RLDamageSystem,							"DamageSystem")
	XT_FIELD(mParameters,						"Parameters")
	XT_FIELD(mImpactTypes,						"ImpactTypes")
	XT_FIELD(mLandingTypes,						"LandingTypes")
	XT_FIELD(mHealthZones,						"HealthZones")
XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//DAMAGE
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//DRAFTING
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(RLDraftingSystem,						"DraftingSystem")
	XT_FIELD(mDraftingSFX,						"DraftingSpecialFX")
	XT_FIELD(mDraftingFadeInSFX,				"DraftingFadeInSpecialFX")
	XT_FIELD(mDraftingFadeOutSFX,				"DraftingFadeOutSpecialFX")

	XT_FIELD(mDraftingAnimTime,					"DraftingAnimationTime")
	XT_FIELD(mDraftingFadeInTime,				"DraftingFadeInTime")
	XT_FIELD(mDraftingFadeOutTime,				"DraftingFadeOutTime")
XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//DRAFTING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//REPORTING
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(RLReportingParameters,					"ReportingParameters")
	XT_FIELD(mProximityReportingDistanceSquared,"ProximityReportingDistanceSquared")

	XT_FIELD(mEndOfRacePercentage,				"EndOfRacePercentage")
	XT_FIELD(meMajorCrashImpactLevel,			"MajorCrashImpactLevel")

	XT_FIELD(mPileUpMinimumVehicles,			"PileUpMinimumVehicles")
	XT_FIELD(mPileUpMinimumDistance,			"PileUpMinimumDistance")
	XT_FIELD(mePileUpMinimumImpactLevel,		"PileUpMinimumImpactLevel")
	XT_FIELD(mPileUpTimer,						"PileUpTimer")
XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//REPORTING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//HAND OF GOD
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(RLGhostTimeAfterSpawnParameters,		"GhostTimeAfterSpawnParameters")
	XT_FIELD(mGhostTimeAfterHoG,				"GhostTimeAfterHoG")
	XT_FIELD(mGhostTimeAfterWrecked,			"GhostTimeAfterWrecked")
XT_END_TYPE


XT_TYPE(RLHandOfGodParameters,					"HandOfGodParameters")
	XT_FIELD(mResetTimer,						"ResetTimer")
	XT_FIELD(mCollisionTimer,					"CollisionTimer")
	XT_FIELD(mHitByPerkTimer,					"HitByPerkTimer")
	XT_FIELD(mFlippedTimer,						"FlippedTimer")
	XT_FIELD(mOffSplineTimer,					"OffSplineTimer")
	XT_FIELD(mTumbleTimer,						"TumblingTimer")
	XT_FIELD(mStuckTimer,						"StuckTimer")
	XT_FIELD(mBackwardsTimer,					"MovingBackwardsTimer")
	XT_FIELD(mSlappedTimer,						"SlappedTimer")
	XT_FIELD(mRidiculousSlappedTimer,			"RidiculousSlappedTimer")

	XT_FIELD(mWaitTime,							"WaitTime")
	XT_FIELD(mShieldTime,						"ShieldTime")
	XT_FIELD(mAIResetTimeAddition,				"AIResetTimeAddition")
	XT_FIELD(mForwardRecoveryTimeAddition,		"ForwardRecoveryTimeAddition")
	XT_FIELD(mAIControlTime,					"AIControlTime")
	XT_FIELD(mForwardDot,						"ForwardDot")
	XT_FIELD(mForwardResetDot,					"ForwardResetDot")
	XT_FIELD(mMinimumSpeedInMPH,				"MinimumSpeedInMPH")
	XT_FIELD(mMinForwardSpeed,					"MinimumForwardSpeedInMetres")
	XT_FIELD(mMaximumAngularVelocity,			"MaximumAngularVelocity")
	XT_FIELD(mMaximumCollisionAngularVelocity,	"MaximumCollisionAngularVelocity")
	XT_FIELD(mMaximumPerkAngularVelocity,		"MaximumPerkAngularVelocity")
	XT_FIELD(mMinDotUp,							"MinDotUp")
	XT_FIELD(mNoteworthyCollisionImpulse,		"NoteworthyCollisionImpulse")
	XT_FIELD(mDriftDistance,					"DriftDistance")
	XT_FIELD(mDriftDot,							"DriftDot")

	XT_FIELD(mGhostParametersStandard,			"StandardGhostParameters")

	XT_FIELD(mFinishedSpinX,					"FinishedSpinX")
	XT_FIELD(mFinishedSpinY,					"FinishedSpinY")
	XT_FIELD(mFinishedSpinZ,					"FinishedSpinZ")
XT_END_TYPE

///////////////////////////////////////////////////////////////////////////////
//HAND OF GOD
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//ANIMATED INTRO CAMERAS
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(RLGenericCameraSequences,				"GenericCameraSequences")
	XT_FIELD(mSequenceArray,					"SequenceArray")
XT_END_TYPE

XT_TYPE(GenericCameraOffset,					"GenericCameraOffset")
	XT_FIELD(mVehicleOffset,					"VehicleOffset")
	XT_FIELD(mSequence,							"Sequence")
XT_END_TYPE

XT_TYPE(GenericCameraOffsets,					"GenericCameraOffsets")
	XT_FIELD(mOffsetList,						"OffsetList")
XT_END_TYPE

///////////////////////////////////////////////////////////////////////////////
//GAME MODE PARAMETERS
///////////////////////////////////////////////////////////////////////////////
XT_TYPE(RLGameMode_Destruction_CarDifficultyData, "CarDifficultyData")
	XT_FIELD(mHitPoints,				"HitPoints")
	XT_FIELD(mWreckedPoints,			"WreckedPoints")
	XT_FIELD(mDistanceSpecialFX,		"DistanceSpecialFX")
XT_END_TYPE

XT_TYPE(RLGameMode_Destruction, "SP_GameMode_Destruction")
	
	XT_FIELD(mMaxDistanceEffects,		"MaxDistanceEffects")
	XT_FIELD(mMinRangeForDistanceEffect,"MinRangeForDistanceEffect")
	XT_FIELD(mMaxRangeForDistanceEffect,"MaxRangeForDistanceEffect")

	XT_FIELD(mMaxAIVehiclesActive,		"MaxAIVehiclesActive")
	XT_FIELD(mMaxWavesActive,			"MaxWavesActive")
	XT_FIELD(mForwardsCutOffDistance,   "ForwardsCutOffDistance")

	XT_FIELD(mFavourPlayersOwnRoute,    "FavourPlayersOwnRoute")
	XT_FIELD(mSpawnExtraAIToFillAllRoutes,"SpawnExtraAIToFillAllRoutes")

	XT_FIELD(mBombLifetime,				"BombLifetime")
	
	XT_FIELD(mEasyCar,					"EasyCar")
	XT_FIELD(mMediumCar,				"MediumCar")
	XT_FIELD(mHardCar,					"HardCar")
	XT_FIELD(mVHardCar,					"VHardCar")


	XT_FIELD(mSpawnSpecialFX,			"SpawnSpecialFX")
	XT_FIELD(mRemovedSpecialFX,			"RemovedSpecialFX")
XT_END_TYPE

XT_TYPE(RLGameModeParameters,			"GameModeSystem")
	XT_FIELD(mDestruction,				"SP_Destruction")			
XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//GAME MODE PARAMETERS
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//ANIMATED INTRO CAMERAS
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//SYSTEM
///////////////////////////////////////////////////////////////////////////////
RLDATABASE_CALLBACK(RLSystem,					mRLSystems)
XT_TYPE(RLSystem,								"System")
	XT_CALLBACK((RLDataHandler<RLSystem,		CallbackRLSystem>))
	XT_BASE(RLBaseObj)

	XT_FIELD(mDamageSystem,						"DamageSystem")
	XT_FIELD(mDraftingSystem,					"DraftingSystem")
	XT_FIELD(mPickupSystem,						"PickupSystem")
	XT_FIELD(mWallRidingSystem,					"WallRidingSystem")
	XT_FIELD(mPickupForceEffects,				"PerkForceEffects")

	XT_FIELD(mReportingParameters,				"ReportingParameters")
	XT_FIELD(mAIPickupController,				"AIPickupController")
	XT_FIELD(mHandOfGodParameters,				"HandOfGodParameters")
	XT_FIELD(mHandOfGodMotormashParameters,		"HandOfGodMotormashParameters")

	XT_FIELD(mAnimatedCameraSequences,			"AnimatedCameraSequences")
	XT_FIELD(mDefaultCameraOffsets,				"DefaultCameraOffsets")
	XT_FIELD(mUICameraParameters,				"UICameraParameters")
	XT_FIELD(mUiCameraMovement,					"UICameraMovement")
	XT_FIELD(mUiLastCameraMovement,				"UILastCameraMovement")
	XT_FIELD(mUiAttachedCameraMovement,			"UIAttachedCameraMovement")
	XT_FIELD(mUiGameFeedCameraMovement,			"UIGameFeedCameraMovement")
	XT_FIELD(mUiCameraTarget,					"UiCameraTarget")
	XT_FIELD(mGameModeParameters,				"GameModeParameters")
XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//SYSTEM
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//WALL RIDING
///////////////////////////////////////////////////////////////////////////////
void WallRidingCallback(XtEventType event, XtRecord* pRecord, RLWallRidingSystem* pRLKeySystem, XtCallbackContext*)
{
	if(event == kXtEtConstruct && pRLKeySystem)
	{
		RLWallRidingKeyFrame keyFrame;

		for(int j = 0; j < pRLKeySystem->mWallRidingGraph.count; j++)
		{
			keyFrame = pRLKeySystem->mWallRidingGraph[j];
			pRLKeySystem->mWallRidingCurve.AddKeyFrame(keyFrame.mSpeed, keyFrame.mMultiplier, keyFrame.mInTangent, keyFrame.mOutTangent);
		}
	}
}

XT_TYPE(RLWallRidingKeyFrame,					"WallRidingKeyFrame")
	XT_FIELD( mSpeed,							"Speed" )
	XT_FIELD( mMultiplier,						"Multiplier" )
	XT_FIELD( mInTangent,						"InTangent" )
	XT_FIELD( mOutTangent,						"OutTangent" )
XT_END_TYPE

XT_TYPE(RLWallRidingSystem,						"WallRidingSystem")
	XT_CALLBACK(WallRidingCallback)
	XT_FIELD( mWallRidingGraph,					"WallRidingGraph")
	XT_FIELD( mClassAScale,						"ClassAScale")
	XT_FIELD( mClassBScale,						"ClassBScale")
	XT_FIELD( mClassCScale,						"ClassCScale")
	XT_FIELD( mClassDScale,						"ClassDScale")
XT_END_TYPE
///////////////////////////////////////////////////////////////////////////////
//WALL RIDING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//ADD YOUR RL TYPE TO THE DATABASE HERE
///////////////////////////////////////////////////////////////////////////////
XT_LIBRARY(RLDATABASE)
	XT_REGISTER_TYPE(RLBaseObj)					//BASE

	//HELPERS
	XT_REGISTER_TYPE(RLMinMax)
	XT_REGISTER_TYPE(RLAngularVelocity)
	XT_REGISTER_TYPE(RLRuleConditionalDisplay)
	XT_REGISTER_TYPE(RLRuleDebugInfo)

	XT_REGISTER_TYPE(RLHermiteKeyFrame)
	XT_REGISTER_TYPE(RLHermiteCurve)
	XT_REGISTER_TYPE(RLAIPickupHermiteKeyFrame)
	XT_REGISTER_TYPE(RLAIPickupHermiteCurve)
	//HELPERS

	//WALL RIDING
	XT_REGISTER_TYPE(RLWallRidingSystem)
	XT_REGISTER_TYPE(RLWallRidingKeyFrame)
	//WALL RIDING

	//PICKUPS
	XT_REGISTER_TYPE(E_PICKUP_TYPE)
	XT_REGISTER_TYPE(E_POSITION_ACTOR_PICKUP_TYPE)
	XT_REGISTER_TYPE(E_PICKUP_AI_TYPE)
	XT_REGISTER_TYPE(Game::GAME_DIFFICULTY)

	XT_REGISTER_TYPE(RLPickupPositionRow)

	XT_REGISTER_TYPE(RLBalancingGraph)
	XT_REGISTER_TYPE(RLBalancingDataBase)
	
	XT_REGISTER_TYPE(RLNitrousBalancingData)
	XT_REGISTER_TYPE(RLNitrousDoubleTapBalancingData)
	XT_REGISTER_TYPE(RLNitrousReverseBalancingData)
	
	XT_REGISTER_TYPE(RLShockBalancingData)
	XT_REGISTER_TYPE(RLShuntBalancingData)
	XT_REGISTER_TYPE(RLShuntDoubleBalancingData)
	XT_REGISTER_TYPE(RLShuntReverseBalancingData)

	XT_REGISTER_TYPE(RLBlindfireBalancingData)
	XT_REGISTER_TYPE(RLBargeBalancingData)
	
	XT_REGISTER_TYPE(RLBombBalancingData)
	XT_REGISTER_TYPE(RLThrownBombBalancingData)
	XT_REGISTER_TYPE(RLBombDoubleTapBalancingData)
	XT_REGISTER_TYPE(RLBombReverseBalancingData)
	XT_REGISTER_TYPE(RLBombModifiedBalancingData)
	
	XT_REGISTER_TYPE(RLRepairBalancingData)
	XT_REGISTER_TYPE(RLShieldBalancingData)
	XT_REGISTER_TYPE(RLSuperShieldBalancingData)

	XT_REGISTER_TYPE(RLSpecialFXBase)
	XT_REGISTER_TYPE(RLShuntSpecialFX)
	XT_REGISTER_TYPE(PickupShuntDoubleSpecialFX)
	XT_REGISTER_TYPE(RLShockSpecialFX)
	XT_REGISTER_TYPE(RLNitrousSpecialFX)
	XT_REGISTER_TYPE(RLNitrousReverseSpecialFX)
	XT_REGISTER_TYPE(RLBlindfireSpecialFX)
	XT_REGISTER_TYPE(RLStopwatchSpecialFX)
	XT_REGISTER_TYPE(RLBombSpecialFX)
	XT_REGISTER_TYPE(RLBargeSpecialFX)
	XT_REGISTER_TYPE(RLShieldSpecialFX)
	XT_REGISTER_TYPE(RLRepairSpecialFX)

	XT_REGISTER_TYPE(RLPickupLevel)
	XT_REGISTER_TYPE(RLPickupSpecialFX)
	XT_REGISTER_TYPE(RLPickupSpecialFXDuration)
	XT_REGISTER_TYPE(RLPickupSpecialFXLocation)
	XT_REGISTER_TYPE(RLPickupPositionModifier)
	XT_REGISTER_TYPE(RLPickupType)
	XT_REGISTER_TYPE(RLPickupParameters)
	XT_REGISTER_TYPE(RLPickupParametersStoredOnline)
	XT_REGISTER_TYPE(RLPickupSystem)
	XT_REGISTER_TYPE(RLPickupForceEffects)
	XT_REGISTER_TYPE(RLForceEffect)
	XT_REGISTER_TYPE(E_FORCE_EFFECT_TYPE)
	//PICKUPS

	//AI PICKUP USE
	XT_REGISTER_TYPE(RLAIPickupController)
	XT_REGISTER_TYPE(AIPickupControllerDifficulty)
	XT_REGISTER_TYPE(AIPickupControllerSetup)
	XT_REGISTER_TYPE(AIPickupGraph)
	//AI PICKUP USE

	//DAMAGE
	XT_REGISTER_TYPE(E_IMPACT_LEVEL)
	XT_REGISTER_TYPE(E_HEALTH_LEVEL)
	XT_REGISTER_TYPE(RLImpactType)
	XT_REGISTER_TYPE(RLLandingType)
	XT_REGISTER_TYPE(RLHealthLevelZone)
	XT_REGISTER_TYPE(RLDamageAllowable)
	XT_REGISTER_TYPE(RLDamageFilter)
	XT_REGISTER_TYPE(RLInertiaScale)
	XT_REGISTER_TYPE(RLVehicleHealthGroup)
	XT_REGISTER_TYPE(RLDamageParameters)
	XT_REGISTER_TYPE(RLDamageSystem)
	XT_REGISTER_TYPE(RLBodyShakeParams)
	XT_REGISTER_TYPE(RLPartShakeParams)
	//DAMAGE

	//DRAFTING
	XT_REGISTER_TYPE(RLDraftingSystem)
	//DRAFTING

	//REPORTING
	XT_REGISTER_TYPE(RLReportingParameters)
	XT_REGISTER_TYPE(RLHandOfGodParameters)
	XT_REGISTER_TYPE(RLGhostTimeAfterSpawnParameters)
	//REPORTING

	XT_REGISTER_TYPE(RLGenericCameraSequences)
	XT_REGISTER_TYPE(GenericCameraOffset)
	XT_REGISTER_TYPE(GenericCameraOffsets)

	//GAMEMODE
	XT_REGISTER_TYPE(RLGameMode_Destruction_CarDifficultyData);
	XT_REGISTER_TYPE(RLGameMode_Destruction)
	XT_REGISTER_TYPE(RLGameModeParameters)
	//GAMEMODE

	//SYSTEM *KEEP THIS AT THE BOTTOM*
	XT_REGISTER_TYPE(RLSystem)
	//SYSTEM *KEEP THIS AT THE BOTTOM*


XT_END_LIBRARY
///////////////////////////////////////////////////////////////////////////////
//ADD YOUR RL TYPE TO THE DATABASE HERE
///////////////////////////////////////////////////////////////////////////////

#ifdef RLC_DEBUG
#pragma optimize ("", on) //RLC_DEBUG
#endif
