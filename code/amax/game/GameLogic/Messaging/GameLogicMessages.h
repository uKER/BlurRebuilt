#ifndef GAMELOGIC_MESSAGES_H
#define GAMELOGIC_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/Messaging/GameMessages.h>
#include <misc_shared/Messaging/GameMessaging.h>
#include <misc_shared/Messaging/MessagingDeclare.h>
#include <misc_shared/Messaging/MessageBase.h>
#include <GameLogic\RaceLogicCore\Data\RLVehicleData.h>

#include <ui\Util\WideString.h>

#include <misc_shared/UID/FourCC.h>

namespace Game
{

enum E_GameLogicMessage
{
	E_GameLogicMessage_None = FOURCC_HI20('AGLM'),

	//passed around when a vehicle does a score
	E_GameLogicMessage_Scoring_Event,

	E_GameLogicMessage_Failed_Rule,

	E_GameLogicMessage_Passed_All_Rule,
	
	E_GameLogicMessage_LapIncremented,
	E_GameLogicMessage_LapDecremented,
	E_GameLogicMessage_SplitCrossed,

	E_GameLogicMessage_Impact,

	E_GameLogicMessage_GameplayImpact,

	E_GameLogicMessage_MastermineSpawnedMinis,
	E_GameLogicMessage_GiftDelayedPickupWithAnim,

	E_GameLogicMessage_End, 
	E_GameLogicMessage_Begin = E_GameLogicMessage_None + 1, 
	E_GameLogicMessage_Count = E_GameLogicMessage_End - E_GameLogicMessage_Begin
};

class C_ImpactMsg : public Messaging::C_MessageDerived<C_ImpactMsg, Game::E_GameLogicMessage_Impact>
{
public:
	Maths::HMat43								mMatrix;
	Maths::HMat43								mMatrixA;
	Maths::HMat43								mMatrixB;

	Maths::HVec3								mVehicleVelocityA;
	Maths::HVec3								mVehicleVelocityB;

	Actor::C_ActorHandle						mActorHandleA;
	Actor::C_ActorHandle						mActorHandleB;

	Vehicle::CollisionInfo						mCollisionInfo;
	Vehicle::C_DamageFilter						mDamageFilter;
	
	U32											mHashValue;

	C_ImpactMsg() : mMatrix(Maths::UNDEFINED), mMatrixA(Maths::UNDEFINED), mMatrixB(Maths::UNDEFINED), mVehicleVelocityA(Maths::UNDEFINED),
		mVehicleVelocityB(Maths::UNDEFINED)
	{
	}
};

class C_GameplayImpactMsg : public Messaging::C_MessageDerived<C_GameplayImpactMsg, Game::E_GameLogicMessage_GameplayImpact>
{
public:
	C_GameplayImpactMsg(U8 vehIdx, int damage, E_IMPACT_LEVEL level) 
		: mVehicleIdx(vehIdx), mOtherVehicleIdx(INVALID_VEHICLE_IDX), mAggressiveVehicleIdx(INVALID_VEHICLE_IDX), mDamage(damage), mImpactLevel(level) {};

	U8											mVehicleIdx;
	U8											mOtherVehicleIdx;
	U8											mAggressiveVehicleIdx;
	int											mDamage;
	E_IMPACT_LEVEL								mImpactLevel;
};

class C_ScoringEventMsg : public Messaging::C_MessageDerived<C_ScoringEventMsg, E_GameLogicMessage_Scoring_Event>
{
public:
	C_ScoringEventMsg(U8 vehIndex,int score,UI::C_WideString64 str){mVehicleIndex=vehIndex;mScore=score;mString=str;};

	U8	mVehicleIndex;
	int mScore;
	UI::C_WideString64 mString;
};

class C_PassedAllRuleMsg : public Messaging::C_MessageDerived<C_PassedAllRuleMsg, E_GameLogicMessage_Passed_All_Rule>
{
public:
	C_PassedAllRuleMsg(U8 vehIndex){mVehicleIndex=vehIndex;};

	U8	mVehicleIndex;
};

class C_FailedRuleMsg : public Messaging::C_MessageDerived<C_FailedRuleMsg, E_GameLogicMessage_Failed_Rule>
{
public:
	C_FailedRuleMsg(U8 vehIndex){mVehicleIndex=vehIndex;};

	U8	mVehicleIndex;
};

class C_LapIncrementedMsg : public Messaging::C_MessageDerived<C_LapIncrementedMsg, E_GameLogicMessage_LapIncremented>
{
public:
	C_LapIncrementedMsg(U8 vehIndex,int lap, float time){mVehicleIndex=vehIndex;mLap=lap;mTime=time;};

	U8		mVehicleIndex;
	int		mLap;
	float	mTime;
	
};

class C_GiftDelayedPickupWithAnimMsg : public Messaging::C_MessageDerived<C_GiftDelayedPickupWithAnimMsg, E_GameLogicMessage_GiftDelayedPickupWithAnim>
{
public:
	C_GiftDelayedPickupWithAnimMsg(U8 vehIndex,int numShotsAwarded, E_PICKUP_TYPE type): mVehicleIndex(vehIndex),mNumShotsAwarded(numShotsAwarded),mPickupType(type){}

	U8				mVehicleIndex;
	int				mNumShotsAwarded;
	E_PICKUP_TYPE 	mPickupType;
};

class C_LapDecrementedMsg : public Messaging::C_MessageDerived<C_LapDecrementedMsg, E_GameLogicMessage_LapDecremented>
{
public:
	C_LapDecrementedMsg(U8 vehIndex,int lap){mVehicleIndex=vehIndex;mLap=lap;};

	U8		mVehicleIndex;
	int		mLap;

};

class C_SplitCrossedMsg : public Messaging::C_MessageDerived<C_SplitCrossedMsg, E_GameLogicMessage_SplitCrossed>
{
public:
	C_SplitCrossedMsg(U8 vehIndex,int split, int lap){mVehicleIndex=vehIndex;mSplit=split;mLap=lap;};

	U8		mVehicleIndex;
	int		mSplit;
	int		mLap;
};

} // namespace Game

#endif // GAMELOGIC_MESSAGES_H
