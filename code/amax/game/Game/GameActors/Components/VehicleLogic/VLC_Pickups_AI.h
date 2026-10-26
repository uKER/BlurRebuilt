#ifndef VLC_PICKUPS_AI_H
#define VLC_PICKUPS_AI_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>
#include <GameLogic/RaceLogicCore/Data/RLEnums.h>
#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <misc_shared\Curve\HermiteCurve.h>
#include "System/misc/Random.h"

namespace Game { class C_RaceLogicLayer; }

namespace Components
{
	class BaseMessageHandler;

	class I_VehicleComponent_Pickup_AI_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_Pickup_AI_CreateInfo() {}

		static Component::C_ComponentTypeId		ClassType() {return E_VehicleComponent_Pickups_AI;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_VehicleComponent_Pickups_AI;}

		Actor::C_ActorHandle	mParentActorHandle;
		Game::C_RaceLogicLayer*	mpRaceLogicLayer;
		int						mVehicleIndex;
	};

	class C_VehicleComponent_Pickups_AI : public I_GameActorComponent
	{
	public:

		static Component::C_ComponentInterfaceId		InterfaceType() {return E_IF_VehicleComponent_Pickups_AI;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return E_IF_VehicleComponent_Pickups_AI;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return E_VehicleComponent_Pickups_AI;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return E_VehicleComponent_Pickups_AI;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);
		
		void											EvaluatePickedUpPickup(E_PICKUP_TYPE type);

		virtual void									Update();

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }
		void											Initialise();

		float											GetRandom(float a, float b);

		void											DecrementTimers();
		void											SetTimers();

		void											GiftPickups();
		void											DropPickups();
		void											CyclePickups();

		void											SwitchToPickup(int idx);
		void											SwitchToPickup(E_PICKUP_TYPE type);

		virtual bool									EvaluatePickupChange(E_PICKUP_TYPE pickupType);
		void											EvaluatePickupUse();
		void											EvaluateShieldUse();

		bool											CheckPickupTriggers(E_PICKUP_TYPE pickupType);
		virtual bool									CheckLevel2PickupTriggers(E_PICKUP_TYPE pickupType);
		E_PICKUP_TYPE									GetLevel2Perk(E_PICKUP_TYPE type);

		bool											CheckPickupCanFireOnTarget(int idx, E_PICKUP_TYPE type);
		bool											CheckCanDropMine(int idx);
		bool											ShockInRangeOfHuman();


		// Gav: These are the player checks, so you want to change these.
		virtual bool									InRangeOfHumanCheck();
		virtual bool									PlayerHitRecentlyCheck();

		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		int												mVehicleIndex;
		Game::C_RaceLogicLayer*							mpRaceLogicLayer;

		Util::C_Random									mRandGenerator;
		bool											mEnabled;

		float											mEvaluateTime;
		float											mShieldEvaluateTime;
		float											mGrantTime;

		float											mInDangerTime;
		float											mRecentlyInDangerTime;

		float											mTimer_HitWithPerkDisableTime;
		float											mTimer_DropPickup;
		float											mTimer_PerkAIDisable;
		float											mTimer_PickupSwitching;

		Btl::Vector<Curve::C_HermiteCurve>				mAccuracyCurves;
		Curve::C_HermiteCurve							mShieldedCurve;
		int												mCurrCurveIdx;
	};

}

#endif //#ifndef VLC_PICKUPS_AI_H
