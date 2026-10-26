#ifndef VLC_PERFORMANCE_MODIFIER_H
#define VLC_PERFORMANCE_MODIFIER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/GameActors/Vehicles/VehiclePerformanceModifiers.h>

namespace Components
{

class BaseMessageHandler;

class I_VehicleComponent_Performance_Modifier_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_Performance_Modifier_CreateInfo() {}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Performance_Modifier;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Performance_Modifier;}

};

class C_VehicleComponent_Performance_Modifier : public I_GameActorComponent
{
public:

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Performance_Modifier;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Performance_Modifier;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Performance_Modifier;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Performance_Modifier;} // from Component::I_GameComponent

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

protected:
	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

	void											Update();

	float											mDeltaTime;

	BaseMessageHandler*								mMessageHandler;

	int												mVehicleIndex;

	Game::Actors::VehiclePerformanceModifiers		mPerformanceModifiers;
};

}

#endif //#ifndef VLC_PERFORMANCE_MODIFIER_H
