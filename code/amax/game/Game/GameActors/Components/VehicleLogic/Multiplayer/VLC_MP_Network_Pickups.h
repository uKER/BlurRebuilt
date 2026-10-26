#ifndef VLC_MP_NETWORK_PICKUPS_H
#define VLC_MP_NETWORK_PICKUPS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>

namespace Components
{

class BaseMessageHandler;

class I_VehicleComponent_MP_Network_Pickups_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_MP_Network_Pickups_CreateInfo() {};

	static Component::C_ComponentTypeId			ClassType() { return Components::E_VehicleComponent_MP_Network_Pickups; }
	virtual Component::C_ComponentTypeId		GetComponentTypeID() const { return Components::E_VehicleComponent_MP_Network_Pickups; }

	Game::C_RaceLogicLayer						*mRaceLogicLayerPtr;
	int											mVehicleIndex;
};

class C_VehicleComponent_MP_Network_Pickups : public C_VehicleComponent_Pickups
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_VehicleComponent_Pickups;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_VehicleComponent_Pickups;} // from Component::I_GameComponent

	static Component::C_ComponentTypeId			ClassType() { return Components::E_VehicleComponent_MP_Network_Pickups; }
	virtual Component::C_ComponentTypeId		GetComponentTypeID() const { return Components::E_VehicleComponent_MP_Network_Pickups; }

	bool										OnMessage(const Messaging::C_MessageBase *pMessage);

	void										AttemptHitByPickup(E_PICKUP_TYPE pickupType,E_PICKUP_SUB_TYPE pickupSubType, E_PICKUP_HIT_TYPE hitType, int firingIdx, E_TARGETTING_DIRECTION hitDir, U32 pickupId, bool noPhysical, Game::PickupHitInfo hitInfo, U32 groupId );

protected:
	bool										CreateImpl();
	void										DropAndRemoveUsedPickup(E_PICKUP_TYPE pickup);

};


} // namespace Components

#endif // VLC_MP_NETWORK_PICKUPS_H
