#ifndef PICKUP_SUPER_SHIELD_H
#define PICKUP_SUPER_SHIELD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>


struct RLSuperShieldBalancingData;

namespace Game
{
	class C_PickupSuperShield : public C_PickupBase
	{
	public:
		C_PickupSuperShield(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual bool							Update();
		void									DebugDraw();
		virtual void							PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);

	//Functions
	private:
		void									InitialiseSFX(	const Actors::C_VehicleActor* pGraphicalActor, 
																const Actors::C_VehicleLogicActor* pLogicActor, 
																const RLPickupType *pPickup, 
																const RLPickupLevel* pPickupLevel ) const;

		void									ApplyAssortedBoosts(const Actors::C_VehicleActor* pGraphicalActor) const;
		const float								GetImpulseMagnitude(const Game::C_LevelState& levelState) const;

		void									StoreOriginalMass(const Gravity::C_Body* pBody);
		void									IncreaseMass(Gravity::C_Body* pBody, const float newMass) const;
		void									RevertMass(Gravity::C_Body* pBody) const;

		void									GetSteeringAndThrottleInputs(float& steeringInput, float& throttleInput) const;
		const float								NormaliseInput(const float input) const;


		void									ApplySteeringBoost(const Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody& rigidBody, const float steeringInput) const;
		void									ApplyAccelerationOrDecelerationBoost(const Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody& rigidBody, const float throttleInput) const;
		void									ApplyDownForceBoost(const Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody& rigidBody) const;

		void									Restart(Gravity::C_Body* pBody);


	private:
		float									mOriginalMass;
	
		RLSuperShieldBalancingData				*m_pBalancingData;
	};
}

#endif //PICKUP_DOUBLE_BOMB_H