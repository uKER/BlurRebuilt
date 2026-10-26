#ifndef VLC_FAN_DEMAND_H
#define VLC_FAN_DEMAND_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\GameActors\Components\GameComponents.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

#include <GameLogic\RaceLogicCore\Data\RLEnums.h>

#include <GameLogic\ActorStudioActors\TriggerRegionActor.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

namespace Components
{

const int FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_NOT_STARTED = -1;
const int FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_COMPLETE = -2;
const int FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_FAILED = -3;

class BaseMessageHandler;

class I_VehicleComponent_Fan_Demand_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_Fan_Demand_CreateInfo() {}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Fan_Demand;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Fan_Demand;}

	Game::C_RaceLogicLayer		*mRaceLogicLayerPtr;
	int						mVehicleIndex;
};

class C_VehicleComponent_Fan_Demand : public I_GameActorComponent
{
public:

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Fan_Demand;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Fan_Demand;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Fan_Demand;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Fan_Demand;} // from Component::I_GameComponent

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

	void											Update();
	void											EndDemand(bool has_passed);


	bool											GetCurrentFanDemandState() {return mFanDemandActive; }
	Actor::C_ActorHandle							GetCurrentFanDemandHandle() {return mCurrentFanDemandHandle; }

protected:
	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

	void											ClearContainers();

	bool											IsInTriggerRegion(int ref);
	bool											HasExitedTriggerRegion(int ref);
	bool											DoneWithinTriggerRegion(XtArray<const Actors::TriggerRegionCreationData*> do_within);

	void											CalculateDriftsandJumps();

	void											EnteredTriggerZone(int ref);
	void											ExitedTriggerZone(int ref);

	void											UpdateGateStates();


	float											mDeltaTime;

	BaseMessageHandler*								mMessageHandler;

	int												mVehicleIndex;

	Game::C_RaceLogicLayer							*mRaceLogicLayerPtr;

	bool											mFanDemandActive;
	float											mTimer;
	Actor::C_ActorHandle							mCurrentFanDemandHandle;

	int												mCurrentGate;
	
	float											mDelayedGateTriggerTimeLeft;

	float											mFans;
	float											mGracePeriod;

	//
	// containers for all the different types
	//

	//whether we've passed yet
	Btl::Vector<bool> mPassedMove;
	Btl::Vector<bool> mPassedMoveThisFrame;

	//scoring
	Btl::Vector<int,Btl::VectorParameters_Array<AMAX_SCORING_EVENT_MAX>	>	mScoreTotals;

	//new scoring
	Btl::Vector<C_VehicleLogicMsg_Scored> mNewScoreMoves;

	//trigger zones
	Btl::Map<int, bool>	mInTriggerZone;

	Btl::Map<int, int>	mTriggerZoneHealthOnEnter;
	Btl::Map<int, int>	mTriggerZoneHealthOnExit;
	
	int mCurrentTriggerZoneSequence;
	int mCurrentTriggerZoneSequenceNext;
	int mCurrentTriggerZoneSequenceNumber;
	bool mCurrentTriggerZoneSequenceClockwise;

	//speed
	float mTimeAtSpeed;

	//overtaking
	int mStartPosition;

	//jumping
	U8 mNumWheelsOnGround;
	float mJumpDistance;
	int mNumberOfJumps;

	//drifting
	float mSlipAmount;
	float mDriftingAngle;
	bool mDrifting;
	bool mHandbrakeInduced;
	float mDriftDistance;
	float mDriftTime;

	float mJumpCompletedDistance;
	float mDriftCompletedDistance;
	float mDriftCompletedTime;

	//pickups
	Btl::Vector<int,Btl::VectorParameters_Array<E_PICKUP_TYPE_MAX> > mPickupAquiredTotals;
	Btl::Vector<int,Btl::VectorParameters_Array<E_PICKUP_TYPE_MAX> > mPickupUseTotals;
	Btl::Vector<int,Btl::VectorParameters_Array<E_PICKUP_TYPE_MAX> > mPickupHitTotals;

	E_PICKUP_TYPE mPreviousTypePickedUp;
	Btl::Vector<int,Btl::VectorParameters_Array<E_PICKUP_TYPE_MAX> > mPickupDoubleAquiredTotals;
	Btl::Vector<int,Btl::VectorParameters_Array<E_PICKUP_TYPE_MAX> > mPickupDoubleUseTotals;
	Btl::Vector<int,Btl::VectorParameters_Array<E_PICKUP_TYPE_MAX> > mPickupDoubleHitTotals;

	//damage
	int mStartHealth;

	//carnage
	int mObjectsHit;

	S32 m_bNewFanCountdown;
};

}

#endif //#ifndef VLC_FAN_DEMAND_H
