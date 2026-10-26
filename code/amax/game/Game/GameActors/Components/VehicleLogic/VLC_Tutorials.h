#ifndef VLC_TUTORIALS_H
#define VLC_TUTORIALS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\GameActors\Components\GameComponents.h>
#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <GameLogic\ActorStudioActors\TriggerRegionActor.h>



namespace Components
{

class BaseMessageHandler;


class I_VehicleComponent_Tutorials_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_Tutorials_CreateInfo() {}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Tutorials;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Tutorials;}

	Game::C_RaceLogicLayer		*mRaceLogicLayerPtr;
	int							mVehicleIndex;
};



class C_VehicleComponent_Tutorials : public I_GameActorComponent
{
public:

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Tutorials;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Tutorials;}
	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Tutorials;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Tutorials;}

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);


protected:
	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

	BaseMessageHandler*								mMessageHandler;
	int												mVehicleIndex;

	Game::C_RaceLogicLayer							*mRaceLogicLayerPtr;
};

}

#endif //#ifndef VLC_TUTORIALS_H
