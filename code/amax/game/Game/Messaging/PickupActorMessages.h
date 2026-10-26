#ifndef PICKUPACTORMESSAGES_H
#define PICKUPACTORMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/Messaging/GameMessages.h>
#include <misc_shared/Messaging/GameMessaging.h>
#include <misc_shared/Messaging/MessagingDeclare.h>
#include <misc_shared/Messaging/MessageBase.h>


namespace Game
{
namespace Actors
{
class PickupPickedUpMsg : public Messaging::C_MessageDerived<PickupPickedUpMsg, Game::E_PickupMessage_PickupPickedUp>
{
//Functions
public:
	PickupPickedUpMsg() : mDroppedFromIndex(-1), mUniqueId(-1) {};

//Variables
public:
	S8											mVehicleIndex;
	Actor::C_ActorHandle						mVehicleActorHandle;

	Actor::C_ActorHandle						mPickupActorHandle;
	enum E_PICKUP_TYPE							mePickupType;
	bool										mFxOnly;

	Maths::HVec3								mPickupPosition;

	S8											mDroppedFromIndex;
	S32											mUniqueId;
	bool										mIsGoldenFanDemand;
};

class C_GrantPickupMsg : public Messaging::C_MessageDerived<C_GrantPickupMsg, Game::E_PickupMessage_GrantPickup>
{
	//Functions
public:
	C_GrantPickupMsg(int vehIdx, E_PICKUP_TYPE pickup, bool refill = false, bool reserved = false){mVehicleIndex=vehIdx;mPickupType=pickup;mRefill=refill;mReserved=reserved;}

	//Variables
public:
	S8											mVehicleIndex;
	enum E_PICKUP_TYPE							mPickupType;
	bool										mRefill;
	bool										mReserved;
};

class PickupSetMsg : public Messaging::C_MessageDerived<PickupSetMsg, Game::E_PickupMessage_SetPickup>
{
	//Functions
public:
	PickupSetMsg( int idx, E_PICKUP_TYPE p, bool r)
	{
		mVehicleIndex = idx;
		mPickupType = p;
		mReset = r;
	};

	//Variables
public:
	S8											mVehicleIndex;
	E_PICKUP_TYPE								mPickupType;
	bool										mReset;
};

class PickupClearHitUpdatersMsg : public Messaging::C_MessageDerived<PickupClearHitUpdatersMsg, Game::E_PickupMessage_ClearHitUpdaters>
{
	//Functions
public:
	PickupClearHitUpdatersMsg( int idx )
	{
		mVehicleIndex = idx;
	};

	//Variables
public:
	S8											mVehicleIndex;
};

class PickupRemovePowerMsg : public Messaging::C_MessageDerived<PickupRemovePowerMsg, Game::E_PickupMessage_RemovePower>
{
	//Functions
public:
	PickupRemovePowerMsg( int idx, int p, bool force = false)
	{
		mVehicleIndex = idx;
		mPower = p;
		mForceRemoval = force;
	};

	//Variables
public:
	S8											mVehicleIndex;
	int											mPower;
	bool										mForceRemoval;			// Removes infinate powerups
};

class PickupShuffleMsg : public Messaging::C_MessageDerived<PickupShuffleMsg, Game::E_PickupMessage_Shuffle>
{
	//Functions
public:
	PickupShuffleMsg( int idx, bool left, int p)
	{
		mVehicleIndex = idx;
		mShuffleLeft = left;
		mPlaces = p;
	};

	//Variables
public:
	S8											mVehicleIndex;
	bool										mShuffleLeft;
	int											mPlaces;
};

//class MissileExplodedMsg : public Messaging::C_MessageDerived<MissileExplodedMsg, Game::E_PickupMessage_MissileExploded>
//{
////Functions
//public:
//	MissileExplodedMsg(){};
//
////Variables
//public:
//	Actor::C_ActorHandle						mMissileActorHandle;
//	Maths::HVec3								mWorldPosition;
//};

//class MineExplodedMsg : public Messaging::C_MessageDerived<MineExplodedMsg, Game::E_PickupMessage_MineExploded>
//{
////Functions
//public:
//	MineExplodedMsg(){};
//
////Variables
//public:
//	Actor::C_ActorHandle						mMineActorHandle;
//	Maths::HVec3								mWorldPosition;
//};

class PickupInitiatedMsg : public Messaging::C_MessageDerived<PickupInitiatedMsg, Game::E_PickupMessage_PickupInitiated>
{
//Functions
public:
	PickupInitiatedMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex, enum E_PICKUP_TYPE ePickupType)
	{
		mActorHandle = actorHandle;
		mVehicleIndex = vehicleIndex;

		mePickupType = ePickupType;
	};

//Variables
public:
	Actor::C_ActorHandle						mActorHandle;
	S8											mVehicleIndex;
	enum E_PICKUP_TYPE							mePickupType;
};

class PickupFinishedMsg : public Messaging::C_MessageDerived<PickupFinishedMsg, Game::E_PickupMessage_PickupFinished>
{
//Functions
public:
	PickupFinishedMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex, enum E_PICKUP_TYPE ePickupType)
	{
		mActorHandle = actorHandle;
		mVehicleIndex = vehicleIndex;

		mePickupType = ePickupType;
	};

//Variables
public:
	Actor::C_ActorHandle						mActorHandle;
	S8											mVehicleIndex;
	enum E_PICKUP_TYPE							mePickupType;
};

class HitByPickupMsg : public Messaging::C_MessageDerived<HitByPickupMsg, Game::E_PickupMessage_HitByPickup>
{
//Functions
public:
	HitByPickupMsg(Actor::C_ActorHandle actorHandle, S8 vehicleIndex, S8 firingVehicleIndex, enum E_PICKUP_TYPE ePickupType)
	{
		mActorHandle = actorHandle;
		mVehicleIndex = vehicleIndex;
		mFiringVehicleIndex = firingVehicleIndex;

		mePickupType = ePickupType;
	};

//Variables
public:
	Actor::C_ActorHandle						mActorHandle;
	S8											mVehicleIndex, mFiringVehicleIndex;
	enum E_PICKUP_TYPE							mePickupType;
};

class HitDecoyMsg : public Messaging::C_MessageDerived<HitDecoyMsg, Game::E_PickupMessage_HitDecoy>
{
//Functions
public:
	HitDecoyMsg(S8 vehicleIndex, S8 firingVehicleIndex)
	{
		mVehicleIndex = vehicleIndex;
		mFiringVehicleIndex = firingVehicleIndex;
	};

//Variables
public:
	S8											mVehicleIndex, mFiringVehicleIndex;
};

class DisablePickupTypeMsg : public Messaging::C_MessageDerived<DisablePickupTypeMsg, Game::E_PickupMessage_DisablePickupType>
{
//Functions
public:
	DisablePickupTypeMsg(enum E_PICKUP_TYPE ePickupType){ mePickupType = ePickupType; };

//Variables
public:
	enum E_PICKUP_TYPE							mePickupType;
};

class EnablePickupTypeMsg : public Messaging::C_MessageDerived<EnablePickupTypeMsg, Game::E_PickupMessage_EnablePickupType>
{
//Functions
public:
	EnablePickupTypeMsg(enum E_PICKUP_TYPE ePickupType){ mePickupType = ePickupType; };

//Variables
public:
	enum E_PICKUP_TYPE							mePickupType;
};

class SetPickupPositionIdMsg : public Messaging::C_MessageDerived<SetPickupPositionIdMsg, Game::E_PickupMessage_SetPickupPositionId>
{
//Functions
public:
	SetPickupPositionIdMsg(const int newPickupPositionId)
	{
		mPickupPositionId = newPickupPositionId;
	}

	//Variables
public:
	int											mPickupPositionId;
};


class C_EnteredPickupMsg : public Messaging::C_MessageDerived<C_EnteredPickupMsg, Game::E_PickupMessage_EnteredPickup>
{
//Functions
public:
	C_EnteredPickupMsg(Actor::C_ActorHandle p_Handle)
	{
		mVehicleActorHandle = p_Handle;
	}

	//Variables
public:
	Actor::C_ActorHandle			mVehicleActorHandle;
};

class C_ExitedPickupMsg : public Messaging::C_MessageDerived<C_ExitedPickupMsg, Game::E_PickupMessage_ExitedPickup>
{
	//Functions
public:
	C_ExitedPickupMsg(Actor::C_ActorHandle p_Handle)
	{
		mVehicleActorHandle = p_Handle;
	}

	//Variables
public:
	Actor::C_ActorHandle			mVehicleActorHandle;
};


class C_UpdateFanDemandStateMsg : public Messaging::C_MessageDerived<C_UpdateFanDemandStateMsg, Game::E_PickupMessage_UpdateFanDemandState>
{
	//Functions
public:
	C_UpdateFanDemandStateMsg(Actor::C_ActorHandle p_Handle)	{}

	//Variables
};




class PickupMessage_ShockedAPickup : public Messaging::C_MessageDerived<PickupMessage_ShockedAPickup, Game::E_PickupMessage_ShockedAPickup>
{
//Functions
public:
	PickupMessage_ShockedAPickup(S8 vehicleIndex, enum E_PICKUP_TYPE ePickupType, S8 shockedfromvehicleIndex)
	{
		mVehicleIndex = vehicleIndex;
		mePickupType = ePickupType;
		mShockedFromVehicleIndex = shockedfromvehicleIndex;
	};

//Variables
public:
	S8											mVehicleIndex;
	enum E_PICKUP_TYPE							mePickupType;
	S8											mShockedFromVehicleIndex;
};

class PickupMessage_ActivatePickups : public Messaging::C_MessageDerived<PickupMessage_ActivatePickups, Game::E_PickupMessage_ActivatePickups>
{
//Functions
public:
	PickupMessage_ActivatePickups(float p_fTime)
	{
		m_fTime = p_fTime;
	};

//Variables
public:
	float	m_fTime;
};

class WakeAudioPickupMsg : public Messaging::C_MessageDerived<WakeAudioPickupMsg, Game::E_PickupMessage_WakeAudio>
{
//Functions
public:
	WakeAudioPickupMsg(enum E_PICKUP_TYPE ePickupType,bool bGoldenDemand,Actor::C_ActorTypeId actorType ){ mePickupType = ePickupType; mIsGoldenFanDemand = bGoldenDemand; mActorType = actorType;}

//Variables
public:
	enum E_PICKUP_TYPE							mePickupType;
	bool										mIsGoldenFanDemand;
	Actor::C_ActorTypeId					mActorType;
};

class SleepAudioPickupMsg : public Messaging::C_MessageDerived<SleepAudioPickupMsg, Game::E_PickupMessage_SleepAudio>
{
//Functions
public:
	SleepAudioPickupMsg(enum E_PICKUP_TYPE ePickupType){ mePickupType = ePickupType; };

//Variables
public:
	enum E_PICKUP_TYPE							mePickupType;
};

}; // namespace Actors

}; // namespace Game

#endif //PICKUPACTORMESSAGES_H
