#ifndef VLC_PICKUPS_H
#define VLC_PICKUPS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <gamelogic\racelogiccore\data\RLEnums.h>
#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\Pickups\PickupBase.h>
#include <GameLogic\Abilities\LasersightAbility.h>

#include <Game\GameActors\Vehicles\VehiclePerformanceModifiers.h>

namespace Game
{
class C_ProfileBase;
class C_LevelStateMultiplayerVehicleAbilities;
struct PickupHitInfo;
};

class C_VehicleLogicMsg_ShieldedPerkMsg; 

namespace Components
{

class BaseMessageHandler;

typedef struct HitUpdater
{
	E_PICKUP_TYPE mType;
	float mTimer;
	int mLevel;
	
}HitUpdater;

struct SpawnInformation
{
	E_PICKUP_TYPE	mType;
	Maths::HMat43	mPosition;
	U32				mUniqueId;
};

class I_VehicleComponent_Pickup_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_Pickup_CreateInfo() {}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Pickups;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Pickups;}

	Game::C_RaceLogicLayer		*mRaceLogicLayerPtr;
	int						mVehicleIndex;
};

class C_VehicleComponent_Pickups : public I_GameActorComponent
{
	static const int BRIBE_PICKUP_PROBABILITY_MULTIPLAYER = 100;

public:

	~C_VehicleComponent_Pickups();

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Pickups;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Pickups;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Pickups;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Pickups;} // from Component::I_GameComponent

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

	void											Update();

	virtual void									AttemptHitByPickup(E_PICKUP_TYPE pickupType,E_PICKUP_SUB_TYPE pickupSubType, E_PICKUP_HIT_TYPE hitType, int firingIdx, E_TARGETTING_DIRECTION hitDir, U32 pickupId, bool noPhysical, Game::PickupHitInfo hitInfo, U32 groupId );

	virtual void									RegisterShieldedPickup(E_PICKUP_TYPE pickupType, int firingIdx, U32 pickupId);
	virtual bool									RegisterHitByPickup(E_PICKUP_TYPE pickupType, E_PICKUP_SUB_TYPE subType, E_PICKUP_HIT_TYPE hitType, int firingIdx, U32 pickupId, E_TARGETTING_DIRECTION hitDir, bool noPhysical, Game::PickupHitInfo hitInfo, U32 groupId );
	virtual E_PICKUP_TYPE							HandleDropPickupOnTrack(SpawnInformation& information);
	virtual E_PICKUP_TYPE							HandleDropDecoyOnTrack(SpawnInformation& information);
	void											GetSpawnInformation(SpawnInformation& information);
	bool											GetPickupsDisabled() const {return (mDisablePickups > 0.0f);}

protected:
	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }
	void											Initialise();

	virtual bool									HitPickUp(E_PICKUP_TYPE type, char dropped_by_index,int numShots, bool infinite_shots = false, bool checkReservedSlots = false);
	virtual void									AddPickUp(E_PICKUP_TYPE type, char dropped_by_index,int numShots, bool infinite_shots = false);
	virtual void									DelayGiftPickUp(E_PICKUP_TYPE type, F32 secondsToDelay, U8 numShots = 1);
	void											GotSomePower(float p);
	void											RemovePower(int amount = 1, bool sameType = false, bool force = false);

	virtual bool									StartPerk(E_PICKUP_TYPE p_PickupType, bool forceBackwardFire, bool forceForwardFire, bool forceDoubleFire);
	virtual void									ActivatePerk(E_PICKUP_TYPE pickup, char dropped_by_index, bool forceBackwardFire, bool forceForwardFire, bool forceDoubleFire);
	virtual void									DropAndRemoveUsedPickup(E_PICKUP_TYPE pickup);

	virtual void									UpdateHitUpdaters();
	int												GetValidTargetIndex();

	void											GiftPickups();
	void											ForceGift();
	void											DropCurrentPickup();

	virtual bool									ShufflePerks(bool left);
	void											PlaySelectedPerkSfx();

	RLPickupLevel*									GetRLSubTypeLevel(E_PICKUP_TYPE type, E_PICKUP_SUB_TYPE subType);

	float											mDeltaTime;
	E_PICKUP_TYPE									mLastFiredPerk;

	BaseMessageHandler*								mMessageHandler;
	int												mVehicleIndex;
	Game::C_RaceLogicLayer							*mRaceLogicLayerPtr;
	Game::Actors::RaceLogicCore*					mRLCPtr;
	Game::C_LevelState*								mLevelStatePtr;

	unsigned char									mUsingPerkLevel;

	float											mUsingShieldTimer;
	float											mTimer_PerkGifting;

	bool											mRaceStarted;

	bool											mIsMultiplayer;

	Btl::Vector<Game::C_PickupLocalEffectsBase*>	mHitUpdaters;

	int												mVehicleTargettedByShock;
	float											mVehicleTargettedByShockTimer;

	float											mDisablePickups;
	float											mDisableSlowFiringPickups;

	U32												mLastFiredPickupId;

	bool											mDropPickup;
	float											mFrontRunnerTimer;
	U8												mSafetyNetNumTimesAbsorbed;
	U8												mSafetyNetNumBlindfiresAbsorbed;

private:
	// Access to the multiplayer abilities of the vehicle that owns this component
	Game::C_LevelStateMultiplayerVehicleAbilities*  mpMultiplayerAbilities;
	void											HandleSafetyNetPickupAward(const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg); 
	void											HandleSafetyNetBlindFire(const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg); 
	void											HandleSafetyNetShock(const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg, bool& showEffect); 
	int												GetNumShotsAwarded(E_PICKUP_TYPE type);
	S32												mLastShockPickupIDToHit; // Keep track of the last shock to hit us (for safety net mod - initialised to -1)

	void											CheckShieldedBoostersActivated();
	bool											CheckShieldedBoostersAbsorbDamage(E_PICKUP_TYPE pickupType, int firingIdx);
	void											AttemptDropPickup();
	bool											CanSafetyNetAbsorbPickupType(const E_PICKUP_TYPE pickupType) const;

	void											HandlePickupSlotChange();

	bool											mBlindfireHitLeft;

	Btl::Vector<F32>								m_PickupProbabilities;

	void											AdjustPosToAvoidExistingPickups(SpawnInformation& information);

	static const float								s_TitaniumArmourIntensity;
#ifndef CFG_GOLD
	void											ProcessDebugUpdate();
#endif //CFG_GOLD

	void											CheckBribeAbility();
	const E_PICKUP_TYPE 							EvaluateBribeChances() const;

	void											CheckFrontRunner(bool wasRespawn = false);

	void											PopulatePickupProbabilities(PickupPercentages& percentages);
	bool											CanUseDouble(E_PICKUP_TYPE p_PickupType);
	void											ApplyBlindfireSpin(bool fromMine);
	void											DisableActiveNitrous(bool slowdownOnly);

	float											mChargingTime;
	bool											mCanCharge;

	int												mGroupPickupId;

	float											mSafetyNetFXCooldownTimer;
	float											mRoadsweeperPickupFXCooldownTimer;
	float											mShieldedBoostersAbsorbFXCooldownTimer;
	float											mBribeDelayTimer; 

	Game::C_LasersightAbility						mLasersight;
	S8												mLastLapAwardedFrontRunner;
	float											mStartingLogicPosition;

	float											mHitByDoubleShieldCooldown;
};

}

#endif //#ifndef VLC_PICKUPS_H
