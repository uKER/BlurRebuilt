#ifndef VLC_MP_PICKUPS_AI_H
#define VLC_MP_PICKUPS_AI_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups_AI.h>

namespace Game { class C_RaceLogicLayer; }

namespace Components
{
class BaseMessageHandler;

class I_VehicleComponent_MP_Pickup_AI_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_MP_Pickup_AI_CreateInfo() {}

	static Component::C_ComponentTypeId		ClassType() {return E_VehicleComponent_MP_Pickups_AI;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_VehicleComponent_MP_Pickups_AI;}

	Actor::C_ActorHandle	mParentActorHandle;
	Game::C_RaceLogicLayer*	mpRaceLogicLayer;
	int						mVehicleIndex;
};

class C_VehicleComponent_MP_Pickups_AI : public C_VehicleComponent_Pickups_AI
{
public:

	static Component::C_ComponentInterfaceId	InterfaceType() {return E_IF_VehicleComponent_Pickups_AI;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return E_IF_VehicleComponent_Pickups_AI;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId	ClassType() {return E_VehicleComponent_MP_Pickups_AI;}
	virtual Component::C_ComponentInterfaceId	GetComponentTypeID() const {return E_VehicleComponent_MP_Pickups_AI;} // from Component::I_GameComponent

	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);

	void										Update();

protected:
	virtual bool								CreateImpl();
	void										SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

	bool										InRangeOfHumanCheck();
	bool										PlayerHitRecentlyCheck();
	bool										CheckLevel2PickupTriggers(E_PICKUP_TYPE pickupType);
	bool										EvaluatePickupChange(E_PICKUP_TYPE pickupType);

	bool										IsAiControlled();
};

}	//	namespace Components

#endif //#ifndef VLC_MP_PICKUPS_AI_H
