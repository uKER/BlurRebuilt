#ifndef VLC_HANDOFGOD_H
#define VLC_HANDOFGOD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

struct RLHandOfGodParameters;

namespace Game
{
	namespace Actors
	{
		class RaceLogicCore;
	}
	class C_LevelState;
}

namespace Components
{

class BaseMessageHandler;

class I_VehicleComponent_HandOfGod_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_HandOfGod_CreateInfo() : mViewportIndex(-1)
	{}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_HandOfGod;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_HandOfGod;}

	Actor::C_ActorHandle	mParentActorHandle;
	int						mVehicleIndex;
	int						mViewportIndex;
	bool					mContinueDespiteNoSplineData;
	RLHandOfGodParameters*  mpHoGParams;
};

class C_VehicleComponent_HandOfGod : public I_GameActorComponent
{
public:

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_HandOfGod;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_HandOfGod;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_HandOfGod;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_HandOfGod;} // from Component::I_GameComponent

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

	enum E_HandOfGodResetCriteria
	{
		E_HandOfGodResetCriteria_Crashed				= (1 << 0),
		E_HandOfGodResetCriteria_Tumbled				= (1 << 1),
		E_HandOfGodResetCriteria_Stuck					= (1 << 2),
		E_HandOfGodResetCriteria_StuckOffGround			= (1 << 3),
		E_HandOfGodResetCriteria_OffSpline				= (1 << 4),
		E_HandOfGodResetCriteria_NotFacingForwards		= (1 << 5),
		E_HandOfGodResetCriteria_Flipped				= (1 << 6),

		E_HandOfGodResetCriteria_All = 	E_HandOfGodResetCriteria_Crashed | E_HandOfGodResetCriteria_Tumbled
										| E_HandOfGodResetCriteria_Stuck | E_HandOfGodResetCriteria_StuckOffGround | E_HandOfGodResetCriteria_OffSpline
										| E_HandOfGodResetCriteria_NotFacingForwards | E_HandOfGodResetCriteria_Flipped,
	};

	void											Update();

	bool											IsManualHOGResetRequired() const;
	void											SetManualHOGResetRequired(const bool required);

	static bool										ResetVehicleOnTrackStatic( Game::Actors::RaceLogicCore* pRLC, Game::Actors::C_VehicleLogicActor* pActor, Maths::HVec3 newPosition, Maths::HVec3 direction, F32 speed, F32 ghostTime, bool isManual );
protected:
	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }


private:

	void											ResetVehicleOnTrack(bool isManual = false, bool atSpeed = false, bool useOverride = false);
	void											ResetVehicleAtVehiclePosition( U8 resetIndex, bool isManual = false, bool atSpeed = false, bool useOverride = false);

	bool											HandleResetForceZone( const Messaging::C_MessageBase * pMessage );
	void											HandleResetAfterWrecked();
	void											CheckForMurderOrSuicide();
	

	void											CheckReset();
	void											CheckResetZone();
	void											CheckResetWarning();

	static const float								GetResetDelayTime();
	void											GetBestStartPosition(Maths::HVec3& position, Maths::HVec3& direction, float& speed) const;

	void											StartHoGCam();
	void											StopHoGCam();
	void											InitiateReset(bool ResetFromWrecked = false);
	void											CancelPerkEffects();

	void											SendSuicideMessageToUI();

	const float										GetGhostTime() const;

	const bool										IsAIDataAvailable() const;

	RLHandOfGodParameters*							mpHoGParams;
	BaseMessageHandler*								mMessageHandler;
	Actor::C_ActorHandle							mActorHandle;
	int												mVehicleIndex;
	int												mViewportIndex;

	U8												mResetCriteria;
	bool											mContinueDespiteNoSplineData;
	E_HandOfGodResetType							mResetType;

	int												mForcedResetZoneBuffered;
	bool											mForcedResetCameraTrackingBuffered;

	float											mFlippedTime;
	float											mOffSplineTime;
	bool											mForcedResetCameraTracking;
	float											mTimeSinceLastReset;

	bool											mEnabled;
	bool											mVehicleImpactThisFrame;
	bool											mDoingHOGResetCam;
	bool											mResetHoGCam;

	float											mHandOfGodResetWarningTimer;
	bool											mNeedsHandOfGodResetWarning;

	float											mResetZoneTimer;
	bool											mResetZoneActive;
	int												mResetZoneId;
	int												mResetIndivId;

	int												mForcedResetZone;

	bool											mCollidedRecently;
	float											mCollidedRecentlyTimer;

	float											mHitByPerkRecentlyTimer;

	bool											mFacingForwardDelayGranted;
	float											mFacingForwardDelay;

	float											mTumbledTime;
	float											mFacingForwardsTimer;

	bool											mStuckRecently;
	float											mStuckTime;

	Maths::HVec3									mResetPosition;
	Maths::HVec3									mResetDirection;
	float											mResetSpeed;

	float											mCurrentlyResettingTimer;
	bool											mCurrentlyResetting;
	bool											mHasBeenReset;
	bool											mCurrentlyBoosting;
	bool											mBeingResetAfterBeingWrecked;

	float											mRidiculousSlappedTime;
	float											mSlappedTime;

	float											mResetDelayTimer;

	bool											mDiedBySuicide;

	// Cache
	bool											mIsAIControlled;	
	Game::Actors::RaceLogicCore*					mpRLC;
	Game::C_LevelState*								mpLevelState;

	// Sometimes we want to manually put a vehicle back on track when it is time to do so at a special position (e.g. destruction)
	bool											mAutomaticHOGPlacementEnabled;
	bool											mManualHOGResetRequired;
};

}

#endif //#ifndef VLC_HANDOFGOD_H
