#ifndef RLDATABASE_H
#define RLDATABASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>
#include "ui/Graphics/Localisation/Loc.h"
#include <SafeString.inl>
#include "../Data/RLEnums.h"
#include "../Data/RLPickupDefinitions.h"
#include <GameLogic/Profile/GameProfile.h>
#include <GameLogic/ActorStudioActors/AnimatedCamera.h>

namespace Game { namespace Actors { class RaceLogicCore; } }

namespace Actors
{
	class AnimatedCameraSequence;
}


struct RLDATA_ID
{
	U32											mId;
	
	inline RLDATA_ID(XtId val = 0) : mId(val)
	{
	}

	inline operator U32()
	{
		return mId;
	}

	inline void operator =(U32 val)
	{
		mId = val;
	}
};


template<typename T,typename C> void RLDataHandler(XtEventType event, XtRecord* record, T* object, XtCallbackContext*)
{
	switch(event)
	{
	case kXtEtConstruct:
		object->mID=record->id;
		C()()->Add(object);
		break;

	case kXtEtDestruct:
		C()()->Destroy((RLDATA_ID)object->mID);
		break;
	}
}


template <typename RLD> class RLData
{
public:
	const RLD operator [](RLDATA_ID id) const
	{
		for(int a=0;a<mData.Size();a++)
		{
			if(mData[a]->ID == (XtId)id)
			{
				return mData[a];
			}
		}

		return NULL;
	}
	const RLD operator [](const char *str) const
	{
		for(int a=0;a<mData.Size();a++)
		{
			if(!strcmp(mData[a]->tag,str))
			{
				return mData[a];
			}
		}

		return NULL;
	}

	const inline RLD operator [](int a) const
	{
		return mData[a];
	}

	inline void Add(RLD data)
	{
		mData.PushBack(data);
	}
	inline void Destroy(RLD data)
	{
		for(int a=0;a<mData.Size();a++)
		{
			if(mData[a] == data)
			{
				mData.EraseByIndex(a);
				return;
			}
		}
	}
	inline void Destroy(RLDATA_ID id)
	{
		for(int a=0;a<mData.Size();a++)
		{
			if(mData[a]->mID == (U32)id)
			{
				mData.EraseByIndex(a);
				return;
			}
		}
	}
	inline void DestroyAll()
	{
		for(int a=0;a<mData.Size();a++)
		{
			mData.EraseByIndex(a);
		}
	}

	inline int Size() const
	{	
		return mData.Size();
	}

	inline void Init()
	{
		mData.Clear();
	}

	void Dump()
	{
	}


	void AlphaSort()
	{
		mSortedData.Clear();
		mSortedData.Resize(mData.Size());

		for(int a=0;a<mSortedData.Size();a++)
		{
			mSortedData[a]=a;
		}

		bool sorting=true;
		while(sorting)
		{
			sorting=false;

			for(int a=0;a<mSortedData.Size()-1;a++)
			{
				WCHAR name1[128];
				WCHAR name2[128];

				Amax::C_SafeString::WideStringPrintF(name1, lengthof(name1), L"%s",UI::C_TextStringID(((RLData *)mData[mSortedData[a]])->loc_string.id).String());
				Amax::C_SafeString::WideStringPrintF(name2, lengthof(name2), L"%s",UI::C_TextStringID(((RLData *)mData[mSortedData[a+1]])->loc_string.id).String());

				int b=0;

				while(name1[b] && name2[b] && (tolower(name1[b]) == tolower(name2[b])))
				{
					b++;

					if(name2[b] == NULL)
					{
						int temp=mSortedData[a];
						mSortedData[a]=mSortedData[a+1];
						mSortedData[a+1]=temp;

						sorting=true;

						if(!wcscmp(name1,name2))
						{
							//LOGWARN( gGameLogicDataLogger,"%S == %S\n",name1,name2);
							//assert(!"There are two names the same in Sorcery");
						}
					}
				}

				if(name1[b] && name2[b] && (tolower(name1[b]) > tolower(name2[b])))
				{
					int temp=mSortedData[a];
					mSortedData[a]=mSortedData[a+1];
					mSortedData[a+1]=temp;

					sorting=true;

					if(!wcscmp(name1,name2))
					{
						//LOGWARN( gGameLogicDataLogger,"%S == %S\n",name1,name2);
						//assert(!"There are two names the same in Sorcery");
					}
				}
			}
		}
	}

	Btl::Vector<int>								mSortedData;
private:
	Btl::Vector<RLD>								mData;
};

struct RLImpactType;
///////////////////////////////////////////////////////////////////////////////
//BASE CLASS
///////////////////////////////////////////////////////////////////////////////
typedef struct RLBaseObj
{
	//XtSz8										mTag;
	XtId										mID;
	//int										mIndex;
	XtLocId										mLoc_string;

}RLBaseObj;
///////////////////////////////////////////////////////////////////////////////
//BASE CLASS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//HELPERS
///////////////////////////////////////////////////////////////////////////////
struct RLMinMax
{
	float										mMax,
												mMin;
};

struct RLAngularVelocity
{
	float										mPitch, 
												mYaw, 
												mRoll;
};

struct RLRuleDebugInfo
{
	bool										mOutputDebugInfo;
};

struct RLRuleConditionalDisplay
{
	bool										mConditionalDisplay,
												mDisplayWhenTrue;
};

struct RLHermiteKeyFrame
{
	float										mTime,
												mValue,
												mInTangent,
												mOutTangent;
};

struct RLAIPickupHermiteKeyFrame
{
	float										mDistance,
												mProbability,
												mInTangent,
												mOutTangent;
};

struct RLHermiteCurve
{
	XtArray<RLHermiteKeyFrame>					mKeyFrames;
};

struct RLAIPickupHermiteCurve
{
	XtArray<RLAIPickupHermiteKeyFrame>					mKeyFrames;
};
///////////////////////////////////////////////////////////////////////////////
//HELPERS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//PICKUPS
///////////////////////////////////////////////////////////////////////////////
struct RLBalancingGraph
{
	XtArray<RLHermiteKeyFrame>					mEffectGraph;
	Curve::C_HermiteCurve						mEffectCurve;
	float										mMaxBoost;
};

struct RLBalancingDataBase
{
public:
	virtual int	GetDamageInflicted() { return 0; }; 

	float mDelay;
	float mHealth;
};

struct RLPickupPositionRow : RLBaseObj
{
	XtFlags<XtU8, E_POSITION_ACTOR_PICKUP_TYPE>	mPickupFlagList;
};

struct RLNitrousBalancingData : RLBalancingDataBase
{
	float										mDuration;
	float										mSlowDownVal;

	float										mDurationCheckpoint;
	float										mSlowDownValCheckpoint;

	float										mClassAModifier;
	float										mClassBModifier;
	float										mClassCModifier;
	float										mClassDModifier;
};

struct RLNitrousDoubleTapBalancingData : RLBalancingDataBase
{
	float										mLowSpeedBoost;
	float										mDraftingBoost;
	float										mDuration;
	float										mRiftTime;

	float										mRiftRadius;
	float										mRiftMinForce;
	float										mRiftMaxForce;
	float										mRiftAngleRange;

	float										mAirBrakeCutOff;
};

struct RLNitrousReverseBalancingData : RLBalancingDataBase
{
	int											mDamageInflicted;
	float										mDuration;
	float										mForwardDot;
	float										mRiftRangeSq;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLShockBalancingData : RLBalancingDataBase
{
	float										mInnerRadius;
	float										mOuterRadius;

	float										mLifetime;
	int											mDamageInflicted;
	float										mHeightDiff;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLBlindfireBalancingData : RLBalancingDataBase
{
	float										mRadius;
	float										mSpeed;
	float										mFallRate;
	float										mHeightFromGround;
	int											mDamageInflicted;

	float										mWarningDistance;
	float										mWarningDot;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLShuntBalancingData : RLBalancingDataBase
{
	float										mRadius;
	int											mDamageInflicted;
	float										mFXDropTime;
	float										mShuntTurnRateScalar;
	float										mFallRate;
	float										mECMActivationDist;
	float										mECMDeactivationDist;
	float										mECMWarningDist;
	bool										mECMWarningEnabled;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLShuntReverseBalancingData : RLBalancingDataBase
{
	float										mRadius;
	int											mDamageInflicted;
	float										mFXDropTime;
	float										mSpeed;
	float										mFallRate;
	float										mHeightFromGround;
	float										mWarningDistance;
	float										mWarningDot;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLShuntDoubleBalancingData : RLBalancingDataBase
{
	float										mShotDelay;
};

struct RLBargeBalancingData : RLBalancingDataBase
{
	float										mRadius;
	float										mAngleRange;
	int											mDamageInflicted;
	float										mDuration;
	float										mMinePushForceMultiplier;
	float										mMinMinePushForce;

	float										mWarningDistance;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLBombBalancingData : RLBalancingDataBase
{
	float										mRadius;
	float										mOuterRadius;
	float										mPlaceBehindDistance;
	float										mLifespan;
	int											mDamageInflicted;
	float										mMasterMinePrimeTimer;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLThrownBombBalancingData : RLBombBalancingData
{
	float										mThrowForceY;
	float										mThrowForceMin;
	float										mThrowForceMax;
};

struct RLBombDoubleTapBalancingData : RLBalancingDataBase
{
	float										mRadius;
	float										mPlaceBehindDistance;
	float										mLifespan;
	int											mDamageInflicted;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLBombReverseBalancingData : RLBalancingDataBase
{
	float										mRadius;
	float										mFallRate;
	float										mMinSpeed;
	float										mScaledSpeed;
	float										mDeceleration;
	float										mHeightFromGround;
	float										mLifetime;
	int											mDamageInflicted;
	float										mWarningDistance;
	float										mWarningDot;

public:
	virtual int	GetDamageInflicted() { return mDamageInflicted; }; 
};

struct RLBombModifiedBalancingData : RLBalancingDataBase
{
	float										mPlaceBehindDistance;
	int											mNoMiniMinesToCreate;
	int											mTotalSpreadWidth;
};

struct RLRepairBalancingData : RLBalancingDataBase
{
	int											mAmountToRepair;
};	

struct RLShieldBalancingData : RLBalancingDataBase
{
	float										mLifetime;
};

struct RLSuperShieldBalancingData : RLBalancingDataBase
{
	int											GetDamageInflicted() { return mDamagePointsOnImpact; }; 
	float										mLifetime;

	bool 										mNitroOnStartup;
	float										mNitroTargetSpeedInMPH;
	
	bool 										mBoostDuringUse;
	float										mSteeringForwardOffset;
	float										mSteeringBoost;
	float										mAccelerationBoost;
	float										mBrakingBoost;
	float										mDownForceBoost;
		
	float										mMinimumTriggerValue;
	bool										mIncreaseMassDuringUse;
	int											mIncreasedMass;
	
	int											mDamagePointsOnImpact;
};

// ------------------- PICKUP FX ----------------------

enum RLPickupSpecialFXLocation
{
	E_PICKUP_SPECIALFX_LOCATION_ACTINGCAR,
	E_PICKUP_SPECIALFX_LOCATION_TARGETCAR,
	E_PICKUP_SPECIALFX_LOCATION_TARGETCAR_FIXEDORIENTATION,
	E_PICKUP_SPECIALFX_LOCATION_WORLDPOS,
	E_PICKUP_SPECIALFX_LOCATION_REARWHEELPOS,
	E_PICKUP_SPECIALFX_LOCATION_VEHICLETOMATRIX,
};

struct RLPickupSpecialFXDuration
{
	float mStart;
	float mEnd;
};

struct RLPickupSpecialFX
{
	RLPickupSpecialFXLocation					mSpecialFXLocation;
	XtHandle<SpecialFX::C_SpecialFX>			mLocalSpecialFX;
	XtHandle<SpecialFX::C_SpecialFX>			mExternalSpecialFX;
	XtHandle<SpecialFX::C_SpecialFX>			mInternalSpecialFX;
	XtHandle<SpecialFX::C_SpecialFX>			mGlobalSpecialFX;
	RLPickupSpecialFXDuration					mDurationIntro;
	RLPickupSpecialFXDuration					mDurationLoop;
	RLPickupSpecialFXDuration					mDurationOutro;
	float										mFrequency;
};

struct RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mChargingSpecialFX;
	XtArray<RLPickupSpecialFX>					mPickedUpSpecialFX;
	XtArray<RLPickupSpecialFX>					mRoadsweeperedSpecialFX;
	XtArray<RLPickupSpecialFX>					mFiredSpecialFX;
	XtArray<RLPickupSpecialFX>					mHitVehicleSpecialFX;
	XtArray<RLPickupSpecialFX>					mHitVehicleSpecialFXDecoy;
	XtArray<RLPickupSpecialFX>					mShieldedSpecialFX;
};

struct RLNitrousSpecialFX : RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mDrifterGiftedNitroSpecialFX;
};

struct RLNitrousReverseSpecialFX : RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mForwardSpecialFX;
};

struct RLShockSpecialFX : RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mShockDomeSpecialFX;
	XtArray<RLPickupSpecialFX>					mSafetyNetShieldedSpecialFX;
	XtArray<RLPickupSpecialFX>					mIronFistDomeSpecialFX;
};

struct RLBombSpecialFX : RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mMineSpecialFX;
	XtArray<RLPickupSpecialFX>					mExplodeSpecialFX;
	XtArray<RLPickupSpecialFX>					mSafetyNetShieldedSpecialFX;
	XtArray<RLPickupSpecialFX>					mIronFistFiredSpecialFX;
	XtArray<RLPickupSpecialFX>					mMasterMinePrimedSpecialFX;
	XtArray<RLPickupSpecialFX>					mMasterMineEquipedSpecialFX;
};

struct RLShuntSpecialFX : RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mProjectileSpecialFX;
	XtArray<RLPickupSpecialFX>					mProjectileDroppedSpecialFX;
	XtArray<RLPickupSpecialFX>					mHitEnvironmentSpecialFX;
	XtArray<RLPickupSpecialFX>					mExplosionSpecialFX;
	XtArray<RLPickupSpecialFX>					mSafetyNetShieldedSpecialFX;
	XtArray<RLPickupSpecialFX>					mIronFistFiredSpecialFX;
};

struct PickupShuntDoubleSpecialFX : RLSpecialFXBase
{
};

struct RLBlindfireSpecialFX : RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mProjectileSpecialFX;
	XtArray<RLPickupSpecialFX>					mHitEnvironmentSpecialFX;
	XtArray<RLPickupSpecialFX>					mSafetyNetShieldedSpecialFX;
	XtArray<RLPickupSpecialFX>					mIronFistFiredSpecialFX;
	XtArray<RLPickupSpecialFX>					mScramblerHitVehicleSpecialFX;
};

struct RLStopwatchSpecialFX : RLSpecialFXBase
{

};

struct RLBargeSpecialFX : RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mFiredFixedPositionSpecialFX;
	XtArray<RLPickupSpecialFX>					mSplashDamageChargingSpecialFX;
	XtArray<RLPickupSpecialFX>					mSplashDamageFiredSpecialFX;
	XtArray<RLPickupSpecialFX>					mSafetyNetShieldedSpecialFX;
	XtArray<RLPickupSpecialFX>					mIronFistFiredSpecialFX; 
	XtArray<RLPickupSpecialFX>					mLastGaspChargingSpecialFX;
	XtArray<RLPickupSpecialFX>					mLastGaspFiredSpecialFX;
	XtArray<RLPickupSpecialFX>					mSplashDamageVehicleHitSpecialFX;
};

struct RLShieldSpecialFX : RLSpecialFXBase
{
	XtArray<RLPickupSpecialFX>					mShieldDomeSpecialFX;
	XtArray<RLPickupSpecialFX>					mShieldDomeShieldedBoostersSpecialFX;
};

struct RLRepairSpecialFX : RLSpecialFXBase
{

};

struct RLPickupLevel 
{
	RLBalancingGraph							mBalancingGraph;
	RLBalancingDataBase*						mBalancingData;
	RLSpecialFXBase*							mSpecialFX;
};

// ----------------------------------------------------

struct RLPickupPositionModifier
{
	int minPos;
	int maxPos;
	float modifier;
};

struct RLPickupType : RLBaseObj
{
	enum E_PICKUP_TYPE							meType;

	XtHandle<RLPickupLevel>						mNormal;
	XtHandle<RLPickupLevel>						mDoubleTap;
	XtHandle<RLPickupLevel>						mReversed;
	XtHandle<RLPickupLevel>						mReversedDoubleTap;
	XtHandle<RLPickupLevel>						mMini;
	XtHandle<RLPickupLevel>						mModified;
	XtHandle<RLPickupLevel>						mReverseModified;

	enum E_PICKUP_AI_TYPE						meAIType;
	float										mBaseProbability;

	bool										mDoNotRandomise;
	XtArray<RLPickupPositionModifier>			mPositionModifiers;

	XtHandle<SpecialFX::C_SpecialFX>			mPickupSpecialFX;
	XtHandle<SpecialFX::C_SpecialFX>			mPickupSpecialFXLocal;
	XtHandle<SpecialFX::C_SpecialFX>			mPickupSpecialFXFaded;
	XtHandle<SpecialFX::C_SpecialFX>			mPickupSpecialFXAggressive;
	XtHandle<SpecialFX::C_SpecialFX>			mPickupSpecialFXAggressiveFaded;
	XtHandle<SpecialFX::C_SpecialFX>			mPickupSpecialFXGolden;
	XtHandle<SpecialFX::C_SpecialFX>			mPickupSpecialFXGoldenFaded;
	XtHandle<SpecialFX::C_SpecialFX>			mPickupSpecialFXDecoy;

	float										m_PickupSpecialFXBeginCycle;
	float										m_PickupSpecialFXEndCycle;

	XtHandle<SpecialFX::C_SpecialFX>			mPickupFiringDelayFX;
	XtHandle<SpecialFX::C_SpecialFX>			mPickupAIFiringDelayFX;
	XtHandle<SpecialFX::C_SpecialFX>			mTargetteeSpecialFX;
	XtHandle<SpecialFX::C_SpecialFX>			mTargettedSpecialFX;
};

struct RLPickupParameters
{
	float										mPickupRadius,					//The maximum distance from the pickup that you have to be before you get given it
												mPickupDeactivateTimer,			//The time it takes for the pickup to reappear after being picked up
												mPostPickupAwardCoolDownTimer;

	bool										mDestroyExistingPickupOnNewPickup;
	float										mMaximumXPMultiplication;

	XtHandle<SpecialFX::C_SpecialFX>			mGarageSpecialFX;
	XtHandle<SpecialFX::C_SpecialFX>			mGarageLockedCarSFX;
};

struct RLPickupParametersStoredOnline
{
	float mRecoveryTime;
	float mRecoveryTimeSpeedMod;
};

struct RLPickupSystem
{
	XtHandle<RLPickupParameters>				mParameters;
	XtHandle<RLPickupParametersStoredOnline>	mParametersStoredOnline;
	XtArray< XtHandle<RLPickupType> >			mTypes;
};


//================================================================================

enum E_FORCE_EFFECT_TYPE
{
	E_FORCE_EFFECT_TYPE_AREA,
	E_FORCE_EFFECT_TYPE_FORWARD,
	E_FORCE_EFFECT_TYPE_SIDE,
};


struct RLForceEffect
{
	E_FORCE_EFFECT_TYPE							m_ForceType;
	XtF32										m_MaxForce;
	XtF32										m_Radius;
	XtF32										m_Time;
	XtF32										m_ConeAngle;
	XtF32										m_UpwardForce;
	XtF32										m_DamageScale;
	bool										m_ElectricalEffect;
	XtRgb										m_ElectricalEffectColour;
};


struct RLPickupForceEffects
{
	RLForceEffect								m_LargeExplosion;
	RLForceEffect								m_SmallExplosion;
	RLForceEffect								m_Shock;
	RLForceEffect								m_ShuntFire;
	RLForceEffect								m_ShuntExplode;
	RLForceEffect								m_Barge;
	RLForceEffect								m_Mine;
	RLForceEffect								m_MineProjectile;
	RLForceEffect								m_Blindfire;
};









///////////////////////////////////////////////////////////////////////////////
//PICKUPS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//AI PICKUP USE
///////////////////////////////////////////////////////////////////////////////
struct AIPickupGraph
{
	XtArray<RLAIPickupHermiteKeyFrame> mAIPickupAccuracyHermiteCurve;
};

struct AIPickupControllerSetup
{
	float	mSharedEvalTime, mSharedShieldEvalTime;
	float	mSharedFiredPerkDisableTime, mSharedBeenHitWithPerkDisableTime, mSharedPlayerHitDisableTime,
			mSharedPickupSwitchTime, mSharedOutOfRangePickupDropTimeMin, mSharedOutOfRangePickupDropTimeMax, mSharedHumanAvoidRatio, mTempFireRatio;

	XtArray<AIPickupGraph>				mAIPickupAccuracyGraphList;
	XtArray<RLAIPickupHermiteKeyFrame>	mAIShieldedHermiteCurve;
};

struct AIPickupControllerDifficulty
{
	XtHandle<AIPickupControllerSetup>	mController;
	Game::GAME_DIFFICULTY						mDifficulty;
};

struct RLAIPickupController : AmaxGameDataObj
{
	XtArray<AIPickupControllerDifficulty> mControllers;
};
///////////////////////////////////////////////////////////////////////////////
//AI PICKUP USE
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//DAMAGE
///////////////////////////////////////////////////////////////////////////////

struct RLImpactType
{
	E_IMPACT_LEVEL								meImpactLevel;
	RLMinMax									mFrontImpulseBounds;
	RLMinMax									mSideImpulseBounds;

	XtHandle<SpecialFX::C_SpecialFX>			mRollScreenEffect;
	XtHandle<SpecialFX::C_SpecialFX>			mFrontScreenEffect;
	XtHandle<SpecialFX::C_SpecialFX>			mSideScreenEffect;
};

struct RLLandingType
{
	E_IMPACT_LEVEL								meImpactLevel;
	RLMinMax									mImpulseBounds;

	XtHandle<SpecialFX::C_SpecialFX>			mScreenEffect;
};

struct RLDamageAllowable
{
	bool					mAllowable;
	E_IMPACT_LEVEL			mImpactRequired;
};

struct RLBodyShakeParams
{
	float	mMinSpeed;
	float	mSpeedRange;
	float	mFrequency;
	XtVec3	mShakeRange;
};

struct RLPartShakeParams
{
	float	mMinSpeed;
	float	mSpeedRange;
	float	mTorque;
};

struct RLDamageFilter
{
	RLDamageAllowable		mScratchDamage;
	RLDamageAllowable		mDeformParts;
	RLDamageAllowable		mDislodgeParts;
	RLDamageAllowable		mDetachParts;
	RLDamageAllowable		mBurstTyres;
	RLDamageAllowable		mDetachWheels;
	RLDamageAllowable		mLightBreak;
	RLDamageAllowable		mLightKnockOut;
	RLDamageAllowable		mRadiatorSmoke;
	RLDamageAllowable		mEngineSmokeLight;
	RLDamageAllowable		mEngineSmokeHeavy;
	RLDamageAllowable		mEngineFire;
};

struct RLHealthLevelZone
{
	E_HEALTH_LEVEL								mHealthZone;
	int											mHealthMin,
												mHealthMax;

	XtHandle<SpecialFX::C_SpecialFX>			mScreenEffect;

	RLDamageFilter								mDamageFilter;

	RLBodyShakeParams							mBodyShake;
	RLPartShakeParams							mPartShake;
};

struct RLInertiaScale
{
	float										mHumanToAiInertiaScaleHuman, 
												mHumanToAiInertiaScaleAi, 
												mHumanToHumanInertiaScale,
												mAiToAiInertiaScale,
												mCarToObjectInertiaScaleCar,
												mCarToObjectInertiaScaleObject,												
												mHumanToAiMassScaleHuman,
												mHumanToAiMassScaleAi,
												mCarToObjectMassScaleCar,
												mCarToObjectMassScaleObject,

												mScaledImpactMaxSpeed,
												mScaledImpactMinSpeed,
												mMinCarToObjectMassValue;
};

struct RLVehicleHealthGroup
{
	AmaxCarHealthGroup							mHealthGroup;
	float										mBaseHealth;
};

struct RLDamageParameters
{
	float										mMaxWorldDamageImpulse,
												mMaxWorldEffectImpulse,
												mMaxVehicleDamageImpulse,
												mMaxVehicleEffectImpulse,
												mMaxObjectDamageImpulse,
												mMaxObjectEffectImpulse,
												mMaxStaticImpulse,
												mMaxDynamicImpulse,
												mResetTimerAfterWrecked,
												
												mCooldownPerk,
												mCooldownCollision;
												
	int											mLightImpactDamage,
												mMediumImpactDamage,
												mHeavyImpactDamage,

												mHealthIncrement;

	bool										mCapGameplayImpulse;

	RLInertiaScale								mIntraVehicularCollisionInertiaScale;

	XtArray<RLVehicleHealthGroup>				mVehicleHealthGroups;
};

struct RLDamageSystem
{
	XtHandle<RLDamageParameters>				mParameters;
	XtArray< XtHandle<RLImpactType> >			mImpactTypes;
	XtArray< XtHandle<RLLandingType> >			mLandingTypes;
	XtArray< XtHandle<RLHealthLevelZone> >		mHealthZones;
};
///////////////////////////////////////////////////////////////////////////////
//DAMAGE
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//DRAFTING
///////////////////////////////////////////////////////////////////////////////
struct RLDraftingSystem
{
	XtHandle<SpecialFX::C_SpecialFX>			mDraftingSFX;
	XtHandle<SpecialFX::C_SpecialFX>			mDraftingFadeInSFX;
	XtHandle<SpecialFX::C_SpecialFX>			mDraftingFadeOutSFX;

	float										mDraftingAnimTime;
	float										mDraftingFadeInTime;
	float										mDraftingFadeOutTime;
};
///////////////////////////////////////////////////////////////////////////////
//DRAFTING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//REPORTING
///////////////////////////////////////////////////////////////////////////////
struct RLReportingParameters
{
	float										mProximityReportingDistanceSquared;
	float										mEndOfRacePercentage;

	enum E_IMPACT_LEVEL							meMajorCrashImpactLevel;

	int											mPileUpMinimumVehicles,
												mPileUpMinimumDistance;
	enum E_IMPACT_LEVEL							mePileUpMinimumImpactLevel;
	float										mPileUpTimer;

};

///////////////////////////////////////////////////////////////////////////////
//REPORTING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//HAND OF GOD
///////////////////////////////////////////////////////////////////////////////
struct RLGhostTimeAfterSpawnParameters
{
	float										mGhostTimeAfterHoG;
	float										mGhostTimeAfterWrecked;
};

struct RLHandOfGodParameters
{
	float										mResetTimer,
												mCollisionTimer,
												mHitByPerkTimer,
												mFlippedTimer,
												mOffSplineTimer,
												mTumbleTimer,
												mStuckTimer,
												mBackwardsTimer,
												mSlappedTimer,
												mRidiculousSlappedTimer,

												mWaitTime,

												mShieldTime,
												mAIResetTimeAddition,
												mForwardRecoveryTimeAddition,

												mDriftDistance,
												mDriftDot,

												mMinimumSpeedInMPH,
												mMinForwardSpeed,
												mMinDotUp,
												mFinishedSpinX,
												mFinishedSpinY,
												mFinishedSpinZ,
												mAIControlTime,
												mForwardDot,
												mForwardResetDot,
												mNoteworthyCollisionImpulse;						

	RLGhostTimeAfterSpawnParameters				mGhostParametersStandard;

	RLAngularVelocity							mMaximumAngularVelocity,
												mMaximumCollisionAngularVelocity,
												mMaximumPerkAngularVelocity;
};

///////////////////////////////////////////////////////////////////////////////
//HAND OF GOD
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//ANIMATED INTRO CAMERAS
///////////////////////////////////////////////////////////////////////////////
struct RLGenericCameraSequences
{
	XtArray< XtHandle<Actors::AnimatedCameraSequence> > mSequenceArray;
};

struct GenericCameraOffset
{
	XtVec3											mVehicleOffset;
	Actors::E_ANIMCAMERASEQ_TYPE					mSequence;
};

struct GenericCameraOffsets
{
//	XtArray< XtHandle<GenericCameraOffset> >		mOffsetList;
	XtArray< GenericCameraOffset >					mOffsetList;
};


///////////////////////////////////////////////////////////////////////////////
//ANIMATED INTRO CAMERAS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//GAME MODE PARAMETERS
///////////////////////////////////////////////////////////////////////////////
struct  RLGameMode_Destruction_CarDifficultyData
{
	int mHitPoints;
	int mWreckedPoints;
	XtHandle<SpecialFX::C_SpecialFX>	mDistanceSpecialFX;
};

struct RLGameMode_Destruction
{
	int mMaxDistanceEffects;
	F32 mMinRangeForDistanceEffect;
	F32 mMaxRangeForDistanceEffect;
	int	mMaxAIVehiclesActive;
	int mMaxWavesActive;
	F32	mForwardsCutOffDistance;
	F32 mBombLifetime;
	bool mFavourPlayersOwnRoute;
	bool mSpawnExtraAIToFillAllRoutes;

	RLGameMode_Destruction_CarDifficultyData mEasyCar;
	RLGameMode_Destruction_CarDifficultyData mMediumCar;
	RLGameMode_Destruction_CarDifficultyData mHardCar;
	RLGameMode_Destruction_CarDifficultyData mVHardCar;

	XtHandle<SpecialFX::C_SpecialFX>	mSpawnSpecialFX;
	XtHandle<SpecialFX::C_SpecialFX>	mRemovedSpecialFX;
};

struct RLGameModeParameters
{
	RLGameMode_Destruction					mDestruction;
};
///////////////////////////////////////////////////////////////////////////////
//GAME MODE PARAMETERS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//WALL RIDING 
///////////////////////////////////////////////////////////////////////////////
struct RLWallRidingKeyFrame
{
	float										mSpeed,
												mMultiplier,
												mInTangent,
												mOutTangent;
};

struct RLWallRidingSystem
{
	Curve::C_HermiteCurve						mWallRidingCurve;
	XtArray<RLWallRidingKeyFrame>				mWallRidingGraph;

	float										mClassAScale;
	float										mClassBScale;
	float										mClassCScale;
	float										mClassDScale;
};
///////////////////////////////////////////////////////////////////////////////
//WALL RIDING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//SYSTEM
///////////////////////////////////////////////////////////////////////////////
struct RLSystem : RLBaseObj
{
	XtHandle<RLDamageSystem>					mDamageSystem;
	XtHandle<RLDraftingSystem>					mDraftingSystem;
	XtHandle<RLPickupSystem>					mPickupSystem;
	XtHandle<RLWallRidingSystem>				mWallRidingSystem;
	XtHandle<RLPickupForceEffects>				mPickupForceEffects;

	XtHandle<RLReportingParameters>				mReportingParameters;
	XtHandle<RLAIPickupController>				mAIPickupController;
	XtHandle<RLHandOfGodParameters>				mHandOfGodParameters;
	XtHandle<RLHandOfGodParameters>				mHandOfGodMotormashParameters;

	XtHandle<RLGenericCameraSequences>			mAnimatedCameraSequences;
	XtHandle<GenericCameraOffsets>				mDefaultCameraOffsets;
	XtHandle<Game::DyCamPropertyDef>			mUICameraParameters;
	XtHandle<Game::DyCamMovementDef>			mUiCameraMovement;
	XtHandle<Game::DyCamMovementDef>			mUiLastCameraMovement;
	XtHandle<Game::DyCamMovementDef>			mUiAttachedCameraMovement;
	XtHandle<Game::DyCamMovementDef>			mUiGameFeedCameraMovement;
	XtHandle<Game::DyCamTargetDef>				mUiCameraTarget;
	XtHandle<RLGameModeParameters>				mGameModeParameters;
};
///////////////////////////////////////////////////////////////////////////////
//SYSTEM
///////////////////////////////////////////////////////////////////////////////

class RLDatabase
{
public:
	static RLDatabase&							Instance() {return s_instance;}

//Variables
public:

	RLData<RLSystem*>							mRLSystems;

	const RLPickupType* GetPickupType(const enum E_PICKUP_TYPE ePickupType) const
	{
		for(int i = 0; i < mRLSystems[0]->mPickupSystem->mTypes.count; i++)
		{
			if(mRLSystems[0]->mPickupSystem->mTypes[i]->meType == ePickupType)
			{
				return mRLSystems[0]->mPickupSystem->mTypes[i];
			}
		}

		return (const RLPickupType*)0;
	}

	const RLPickupParametersStoredOnline* GetPickupParametersStoredOnline()
	{
		if (mRLSystems.Size()>0&&mRLSystems[0]!=0)
		{
			const XtHandle<RLPickupSystem>& pickupSystem=mRLSystems[0]->mPickupSystem;
			if (pickupSystem)
			{
				const XtHandle<RLPickupParametersStoredOnline>& parametersStoredOnline=pickupSystem->mParametersStoredOnline;
				return parametersStoredOnline;
			}
		}
		return 0;
	}

private:
	static RLDatabase							s_instance;
};

#endif //RLDATABASE_H
