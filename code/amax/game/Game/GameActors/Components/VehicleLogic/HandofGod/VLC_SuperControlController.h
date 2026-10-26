#ifndef VLC_SUPERCONTROLCONTROLLER_H
#define VLC_SUPERCONTROLCONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

namespace Game
{
	class C_LevelState;

	namespace Actors
	{
		class C_VehicleActor;
		class C_VehicleLogicActor;
	};
};


namespace Components
{
	class BaseMessageHandler;

	class I_VehicleComponent_SuperControlController_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_SuperControlController_CreateInfo() {}

		static Component::C_ComponentTypeId				ClassType() {return Components::E_VehicleComponent_SuperControlController;}
		virtual Component::C_ComponentTypeId			GetComponentTypeID() const {return Components::E_VehicleComponent_SuperControlController;}

		Actor::C_ActorHandle							mParentActorHandle;
		int												mVehicleIndex;
		Game::C_LevelState								*m_pLevelState;
		Game::C_RaceLogicLayer							*m_pRaceLogicLayer;
		Game::Actors::C_VehicleActor					*m_pGraphicalActor;
		Game::Actors::C_VehicleLogicActor				*m_pLogicActor;
	};

	class C_VehicleComponent_SuperControlController : public I_GameActorComponent
	{
	public:
		static const float MINIMUM_CONTROLLER_INPUT;

		~C_VehicleComponent_SuperControlController();
	
		static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_SuperControlController;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_SuperControlController;} // from Component::I_GameComponent

		static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_SuperControlController;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_SuperControlController;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		void											Update();

	protected:
		bool											InitialiseImpl() OVERRIDE;
		bool											CreateImpl() OVERRIDE;
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

	private:
		void											GetSteeringThrottleAndHandbrakeInputs(float& steeringInput, float& throttleInput, bool& handbrakeInput) const;
		void											GetAccelerationOrDecelerationBoostMultiplier(const float throttleInput, const float speedInMPH, float& boostMultiplier) const;
		const float										GetSpeedInMPH() const;

		const bool 										ApplySteeringBoost(const Game::Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody* pRigidBody, const float throttleInput, const float steeringInput, const bool handbrakeInput, const float absSpeedInMPH) const;
		const bool 										ApplyAccelerationOrDecelerationBoost(const Game::Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody* pRigidBody, const float throttleInput, const bool handbrakeInput, const float speedInMPH) const;
		const bool 										ApplyDownForceBoost(const Game::Actors::C_VehicleActor* pGraphicalActor, SharedPhysics::C_RigidBody* pRigidBody) const;

		const bool										ShouldWeBeApplyingAccelerationOrDecelerationBoost(const bool handbrakeInput) const;
		const bool										ShouldWeBeApplyingSteeringBoost(const float throttleInput, const float absSpeedInMPH) const;

		static const float								NormaliseInput(const float input);

	//Variables
	private:
		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		int												mVehicleIndex;
		int												mControllerIndex;

		bool											mRaceStarted;

		Game::Actors::C_VehicleActor					*m_pGraphicalActor;
		Game::Actors::C_VehicleLogicActor				*m_pLogicActor;
		SharedPhysics::C_RigidBody						*m_pRigidBody;
	};

}

#endif //#ifndef VLC_SUPERCONTROLCONTROLLER_H
