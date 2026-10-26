#ifndef VLC_AICONTROLLER_H
#define VLC_AICONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

namespace Components
{

	class BaseMessageHandler;

	class I_VehicleComponent_AIController_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_AIController_CreateInfo() {}

		static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_AIController;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_AIController;}

		Actor::C_ActorHandle	mParentActorHandle;
		int mVehicleIndex;
	};

	class C_VehicleComponent_AIController : public I_GameActorComponent
	{
	public:

		static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_AIController;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_AIController;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_AIController;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_AIController;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		void											Update();

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		int												mVehicleIndex;

		bool											mInitialDisable;
		bool											mEnabled;
	};

}

#endif //#ifndef VLC_AICONTROLLER_H
