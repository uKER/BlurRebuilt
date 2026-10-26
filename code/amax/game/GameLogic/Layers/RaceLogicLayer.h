#ifndef RACE_LOGIC_LAYER_H
#define RACE_LOGIC_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "RaceLogicLayerEnums.h"

#include <Framework/Layer/Layer.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <actor_system/ActorSys.h>

#include <Gravity/SDK/CollisionCallback.h>
#include <Gravity/SharedPhysics/ContactStreamIterator.h>
#include <Gravity/SDK/EventCallback.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <GameLogic/ActorStudioActors/PickupPositionActor.h>
#include <GameLogic/Pickups/PickupPositionManager.h>

#include "System/misc/Random.h"
#include "Game/AmaxCam/VFCamera.h"

namespace Actors 
{ 
	class C_PhysicalObject;
	class DynamicObjectActor;
}

namespace Vehicle { class CollisionInfo; }

struct RLImpactType;
struct RLLandingType;
struct RLDamageSystem;
struct RLWallRidingSystem;

namespace Game
{
	class C_PickupFireMineBase;
	class C_GameActorList;

namespace Actors
{
	class C_VehicleLogicActor;
}

typedef struct RaceLogicCarInfo
{
	Maths::HVec3 mLastPosition;
	Maths::HVec3 mPosition;

	Maths::HMat43 mMatrix;
}RaceLogicCarInfo;

// -----------------------------------------
// Pickup effects
// -----------------------------------------
struct PickupEffectInfo
{
	void InitNormalValues(bool shuffleOnRespawn);

	float				shuffleTime;		
	bool				shuffleOnPickedUp;
	float				respawnTime;
};
// -----------------------------------------

class C_TriggerEventCallback : public Gravity::C_TriggerEventCallback
{
public:
	virtual void ProcessTriggerEvents(SharedPhysics::C_MemoryIterator<Gravity::C_TriggerEvent>& iter, const U32 numEvents);
};

#define NUM_RECORDED_EVENTS 500

class C_DebugCollisionEventRecorder
{
public:
	C_DebugCollisionEventRecorder() {}

	void Reset()
	{
		for (S32 iVehicle = 0; iVehicle < m_CollisionEventStream.Size(); iVehicle++)
		{
			m_CollisionEventStream[iVehicle].m_Collisions.Clear();
		}
	}

	void AddEvent(S32 p_VehicleIndex, Maths::HMat43 p_Matrix, Maths::HVec3 p_Position, Maths::HVec3 p_Normal, float p_MaxImpulse, float p_Speed)
	{
		if (p_VehicleIndex < 0 || p_VehicleIndex >= MAX_RACELOGIC_VEHICLES)
			return;
	
		if (m_CollisionEventStream[p_VehicleIndex].m_Collisions.Size() >= NUM_RECORDED_EVENTS)
		{
			S32 mid = NUM_RECORDED_EVENTS / 2;
			Btl::Vector<CollisionEventStream::CollisionEvent> tmpEvents;
			for (S32 i = mid; i < NUM_RECORDED_EVENTS; i++)
			{
				tmpEvents.PushBack(m_CollisionEventStream[p_VehicleIndex].m_Collisions[i]);
			}
			m_CollisionEventStream[p_VehicleIndex].m_Collisions.Clear();
			m_CollisionEventStream[p_VehicleIndex].m_Collisions = tmpEvents;
		}

		m_CollisionEventStream[p_VehicleIndex].m_Collisions.PushBack();
		m_CollisionEventStream[p_VehicleIndex].m_Collisions.Back().m_VehicleMatrix	= p_Matrix;
		m_CollisionEventStream[p_VehicleIndex].m_Collisions.Back().m_Position		= p_Position;
		m_CollisionEventStream[p_VehicleIndex].m_Collisions.Back().m_Normal			= p_Normal;
		m_CollisionEventStream[p_VehicleIndex].m_Collisions.Back().m_MaxImpulse		= p_MaxImpulse;
		m_CollisionEventStream[p_VehicleIndex].m_Collisions.Back().m_Speed			= p_Speed;
	}

	void Render(S32 p_VehicleIndex);

public:

	struct CollisionEventStream
	{
		struct CollisionEvent
		{
			Maths::HMat43	m_VehicleMatrix;
			Maths::HVec3	m_Position;
			Maths::HVec3	m_Normal;
			float			m_MaxImpulse;
			float			m_Speed;
		};
		
		Btl::Vector<CollisionEvent> m_Collisions;
	};
	Btl::Vector<CollisionEventStream, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> > m_CollisionEventStream;
};
 
class C_SoceryDataList
{
public:
	C_SoceryDataList()
	{
	}

	Maths::HF32& operator[](const U32 index)
	{
		BIZ_ASSERT(index < 14);				// w 14

		return m_Parameters[index];
	}
	Maths::HF32 m_Parameters[14];			// w 14
};

enum E_RaceState
{
	E_RaceState_None,

	E_RaceState_Loading,
	E_RaceState_Intro,
	E_RaceState_Starting,
	E_RaceState_Racing,
	E_RaceState_Finished,
	E_RaceState_Outro,
	E_RaceState_Quitting,
	E_RaceState_Restarting,
	E_RaceState_GoingToNextEvent,

	E_RaceState_Max,
}; 

struct C_CachedBodyEvent
{
	C_CachedBodyEvent(const Gravity::C_BodyEvent& p_BodyEvent, Actor::C_ActorHandle& p_ActorHandle)
	{
		m_BodyEvent		= p_BodyEvent;
		m_ActorHandle	= p_ActorHandle;
	}
	Gravity::C_BodyEvent		m_BodyEvent;
	Actor::C_ActorHandle		m_ActorHandle;
};

#define MAX_CACHED_BODY_EVENTS 200

typedef Btl::List<C_CachedBodyEvent, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter, Btl::ListElementSingle, Btl::StaticPool<Btl::NullType, MAX_CACHED_BODY_EVENTS> > > CachedBodyList;

struct CarSort
{
	CarSort()
	{
		m_Distance	= 99999999.0f;
		m_Index		= -1;
	}

	bool operator < (const CarSort& other)
	{ 
		return m_Distance < other.m_Distance; 
	}

	float	m_Distance;
	S32		m_Index;
};	

class C_RaceLogicLayer : public Framework::C_Layer, public Messaging::C_MessageHandler, Gravity::C_CollisionCallback, Gravity::C_EventCallback
{
//Functions
public:
	friend class DataGathering;
	C_RaceLogicLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
	virtual										~C_RaceLogicLayer();

	HRESULT										ProcessMessage(const Framework::E_LayerMessage message);

	HRESULT										Create(bool quickRestart = false);
	HRESULT										Destroy(bool quickRestart = false);
	HRESULT										Update();
	HRESULT										GlobalUpdate();
	void										Reset();

	void										SortVehiclesAndLodBehaviour();

	Actor::C_ActorHandle						GetLogicVehicleHandle(int idx) { return idx < mVehicleHandles.Size() ? mVehicleHandles[idx] : Actor::C_ActorHandle(); }
	Actors::C_VehicleLogicActor					*GetLogicVehicleActor(int i);
	Actors::C_VehicleActor						*GetVehicleActor(int i);

	int											GetNumVehicles() { return mVehicleHandles.Size(); };
	int											GetNumLiveVehicles();
	void										DisableVehicle() { mNumVehiclesDisabled++; };

	bool										MessageHandler(const Messaging::C_MessageBase* pMessage);

	bool										HandleCollisionLogic(const Actor::C_Actor* p_pActorA, const Actor::C_Actor* p_pActorB, C_ImpactMsg* p_pImpactMsg, 
																		SharedPhysics::C_ContactStreamResult* p_pResult, F32& gameimpulseScale);

	void										ProcessCollisions(SharedPhysics::C_ContactStreamIterator& iter, SharedPhysics::TCollisionCallbackPhase phase);
	void										ProcessEvents(SharedPhysics::C_MemoryIterator<Gravity::C_BodyEvent>& eventIter, const U32 numEvents);
	void										ProcessCachedEvents();

	void										ProcessFluidEvents(Gravity::C_FluidEvent* m_pFluidEvents, U32 m_NumFluidEvents);


	float										CacheDynamicObjectCollision(::Actors::DynamicObjectActor* p_pObjActor,
																								Actor::C_Actor* p_pColliderActor, 
																								Vehicle::CollisionInfo* p_pCollisionInfo);

	float										CachePhysicsObjectCollision(::Actors::C_PhysicalObject* p_pObjActor,
																								Actor::C_Actor* p_pColliderActor, 
																								Vehicle::CollisionInfo* p_pCollisionInfo);

	void										ProcessObjectCollisions();
	bool										CacheImpactMessage(Gravity::C_Body* pBody, C_ImpactMsg& impactMsg, float impulse);
	void										ProcessImpactMessages();
	void										ClearImpactMessages() { m_CachedImpactMessages.Clear(); }
	
	const E_IMPACT_LEVEL						CalculateImpactLevel(const float gameplayImpulse, bool isSideImpact) const;
	const RLImpactType*							CalculateImpactType(const float gameplayImpulse, bool isSideImpact);
	const RLLandingType*						CalculateLandingType(const float gameplayImpulse);

	void										SendImpactMessagePatch(Gravity::C_Body* pBody, C_ImpactMsg& impactMsg, float impulse = 0.0f);
	void										SendImpactMessagePoint(Gravity::C_Body* pBody, C_ImpactMsg& impactMsg, float impulse);

	Util::C_Random&								GetRandomGenerator()				{ return mRandGenerator; };
	void										SeedRandom(U32 p_Seed)				{ mRandGenerator.Seed(p_Seed); };
	int											GetRandomInt(int i)					{ return mRandGenerator.Rand(i); };
	float										GetRandomFloat(float a, float b)	{ return mRandGenerator.FRand(a, b); };

	//	Checks to see if we can spawn another pickup on track (total pickups within normal parameters)
	bool										CanSpawnPickupOnTrack();
	void										AddPickupPositionActor(U32 key, Actor::C_ActorHandle handle) { mPickupPositionActorMap.Insert(key, handle); };
	C_PickupPositionManager&					GetPickupPositionManager() { return m_PickupPositionManager; };
	void										DestroyDroppedPickupPositionActor(U32 id);

	// Use responsibly! Probably not wise to store a pointer to an actor over multiple frames.
	::Actors::PickupPositionActor*				GetPickupPositionActor(U32 id);
	void										ForceRandomisePickups(U32 p_Seed);

	const int									GetTotalLivePickups() const;
	void										IncrementTotalLivePickups(const Actor::C_ActorHandle& actorHandle);
	void										DecrementTotalLivePickups(const Actor::C_ActorHandle& actorHandle);

	const char*									ConvertPickupTypeToString(E_PICKUP_TYPE type);

	bool										HasStarted321Go(){return mStarted321;};
	void										MidGamePause();

	bool										ShowMidPointHelp();

	bool										IsQuickRestart() { return m_IsQuickRestart; }

	void										CalculateBiggestOnScreenVehicles();
	Btl::Vector<float>							mBiggestOnScreenVehicles;

	//Data
	Btl::Vector<RaceLogicCarInfo,Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES > >	mVehiclePositions;

	// Central store of pickup actors for Darren's AI, Replays, maybe others.
	Btl::Map<U32, Actor::C_ActorHandle>			mPickupPositionActorMap;
	Btl::Map<U32, Actor::C_ActorHandle>			mDroppedPickupPositionActorMap;

	// Used By: VLC_Pickups_AI
	//float mTimer_PerkAIDisable;
	float mTimer_PerkPlayerHitAIDisable;

	C_TriggerEventCallback						m_TriggerEventCallback;

	Btl::Vector<int>							mLivePickups;
	int											mNumFanPickups;
	bool										mFanDemandActive;

	PickupEffectInfo							mPickupSpawnInfo;

	void										StartObjectHitCounting(Actor::C_ActorHandle handle);
	inline bool									IsObjectHitCounting(){return mCountObjectsHit;};
	int											GetObjectHitCounting();
	void										EndObjectHitCounting();

	float										GetPerksActivatedIn(){return mPerksActivatedIn;};

	U32											DropPickup(U32 uniqueId, S32 dropperIndex, E_PICKUP_TYPE type, Maths::Mat43 mat);
	U32											DropDecoyPickup(U32 uniqueId, S32 dropperIndex, E_PICKUP_TYPE type, Maths::Mat43 mat);

	bool										PowerupsActivated() { return mPickupsActivated; }

	Btl::Vector<CarSort>						m_SortedVehicles;

	E_RaceState									GetRaceState() { return m_RaceState; }

private:

	void										CreateDataLogging();
	void										UpdateDataLogging();
	void										EndDataLogging();

	void										UpdateFanRacePositions();
	
	const bool									HandleProjectiles(const Gravity::C_Body *pBodyA, const Gravity::C_Body *pBodyB) const; //CONST ftw!
	const bool									CheckMineCollision(const C_BodyUserDataBase* pUserData, const int victimVehicleIndex) const;	// [Blur2PC] was default-int

	Btl::Vector<Actor::C_ActorHandle>			mVehicleHandles;
	Btl::Vector<bool>							mVehicleStarted;
	U8											mNumVehiclesDisabled;			//	Number of vehicles in current race which have been wrecked/eliminated (or have left a race in Multiplayer).
	
	Btl::Vector<float, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> > mWallRidingScalingFactorList;

	// ------------------------------------------------------------------------------------------
	//									Targetting Optimisation
	// ------------------------------------------------------------------------------------------
public:
	bool										CanEvaluateVehicle(U8 idx);
	void										IncrementEvaluationVehicle();
private:
	U8											mCurrentVehicleEvaluationCheck;
	// ------------------------------------------------------------------------------------------

	Gravity::C_TriggerLayer*					GetPickupTriggerLayer() { return m_pPickupTriggerLayer; }
	Gravity::C_TriggerLayer*					m_pPickupTriggerLayer;

	Util::C_Random								mRandGenerator;

	C_SoceryDataList							m_dataList;

	// Data for the data logging system
	float										mDataLogVehicleDistancesTime;
	float										m_dataLog_PositionUpdateTime;

	bool										mHandbrakeApplied;
	bool										mBrakeApplied;
	bool										m_restartInProgress;
	bool										m_IsQuickRestart;

	int											mNumPushes_Y;
	int											mNumPushes_X;
	int											mNumPushes_B;
	int											mNumPushes_A;
	int											mNumLeftStick;
	int											mNumRightStick;
	int											mNumRB;
	int											mNumLB;
	int											mNumLT;
	int											mNumRT;

	bool										mActiveLeftStick;
	bool										mActiveRightStick;
	bool										mActiveRT;
	bool										mActiveLT;

	bool										mPickupsActivated;
	bool										mStarted321;
	float										mMidGamePauseTimer;

	RLWallRidingSystem*							mpWallRidingSystem;


	Btl::Vector<Btl::Pair<float, Util::C_String>, Btl::VectorParameters_Array<AmaxCam::E_VFCamera_Count > >  m_dataLog_CameraDurations;	

	// ------------------------------------------------------------------------------------------
	//									Fan Race Position Tracking
	// ------------------------------------------------------------------------------------------
	struct FanRacePlayerPosition
	{
		int		mVehIdx;
		float	mDistFromPlayer;
	};

	struct FanRacePlayerPositionSort
	{
	public:
		FanRacePlayerPositionSort() {};

		bool operator()(FanRacePlayerPosition lhs, FanRacePlayerPosition rhs)
		{
			return (lhs.mDistFromPlayer > rhs.mDistFromPlayer);
		}
	};

	Btl::Vector< FanRacePlayerPosition,Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES> > mFanRacePositions;
	// ------------------------------------------------------------------------------------------

	C_DebugCollisionEventRecorder				m_debugCollisionEvents;

	float										mPerksActivatedIn;

	E_RaceState									m_RaceState;

	struct CachedObjectCollision
	{
		Vehicle::CollisionInfo		m_CollisionInfo;
		Actor::C_Actor*				m_pObjectActor;
		Actor::C_Actor*				m_pColliderActor; 
	};

	Btl::Vector<CachedObjectCollision> m_CachedObjectCollisions;

	struct CachedImpactMessage
	{
		C_ImpactMsg					m_ImpactMessage;
		Gravity::C_Body*			m_pBody;
		float						m_Impulse;
	};

	Btl::Vector<CachedImpactMessage> m_CachedImpactMessages;

	CachedBodyList m_PhysicsBodyEvents;

	C_PickupPositionManager m_PickupPositionManager;

	bool										m_RaceLoaded;

	//store away how many objects have been hit for fan demands
	bool										mCountObjectsHit;
	Actor::C_ActorHandle						mCountObjectsHandle;
	Btl::Set<Actor::C_ActorHandle>				mObjectsHit;

	const RLDamageSystem*						m_pRLDamageSystem;
	Game::C_GameActorList*						m_pGameActorList;

	int											mTotalLivePickups;


//============================================================================

public:
	S32											GetNumberOfPlayerOvertakes()	{ return(m_overtake_number); }

private:
	Btl::Vector< float, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> >		m_overtake_relative_pos;
	bool										m_overtake_initialised;
	S32											m_overtake_number;


	void										InitialisePlayerOvertakeCheck();
	void										StopPlayerOvertakeCheck();
	void										UpdatePlayerOvertakeCheck();
	F32											GetPlayerRelativePos(F32 playerpos, F32 vehpos);

};

} // namespace Game

#endif // RACE_LOGIC_LAYER_H
