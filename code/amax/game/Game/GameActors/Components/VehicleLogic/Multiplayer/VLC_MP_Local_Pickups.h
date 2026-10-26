#ifndef VLC_MP_LOCAL_PICKUPS_H
#define VLC_MP_LOCAL_PICKUPS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>

// Forward decls
namespace Game
{
	class C_LevelStateMultiplayerVehicleAbilities;
}

namespace Components
{

class BaseMessageHandler;

class I_VehicleComponent_MP_Local_Pickups_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_MP_Local_Pickups_CreateInfo() {};

	static Component::C_ComponentTypeId			ClassType() { return Components::E_VehicleComponent_MP_Local_Pickups; }
	virtual Component::C_ComponentTypeId		GetComponentTypeID() const { return Components::E_VehicleComponent_MP_Local_Pickups; }

	Game::C_RaceLogicLayer						*mRaceLogicLayerPtr;
	int											mVehicleIndex;
};

class C_VehicleComponent_MP_Local_Pickups : public C_VehicleComponent_Pickups
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_VehicleComponent_Pickups;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_VehicleComponent_Pickups;} // from Component::I_GameComponent

	static Component::C_ComponentTypeId			ClassType() { return Components::E_VehicleComponent_MP_Local_Pickups; }
	virtual Component::C_ComponentTypeId		GetComponentTypeID() const { return Components::E_VehicleComponent_MP_Local_Pickups; }

	bool										OnMessage(const Messaging::C_MessageBase *pMessage);

	bool										StartPerk(E_PICKUP_TYPE p_PickupType, bool forceBackwardFire, bool forceForwardFire, bool forceDoubleFire);

	void										RegisterShieldedPickup(E_PICKUP_TYPE pickupType, int firingIdx, U32 pickupId);
	bool										RegisterHitByPickup(E_PICKUP_TYPE pickupType, E_PICKUP_SUB_TYPE subType, E_PICKUP_HIT_TYPE hitType, int firingIdx, U32 pickupId, E_TARGETTING_DIRECTION hitDir, bool noPhysical, Game::PickupHitInfo hitInfo, U32 groupId);

protected:
	bool										CreateImpl();
	E_PICKUP_TYPE								HandleDropPickupOnTrack(SpawnInformation& information);
	E_PICKUP_TYPE								HandleDropDecoyOnTrack(SpawnInformation& information);

	Btl::List<U32>								m_DecoyUniqueIds; 
};


} // namespace Components

#endif // VLC_MP_LOCAL_PICKUPS_H
