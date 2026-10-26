#ifndef VLC_MP_E3CONTROLLER_H
#define VLC_MP_E3CONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/GameActors/Components/GameComponents.h>

namespace Game
{
	class C_RaceLogicLayer;
}

namespace Components
{

class BaseMessageHandler;

class I_VehicleComponent_E3Controller_CreateInfo: public I_GameComponentCreator
{
public:
	I_VehicleComponent_E3Controller_CreateInfo() {}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_E3Controller;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_E3Controller;}

	Game::C_RaceLogicLayer		*mRaceLogicLayerPtr;
	int							mVehicleIndex;
};

class C_VehicleComponent_E3Controller : public I_GameActorComponent
{
public:

	~C_VehicleComponent_E3Controller();

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_E3Controller;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_E3Controller;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_E3Controller;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_E3Controller;} // from Component::I_GameComponent

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

protected:
	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

	void											Update();

	F32												mDeltaTime;
	F32												mIdleTimer;

	BaseMessageHandler*								mMessageHandler;
	int												mVehicleIndex;
	Game::C_RaceLogicLayer							*mRaceLogicLayerPtr;

	bool											mRaceStarted;
};






} // namespace Components


#endif // VLC_MP_E3CONTROLLER_H