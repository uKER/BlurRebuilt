#ifndef VLC_PICKUPS_TARGETTING_H
#define VLC_PICKUPS_TARGETTING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <game\GameActors\Components\GameComponents.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

namespace Components
{
	class BaseMessageHandler;

	class I_VehicleComponent_Pickup_Targetting_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_Pickup_Targetting_CreateInfo() {}

		static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Pickups_Targetting;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Pickups_Targetting;}

		Actor::C_ActorHandle	mParentActorHandle;
		Game::C_RaceLogicLayer*	mpRaceLogicLayer;
		int						mVehicleIndex;
	};

	class C_VehicleComponent_Pickups_Targetting : public I_GameActorComponent
	{
	public:

		static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Pickups_Targetting;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Pickups_Targetting;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Pickups_Targetting;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Pickups_Targetting;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		void											Update();

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }


	private:
		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		int												mVehicleIndex;
		Game::C_RaceLogicLayer*							mpRaceLogicLayer;
	};

}

#endif //#ifndef VLC_PICKUPS_TARGETTING_H