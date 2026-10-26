#ifndef VEHICLEACTORMESSAGES_H
#define VEHICLEACTORMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <actor_system/ActorDeclare.h>
//#include <actor_system/ActorMessages.h>
//#include <shared_actors/SharedActorMessages.h>

//#include <Game/GameFlow/GameFlowMessages.h>
#include <Game/Messaging/GameMessages.h>
#include <misc_shared/Messaging/GameMessaging.h>
#include <misc_shared/Messaging/MessagingDeclare.h>
#include <misc_shared/Messaging/MessageBase.h>

#include <Vehicle/Damage/CollisionInfo.h>
#include <RacingPhysics/RacingPhysics.h>

#include "../../GameLogic/RaceLogicCore/Data/RLEnums.h"
#include "../GameActors/Vehicles/VehiclePerformanceModifiers.h"
#include "../GameActors/Vehicles/VehiclePickupEffect.h"
#include <Vehicle/Damage/DamageLayer.h>
#include "Vehicle/PartCodes.h"

namespace WorldRender { class C_LightSampler; }

namespace Actors
{

	class C_GameActorCreate;
}


namespace Game
{

namespace Actors
{

class C_GameActorCreate;

//using Messaging::C_MessageBase;

class C_VehicleUpdateControlsMsg : public Messaging::C_MessageDerived<C_VehicleUpdateControlsMsg, Game::E_VehicleMessage_UpdatePlayerControls>
{
public:

	C_VehicleUpdateControlsMsg(){Clear();}

	void Clear()
	{
		m_fThrottle = 0.0f;
		m_fBrake = 0.0f;
		m_fSteering = 0.0f;
		m_fPitch = 0.0f;
		m_bHandbrake = false;
		m_bGearUp = false;
		m_bGearDown = false;
		m_bHorn = false;
		m_bLights = false;
	}

public:

	F32											m_fThrottle;
	F32											m_fBrake;
	F32											m_fSteering;
	F32											m_fPitch;

	bool										m_bHandbrake;
	bool										m_bGearUp;
	bool										m_bGearDown;
	bool										m_bHorn;
	bool										m_bLights;
	Maths::HVec3								m_torque;
	Maths::HVec3								m_localForce;
};

class C_VehicleUpdateAIControlsMsg : public Messaging::C_MessageDerived<C_VehicleUpdateAIControlsMsg, Game::E_VehicleMessage_UpdateAIControls>
{
public:

	C_VehicleUpdateAIControlsMsg() :	m_fThrottle(0.0f), m_fBrake(0.0f), m_fSteering(0.0f), m_fPitch(0.0f),
		m_bHandbrake(false), m_bGearUp(false), m_bGearDown(false), m_bHorn(false), m_bLights(false) {}

public:

	F32											m_fThrottle;
	F32											m_fBrake;
	F32											m_fSteering;
	F32											m_fPitch;

	bool										m_bHandbrake;
	bool										m_bGearUp;
	bool										m_bGearDown;
	bool										m_bHorn;
	bool										m_bLights;
	Maths::HVec3								m_torque;
	Maths::HVec3								m_localForce;
};

class C_VehicleUpdateNetworkControlsMsg : public Messaging::C_MessageDerived<C_VehicleUpdateNetworkControlsMsg, Game::E_VehicleMessage_UpdateNetworkControls>
{
public:

	C_VehicleUpdateNetworkControlsMsg()
	{
	}

public:
	RacingPhysics::Vehicle::CompressedNetworkControls	m_CarControls;
	RacingPhysics::Vehicle::CompressedNetworkState		m_CarState;
};

class C_VehicleUpdatePerformanceModifiersMsg : public Messaging::C_MessageDerived<C_VehicleUpdatePerformanceModifiersMsg, Game::E_VehicleMessage_UpdatePerformanceModifiers>
{
public:

	C_VehicleUpdatePerformanceModifiersMsg(	Actor::C_ActorHandle actorHandle, 
											S8 vehicleIndex, 
											VehiclePerformanceModifiers& performanceModifiers)
	{
		mActorHandle = actorHandle; 
		mVehicleIndex = vehicleIndex;

		mPerformanceModifiers = performanceModifiers;
	}

public:
	Actor::C_ActorHandle						mActorHandle;
	S8											mVehicleIndex;

	VehiclePerformanceModifiers					mPerformanceModifiers;
};

class C_VehicleAudioImpactMsg : public Messaging::C_MessageDerived<C_VehicleAudioImpactMsg, Game::E_VehicleMessage_AudioImpactEvent>
{
public:

	C_VehicleAudioImpactMsg()
	{
	}

public:

	Vehicle::E_DefferedEvent m_Event;
	Maths::HVec3 m_Position;
	Maths::HVec3 m_Direction;
	float m_Scale;
	Vehicle::E_PartSubCode m_PartSubCode;
	Vehicle::E_PartCode m_PartCode;
	bool m_Attached;

};

class C_VehicleAudioWaterSplashMsg : public Messaging::C_MessageDerived<C_VehicleAudioWaterSplashMsg, Game::E_VehicleMessage_AudioWaterSplash>
{
public:

	C_VehicleAudioWaterSplashMsg()	{};
	~C_VehicleAudioWaterSplashMsg()	{};



public:

};

class C_VehicleAudioVehiclePartSlamming : public Messaging::C_MessageDerived<C_VehicleAudioVehiclePartSlamming, Game::E_VehicleMessage_AudioVehiclePartSlamming>
{
public:

	C_VehicleAudioVehiclePartSlamming()
	{
	}

public:

	Vehicle::E_DefferedEvent m_Event;
	Maths::HVec3 m_Position;
	Maths::HVec3 m_Direction;
	float m_Scale;
	Vehicle::E_PartSubCode m_PartSubCode;
	Vehicle::E_PartCode m_PartCode;
	bool m_Attached;

};


class C_VehicleShieldedImpactMsg : public Messaging::C_MessageDerived<C_VehicleShieldedImpactMsg, Game::E_VehicleMessage_ShieldedImpactEvent>
{
public:

	C_VehicleShieldedImpactMsg() : m_VehicleIndex(-1), m_OtherVehicleIndex(-1) {}

public:

	Vehicle::CollisionInfo						m_CollisionInfo;
	S32											m_VehicleIndex;
	S32											m_OtherVehicleIndex;
	F32											m_ImpulseReductionPercentage;
	Maths::HMat43								m_Matrix;
};


class C_VehicleResetAtWorldPositionMsg : public Messaging::C_MessageDerived<C_VehicleResetAtWorldPositionMsg, Game::E_VehicleMessage_ResetAtWorldPosition>
{
//Functions
public:
	C_VehicleResetAtWorldPositionMsg()
	{
		mVehicleIndex = -1;
	}

	C_VehicleResetAtWorldPositionMsg(	Actor::C_ActorHandle actorHandle, 
										S8 vehicleIndex,
										Maths::HMat43 newWorldMatrix,
										Maths::HVec3 velocity)
	{
		mActorHandle = actorHandle; 
		mVehicleIndex = vehicleIndex;
		mNewWorldMatrix = newWorldMatrix;
		mVelocity = velocity;
	}

//Variables
public:
	Actor::C_ActorHandle						mActorHandle;
	S8											mVehicleIndex;
	Maths::HMat43								mNewWorldMatrix;
	Maths::HVec3								mVelocity;
};

class C_VehicleResetAtVehiclePositionMsg: public Messaging::C_MessageDerived<C_VehicleResetAtVehiclePositionMsg, Game::E_VehicleMessage_ResetAtVehiclePosition>
{
public:
	C_VehicleResetAtVehiclePositionMsg(U8 p_ResetVehicleIndex): mResetVehicleIndex(p_ResetVehicleIndex) {}

	U8											mResetVehicleIndex;
};

class C_VehicleResetOnTrack: public Messaging::C_MessageDerived<C_VehicleResetOnTrack, Game::E_VehicleMessage_ResetOnTrack>
{
//Functions
public:
	C_VehicleResetOnTrack(	Actor::C_ActorHandle actorHandle )
	{
		mActorHandle = actorHandle; 
		mOverride = false;
	}

//Variables
public:
	Actor::C_ActorHandle						mActorHandle;
	S8											mVehicleIndex;
	Maths::HVec3								mPosition;
	Maths::HVec3								mDirection;
	F32											mVelocity;
	bool										mOverride;
};

class C_VehicleResetForceCameraZone: public Messaging::C_MessageDerived<C_VehicleResetForceCameraZone, Game::E_VehicleMessage_ResetForceCameraZone>
{
	//Functions
public:
	C_VehicleResetForceCameraZone(bool follow) : mCameraFollow(follow) {};

	bool mCameraFollow;
};

class C_VehicleResetEnteredCameraZone: public Messaging::C_MessageDerived<C_VehicleResetEnteredCameraZone, Game::E_VehicleMessage_ResetEnteredCameraZone>
{
	//Functions
public:
	C_VehicleResetEnteredCameraZone(bool follow) : mCameraFollow(follow) {};

	bool mCameraFollow;
};

class C_VehicleResetExitedCameraZone: public Messaging::C_MessageDerived<C_VehicleResetExitedCameraZone, Game::E_VehicleMessage_ResetExitedCameraZone>
{
	//Functions
public:
	C_VehicleResetExitedCameraZone() {};
};

class C_VehicleResetTwoWayZone: public Messaging::C_MessageDerived<C_VehicleResetTwoWayZone, Game::E_VehicleMessage_ResetTwoWayZone>
{
	//Functions
public:
	C_VehicleResetTwoWayZone(F32 timer, Maths::HVec3 pos, Maths::HVec3 dir, S32 zoneId, S32 indivId) : mTimer(timer), mResetPosition(pos), 
		mResetDirection(dir), mResetZoneId(zoneId), mResetIndividualId(indivId) {};

	F32				mTimer;
	Maths::HVec3	mResetPosition;
	Maths::HVec3	mResetDirection;
	S32				mResetZoneId;
	S32				mResetIndividualId;
};

class C_VehicleResetForceZone: public Messaging::C_MessageDerived<C_VehicleResetForceZone, Game::E_VehicleMessage_ResetForceZone>
{
public:
	C_VehicleResetForceZone()
	{
		mResetPositions=0;
		mDelay=0.f;
	}

	XtArray< ::Actors::C_GameActorCreate*>*	mResetPositions;
	float mDelay;
	
};

class C_VehicleResetEntryZone: public Messaging::C_MessageDerived<C_VehicleResetEntryZone, Game::E_VehicleMessage_ResetEntryZone>
{
	//Functions
public:
	C_VehicleResetEntryZone(F32 timer, Maths::HVec3 pos, Maths::HVec3 dir, S32 id) : mTimer(timer), mResetPosition(pos), 
																					 mResetDirection(dir), mResetZoneId(id) {};

	F32				mTimer;
	Maths::HVec3	mResetPosition;
	Maths::HVec3	mResetDirection;
	S32				mResetZoneId;
};

class C_VehicleResetExitZone: public Messaging::C_MessageDerived<C_VehicleResetExitZone, Game::E_VehicleMessage_ResetExitZone>
{
	//Functions
public:
	C_VehicleResetExitZone(S32 id) : mResetZoneId(id) {};

	S32				mResetZoneId;
};


class C_VehicleResetAtCameraPositionMsg : public Messaging::C_MessageDerived<C_VehicleResetAtCameraPositionMsg, Game::E_VehicleMessage_ResetAtCameraPosition>
{
//Functions
public:
	C_VehicleResetAtCameraPositionMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex)
	{
		mActorHandle = actorHandle; 
		mVehicleIndex = vehicleIndex;
	}

//Variables
public:
	Actor::C_ActorHandle						mActorHandle;
	S8											mVehicleIndex;
};

class C_VehicleResetAtSpawnPositionMsg : public Messaging::C_MessageDerived<C_VehicleResetAtSpawnPositionMsg, Game::E_VehicleMessage_ResetAtSpawn>
{
//Functions
public:
	C_VehicleResetAtSpawnPositionMsg(S8 vehicleIndex)
	{
		mVehicleIndex = vehicleIndex;
	}
//Variables
public:
	S8											mVehicleIndex;
};

class C_VehicleWorldPositionMsg : public Messaging::C_MessageDerived<C_VehicleWorldPositionMsg, Game::E_VehicleMessage_VehicleWorldPosition>
{
//Functions
public:
	C_VehicleWorldPositionMsg(){};

//Variables
public:
	Actor::C_ActorHandle						mActorHandle;
	S8											mVehicleIndex;

	Maths::HMat43								mWorldMatrix;
	Maths::HVec3								mWorldPosition;
};

class C_VehicleFocusPositionMsg : public Messaging::C_MessageDerived<C_VehicleFocusPositionMsg, Game::E_VehicleMessage_VehicleFocusPosition>
{
//Functions
public:
	C_VehicleFocusPositionMsg(){};

//Variables
public:
	S8											mVehicleIndex;
	S8											mViewIndex;

	Maths::HMat43								mWorldMatrix;
	Maths::HVec3								mGroundPosition;
	Maths::C_Normal								mGroundNormal;
};

class C_VehicleLoadedMsg : public Messaging::C_MessageDerived<C_VehicleLoadedMsg, Game::E_VehicleMessage_Loaded>
{
//Functions
public:
	C_VehicleLoadedMsg(Actor::C_ActorHandle actorHandle, S8 p_VehicleIndex, bool p_LoadStatus)
	{
		m_VehicleIndex	= p_VehicleIndex;
		m_LoadStatus	= p_LoadStatus;
		m_ActorHandle	= actorHandle;
	}
//Variables
public:
	S8											m_VehicleIndex;
	bool										m_LoadStatus;
	Actor::C_ActorHandle						m_ActorHandle;
};

class C_VehicleForceHumanControlMsg : public Messaging::C_MessageDerived<C_VehicleForceHumanControlMsg, Game::E_VehicleMessage_ForceHumanControl >
{
//Functions
public:
	C_VehicleForceHumanControlMsg(S8 vehicleIndex)
	{
		m_VehicleIndex	= vehicleIndex;
	}

//Variables
public:
	S8											m_VehicleIndex;
};

class C_VehicleForceAIControlMsg : public Messaging::C_MessageDerived<C_VehicleForceAIControlMsg, Game::E_VehicleMessage_ForceAIControl >
{
//Functions
public:
	C_VehicleForceAIControlMsg(S8 vehicleIndex)
	{
		m_VehicleIndex	= vehicleIndex;
	}

//Variables
public:
	S8											m_VehicleIndex;
};

class C_VehicleUpdateDriverMsg : public Messaging::C_MessageDerived<C_VehicleUpdateDriverMsg, Game::E_VehicleMessage_UpdateDriver>
{
public:

	C_VehicleUpdateDriverMsg() :
		m_fSteering(0.0f), m_fSpeed(0.f), m_fWheelsOnGroundFraction(1.f), m_fDeltaTime(1.f / 30.f),
		m_iGear(0), m_iVehicleIndex(-1), m_pLightSampler(NULL),  m_pCubeMapTexture(NULL),
		m_bHandbrake(false) {}
	
public:

	Maths::HMat43					m_mMatrix;
	Maths::HVec3					m_vVelocity;
	Maths::HVec3					m_vAcceleration;
	Maths::HVec3					m_vAngularVelocity;
	F32								m_fSteering;
	F32								m_fSpeed;
	F32								m_fWheelsOnGroundFraction;
	F32								m_fDeltaTime;
	int								m_iGear;
	int								m_iVehicleIndex;
	WorldRender::C_LightSampler*	m_pLightSampler;
	const void*						m_pCubeMapTexture;
	bool							m_bHandbrake;
};

class C_VehicleControlsDisableMsg : public Messaging::C_MessageDerived<C_VehicleControlsDisableMsg, Game::E_VehicleMessage_ControlsDisable>
{
public:

	C_VehicleControlsDisableMsg(S8 vehicleIndex)
	{
		m_VehicleIndex	= vehicleIndex;
	}

//Variables
public:
	S8											m_VehicleIndex;
};

class C_VehicleControlsEnableMsg : public Messaging::C_MessageDerived<C_VehicleControlsEnableMsg, Game::E_VehicleMessage_ControlsEnable>
{
public:

	C_VehicleControlsEnableMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex)
	{
		m_VehicleIndex	= vehicleIndex;
		m_ActorHandle	= actorHandle;
	}

//Variables
public:
	S8											m_VehicleIndex;
	Actor::C_ActorHandle						m_ActorHandle;
};

class C_VehicleFatallyDamagedMsg : public Messaging::C_MessageDerived<C_VehicleFatallyDamagedMsg, Game::E_VehicleMessage_FatallyDamaged>
{
public:

	C_VehicleFatallyDamagedMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex)
	{
		m_VehicleIndex	= vehicleIndex;
		m_ActorHandle	= actorHandle;
	}

//Variables
public:
	S8											m_VehicleIndex;
	Actor::C_ActorHandle						m_ActorHandle;
};


class C_EnableDisableAutoHOGPlacementMsg : public Messaging::C_MessageDerived<C_EnableDisableAutoHOGPlacementMsg, Game::E_VehicleMessage_EnableDisableAutoHOGPlacement>
{
public:
	C_EnableDisableAutoHOGPlacementMsg(bool enabled) : m_Enabled(enabled) {}
	bool			m_Enabled;
};


class C_VehicleHealthLevelChangedMsg : public Messaging::C_MessageDerived<C_VehicleHealthLevelChangedMsg, Game::E_VehicleMessage_ChangedHealthLevel>
{
public:

	C_VehicleHealthLevelChangedMsg(S8 vehicleIndex, E_HEALTH_LEVEL healthLevel)
	{
		mVehicleIndex	= vehicleIndex;
		mHealthlevel	= healthLevel;
	}

	//Variables
public:
	S8											mVehicleIndex;
	E_HEALTH_LEVEL								mHealthlevel;

};

class C_VehicleResetFatalDamageMsg : public Messaging::C_MessageDerived<C_VehicleResetFatalDamageMsg, Game::E_VehicleMessage_ResetFatalDamage>
{
public:

	C_VehicleResetFatalDamageMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex, float repairAmount)
	{
		m_VehicleIndex	= vehicleIndex;
		m_ActorHandle	= actorHandle;
		m_RepairAmount	= repairAmount;
	}

//Variables
public:
	S8											m_VehicleIndex;
	Actor::C_ActorHandle						m_ActorHandle;
	float										m_RepairAmount;
};

class C_VehicleRepairDamageMsg : public Messaging::C_MessageDerived<C_VehicleRepairDamageMsg, Game::E_VehicleMessage_RepairDamage>
{
public:

	C_VehicleRepairDamageMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex, float repairAmount, float currentDamagePercentage, float repairDuration, float repairPointsAmount)
	{
		m_VehicleIndex	= vehicleIndex;
		m_ActorHandle	= actorHandle;
		m_RepairAmount	= Maths::Clamped(repairAmount, 0.f, 1.f);
		m_RepairDuration= repairDuration;
		m_CurrentDamagePercentage = currentDamagePercentage;
		m_RepairPointsAmount = repairPointsAmount;
	}

//Variables
public:
	S8											m_VehicleIndex;
	Actor::C_ActorHandle						m_ActorHandle;
	float										m_RepairAmount;
	float										m_RepairDuration;
	float										m_CurrentDamagePercentage;
	float										m_RepairPointsAmount;
};

class C_VehicleOvertakeMsg : public Messaging::C_MessageDerived<C_VehicleOvertakeMsg, Game::E_VehicleMessage_Overtake>
{
public:

	C_VehicleOvertakeMsg(S8 overtakerVehicleIndex, S8 overtakeeVehicleIndex)
	{
		mOvertakerVehicleIndex	= overtakerVehicleIndex;
		mOvertakeeVehicleIndex	= overtakeeVehicleIndex;
	}

//Variables
public:
	S8											mOvertakerVehicleIndex;
	Actor::C_ActorHandle						mOvertakerActorHandle;
	S8											mOvertakerRacePosition;

	S8											mOvertakeeVehicleIndex;
	Actor::C_ActorHandle						mOvertakeeActorHandle;
};

class C_VehicleOvertakeWithNitroMsg : public Messaging::C_MessageDerived<C_VehicleOvertakeWithNitroMsg, Game::E_VehicleMessage_OvertakeWithNitro>
{
public:

	C_VehicleOvertakeWithNitroMsg(S8 overtakerVehicleIndex, S8 overtakeeVehicleIndex)
	{
		mOvertakerVehicleIndex	= overtakerVehicleIndex;
		mOvertakeeVehicleIndex	= overtakeeVehicleIndex;
	}

	//Variables
public:
	S8											mOvertakerVehicleIndex;
	S8											mOvertakeeVehicleIndex;
};

class C_VehicleCloseProximityMsg : public Messaging::C_MessageDerived<C_VehicleOvertakeMsg, Game::E_VehicleMessage_CloseProximity>
{
public:

	C_VehicleCloseProximityMsg(Actor::C_ActorHandle actorHandle1, S8 vehicleIndex1, Actor::C_ActorHandle actorHandle2, S8 vehicleIndex2, float distanceBetweenSquared)
	{
		mVehicleIndex1 = vehicleIndex1;
		mActorHandle1 = actorHandle1;
		
		mVehicleIndex2 = vehicleIndex2;
		mActorHandle2 = actorHandle2;

		mDistanceBetweenSquared = distanceBetweenSquared;

	}

//Variables
public:
	S8											mVehicleIndex1;
	Actor::C_ActorHandle						mActorHandle1;

	S8											mVehicleIndex2;
	Actor::C_ActorHandle						mActorHandle2;

	float										mDistanceBetweenSquared;
};

class C_VehicleExhaustPopMsg : public Messaging::C_MessageDerived<C_VehicleExhaustPopMsg, Game::E_VehicleMessage_ExhaustPopEvent>
{
public:

enum E_EXHAUSTPOP_TYPE
{
	E_EXHAUSTPOP_POP,
	E_EXHAUSTPOP_BANG,
	E_EXHAUSTPOP_LEFTPOP,
	E_EXHAUSTPOP_RIGHTPOP,
	E_EXHAUSTPOP_LEFTBANG,
	E_EXHAUSTPOP_RIGHTBANG,

};

	C_VehicleExhaustPopMsg() : m_VehicleIndex(-1){}

public:

	S32	m_VehicleIndex;
	E_EXHAUSTPOP_TYPE	m_ExhaustPopType;				
	
};

class C_VehicleLeftRaceMsg : public Messaging::C_MessageDerived<C_VehicleLeftRaceMsg, Game::E_VehicleMessage_LeftRace>
{
public:
	C_VehicleLeftRaceMsg(S32 vehicleIndex, float hideTime)
	{
		m_VehicleIndex = vehicleIndex;
		m_HideTime = hideTime;
	}

public:
	S32	m_VehicleIndex;
	float m_HideTime;
};

class C_VehicleNearlyFinishedRaceMsg : public Messaging::C_MessageDerived<C_VehicleNearlyFinishedRaceMsg, Game::E_VehicleMessage_NearlyFinishedRace>
{
public:
	C_VehicleNearlyFinishedRaceMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex, float racePercentage)
	{
		mVehicleIndex = vehicleIndex;
		mActorHandle = actorHandle;
		mRacePercentage = racePercentage;
	}

public:
	S8											mVehicleIndex;
	Actor::C_ActorHandle						mActorHandle;
	float										mRacePercentage;
};

class C_VehicleTakenOutMsg : public Messaging::C_MessageDerived<C_VehicleTakenOutMsg, Game::E_VehicleMessage_TakeOut>
{
public:
	C_VehicleTakenOutMsg(Actor::C_ActorHandle takenOutActorHandle, S8 takenOutVehicleIndex, Actor::C_ActorHandle responsibleActorHandle, S8 responsibleVehicleIndex)
	{
		mTakenOutVehicleIndex = takenOutVehicleIndex;
		mTakenOutActorHandle = takenOutActorHandle;

		mResponsibleVehicleIndex = responsibleVehicleIndex;
		mResponsibleActorHandle = responsibleActorHandle;
	}

public:
	S8											mTakenOutVehicleIndex;
	Actor::C_ActorHandle						mTakenOutActorHandle;

	S8											mResponsibleVehicleIndex;
	Actor::C_ActorHandle						mResponsibleActorHandle;
};


class C_VehicleFirePickupMsg : public Messaging::C_MessageDerived<C_VehicleFirePickupMsg, Game::E_VehicleMessage_FirePickup>
{
public:
	C_VehicleFirePickupMsg(S8 vehIdx, E_PICKUP_TYPE pickupType = E_PICKUP_TYPE_NONE, U32 ID = 0, bool backwards = false, bool forwards = false)
	{
		mVehicleIndex = vehIdx;
		//mActorHandle = actorHandle;
		mPickupOverride = pickupType;
		mUniqueID = ID;
		mBackwards = backwards;
		mForwards = forwards;
	}

public:
	S8											mVehicleIndex;
	//Actor::C_ActorHandle						mActorHandle;
	E_PICKUP_TYPE								mPickupOverride;
	U32											mUniqueID;
	bool										mBackwards;
	bool										mForwards;
};

class C_VehicleApplyShockedEffectMsg : public Messaging::C_MessageDerived<C_VehicleApplyShockedEffectMsg, Game::E_VehicleMessage_ApplyShockedEffect>
{
public:
	C_VehicleApplyShockedEffectMsg(S8 vehicleIndex, float time)
	{
		mVehicleIndex = vehicleIndex;
		mMaxTime = time;
	}

public:
	S8											mVehicleIndex;
	float										mMaxTime;
};

class C_VehicleEliminationMessage : public Messaging::C_MessageDerived<C_VehicleEliminationMessage, Game::E_VehicleMessage_Eliminated>
{
public:
	C_VehicleEliminationMessage(Actor::C_ActorHandle actorHandle, S8 vehicleIndex)
	{
		mVehicleIndex = vehicleIndex;
		mActorHandle = actorHandle;
	}

public:
	S8											mVehicleIndex;
	Actor::C_ActorHandle						mActorHandle;
};

class C_VehicleCheckTriggerRegionMsg : public Messaging::C_MessageDerived<C_VehicleCheckTriggerRegionMsg, Game::E_VehicleMessage_CheckTriggerRegion>
{
public:
	C_VehicleCheckTriggerRegionMsg(Actor::C_ActorHandle actorHandle, S32 eventType)
	{
		mVehicleActorHandle = actorHandle;
		mEventType = eventType;
	}

public:
	Actor::C_ActorHandle						mVehicleActorHandle;
	S32											mEventType;
};

class C_VehicleCheckTriggerActorEvent : public Messaging::C_MessageDerived<C_VehicleCheckTriggerActorEvent, Game::E_VehicleMessage_TriggerActorEvent>
{
public:
	C_VehicleCheckTriggerActorEvent(Actor::C_ActorHandle actorHandle, Gravity::C_TriggerEvent* pEvent,const Actor::C_Actor* pTriggerObject,const Actor::C_Actor* pTriggerActor )
	{
		mVehicleActorHandle = actorHandle;
		mEvent = pEvent;
		mTriggerObject = pTriggerObject;
		mTriggerActor = pTriggerActor;
	}

public:
	Actor::C_ActorHandle						mVehicleActorHandle;
	Gravity::C_TriggerEvent* mEvent;
	const Actor::C_Actor*						mTriggerObject;
	const Actor::C_Actor*						mTriggerActor;
};


class C_VehicleFreezeMsg : public Messaging::C_MessageDerived<C_VehicleFreezeMsg, Game::E_VehicleMessage_Freeze>
{
public:
	C_VehicleFreezeMsg(S32 vehicleIndex, bool freeze)
	{
		m_VehicleIndex	= vehicleIndex;
		m_Freeze		= freeze;
	}

public:
	S32			m_VehicleIndex;
	bool		m_Freeze;
};

class C_VehicleDebugDamageMsg : public Messaging::C_MessageDerived<C_VehicleDebugDamageMsg, Game::E_VehicleMessage_ApplyDebugDamage>
{
public:
	C_VehicleDebugDamageMsg(Maths::HVec3 p_Position, Maths::HVec3 p_Direction) 
	{
		m_Position		= p_Position;
		m_Direction		= p_Direction;
	}

public:
	Maths::HVec3		m_Position;
	Maths::HVec3		m_Direction;
};

class C_VehicleResprayMsg : public Messaging::C_MessageDerived<C_VehicleResprayMsg, Game::E_VehicleMessage_Respray>
{
public:
	C_VehicleResprayMsg(S32 p_VehicleIndex, XtId p_PaintJobXtId) 
	{
		m_VehicleIndex		= p_VehicleIndex;
		m_PaintJobXtId		= p_PaintJobXtId;
	}

public:
	S32						m_VehicleIndex;
	XtId					m_PaintJobXtId;
};

class C_VehicleResetAfterWreckedMsg : public Messaging::C_MessageDerived<C_VehicleResetAfterWreckedMsg, E_VehicleMessage_ResetAfterWrecked>
{
public:
	C_VehicleResetAfterWreckedMsg(U8 vehicleIndex) : mVehicleIndex(vehicleIndex) {}

public:
	U8						mVehicleIndex;

};

class C_CancelWreckedTimerMsg : public Messaging::C_MessageDerived<C_CancelWreckedTimerMsg, E_VehicleMessage_CancelWreckedTimer>
{
public:
	C_CancelWreckedTimerMsg(U8 vehicleIndex) : mVehicleIndex(vehicleIndex) {}

public:
	U8						mVehicleIndex;

};

class C_SetVehicleVisibility : public Messaging::C_MessageDerived<C_SetVehicleVisibility, E_VehicleMessage_SetVisibility>
{
public:
	C_SetVehicleVisibility(bool visibility, S8 viewportIndex) : m_Visibility(visibility), m_ViewportIndex(viewportIndex){}

public:
	bool					m_Visibility;
	S8						m_ViewportIndex;
};

} // Actor

} // Game



#endif // VEHICLEACTORMESSAGES_H
