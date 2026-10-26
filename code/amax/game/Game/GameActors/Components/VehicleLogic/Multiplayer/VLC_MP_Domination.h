#ifndef VLC_MP_DOMINATION_H
#define VLC_MP_DOMINATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <GameLogic\Layers\RaceLogicLayer.h>

#include <GameLogic/RaceLogicCore/Data/RLEnums.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

namespace Components
{

enum E_DominationMode
{
	E_DominationMode_Idle,
	E_DominationMode_Dominating,
	E_DominationMode_Dominated,
	E_DominationMode_Banking,
	E_DominationMode_Lost,
	E_DominationMode_Reset,
};


class BaseMessageHandler;

class I_VehicleComponent_MP_Domination_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_MP_Domination_CreateInfo() {};

	static Component::C_ComponentTypeId			ClassType() { return Components::E_VehicleComponent_MP_Domination; }
	virtual Component::C_ComponentTypeId		GetComponentTypeID() const { return Components::E_VehicleComponent_MP_Domination; }

	Game::C_RaceLogicLayer						*mRaceLogicLayerPtr;
	int											mVehicleIndex;
};

class C_VehicleComponent_MP_Domination : public I_GameActorComponent
{
public:

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Domination;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Domination;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_MP_Domination;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_MP_Domination;} // from Component::I_GameComponent

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

private:

	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

	void											Update();
	void											CrossedLapLine();

	BaseMessageHandler*								mMessageHandler;
	int												mVehicleIndex;
	Game::C_RaceLogicLayer							*mRaceLogicLayerPtr;

	E_DominationMode								m_CurrentMode;

	F32												m_DeltaTime;		//	Time since last update
	bool											mRaceStarted;

	////	OLD DOMINATION
	//U8												m_DominatingVehicleLocalIndex;	//	Index of last vehicle to dominate
	//F32												m_DominationDisplayTimer;	//	Remaining time during which we display the Dominator's name
	//F32												m_DominationFans;
	//F32												m_DisplayFansValue;	//	We might want to display a value of fans other than the current level (how much you just banked etc)

	U8												m_DominatingVehicleLocalIndex;	//	Index of last vehicle to dominate
	F32												m_DominationDisplayTimer;	//	Remaining time during which we display the Dominator's name

};

} // namespace Components



#endif // VLC_MP_DOMINATION_H