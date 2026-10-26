#ifndef VEHICLE_LOGIC_ACTOR_H
#define VEHICLE_LOGIC_ACTOR_H

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic\Structures\VehiclePickupInfo.h>
#include <GameLogic\Structures\VehiclePositionInfo.h>
#include <GameLogic\Structures\VehicleLapsInfo.h>
#include <GameLogic\Structures\VehicleScoringInfo.h>

#include <GameLogic\Layers\RaceLogicLayer.h>

namespace Kanzen
{
	class C_VLC_Sensor;
}

namespace Components
{
	class C_VehicleComponent_Damage;
	class C_VehicleComponent_Statistics;
	class C_VehicleComponent_Pickups;
	class C_VehicleComponent_Pickups_AI;
	class C_VehicleComponent_Pickups_Targetting;
	class C_VehicleComponent_HandOfGod;
	class C_VehicleComponent_AIController;
	class C_VehicleComponent_MP_Domination;
	class C_VehicleComponent_MP_Network_Damage;
	class C_VehicleComponent_Drafting;
	class C_VehicleComponent_Fan_Demand;
	class C_VehicleComponent_Tutorials;
	class C_VehicleComponent_Rumble;
	class C_VehicleComponent_MP_Challenges;
	class C_VehicleComponent_PlayerRubberBanding;
	class C_VehicleComponent_MP_Network_Pickups;
	class C_VehicleComponent_MP_Local_Pickups;
	class C_VehicleComponent_MP_Pickups_AI;
	class C_VehicleComponent_PointsSystem;
	class C_VehicleComponent_MP_IdleKickController;
	class C_VehicleComponent_NewScoring;
	class C_VehicleComponent_Abilities;
	class C_VehicleComponent_MP_Challenges;
	class C_VehicleComponent_SuperControlController;
}

namespace Game
{

class RaceModeRule_DriverGetFastestLap;
class RaceModeRule_DriverHitDriverXTimesWithPerk;
class RaceModeRule_CauseXDamageToDriver;
class RaceModeRule_DriverDistanceFromDriver;
class RaceModeRule_DriverHitXObjects;

class C_PickupFireBase;
class C_PickupFireGhost;
class C_PickupFireShock;
class C_PickupFireBarge;
class C_PickupMasterMineBomb;

class C_LevelStateMultiplayerVehicleAbilities;

namespace Actors
{
	
#define  NO_TRAILS	31

struct VehicleLogicActorDebugPoint
{
	Maths::HVec3	mPos;
	Maths::RGBA		mColour;
};

// ----------------------------------------------------------------------

struct ShuntEvaluation
{
	float			mScore;
	int				mVehIdx;
	bool			mOnSpline;
	bool			mAtDistance;
};

struct ShuntEvaluationSort
{
public:
	bool operator()(ShuntEvaluation& lhs, ShuntEvaluation& rhs)
	{
		return((!lhs.mAtDistance && rhs.mAtDistance) || lhs.mScore < rhs.mScore);
	}
};

// ----------------------------------------------------------------------

class C_VehicleLogicActorCreate : public ::Actors::C_PosActorCreate
{

public:

	C_VehicleLogicActorCreate()
	{
		mVehicleIndex		= 0;
		mRaceLogicLayerPtr = NULL;
		mPickupsAllowed = true;
	}

	static Actor::C_ActorTypeId					ClassType() { return E_ActorTypeId_LogicVehicle; }
	const Actor::C_ActorTypeId					GetType() const { return E_ActorTypeId_LogicVehicle; }

	U8											mVehicleIndex;
	C_RaceLogicLayer							*mRaceLogicLayerPtr;
	bool										mPickupsAllowed;
};



class C_VehicleLogicActor : public C_GameActor
{
public:
	static const float EFFECTIVELY_INFINITE_GHOST_TIME;
	
	C_VehicleLogicActor();
	~C_VehicleLogicActor();

	static const Actor::C_ActorTypeId			TypeId()	{ return E_ActorTypeId_LogicVehicle; }
	static const Btl::String32					TypeName()	{ return "LogicVehicle"; }

	void										Update();
	void										AddSubscribers();
	void										RemoveSubscribers();
	void										KillImpl();

	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);

	Game::Actors::RaceLogicCore*				GetRLC() const;
	C_VehicleActor*								GetGraphicalActor(int index) const;	
	U8											GetVehicleIndex() const			{ return mVehicleIndex; };
	bool										IsFatallyDamaged() const		{ return mFatallyDamaged; };

	bool										GetIsInLastGaspMode() const		{ return mIsInLastGaspMode; };
	void										SetIsInLastGaspMode(const bool mode) { mIsInLastGaspMode = mode; };
	float										GetTimeTillLastGaspForceFire() const; 
	void										FireLastGasp();
	float										GetWreckedSequenceDuration() const;


	void										CalcMaxHealth();
	int											GetMaxHealth() const			{ return mMaxHealth; };
	int											GetCurrentHealth() const		{ return mCurrentHealth; };
	E_HEALTH_LEVEL								GetCurrentHealthLevel() const	{ return mCurrentHealthLevel; };
	const RLHealthLevelZone*					GetCurrentHealthZone() const	{ return &mCurrentHealthZone; };
	void										ResetHealth();

	bool										IsWreckedOrEliminated() { return ( mCurrentHealthLevel == E_HEALTH_LEVEL_WRECKED || GetGraphicalActor(mVehicleIndex)->IsEliminated() ); };
	int											GetFansAwarded() const			{return mScoringInfo.mFinalScoreForUI; };

	bool										GetToggledToAiControl() const	{return m_ToggledToAiControl; }

	bool										CanHavePickUp(E_PICKUP_TYPE type) const;	// [Blur2PC] retail signature (0x004390b0)

	const Maths::HMat43&						GetWorldMatrix() const { return GetGraphicalActor(mVehicleIndex)->GetMatrix(); };
	const Maths::HVec3&							GetVelocity() const { return GetGraphicalActor(mVehicleIndex)->GetVelocity(); };

	void										Ghost(const float time = EFFECTIVELY_INFINITE_GHOST_TIME, const float targetAlpha = 0.6f);
	const bool									Deghost();

	void										SecretlyGhost();
	void										SecretlyDeGhost();
	E_PICKUP_TYPE								GetCurrentlySelectedPickup();

	bool										IsGhosted(){return mGhosted;};
	bool										IsSecretlyGhosted(){return mSecretGhosting;};

	void										SetAwayFromBody(bool isAway) { mAwayFromBody = isAway; };
	bool										IsAwayFromBody() { return mAwayFromBody; };

	void										EngageCloak(const Curve::C_HermiteCurve *pAlphaTransitionCurve = NULL);
	void										DisEngageCloak(const Curve::C_HermiteCurve *pAlphaTransitionCurve = NULL);
	const bool									IsCloaked() const;

	void										SetLastPickupIdCollided(const int idx) {mLastPickupIdCollided = idx;}
	const int									GetLastPickupIdCollided() const {return mLastPickupIdCollided;}

	void										OverrideCurrentPickup(E_PICKUP_TYPE type);
	bool										IsInvunerable(){return mInvunTimer ? true : false;};
	bool										LineOfSight(Maths::HVec3 position, Maths::HVec3 targetPosition);
	
	void										SetRemoveWhenWrecked(bool state) {mRemoveWhenWrecked = state;}
	void										SetAlwaysShieldState(bool state) {mAlwaysShields = state;}
	bool										GetAlwaysShieldState() {return(mAlwaysShields);}

	
	static void									SetSchedulerGroup(Scheduler::GroupHandle handle);
	static Scheduler::GroupHandle				GetSchedulerGroup() { return m_hSchedulerGroup; }

	void										RecordAwardStats( const float frameTime );

	
	void										ForceTargetVehicleEvaluation() { mPickupInfo.mCurrentTargetIdx = -1; mForceTargetVehicleEvaluation = true; };
	void										ModifyEcmWarningCount(const bool giveWarning);

	Game::Actors::RaceLogicCore*				m_pRLC;

	//debug / single player
	Components::C_VehicleComponent_Pickups				*pPickupComponent;
	Components::C_VehicleComponent_Pickups_AI			*pPickupAIComponent;
	Components::C_VehicleComponent_AIController			*pAIComponent;
	Components::C_VehicleComponent_Pickups_Targetting	*pPickupTargettingComponent;
	Components::C_VehicleComponent_HandOfGod			*pHOGComponent;
	Components::C_VehicleComponent_Statistics			*pStatisticsComponent;
	Components::C_VehicleComponent_Drafting				*pDraftingComponent;
	Components::C_VehicleComponent_Rumble				*pRumbleComponent;
	Components::C_VehicleComponent_Damage				*pDamageComponent;
	Components::C_VehicleComponent_Fan_Demand			*pFanDemandComponent;
	Components::C_VehicleComponent_Tutorials			*pTutorialsComponent;
	Kanzen::C_VLC_Sensor								*pSensorComponent;
	Components::C_VehicleComponent_NewScoring			*pNewScoringComponent;
	Components::C_VehicleComponent_Abilities			*pAbilitiesComponent;

	Components::C_VehicleComponent_SuperControlController*pSuperControlControllerComponent;

	// world tour / network custom
	Components::C_VehicleComponent_MP_Challenges		*pMPChallengesComponent;
	Components::C_VehicleComponent_MP_IdleKickController*pMPIdleKickControllerComponent;

	//split screen
	Components::C_VehicleComponent_PlayerRubberBanding	*pSSPlayerRubberBandingComponent;
	Components::C_VehicleComponent_MP_Network_Damage	*pSSNetworkDamageComponent;
	Components::C_VehicleComponent_Rumble				*pSSRumbleComponent;
	Components::C_VehicleComponent_MP_Pickups_AI		*pSSMPPickupAIComponent;
	Components::C_VehicleComponent_Pickups_Targetting	*pSSPickupTargettingComponent;
	Components::C_VehicleComponent_HandOfGod			*pSSHandOfGodComponent;
	Components::C_VehicleComponent_Statistics			*pSSStatisticsComponent;
	Components::C_VehicleComponent_Damage				*pSSDamageComponent;
	Components::C_VehicleComponent_Drafting				*pSSDraftingComponent;
	Components::C_VehicleComponent_PointsSystem			*pSSPointsSystemComponent;
	
	// ----------------------------------------------------
	// Shared Data: Please state if your classes touch this
	// ----------------------------------------------------

	//	RaceModeMultiplayer uses most 



	// Used By: AI_Amax, VLC_HandOfGod
	bool					mTemporaryAIControl;

	//	Used By: RLC (for Multiplayer)
	bool					m_ToggledToAiControl;

	// Used By: VLC_Pickups, VLC_Pickups_AI
	float					mShieldTime;
	float					mShieldHealth;
	U32						mLastShieldId;
	bool					mDoubleShield;
	bool					mSuperShield;

	// Used By: VLC_Pickups_AI, RaceLogicLayer
	VehicleProximities		mVehicleProximities;

	// Used by RaceRulesManager
	const bool				GetStillInRace() const {return mStillInRace;}
	void					SetStillInRace(const bool stillInRace) {mStillInRace = stillInRace;}

	// Used By: VLC_Pickups_Targetting
	E_PICKUP_LOCK_ON		mTargetLockedOn;

	int						mVehicleStrength;

	Maths::HVec3			mPickupHitPosition;
	Maths::HVec3			mPickupHitVector;

	// Used By: VLC_Drafting
	float					mDraftingAmount;

	void					ClearDebugPoints(){mDebugPoints.Destroy();};
	void					AddDebugPoint(Maths::HVec3 pos, Maths::RGBA col);

	void										DeferredUpdate();
	void										PreRemoveFromRace(float hideTime);			//	Ghost vehicle, apply handbrake and, if disableVehicle is true, remove from position logic calculations
	void										RemoveFromRace();


	void										CreateSnailTrails(const Maths::HMat43& worldMatrix);
	Maths::HVec3								m_snailTrailPositions[NO_TRAILS];		// is this enough...
	Maths::HVec3								m_SnailTrailPosition;
	int											m_TrailNo;

	VehicleRaceStatistics&						GetStatistics();				//Public accessor for newly Norbinated statistics info
	VehicleScoringInfo&							GetScoring();					//Public accessor for newly Norbinated scoring info
	VehiclePickupInfo&							GetPickupInfo();
	VehiclePositionInfo&						GetPositionInfo();
	VehicleLapsInfo&							GetLapInfo();


	const VehicleRaceStatistics&				GetStatistics() const;			//Public const accessor for newly Norbinated statistics info
	const VehicleScoringInfo&					GetScoring() const;				//Public const accessor for newly Norbinated scoring info
	const VehiclePickupInfo&					GetPickupInfo() const;
	const VehiclePositionInfo&					GetPositionInfo() const;
	const VehicleLapsInfo&						GetLapInfo() const;

	const float									EvaluateShuntTarget(const int potentialTargetVehicleIndex, const float checkAngle, bool& onSameSpline, bool& atDistance);
	const int									EvaluateShockTarget();

	void										ClearUpSilentRunningSFXForEndOfRace();

protected:

	virtual bool								InitialiseImpl();
	virtual bool								CreateImpl();

private:
	
	void										PreUpdate();

	void										DrawDebugBarge(Maths::HVec3 corner1, Maths::HVec3 corner2, Maths::HVec3 corner3, Maths::HVec3 corner4);
	float										InsideRectangle(Maths::HVec3 point, Maths::HVec3 corner1, Maths::HVec3 corner2, Maths::HVec3 corner3);

	void										CalculateVehicleProximities();
	void										CalculateVehicleTargetIndex();
	bool										CheckInVisionRangeOfOtherVehicle(int vehId);

	const bool									CanISafelyTurnMyCollisionsBackOn() const;

	const bool									AddAbilitiesComponent(Game::C_LevelState& levelState);
	const bool									AddNetworkAbilitiesComponent(Game::C_LevelState& levelState);

	const bool									AddSuperControlControllerComponent(const bool addDynamically = true);
	const bool									RemoveSuperControlControllerComponent();

	void										SetSilentRunningSFXEnabled(const bool silentRunningSFXEnabled = true);
	
	void										ResetHitBy(U8 hitByVehicleIndex);

	static Scheduler::GroupHandle				m_hSchedulerGroup;
	static bool									m_TasksFlushed;

	U8											mVehicleIndex;
	C_RaceLogicLayer							*mRaceLogicLayerPtr;

	Btl::Vector<VehicleLogicActorDebugPoint>	mDebugPoints;
	U32											mRecentlyAvoidedPerkId;

	int											mGameMode;
	bool										mStillInRace;
	bool										mLeftRace;
	bool										mRemoveWhenWrecked;
	bool										mAlwaysShields;
	bool										mPickupsAllowed;
	bool										mIsInLastGaspMode;

	int											mLastPickupIdCollided;
	bool										mForceTargetVehicleEvaluation;

	bool										mAwayFromBody;

	// ----------------------------------------------------
	// Shared Data: Please state if your classes touch this
	// ----------------------------------------------------

	// Used By: VLC_Damage, UIDamage, VLC_Network_Damage
	RLHealthLevelZone		mCurrentHealthZone;
	E_HEALTH_LEVEL			mCurrentHealthLevel;
	int						mCurrentHealth;
	int						mMaxHealth;

	// Used By: VLC_Fan_Power, UISHield, VLC_Pickups_AI, VLC_Pickups, PickupShock
	float					mFanPower;

	// Used By: VLC_AIController, VLC_HandofGod, VLC_HandofGod_AI, VehicleActor, PickupFireBase
	bool					mGhosted;
	bool					mGhostPoppingActive;
	bool					mSecretGhosting; // Shhh, don't tell the player.
	bool					mCloaked;	//Used for the silent running mod please access through EngageCloak()/DisEngageCloak()/Cloaked();
	float					mGhostTime;
	float					mGhostForceTimer;

	// Used only be VehicleLogicActor:
	bool					mFatallyDamaged;
	Btl::Vector< float, Btl::VectorParameters_Array< Game::MAX_RACELOGIC_VEHICLES > > mOvertakenTimers;
	Btl::Vector< ShuntEvaluation, Btl::VectorParameters_Array< Game::MAX_RACELOGIC_VEHICLES > > mShuntEvaluations;
	float					mInvunTimer;
	float					mHideTimer;
	bool					mGraphicalActorRemovedFromRace; // simple flag to prevent removal being repeated in update when mLeftRace is true; 

	// -----------------------------------
	// So we can easily access shared data
	// -----------------------------------
	friend class Components::C_VehicleComponent_Damage;
	friend class Components::C_VehicleComponent_Statistics;
	friend class Components::C_VehicleComponent_Pickups;
	friend class Components::C_VehicleComponent_Pickups_AI;
	friend class Components::C_VehicleComponent_Pickups_Targetting;
	friend class Components::C_VehicleComponent_HandOfGod;
	//friend class Components::C_VehicleComponent_AIController; //Removed so that its forced to use public accessors, see CharlieA
	friend class Components::C_VehicleComponent_MP_Domination;
	friend class Components::C_VehicleComponent_MP_Network_Damage;
	friend class Components::C_VehicleComponent_Drafting;
	friend class Components::C_VehicleComponent_Fan_Demand;
	friend class Components::C_VehicleComponent_Tutorials;
	friend class Components::C_VehicleComponent_NewScoring;
	friend class Components::C_VehicleComponent_MP_Challenges;
	friend class C_VehicleActor;

	friend class Kanzen::C_VLC_Sensor;

	friend class Game::C_PickupFireBase;
	friend class Game::C_PickupFireGhost;
	friend class Game::C_PickupFireShock;
	friend class Game::C_PickupFireBarge;
	friend class Game::RaceModeRule_DriverGetFastestLap;
	friend class Game::RaceModeRule_DriverHitDriverXTimesWithPerk;
	friend class Game::RaceModeRule_CauseXDamageToDriver;
	friend class Game::RaceModeRule_DriverDistanceFromDriver;
	friend class Game::RaceModeRule_DriverHitXObjects;
	friend class Game::C_LevelStateMultiplayerVehicleAbilities;
	friend class Game::C_PickupMasterMineBomb;

	// Used By: VLC_Statistics, RaceRule_GetFastestLap, RaceRule_HitDriverXTimesWithPerk, RaceModeRule_CauseXDamageToDriver
	VehicleRaceStatistics						mStatistics;
	
	//Used By: VLC_Scoring, AchievementsManager
	VehicleScoringInfo							mScoringInfo;

	// Used By: VLC_Pickups, VLC_Pickups_AI, VLC_Pickups_Targetting, AI_Amax
	VehiclePickupInfo							mPickupInfo;

	//Used By a huge number of files, as they are key data blocks.
	VehiclePositionInfo							mPositionInfo;
	VehicleLapsInfo								mLapsInfo;

	// Need to essentially reference count shunts that are within ECM range so we can make the bananna display the correct colour whilst ANY shunt
	// Is still Within ECM range
	S8											mNumShuntsChasingInECMRange;
	bool										mECMWarningActive;
	void										UpdateECMEvadeWarningDisplay();
	void										SetECMEvadeUIStatus(const bool active);

};

void DeferredVehicleLogicUpdate(Scheduler::Task* pTask);

}

}
#endif //VEHICLE_LOGIC_ACTOR_H
