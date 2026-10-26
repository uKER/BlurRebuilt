#ifndef VLC_NETWORK_DAMAGE_H
#define VLC_NETWORK_DAMAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\GameActors\Components\VehicleLogic\VLC_Damage.h>

namespace Components
{

class BaseMessageHandler;

class I_VehicleComponent_MP_Network_Damage_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_MP_Network_Damage_CreateInfo() {}

	static Component::C_ComponentTypeId			ClassType() {return Components::E_VehicleComponent_MP_Network_Damage;}
	virtual Component::C_ComponentTypeId		GetComponentTypeID() const {return Components::E_VehicleComponent_MP_Network_Damage;}

	Actor::C_ActorHandle	mParentActorHandle;
	int mVehicleIndex;
};

class C_VehicleComponent_MP_Network_Damage : public C_VehicleComponent_Damage
{
public:

	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_VehicleComponent_Damage;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_VehicleComponent_Damage;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId	ClassType() {return Components::E_VehicleComponent_MP_Network_Damage;}
	virtual Component::C_ComponentInterfaceId	GetComponentTypeID() const {return Components::E_VehicleComponent_MP_Network_Damage;} // from Component::I_GameComponent

	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);

protected:
	virtual bool								CreateImpl();
	void										ApplyDamage(int damagePoints, E_IMPACT_LEVEL impactLevel = E_IMPACT_LEVEL_NONE) { return; }
	void										RepairVehicle(int repairAmount = 0);
	void										SetCurrentHealth(int p_Health);

	E_HEALTH_LEVEL								mLastKnownHealthLevel;
};

}

#endif // VLC_NETWORK_DAMAGE_H