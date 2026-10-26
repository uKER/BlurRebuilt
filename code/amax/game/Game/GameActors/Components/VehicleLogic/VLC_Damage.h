#ifndef VLC_DAMAGE_H
#define VLC_DAMAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <GameLogic/Messaging/GameLogicMessages.h>	// [Blur2PC]
#include <Game/GameActors/Components/GameComponents.h>

#include <GameLogic/RaceLogicCore/Data/RLEnums.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include<vehicle\damage\CollisionInfo.h>

// Forward decls
namespace Game
{
	class C_ImpactMsg;
	class C_RaceLogicLayer;
	class C_LevelStateMultiplayerVehicleAbilities;

	namespace Actors
	{
		class RaceLogicCore;
	};

	class C_CachedMPAbility
	{
	public:

		C_CachedMPAbility(	Game::C_LevelStateMultiplayerVehicleAbilities* p_pAbilities, UI::E_MultiplayerVehicleAbilities p_Type, 
							bool p_IsFiring, bool p_NotLocal, U8 p_VehicleIndex, U8 p_VictimIndex);

		void SetFiring();

	private:

		Game::C_LevelStateMultiplayerVehicleAbilities*	m_pAbilities;
		UI::E_MultiplayerVehicleAbilities				m_Type;
		bool											m_IsFiring;
		bool											m_NotLocal;
		U8												m_VehicleIndex;
		U8												m_VictimIndex;
	};
}

namespace AmaxCam
{
	enum E_VFCamera;
}

class C_VehicleLogicMsg_HitByPerkMsg;

namespace Components
{
	class BaseMessageHandler;

	class I_VehicleComponent_Damage_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_Damage_CreateInfo() {}

		static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Damage;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Damage;}

		Actor::C_ActorHandle	mParentActorHandle;
		int mVehicleIndex;
	};

	class C_VehicleComponent_Damage : public I_GameActorComponent
	{
	public:

		static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Damage;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Damage;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Damage;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Damage;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		virtual void									Update();

		float											GetTimeTillLastGaspForceFire() const { return mLastGaspModData.mRemainingTimeTillForceFire; }
		void											FireLastGasp(bool damagePlayer = true);
		bool											GetHasLastGaspAppliedWreckedImpulse() const;
		bool											WasLastDamageCausedByLastGasp() { return  mLastDamageWasInflictedLastGasp;}  // note: non self-inflicted damage

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }
		void											Initialise();

		virtual void									RepairVehicle(int repairAmount = 0);

		void											HandlePickupDamage(E_PICKUP_HIT_TYPE p_PickupHitType, E_PICKUP_TYPE p_PickupType);
		void											PopulateDamageFilter(Vehicle::C_DamageFilter& filter, Maths::HF32 impulse);
		void											VictimDamaged(int damage, int aggressiveVehicleIdx);
		virtual void									ApplyDamage(int damagePoints, int aggressiveVehicleIdx);

		void											GetDamagePointsFromImpact(Game::C_GameplayImpactMsg& gameplayImpactMsg);

		const int										GetDamagePointsFromImpactLevel(const RLDamageParameters* pRLDamageParameters, const E_IMPACT_LEVEL eImpactLevel) const;
		const E_IMPACT_LEVEL							GetImpactLevelFromDamagePoints(const RLDamageParameters* pRLDamageParameters, const int damagePoints) const;

		Maths::HMat33									CalculateImpactShakeTransform(Game::Actors::C_VehicleActor* vehActor) const;
		bool											IsFrontalImpact(const Game::C_ImpactMsg* pMsg, const Game::Actors::C_VehicleActor* vehActor) const;

		int												DetermineAggressiveVehicle(Game::Actors::C_VehicleActor* pActorA, Game::Actors::C_VehicleActor* pActorB, Game::C_ImpactMsg* pMsg);
		int												DetermineOtherVehicle(Game::Actors::C_VehicleActor* pActorA, Game::Actors::C_VehicleActor* pActorB);

		void											SetCurrentHealthLevel(const bool ignoreWreckedScreenEffects = false, const bool forceReApply = false);
		const RLHealthLevelZone*						GetCurrentHealthZone();

		void											AdjustHealth(int healthPoints);

		void											GetCollisionActorName(const Actor::C_Actor* pActor, Util::C_String& string) const;

		void											SendGameplayImpactMessageToComponents(const Game::C_GameplayImpactMsg& gameplayImpactMsg,Game::Actors::C_VehicleLogicActor* pActor);

		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		int												mVehicleIndex;

		bool											mEnabled;
		float											mWreckedTimer;
		Maths::HVec3									mPrevLinearVelocity;

		void											CacheMultiplayerAbilities(Game::C_LevelStateMultiplayerVehicleAbilities*);

		// Access to the multiplayer abilities of the vehicle that owns this component
		Game::C_LevelStateMultiplayerVehicleAbilities*   mpMultiplayerAbilities;

	//Functions
	private:

		void											HandleCancelWreckedTimer();
		
		void											CheckPowerUpDamageModifiers(const int firingVehicleIndex, int& damage, const E_PICKUP_TYPE pickupType);
		void											CheckImpactDamageModifiers(const int otherVehicleIndex, const int firingVehicleIndex, int& damage, const E_IMPACT_LEVEL impactLevel);
		void											CheckGameplayModifiers(const enum E_PICKUP_TYPE ePickupType, const enum E_PICKUP_SUB_TYPE ePickupSubType, const int firingVehicleIndex);
		bool											CanPickupTypeCauseScrambler(const E_PICKUP_TYPE ePickupType, const enum E_PICKUP_SUB_TYPE ePickupSubType) const;
		
		const bool										ShouldWeBeApplyingDamageMultipliers() const;

		void											ApplyDamageMultiplier(int& damage) const;	

		void											HandleImpact(Game::C_ImpactMsg *pImpactMsg);
		void											HandleImpactCameraEffects(const Game::C_ImpactMsg *pImpactMsg, Game::Actors::C_VehicleActor *pGraphicalActor);
		void											CheckJumpTheGun();
		void											CheckVampiricWreck();
		void											ActivateLastGasp();
		void											CheckBatteringRam(	Game::C_GameplayImpactMsg& gameplayImpactMsg, 
																			const Game::Actors::C_VehicleActor *pVehicleActorA, 
																			const Game::Actors::C_VehicleActor *pVehicleActorB);

		const int										GetAFractionOfAHealthPipInDamagePoints(const float healthPipFraction) const;
		

		void											ApplyVehicleToVehicleBatteringRamPhysicsEffect(const U8 victimVehicleIndex,const U8 aggressorVehicleIndex);
		E_TARGETTING_DIRECTION							GetBatteringRamHitDirection(Game::Actors::C_VehicleLogicActor* victimActor, Game::Actors::C_VehicleLogicActor* aggressiveActor);
		void											GetDamageInflicted(const C_VehicleLogicMsg_HitByPerkMsg* pHitByPerkMessage, int& damage);
		void											HandleDelayedWreckRespawn();
		void											StartBatteringRamPhysicsEffect(Game::Actors::C_VehicleActor* pVictimGraphicalActor, 
																						const enum E_TARGETTING_DIRECTION eHitDir,
																						const bool heavyHit) const;
		void											CheckActivateBatteringRam();
		const bool										ShouldBatteringRamFire(	const Game::Actors::C_VehicleActor *pMyVehicleActor,
																				const Game::Actors::C_VehicleActor *pOtherVehicleActor) const;

		const bool										ValidVehicleToVehicleCollision(	const Game::Actors::C_VehicleActor *pVehicleActorA,
																						const Game::Actors::C_VehicleActor *pVehicleActorB) const;


#ifndef CFG_GOLD
		void											UpdateDebugFunctionality();

		void											LogImpact(const Actor::C_Actor* pActorA, const Actor::C_Actor* pActorB, const Game::C_ImpactMsg* pImpactMsg, const int damagePoints, const F32 raceTime) const;
#endif //CFG_RELEASE

	//Variables
	private:

		// Record any stats needed for damage
		void  UpdateDamageStats(); 

		int   mBlindFireChainLength; 
		float mBlindFireChainTimer; 
		float mBlindFireChainTimerIncrement;
		int	  mNumBlindFireRequiredToCauseScrambler; // Cached on initialise from sorcery
		float mScramblerPowerupBayDisableTime; 
		bool  mScramblerModBoltsStack;
		float mScramblerBoltDisableTime;


		int	mTotalBlindFireImpacts;

		float mCooldown;
		bool mResetInProgress; 

		// Cache
		Game::C_RaceLogicLayer*			mpRaceLogicLayer;
		Game::Actors::RaceLogicCore*	mpRLC;
		Game::C_LevelState*				mpLevelState;


	private:
		// Private struct decl
		struct LastGaspModData 
		{
			void  Initialise(Game::C_LevelState* pLevelState, const U8 vehicleIndex);
			void  SetupTimers();
			void UpdateTimers( bool& respawn, bool& fireLastGasp, const U8 vehicleIndex ); //returns true if time to call HandleWreckedDelayedRespawn()
			void CheckSoundEffects( const float frameTime );

			bool  mHasLastGaspModEquipped;
			bool  mHasJumpTheGunModEquipped;
			bool  mHealthReplenishedThisLife;
			float mRemainingTimeTillForceFire; 
			float mNextSoundEffectIn;
			bool  mForceFireTimer;
			bool  mExplosionFiredThisLife;
			S8	  mVehicleCausingLastGaspID; 
			float mWreckedSequenceDelay;
			bool  mWreckedSequenceDelayed;
			float mWreckedSequenceDelayTimer;
			bool  mHasAppliedWreckedImpulse;
			float mInitialDisableTimer;  // Time After entering last gasp mode that car can be detonated.
			float mDamageCooldownTimer;  // Time after entering last gasp mode that can can take damage again.
			float mTimeSinceLastDamaged; 
			AmaxCam::E_VFCamera mStartingCamType; // record wether player in bumper or chase when last gasp starts, so we can force chase, then revert to previous settings when done. 

		private:
			// Remote vehicles run simplified update logic for last gasp, they just need to update some visual fx
			void UpdateTimers_Local( bool& respawn, bool& fireLastGasp, const U8 vehicleIndex, const float frameTime );
			void UpdateTimers_Remote( const U8 vehicleIndex, const float frameTime);

		};

		// And an actual object of this to use :)
		LastGaspModData mLastGaspModData;
		bool mLastDamageWasInflictedLastGasp;

		struct BatteringRamFakePhysicsModData
		{
			Maths::HVec3								mImpulseScale;
			Maths::HVec3								mKickForce;
			Maths::HVec3								mConstantKickForce;
		};

		struct BatteringRamModData
		{
			void Initialise(const MultiplayerConfig* pConfig);
			void CopyToFakePhysics();

			float mTargettingCone;
			float mMinimumImpactSpeedInMPH;
			float mCoolDownTimer;

			BatteringRamFakePhysicsModData	mLightHitSettings;
			BatteringRamFakePhysicsModData	mHeavyHitSettings;
		};

		BatteringRamModData mBatteringRamModData;
		float mBatteringRamCoolDownTimer;

		// Some handy camera manipulation functions
		void ApplyLastGaspCameraManipulation();
		void RemoveLastGaspCameraManipulation();
		void SwitchLastGaspCameraType(const AmaxCam::E_VFCamera toType);	// Toggle between cam types (e.g. chase and bumpercam_
		void SetPlayerCameraLock(const bool isLocked);						// Lock/unlock player's ability to select camera
		void HideLowHealthScreenEffect();

		Btl::Vector<Game::C_CachedMPAbility> m_CachedAbilities;

	public:
		LastGaspModData*				GetLastGaspModData(){return &mLastGaspModData;}
	};

}

#endif //#ifndef VLC_DAMAGE_H
