#ifndef VEHICLE_LOGIC_MESSAGES_H
#define VEHICLE_LOGIC_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/uid/FourCC.h>
#include "misc_shared/Messaging/MessageValue.h"
#include "StringHash.h"

#include <gamelogic\racelogiccore\data\RLEnums.h>
#include <GameLogic\GameData\MultiplayerData.h>
#include <actor_system/ActorDeclare.h>
#include <ui/Util/Widestring.h>

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>

namespace Game
{
	class C_PickupBase;
}

enum E_VehicleLogicMsg
{
	E_VehicleLogicMsg_HiBits = FOURCC_HI20('VLCM'),
	
	//E_VehicleLogicMsg_PickUpInfo,
	E_VehicleLogicMsg_Skillshot,
	E_VehicleLogicMsg_Wrecked,
	E_VehicleLogicMsg_FansAwarded,
	E_VehicleLogicMsg_FiredPerk,
	E_VehicleLogicMsg_HitByPerk,
	E_VehicleLogicMsg_TargettedHitByPerk,
	E_VehicleLogicMsg_ShieldedPerk,
	E_VehicleLogicMsg_MissedPerk,
	E_VehicleLogicMsg_DropPickupOnTrack,
	E_VehicleLogicMsg_InstaPickupFire,
	E_VehicleLogicMsg_DecoyPickupDropOnTrack,

	E_VehicleLogicMsg_HitByPerkSpecialEventRevenge,
	E_VehicleLogicMsg_HitByPerkSpecialEventCollided,
	E_VehicleLogicMsg_HitByPerkSpecialEventFlipped,
	E_VehicleLogicMsg_HitByPerkSpecialEventFatallyDamaged,

	E_VehicleLogicMsg_ResetWarningMsg,

	E_VehicleLogicMsg_ForceDamage,
	E_VehicleLogicMsg_SetHealth,
	E_VehicleLogicMsg_Repair,
	E_VehicleLogicMsg_ShieldTime,
	E_VehicleLogicMsg_TargetVehicle,
	E_VehicleLogicMsg_Ghosted,
	E_VehicleLogicMsg_Deghosted,
	E_VehicleLogicMsg_PerkUpdateMatrix,

	E_VehicleLogicMsg_DroppedPerk,
	E_VehicleLogicMsg_PerkAudioTrigger,

	E_VehicleLogicMsg_AddOnTrackScore,
	E_VehicleLogicMsg_AggressiveScore,
	E_VehicleLogicMsg_Domination,
	E_VehicleLogicMsg_DominationScore,
	E_VehicleLogicMsg_PerkEnded,
	E_VehicleLogicMsg_Recovered,

	E_VehicleLogicMsg_StartDelayToPerk,

	//	Multiplayer AI-switching
	E_VehicleLogicMsg_ToggleAiControl,
	E_VehicleLogicMsg_TogglePlayerControl,
	E_VehicleLogicMsg_PerkUpdate,

	E_VehicleLogicMsg_EnableRumble,
	E_VehicleLogicMsg_DisableRumble,

	E_VehicleLogicMsg_EnableDisablePickupSystem,

	E_VehicleLogicMsg_HOGReset,

	E_VehicleLogicMsg_Scored,
	E_VehicleLogicMsg_Fan_Demand_Passed,
	E_VehicleLogicMsg_Challenge_Completed,

	E_VehicleLogicMsg_SurvivedElimination,

	E_VehicleLogicMsg_ChainCompleted,

	E_VehicleLogicMsg_VehicleMurdered,
	E_VehicleLogicMsg_VehicleSuicided,

	E_VehicleLogicMsg_DestroyedPickupWithPickup,

	E_VehicleLogicMsg_Landed,

	E_VehicleLogicMsg_HitWithAllDomes,
	E_VehicleLogicMsg_AvoidedAllDomes,
	E_VehicleLogicMsg_CrossedCheckpoint,

	E_VehicleLogicMsg_EvadedPowerUp,
	E_VehicleLogicMsg_PowerUpDeleted,
	E_VehicleLogicMsg_SideSwipe,

	E_VehicleLogicMsg_PowerUpDestroyedByPowerUp,
	E_VehicleLogicMsg_PowerUpFXDurations,

	E_VehicleLogicMsg_ModActivated,

	E_VehicleLogicMsg_End, 
	E_VehicleLogicMsg_Begin = E_VehicleLogicMsg_HiBits + 1, 
	E_VehicleLogicMsg_Count = E_VehicleLogicMsg_End - E_VehicleLogicMsg_Begin
};

class C_VehicleLogicMsg_ModActivated: public Messaging::C_MessageDerived<C_VehicleLogicMsg_ModActivated, E_VehicleLogicMsg_ModActivated>
{
public:
	C_VehicleLogicMsg_ModActivated(U8 vehicle_index, UI::E_MultiplayerVehicleAbilities modActivated)
	{
		mVehicleIndex = vehicle_index;
		mModActivated = modActivated;
	}

	U8												mVehicleIndex;
	UI::E_MultiplayerVehicleAbilities				mModActivated;
};

class C_VehicleLogicMsg_HoGRecoveryMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HoGRecoveryMsg, E_VehicleLogicMsg_Recovered>
{
public:
	C_VehicleLogicMsg_HoGRecoveryMsg(U8 vehicle_index)
	{
		mVehicleIndex=vehicle_index;
	}

	U8												mVehicleIndex;
};

class C_VehicleLogicMsg_HoGResetMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HoGRecoveryMsg, E_VehicleLogicMsg_HOGReset>
{
public:
	C_VehicleLogicMsg_HoGResetMsg(U8 vehicle_index)
	{
		mVehicleIndex=vehicle_index;
	}

	U8												mVehicleIndex;
};

class C_VehicleLogicMsg_ResetWarningMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_ResetWarningMsg, E_VehicleLogicMsg_ResetWarningMsg>
{
public:
	C_VehicleLogicMsg_ResetWarningMsg(U8 vehicle_index)
	{
		mVehicleIndex=vehicle_index;
	}

	U8												mVehicleIndex;
};

class C_VehicleLogicForceDamageMsg: public Messaging::C_MessageDerived<C_VehicleLogicForceDamageMsg, E_VehicleLogicMsg_ForceDamage>
{
public:
	C_VehicleLogicForceDamageMsg(U8 vehicle_index, int damage)
	{
		mVehicleIndex=vehicle_index;
		mDamage=damage;
	}

	U8												mVehicleIndex;
	int												mDamage;
};

class C_VehicleLogicSetHealthMsg: public Messaging::C_MessageDerived<C_VehicleLogicForceDamageMsg, E_VehicleLogicMsg_SetHealth>
{
public:
	C_VehicleLogicSetHealthMsg(U8 vehicle_index, S32 health)
	{
		mVehicleIndex = vehicle_index;
		mHealthPoints = health;
	}

	U8												mVehicleIndex;
	S32												mHealthPoints;
};

class C_VehicleLogicRepairMsg: public Messaging::C_MessageDerived<C_VehicleLogicRepairMsg, E_VehicleLogicMsg_Repair>
{
public:
	C_VehicleLogicRepairMsg(U8 vehicle_index, int healthPoints)
	{
		mVehicleIndex=vehicle_index;
		mHealAmount=healthPoints;
	}

	U8												mVehicleIndex;
	int												mHealAmount;
};

class C_VehicleLogicShieldTimeMsg: public Messaging::C_MessageDerived<C_VehicleLogicShieldTimeMsg, E_VehicleLogicMsg_ShieldTime>
{
public:
	C_VehicleLogicShieldTimeMsg(U8 vehicleIndex, float shieldTime, bool useFanPower = true)
	{
		m_VehicleIndex = vehicleIndex;
		m_ShieldTime = shieldTime;
		m_UseFanPower = useFanPower;
	}

	U8												m_VehicleIndex;
	float											m_ShieldTime;
	bool											m_UseFanPower;
};

class C_VehicleLogicTargetVehicleMsg: public Messaging::C_MessageDerived<C_VehicleLogicTargetVehicleMsg, E_VehicleLogicMsg_TargetVehicle>
{
public:
	C_VehicleLogicTargetVehicleMsg(U8 vehicleIndex, S8 targetIndex)
	{
		m_VehicleIndex = vehicleIndex;
		m_TargetIndex = targetIndex;
	}

	U8												m_VehicleIndex;
	S8												m_TargetIndex;
};

class C_VehicleLogicMsg_GhostedMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_GhostedMsg, E_VehicleLogicMsg_Ghosted>
{
public:
	C_VehicleLogicMsg_GhostedMsg(U8 vehicleIndex, F32 ghostTime, U8 viewport): m_VehicleIndex(vehicleIndex), m_GhostTime(ghostTime), m_Viewport(viewport) { }

	U8												m_VehicleIndex;
	F32												m_GhostTime;
	U8												m_Viewport;
};

class C_VehicleLogicMsg_DeghostedMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_GhostedMsg, E_VehicleLogicMsg_Deghosted>
{
public:
	C_VehicleLogicMsg_DeghostedMsg(U8 vehicleIndex, U8 viewport): m_VehicleIndex(vehicleIndex), m_Viewport(viewport) { }

	U8												m_VehicleIndex;
	U8												m_Viewport;
};


//class C_VehicleLogicPickUpInfoMsg: public Messaging::C_MessageDerived<C_VehicleLogicPickUpInfoMsg, E_VehicleLogicMsg_PickUpInfo>
//{
//public:
//	C_VehicleLogicPickUpInfoMsg() : mVehicleIndex( -1 ), mCurrentPickupType( E_PICKUP_TYPE_NONE )  {  }
//
//	C_VehicleLogicPickUpInfoMsg(S8 mvehIdx,
//								E_PICKUP_TYPE type,
//								int power_level,
//								Btl::Vector<float> power,
//								Btl::Vector<E_PICKUP_TYPE> types)
//	{
//		mCurrentPickupType=type;
//		mVehicleIndex = mvehIdx;
//
//		mPowerLevel=power_level;
//		mPower=power;
//
//		mCurrentTypes=types;
//	}
//	
//	S8												mVehicleIndex;
//	E_PICKUP_TYPE									mCurrentPickupType;
//	
//	unsigned char									mPowerLevel;
//	Btl::Vector<float>								mPower;
//	
//	Btl::Vector<E_PICKUP_TYPE>						mCurrentTypes;
//};

class C_VehicleLogicMsg_SkillshotMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_SkillshotMsg, E_VehicleLogicMsg_Skillshot>
{
public:

	C_VehicleLogicMsg_SkillshotMsg(UI::SkillshotType skillshotType, E_PICKUP_TYPE pickupType) : mPickupType(pickupType), mSkillshotType(skillshotType) {}

	E_PICKUP_TYPE		mPickupType;
	UI::SkillshotType	mSkillshotType;
	
};

class C_VehicleLogicMsg_WreckedMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_WreckedMsg, E_VehicleLogicMsg_Wrecked>
{
public:

	C_VehicleLogicMsg_WreckedMsg(int wreckerIdx, int wreckeeIdx, E_PICKUP_TYPE pickup) : mWreckerIdx(wreckerIdx), mWreckeeIdx(wreckeeIdx), mPickupType(pickup) {}

	int				mWreckerIdx;
	int				mWreckeeIdx;
	E_PICKUP_TYPE	mPickupType;
};

class C_VehicleLogicMsg_FansAwarded : public Messaging::C_MessageDerived<C_VehicleLogicMsg_FansAwarded, E_VehicleLogicMsg_FansAwarded>
{
public:

	C_VehicleLogicMsg_FansAwarded(U8 vehIdx, int fans) : vehicleIndex(vehIdx), numFans(fans) {};

	U8 vehicleIndex;
	int numFans;
};

class C_VehicleLogicMsg_FiredPerkMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_FiredPerkMsg, E_VehicleLogicMsg_FiredPerk>
{
public:

	C_VehicleLogicMsg_FiredPerkMsg(E_PICKUP_TYPE type,char level,U8 vehicle_index,U32 unique_id,Maths::HMat43 mat, float time, bool flipSFX, E_PICKUP_SUB_TYPE pickupSubType= E_PICKUP_SUB_TYPE_NORMAL, bool samePerk = false, float speed = 1, bool noVehicleFiringIndex = false)
	{
		mPickupType=type;
		mLevel=level;
		mVehicleIndex=vehicle_index;
		mUniqueId=unique_id;
		mMatrix=mat;
		mLiveTime = time;
		mFlipSFX = flipSFX;
		mSamePerkComponent = samePerk;
		mSpeed = speed;
		mPickupSubType = pickupSubType;
		mNoVehicleFiringIndex = noVehicleFiringIndex;
	}

	C_VehicleLogicMsg_FiredPerkMsg(E_PICKUP_TYPE type,char level,Actor::C_ActorHandle handle, U8 vehicle_index,U32 unique_id,Maths::HMat43 mat, float time, 
		bool flipSFX, E_PICKUP_SUB_TYPE pickupSubType= E_PICKUP_SUB_TYPE_NORMAL, bool samePerk = false, float speed = 1,bool noVehicleFiringIndex = false)
	{
		mPickupType=type;
		mLevel=level;
		mVehicleIndex=vehicle_index;
		mUniqueId=unique_id;
		mMatrix=mat;
		mActorHandle=handle;
		mLiveTime = time;
		mFlipSFX = flipSFX;
		mSamePerkComponent = samePerk;
		mSpeed = speed;
		mPickupSubType = pickupSubType; 
		mNoVehicleFiringIndex = noVehicleFiringIndex;
	}
	
	U8												mVehicleIndex;
	Actor::C_ActorHandle							mActorHandle;
	E_PICKUP_TYPE									mPickupType;
	E_PICKUP_SUB_TYPE								mPickupSubType;
	char											mLevel;
	U32												mUniqueId;
	Maths::HMat43									mMatrix;
	float											mLiveTime;
	bool											mFlipSFX;
	bool											mSamePerkComponent;
	float											mSpeed;
	bool											mNoVehicleFiringIndex;
};

class C_VehicleLogicMsg_HitByPerkMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HitByPerkMsg, E_VehicleLogicMsg_HitByPerk>
{
public:
	//[ciano] added target index for audio splash damage. has a default of -1 if parameter not passed
	C_VehicleLogicMsg_HitByPerkMsg(E_PICKUP_TYPE type,char level,U8 vehicle_index, U8 firer_index,U32 ID, S8 dropped_by, U8 TargetedVehicle, U32 groupId, 
		E_PICKUP_SUB_TYPE subType= E_PICKUP_SUB_TYPE_NORMAL, E_PICKUP_HIT_TYPE hitType = E_PICKUP_HIT_TYPE_MAJOR )
	{
		mPickupType=type;
		mLevel=level;
		mVehicleIndex=vehicle_index;
		mFiringVehicleIndex=firer_index;
		mUniqueID=ID;
		mTargetedVehicleIndex = TargetedVehicle; //set to -1, explicitly set it for pickupdamage effects
		mGroupID=groupId;
		mDroppedByIndex = dropped_by;
		mPickupSubType = subType;
		mPickupHitType = hitType;
	}
	
	U8												mFiringVehicleIndex;
	U8												mVehicleIndex;
	U8												mTargetedVehicleIndex;
	E_PICKUP_TYPE									mPickupType;
	char											mLevel;
	U32												mUniqueID;
	U32												mGroupID;
	S8												mDroppedByIndex;
	E_PICKUP_SUB_TYPE								mPickupSubType;
	E_PICKUP_HIT_TYPE								mPickupHitType;
};

class C_VehicleLogicMsg_TargettedHitByPerkMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_TargettedHitByPerkMsg, E_VehicleLogicMsg_TargettedHitByPerk>
{
public:

	C_VehicleLogicMsg_TargettedHitByPerkMsg(E_PICKUP_TYPE type,char level,U8 vehicle_index, U8 firer_index,U32 ID)
	{
		mPickupType=type;
		mLevel=level;
		mVehicleIndex=vehicle_index;
		mFiringVehicleIndex=firer_index;
		mUniqueID=ID;
	}

	U8												mFiringVehicleIndex;
	U8												mVehicleIndex;
	E_PICKUP_TYPE									mPickupType;
	char											mLevel;
	U32												mUniqueID;
};

class C_VehicleLogicMsg_ShieldedPerkMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_ShieldedPerkMsg, E_VehicleLogicMsg_ShieldedPerk>
{
public:

	C_VehicleLogicMsg_ShieldedPerkMsg(E_PICKUP_TYPE type, U8 firerIdx, U8 defenderIdx, char level, U32 unique_id, float time) 
		: mPickupType(type), mFiringVehicleIndex(firerIdx), mDefendingVehicleIndex(defenderIdx), mUniqueID(unique_id), mTime(time), mLevel(level)
	{
	}

	U8												mFiringVehicleIndex;
	U8												mDefendingVehicleIndex;
	U32												mUniqueID;
	E_PICKUP_TYPE									mPickupType;
	char											mLevel;
	float											mTime;
};

class C_VehicleLogicMsg_MissedPerkMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_MissedPerkMsg, E_VehicleLogicMsg_MissedPerk>
{
public:

	C_VehicleLogicMsg_MissedPerkMsg(E_PICKUP_TYPE type, U8 firerIdx, U8 defenderIdx, Maths::HMat43 mat, U32 unique_id) 
		: mFiringVehicleIndex(firerIdx), mDefendingVehicleIndex(defenderIdx), mUniqueID(unique_id), mMatrix(mat), mPickupType(type)
	{
	}

	U8												mFiringVehicleIndex;
	U8												mDefendingVehicleIndex;
	U32												mUniqueID;
	Maths::HMat43									mMatrix;
	E_PICKUP_TYPE									mPickupType;
};

class C_VehicleLogicDropPickupOnTrackMsg: public Messaging::C_MessageDerived<C_VehicleLogicDropPickupOnTrackMsg, E_VehicleLogicMsg_DropPickupOnTrack>
{
public:
	C_VehicleLogicDropPickupOnTrackMsg(U8 dropperIndex): m_VehicleIndex(dropperIndex)
	{
	}

	U8												m_VehicleIndex;
};

class C_VehicleLogicDecoyPickupDropOnTrackMsg : public Messaging::C_MessageDerived<C_VehicleLogicDecoyPickupDropOnTrackMsg, E_VehicleLogicMsg_DecoyPickupDropOnTrack>
{
public:
	C_VehicleLogicDecoyPickupDropOnTrackMsg(U8 decoyDropperVehicleIndex) : mDecoyDropperVehicleIndex(decoyDropperVehicleIndex)
	{
	}

	U8												mDecoyDropperVehicleIndex;
};

class C_VehicleLogicInstaPickupFireMsg: public Messaging::C_MessageDerived<C_VehicleLogicInstaPickupFireMsg, E_VehicleLogicMsg_InstaPickupFire>
{
public:
	C_VehicleLogicInstaPickupFireMsg(E_PICKUP_TYPE type, U32 uid): m_Type(type), m_PickupUid(uid)
	{
	}

	E_PICKUP_TYPE									m_Type;
	U32												m_PickupUid;
};

class C_VehicleLogicDominationMsg: public Messaging::C_MessageDerived<C_VehicleLogicDominationMsg, E_VehicleLogicMsg_Domination>
{
public:
	C_VehicleLogicDominationMsg(U8 dominatingIndex, U32 fans) : m_DominatingVehicleIndex(dominatingIndex), m_DominationFans(fans)
	{
	}

	U8												m_DominatingVehicleIndex;
	U32												m_DominationFans;
};

// ------------------------------------------------------------------------------------------------------------------------
// SPECIAL EVENTS
// ------------------------------------------------------------------------------------------------------------------------

class C_VehicleLogicMsg_HitByPerkSpecialRevengeMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HitByPerkSpecialRevengeMsg, E_VehicleLogicMsg_HitByPerkSpecialEventRevenge>
{
public:

	C_VehicleLogicMsg_HitByPerkSpecialRevengeMsg( U8 vehIdx, U8 recentHitIdx ) : 
	  mFiringVehicleIndex(vehIdx), mHitVehicleIndex(recentHitIdx) {};

	  U8												mFiringVehicleIndex;
	  U8												mHitVehicleIndex;
};

class C_VehicleLogicMsg_HitByPerkSpecialEventShieldedMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HitByPerkSpecialEventShieldedMsg, E_VehicleLogicMsg_HitByPerkSpecialEventCollided>
{
public:

	C_VehicleLogicMsg_HitByPerkSpecialEventShieldedMsg( U8 vehIdx, U8 attackerIdx ) : 
	  mFiringVehicleIndex(attackerIdx), mHitVehicleIndex(vehIdx) {};

	  U8												mFiringVehicleIndex;
	  U8												mHitVehicleIndex;
};

class C_VehicleLogicMsg_HitByPerkSpecialEventCollidedMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HitByPerkSpecialEventCollidedMsg, E_VehicleLogicMsg_HitByPerkSpecialEventCollided>
{
public:

	C_VehicleLogicMsg_HitByPerkSpecialEventCollidedMsg( U8 vehIdx, U8 recentHitIdx ) : 
		mFiringVehicleIndex(recentHitIdx), mHitVehicleIndex(vehIdx), mCollidedVehicleIndex(-1) {};

	U8												mFiringVehicleIndex;
	U8												mHitVehicleIndex;
	U8												mCollidedVehicleIndex; // if -1, then we hit the environment.
};

class C_VehicleLogicMsg_HitByPerkSpecialEventFatallyDamagedMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HitByPerkSpecialEventFatallyDamagedMsg, E_VehicleLogicMsg_HitByPerkSpecialEventFatallyDamaged>
{
public:

	C_VehicleLogicMsg_HitByPerkSpecialEventFatallyDamagedMsg( U8 vehIdx, U8 recentHitIdx ) : 
	  mFiringVehicleIndex(recentHitIdx), mHitVehicleIndex(vehIdx) {};

	U8												mFiringVehicleIndex;
	U8												mHitVehicleIndex;
};

class C_VehicleLogicMsg_HitByPerkSpecialEventFlippedMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HitByPerkSpecialEventFlippedMsg, E_VehicleLogicMsg_HitByPerkSpecialEventFlipped>
{
public:

	C_VehicleLogicMsg_HitByPerkSpecialEventFlippedMsg( U8 vehIdx, U8 recentHitIdx ) : 
	  mFiringVehicleIndex(recentHitIdx), mHitVehicleIndex(vehIdx) {};

	U8												mFiringVehicleIndex;
	U8												mHitVehicleIndex;
};

// ------------------------------------------------------------------------------------------------------------------------

class C_VehicleLogicMsg_PerkUpdateMatrixMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_PerkUpdateMatrixMsg, E_VehicleLogicMsg_PerkUpdateMatrix>
{
public:

	C_VehicleLogicMsg_PerkUpdateMatrixMsg(U32 id, Maths::HMat43 matrix, int vehicle_index, Maths::HVec3	velocity) : mMatrix(matrix), mUniqueID(id), mVehicleIndex(vehicle_index), mVelocity(velocity) {};

	Maths::HMat43	mMatrix;
	U32				mUniqueID;
	int				mVehicleIndex;
	Maths::HVec3	mVelocity;
};

class C_VehicleLogicMsg_DroppedPerkMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_DroppedPerkMsg, E_VehicleLogicMsg_DroppedPerk>
{
public:

	C_VehicleLogicMsg_DroppedPerkMsg(E_PICKUP_TYPE type, U8 vehIdx, char level,Maths::HMat43 mat,float timer,U32 ID)
	{
		mPickupType=type;
		mFiringVehicle = vehIdx;
		mLevel=level;
		mMatrix=mat;
		mTimer=timer;
		mUniqueID=ID;
	}
	
	U8												mFiringVehicle;
	Maths::HMat43									mMatrix;
	E_PICKUP_TYPE									mPickupType;
	char											mLevel;
	float											mTimer;
	U32												mUniqueID;
};

class C_VehicleLogicMsg_PerkAudioTriggerMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_PerkAudioTriggerMsg, E_VehicleLogicMsg_PerkAudioTrigger>
{
public:

	C_VehicleLogicMsg_PerkAudioTriggerMsg(E_PICKUP_AUDIO_TRIGGER trigger)
	{
		mAudioTrigger= trigger;

	}

	E_PICKUP_AUDIO_TRIGGER							mAudioTrigger;

};

class C_VehicleLogicMsg_AddOnTrackScore: public Messaging::C_MessageDerived<C_VehicleLogicMsg_AddOnTrackScore, E_VehicleLogicMsg_AddOnTrackScore>
{
public:
	C_VehicleLogicMsg_AddOnTrackScore(U8 vehicle_index, AMAX_SCORING_EVENT scoring_event)
	{
		mVehicleIndex=vehicle_index;
		mScoringEvent=scoring_event;
	}

	U8												mVehicleIndex;
	AMAX_SCORING_EVENT								mScoringEvent;
};

class C_VehicleLogicMsg_AggressiveScore: public Messaging::C_MessageDerived<C_VehicleLogicMsg_AggressiveScore, E_VehicleLogicMsg_AggressiveScore>
{
public:

	C_VehicleLogicMsg_AggressiveScore(U8 vehicleIndex, U8 aboutVehicleIndex, Components::BlurScoreMoveSummary& summary)
	{
		mVehicleIndex = vehicleIndex;
		mAboutVehicleIndex = aboutVehicleIndex;
	
		mBase				= summary.mBase;
		mBaseType			= summary.mBaseType;
		mSkill				= summary.mSkill;
		mBonusScoringEvent	= summary.mBonus;
		mBonusLevel			= summary.mBonusLevel;
	}

	C_VehicleLogicMsg_AggressiveScore(U8 vehicleIndex, U8 aboutVehicleIndex, E_BLUR_SCORE_MOVE_BASE base, E_BLUR_SCORE_MOVE_BASE_TYPE baseType, E_BLUR_SCORE_MOVE_SKILL skill, E_BLUR_SCORE_MOVE_BONUS bonus, E_BLUR_SCORE_MOVE_BONUS_LEVEL level)
	{
		mVehicleIndex = vehicleIndex;
		mAboutVehicleIndex = aboutVehicleIndex;
	
		mBase				 = base;
		mBaseType			 = baseType;
		mSkill				 = skill;
		mBonusScoringEvent   = bonus;
		mBonusLevel			 = level;
	}

	U8												mVehicleIndex;
	U8												mAboutVehicleIndex;

	E_BLUR_SCORE_MOVE_BASE							mBase;
	E_BLUR_SCORE_MOVE_BASE_TYPE						mBaseType;
	E_BLUR_SCORE_MOVE_SKILL							mSkill;
	E_BLUR_SCORE_MOVE_BONUS							mBonusScoringEvent;
	E_BLUR_SCORE_MOVE_BONUS_LEVEL					mBonusLevel;

};

class C_VehicleLogicMsg_PerkEnded: public Messaging::C_MessageDerived<C_VehicleLogicMsg_PerkEnded, E_VehicleLogicMsg_PerkEnded>
{
public:
	C_VehicleLogicMsg_PerkEnded(U8 vehicle_index,U32 unique_id,E_PICKUP_TYPE type,Game::E_BodyUserDataType userType = Game::E_BodyUserDataType_Unknown,E_PICKUP_SUB_TYPE	pickupSubType = E_PICKUP_SUB_TYPE_NORMAL)
	{
		mVehicleIndex=vehicle_index;
		mPickupType=type;
		mUniqueID=unique_id;
		mUserBodyType = userType;
		mPickupSubType = pickupSubType;
	}

	U8												mVehicleIndex;
	E_PICKUP_TYPE									mPickupType;
	U32												mUniqueID;
	Game::E_BodyUserDataType						mUserBodyType;
	E_PICKUP_SUB_TYPE								mPickupSubType;
};


class C_VehicleLogicMsg_StartDelayToPerk: public Messaging::C_MessageDerived<C_VehicleLogicMsg_StartDelayToPerk, E_VehicleLogicMsg_StartDelayToPerk>
{
public:

	C_VehicleLogicMsg_StartDelayToPerk(E_PICKUP_TYPE type,char level,Actor::C_ActorHandle handle, U8 vehicle_index,float sfx_time_limit,U32 unique_id)
	{
		mActorHandle=handle;
		mPickupType=type;
		mLevel=level;
		mVehicleIndex=vehicle_index;
		mSFXTimeLimit=sfx_time_limit;
		mUniqueId=unique_id;
	}
	
	U8												mVehicleIndex;
	Actor::C_ActorHandle							mActorHandle;
	E_PICKUP_TYPE									mPickupType;
	char											mLevel;
	float											mSFXTimeLimit;
	U32												mUniqueId;
};

class C_VehicleLogicMsg_PerkUpdate: public Messaging::C_MessageDerived<C_VehicleLogicMsg_PerkUpdate, E_VehicleLogicMsg_PerkUpdate>
{
public:
	C_VehicleLogicMsg_PerkUpdate(U8 vehicle_index,U32 unique_id,E_PICKUP_TYPE type)
	{
		mVehicleIndex=vehicle_index;
		mPickupType=type;
		mUniqueID=unique_id;
	}

	U8												mVehicleIndex;
	E_PICKUP_TYPE									mPickupType;
	U32												mUniqueID;
};


class C_VehicleLogicMsg_EnableDisablePickupSystem: public Messaging::C_MessageDerived<C_VehicleLogicMsg_EnableDisablePickupSystem, E_VehicleLogicMsg_EnableDisablePickupSystem>
{
public:

	C_VehicleLogicMsg_EnableDisablePickupSystem(U8 vehicle_index,float pickup_time, float shield_time, bool incremental = false)
	{
		mVehicleIndex=vehicle_index;
		mDisablePickupTime = pickup_time;
		mDisableShieldTime = shield_time;
		mIncremental = incremental;
	}
	
	U8												mVehicleIndex;
	float											mDisablePickupTime;
	float											mDisableShieldTime;
	bool											mIncremental;
};

class C_VehicleLogicMsg_Scored: public Messaging::C_MessageDerived<C_VehicleLogicMsg_Scored, E_VehicleLogicMsg_Scored>
{
public:

	C_VehicleLogicMsg_Scored(U8 vehicle_index, AMAX_SCORING_EVENT score)
	{
		mVehicleIndex=vehicle_index;
		mScore=score;
	}

	C_VehicleLogicMsg_Scored(U8 vehicle_index, E_BLUR_SCORE_MOVE_BASE base_move, Btl::Vector<E_BLUR_SCORE_MOVE_SKILL> skill_moves, Btl::Vector<E_BLUR_SCORE_MOVE_BONUS> bonus_moves)
	{
		mBaseMove=base_move;
		mVehicleIndex=vehicle_index;
		mSkillMoves=skill_moves;
		mBonusMoves=bonus_moves;
	}

	U8												mVehicleIndex;
	AMAX_SCORING_EVENT								mScore;

	//new scoring system
	E_BLUR_SCORE_MOVE_BASE							mBaseMove;
	Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>			mSkillMoves;
	Btl::Vector<E_BLUR_SCORE_MOVE_BONUS>			mBonusMoves;
};

class C_VehicleLogicMsg_ChainCompleted: public Messaging::C_MessageDerived<C_VehicleLogicMsg_ChainCompleted, E_VehicleLogicMsg_ChainCompleted>
{
public:

	C_VehicleLogicMsg_ChainCompleted(U8 vehicle_index, int chainLength)
	{
		mVehicleIndex=vehicle_index;
		mChainLength=chainLength;
	}

	U8												mVehicleIndex;
	int												mChainLength;
};

class C_VehicleLogicMsg_Fan_Demand_Passed: public Messaging::C_MessageDerived<C_VehicleLogicMsg_Fan_Demand_Passed, E_VehicleLogicMsg_Fan_Demand_Passed>
{
public:

	//[PK} apologies for high jacking this message to be a failed, it just seemed pointless making a brand new one.
	C_VehicleLogicMsg_Fan_Demand_Passed(U8 vehicle_index, int fans, bool perfect, int pickups_left, bool fan_run, bool actually_failed=false)
	{
		mVehicleIndex=vehicle_index;
		mFans=fans;
		mActuallyFailed=actually_failed;
		mPerfect=perfect;
		mPickupsLeft=pickups_left;
		mFanRun=fan_run;
	}

	U8												mVehicleIndex;
	int												mFans;
	bool											mActuallyFailed;
	bool											mPerfect;
	int												mPickupsLeft;
	bool											mFanRun;
};


class C_VehicleLogicMsg_Challenge_Completed: public Messaging::C_MessageDerived<C_VehicleLogicMsg_Challenge_Completed, E_VehicleLogicMsg_Challenge_Completed>
{
public:

	C_VehicleLogicMsg_Challenge_Completed(U8 vehicle_index, int fans, U8 part, U8 partCount,UI::C_WideString128& completionString )
	{
		mVehicleIndex=vehicle_index;
		mFans=fans;
		mPart = part;
		mPartCount = partCount;
		mString = completionString;
	}

	U8												mVehicleIndex;
	U8												mPart;
	U8												mPartCount;
	int												mFans;
	UI::C_WideString128								mString;
};

class C_VehicleLogicMsg_Survived_EliminationMsg: public Messaging::C_MessageDerived<C_VehicleLogicMsg_Survived_EliminationMsg, E_VehicleLogicMsg_SurvivedElimination>
{
public:

	C_VehicleLogicMsg_Survived_EliminationMsg(S32 barrageNumberSurvived, S32 fans)
	{
		m_BarrageNumberSurvived = barrageNumberSurvived;
		m_SurvivalFans = fans;
	}

	S32												m_BarrageNumberSurvived;
	S32												m_SurvivalFans;
};

class C_VehicleLogicMsg_VehicleMurdered : public Messaging::C_MessageDerived<C_VehicleLogicMsg_VehicleMurdered, E_VehicleLogicMsg_VehicleMurdered>
{
public:

	C_VehicleLogicMsg_VehicleMurdered(U8 victimVehicleIndex, U8 murdererVehicleIndex) : mVictimVehicleIndex(victimVehicleIndex),
																						mMurdererVehicleIndex(murdererVehicleIndex)
	{
	}


	U8												mVictimVehicleIndex;
	U8												mMurdererVehicleIndex;
};

class C_VehicleLogicMsg_VehicleSuicided : public Messaging::C_MessageDerived<C_VehicleLogicMsg_VehicleMurdered, E_VehicleLogicMsg_VehicleSuicided>
{
public:

	C_VehicleLogicMsg_VehicleSuicided(U8 victimVehicleIndex) : mVictimVehicleIndex(victimVehicleIndex)
	{
	}


	U8												mVictimVehicleIndex;
};

class C_VehicleLogicMsg_LandedMsg : public Messaging::C_MessageDerived<C_VehicleLogicMsg_LandedMsg, E_VehicleLogicMsg_Landed>
{
public:

	C_VehicleLogicMsg_LandedMsg(U8 airborneVehicleIndex, F32 timeInAir) : mAirborneVehicleIndex(airborneVehicleIndex), mTimeInAir(timeInAir)
	{
	}
		
	F32												mTimeInAir;
	U8												mAirborneVehicleIndex;
};

class C_VehicleLogicMsg_DestroyedPickupWithPickup : public Messaging::C_MessageDerived<C_VehicleLogicMsg_DestroyedPickupWithPickup, E_VehicleLogicMsg_DestroyedPickupWithPickup>
{
public:

	C_VehicleLogicMsg_DestroyedPickupWithPickup(E_PICKUP_TYPE pickupKillerType, E_PICKUP_SUB_TYPE pickupKillerSubType, U8 pickupKillerOwnerID, U32 pickupKillerActorID,
											    E_PICKUP_TYPE pickupVictimType, E_PICKUP_SUB_TYPE pickupVictimSubType, U8 pickupVictimOwnerID, U32 pickupVictimActorID ): mPickupKillerType(pickupKillerType),
																																										  mPickupKillerSubType(pickupKillerSubType),
																																										  mPickupKillerOwnerID(pickupKillerOwnerID),
																																										  mPickupKillerActorID(pickupKillerActorID),
																																									      mPickupVictimType(pickupVictimType),
																																										  mPickupVictimSubType(pickupVictimSubType),
																																										  mPickupVictimOwnerID(pickupVictimOwnerID),
																																										  mPickupVictimActorID(pickupVictimActorID)
																																										  {}
	  

	E_PICKUP_TYPE									mPickupVictimType;
	E_PICKUP_SUB_TYPE								mPickupVictimSubType;
	U8												mPickupVictimOwnerID; 
	U32												mPickupVictimActorID; 

	E_PICKUP_TYPE									mPickupKillerType;
	E_PICKUP_SUB_TYPE								mPickupKillerSubType;
	U8												mPickupKillerOwnerID; 
	U32												mPickupKillerActorID; 

};

class C_VehicleLogicMsg_HitWithAllDomes: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HitWithAllDomes, E_VehicleLogicMsg_HitWithAllDomes>
{
public:

	C_VehicleLogicMsg_HitWithAllDomes(U8 vehicle_index)
	{
		mVehicleIndex=vehicle_index;
	}

	U8												mVehicleIndex;
};

class C_VehicleLogicMsg_AvoidedAllDomes: public Messaging::C_MessageDerived<C_VehicleLogicMsg_HitWithAllDomes, E_VehicleLogicMsg_AvoidedAllDomes>
{
public:

	C_VehicleLogicMsg_AvoidedAllDomes(U8 firerIdx, U8 targetIdx)
	{
		mFirerIdx=firerIdx;
		mTargetIdx=targetIdx;
	}

	U8												mFirerIdx;
	U8												mTargetIdx;
};

class C_VehicleLogicMsg_CrossedCheckpoint: public Messaging::C_MessageDerived<C_VehicleLogicMsg_CrossedCheckpoint, E_VehicleLogicMsg_CrossedCheckpoint>
{
public:

	C_VehicleLogicMsg_CrossedCheckpoint(U8 vehicle_index)
	{
		mVehicleIndex=vehicle_index;
	}

	U8												mVehicleIndex;
};

class C_VehicleLogicMsg_EvadedPowerUp: public Messaging::C_MessageDerived<C_VehicleLogicMsg_EvadedPowerUp, E_VehicleLogicMsg_EvadedPowerUp>
{
public:

	C_VehicleLogicMsg_EvadedPowerUp(U8 vehicle_index, U8 firer_index, E_PICKUP_TYPE pickupType, U32 unique_id)
	{
		mVehicleIndex=vehicle_index;
		mFirerIndex=firer_index;
		mPickupType=pickupType;
		mUniqueId=unique_id;
	}

	U8												mVehicleIndex;
	U8												mFirerIndex;
	E_PICKUP_TYPE									mPickupType;
	U32												mUniqueId;
};

class C_VehicleLogicMsg_PowerUpDeleted: public Messaging::C_MessageDerived<C_VehicleLogicMsg_PowerUpDeleted, E_VehicleLogicMsg_PowerUpDeleted>
{
public:

	C_VehicleLogicMsg_PowerUpDeleted(U8 vehicle_index, E_PICKUP_TYPE pickupType, U32 unique_id, U32 group_id)
	{
		mVehicleIndex=vehicle_index;
		mPickupType=pickupType;
		mUniqueId=unique_id;
		mGroupId=group_id;
	}

	U8												mVehicleIndex;
	E_PICKUP_TYPE									mPickupType;
	U32												mUniqueId;
	U32												mGroupId;
};

class C_VehicleLogicMsg_SideSwipe: public Messaging::C_MessageDerived<C_VehicleLogicMsg_SideSwipe, E_VehicleLogicMsg_SideSwipe>
{
public:

	C_VehicleLogicMsg_SideSwipe(U8 vehicle_index, U8 side_swiped_vehicle_index, float impact)
	{
		mVehicleIndex=vehicle_index;
		mImpact=impact;
		mSideSwipedVehicleIndex=side_swiped_vehicle_index;
	}

	U8												mVehicleIndex;
	U8												mSideSwipedVehicleIndex;
	float											mImpact;
};

class C_VehicleLogicMsg_PowerUpDestroyedByPowerUp: public Messaging::C_MessageDerived<C_VehicleLogicMsg_PowerUpDestroyedByPowerUp, E_VehicleLogicMsg_PowerUpDestroyedByPowerUp>
{
public:

	C_VehicleLogicMsg_PowerUpDestroyedByPowerUp(U8 vehicle_index, U8 firer_index, E_PICKUP_TYPE pickupType, E_PICKUP_SUB_TYPE subType, U32 unique_id)
	{
		mVehicleIndex=vehicle_index;
		mFirerIndex=firer_index;
		mPickupType=pickupType;
		mPickupSubType=subType;
		mUniqueId=unique_id;
	}

	U8												mVehicleIndex;
	U8												mFirerIndex;
	E_PICKUP_TYPE									mPickupType;
	E_PICKUP_SUB_TYPE								mPickupSubType;
	U32												mUniqueId;
};

class C_VehicleLogicMsg_PowerUpFXDurations: public Messaging::C_MessageDerived<C_VehicleLogicMsg_PowerUpFXDurations, E_VehicleLogicMsg_PowerUpFXDurations>
{
public:
	C_VehicleLogicMsg_PowerUpFXDurations(U8 vehicle_index,U32 unique_id,E_PICKUP_TYPE type,Game::E_BodyUserDataType userType = Game::E_BodyUserDataType_Unknown,E_PICKUP_SUB_TYPE pickupSubType = E_PICKUP_SUB_TYPE_NORMAL,
		float durationIntro = 0, float durationLoop = 0, float durationOutro = 0)
	{
		mVehicleIndex=vehicle_index;
		mPickupType=type;
		mUniqueID=unique_id;
		mUserBodyType = userType;
		mPickupSubType = pickupSubType;
		mDurationIntro = durationIntro;
		mDurationLoop = durationLoop;
		mDurationOutro = durationOutro;
	}

	U8												mVehicleIndex;
	E_PICKUP_TYPE									mPickupType;
	U32												mUniqueID;
	Game::E_BodyUserDataType						mUserBodyType;
	E_PICKUP_SUB_TYPE								mPickupSubType;
	float											mDurationIntro;
	float											mDurationLoop;
	float											mDurationOutro;
};

#endif
