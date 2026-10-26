#ifndef PICKUPPOSITIONACTOR_H
#define PICKUPPOSITIONACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>
#include "ui/Graphics/Localisation/Loc.h"

#include <SafeString.inl>
#include <shared_actors/XtActorBase.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <GameLogic\RaceLogicCore\Data\RLEnums.h>

// [Blur2PC] pointer members below need these declared
namespace Game { class C_RaceLogicLayer; class C_LevelState; namespace Actors { class RaceLogicCore; } }

#include "Game/ActorDef/Actortypes.h"

//FORWARD DECLARATIONS
struct RLPickupType;
struct RLPickupPositionRow;

namespace Gravity
{
	class C_Trigger;
}

namespace SpecialFX
{
	class I_SpecialFXInstance;
};

namespace Game
{
	class C_RaceLogicLayer;
	class C_GameWorld;

	namespace Actors
	{
		class RaceLogicCore;
	};
};

namespace Actors
{
	
class C_VisibleDetect;

const int MAX_VEHICLES = 20;
const int MAX_VEHICLES_COLLIDING = 4;
	
//Used to create the actor
class PickupPositionCreate : public C_PosActorCreate
{
public:

	PickupPositionCreate();

	static Actor::C_ActorTypeId					ClassType()				{return Game::E_ActorTypeId_PickupPosition; }
	const Actor::C_ActorTypeId					GetType() const			{return Game::E_ActorTypeId_PickupPosition;}

	XtHandle<RLPickupType>						m_pPickupType;	
	XtArray< XtHandle<RLPickupType> >			m_pExcludedPickupTypes;
	int											m_iPickupActiveLevel;
	float										m_fActivationTimer;
	int											m_iActivationLap;
	float										m_fRespawnTimerOverride;
	bool										m_bConstantShuffle;
	XtFlags<XtU8, E_POSITION_ACTOR_PICKUP_TYPE>	m_uShuffleTypes;

	XtHandle<RLPickupPositionRow>				m_RowType;
	U32											m_uRowId;

//	bool										m_Golden;				// Special golden version of pickup

	bool										m_OneShot;				// Below are not annotated
	bool										m_ForceSelectedType;
	bool										m_bRotateGraphic;
	char										m_DroppedFromVehicleIndex;
	bool										m_AllowRandomPickupOverride;
	U32											m_UniqueDropID;
	F32											m_OneShotTime;

};

//Pickup Position Actor
class PickupPositionActor : public Actor::C_Actor
{
protected:
	struct VehiclePickupPosition
	{
		Maths::HVec3							mWorldPosition;
		Actor::C_ActorHandle					mActorHandle;
		int										mVehicleIndex;
		bool									mCollided;
		bool									mAwardSfxOnly;
	};


public:
	PickupPositionActor();
	~PickupPositionActor();

	static const Actor::C_ActorTypeId			TypeId()				{ return Game::E_ActorTypeId_PickupPosition; }
	static const Actor::C_ActorTypeName			TypeName()				{ return "PickupPosition"; }
	bool										InitialiseImpl();
	bool										CreateImpl();
	void										Destroy();
	void										CreateTrigger(float p_Radius);
	void										DestroyTrigger();
	virtual void								SetupVisualPickupIcon();

	const PickupPositionCreate*					GetCreationData() const {return static_cast<const PickupPositionCreate*>(C_Actor::GetCreationData().GetData());}
	const Maths::HMat43&						GetMatrix() const		{return mWorldMatrix;}
	const Maths::C_Vertex						GetPosition() const		{return mWorldMatrix.AW();}
	const Maths::C_Vector						GetDirection() const	{return mWorldMatrix.AZ();}
	const Maths::C_Vertex						GetUp() const			{return mWorldMatrix.AY();}
	const Maths::C_Vertex						GetRight() const		{return mWorldMatrix.AX();}

	E_PICKUP_TYPE								GetEnumPickupType() const {return mPickupType;}
	const RLPickupType*							GetPickupType() const;
	void										SetPickupType(E_PICKUP_TYPE type);
	void										SetNextPickupType(E_PICKUP_TYPE type);
	virtual bool								SetRandomPickupType(bool checkAgainstNext = false, bool forceChange = false);
	void										SetRandomPickupTypeMpSetup();
	const int									GetPickupPositionId() const {return mId;}
	void										SetVehicleCollectedLap(int vehicleIndex, S8 lap, bool force =false);
	
	void										SetFanDemandRenderState();
	void										HideSetShowCOMBO();
	
	void										ActivatePickups(float p_fTime);
	
	bool										OnMessage(const Messaging::C_MessageBase *pMessage);

	void										DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;

	inline bool									IsActive(){return mPickupActive;}
	inline bool									IsEnabled(){return mEnabled;}
	bool										ViewportIsGhosted(int vp);

	const U32									GetUniqueDropID() const { return m_UniqueDropID; }

	const int									GetTimesPickedUpByLocalPlayer() const { return m_TimesPickedUpByLocalPlayer; }
	void										ForceNetRespawnDelay(const F32 respawnDelayMultiplier, const U8 localCollectorIdx); 

protected:
	virtual void								Update(const float frameTime);
	virtual void								PreUpdate(const float frameTime);
	virtual bool								CheckPickup(VehiclePickupPosition& vehiclePosition);
	virtual void								GrantPickup(VehiclePickupPosition& vehiclePosition);

private:
	void										AnimatePickup(F32 frameTime);
	void										Render();
	bool										AwardPickup(const VehiclePickupPosition& vehiclePosition, bool fxOnly);

	void										InitialiseDebugLineVertices();

	void										ShowPickupSpecialFX(int idx);
	void										HidePickupSpecialFX(int idx);
	void										ShowAllPickupSpecialFX();
	void										HideAllPickupSpecialFX();
	void										SendPowerupTypeNetworkMessage(const E_PICKUP_TYPE type);
	void										SendNextPowerupTypeNetworkMessage(const E_PICKUP_TYPE type);

	float										GetRespawnDelayMultiplier();
	void										RecordCampingStats( U8 collectedByVehIdx );
	void										SetRespawnTimer();
	void										GenerateNextPickupType(int vehIdx = -1);

private:
	int											mId;
	Maths::HMat43								mWorldMatrix;
	Maths::HMat43								mInitialMatrix;
	
	//DEBUG DEBUG DEBUG
	Maths::HVec3								mV1, mV2, mV3, mV4;		//To store the vertices of the tetra hedron debug shape
	Maths::RGBA									mColour;
	//DEBUG DEBUG DEBUG

	bool										mDisplay,
												mPickupActive,
												mAllowPickup,
												mConstantShuffle,
												mNeedShuffle,
												mRotateGraphic,

												mEnabled,
												mSemiHidden,			// True for fan demand that are visible but can't be picked up
												mDisableDemand,			// True to disable this fan demand
												mCheckCampingPreventionTimeout;

	float										mReSpawnTimer,			//0.f when active and counts down from mDeactivatedTimer when deac
												mShuffleTimer,
												mTimeSinceRespawned, 
												mCampingPreventionTimeout, //		// Disable any respawn delay multiplayers if not picked up for certain time period. 
			
												mSpawnDelayTime,		//The time it takes to respawn
												mCollisionRadius,		//We do the collision (currently) based upon a 
																		//sphere from the position of the pickup
												mRotateSpeed;

	U8											mShuffleFlags;

	Btl::Vector<int, Btl::VectorParameters_Array<MAX_VEHICLES> > mVehicleCollectedLap;

	float										m_PickupSpecialFXTime;
	float										m_BeginTime;
	float										m_EndTime;
	float										m_Duration;

	Game::C_GameWorld*							m_pGameWorld;
	Gravity::C_Trigger*							m_pTriggerObject;

	int											mLastPickedUpPos;
	
	bool										m_Selected;

	bool										m_OneShot;
	bool										m_Respawn;
	bool										m_bVisible;
	bool										m_PickupOnLapZero;

	int											m_ActivationLap;
	float										m_ActivationTimer;
	bool										m_PickedUpByLocalPlayer;
	int											m_TimesPickedUpByLocalPlayer;
	U32											m_UniqueDropID;
	F32											m_OneShotTime;

	S8											m_InsideCameraTriggerRegion;

	U32											m_NumSuccessiveCollectionsBySameVehicle;
	S8											m_LastVehicleIdxCollectedBy; 
	bool										m_CampingPreventionEnabled;

protected:
	Maths::HMat43								GetWorldMatrix() const { return mWorldMatrix; }
	void										SetWorldMatrix(Maths::HMat43 mat) { mWorldMatrix = mat; }
	void										SetWorldPosition(Maths::HVec3 pos) { mWorldMatrix.AW(pos); }
	const Game::C_GameWorld*					GetGameWorldPtr() const { return m_pGameWorld; }
	float										GetOneShotTimer() const { return m_OneShotTime; }

	int											GetActiveLevel();

	float										mRotationSpeed;
	bool										mIsNetwork;
	bool										mFixedType;

	Game::C_RaceLogicLayer*						mpRaceLogicLayer;
	Game::C_LevelState*							mpLevelState;
	Game::Actors::RaceLogicCore					*m_pRLC;

	C_VisibleDetect*							m_pVisDetect;

	SpecialFX::I_SpecialFXInstance*				m_pPickupSpecialFX;

	E_PICKUP_TYPE								mPickupType,
												mNextPickupType;

	Btl::Vector< float, Btl::VectorParameters_Array<MAX_VEHICLES> >								mVehicleTimeToAllowPickup;
	Btl::Vector< VehiclePickupPosition, Btl::VectorParameters_Array<MAX_VEHICLES_COLLIDING> >	mVehiclesCollidedThisFrame;
};

}; //namespace Actors

#endif //PICKUPPOSITIONACTOR_H
