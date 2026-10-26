#ifndef VLC_FAN_POWER_H
#define VLC_FAN_POWER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/GameActors/Vehicles/VehiclePerformanceModifiers.h>

#include <GameLogic\Layers\RaceLogicLayer.h>

// Forward decls
namespace Game
{
	class C_LevelStateMultiplayerVehicleAbilities;
}

namespace Components
{

class BaseMessageHandler;

class I_VehicleComponent_Fan_Power_CreateInfo : public I_GameComponentCreator
{
public:
	I_VehicleComponent_Fan_Power_CreateInfo() {}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Fan_Power;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Fan_Power;}

	Game::C_RaceLogicLayer		*mRaceLogicLayerPtr;
	int						mVehicleIndex;
};

class C_VehicleComponent_Fan_Power : public I_GameActorComponent
{
public:

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Fan_Power;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Fan_Power;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Fan_Power;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Fan_Power;} // from Component::I_GameComponent

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

	void											Update();

protected:
	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

	void											RepelNearbyVehicles();

	float											mDeltaTime;

	BaseMessageHandler*								mMessageHandler;

	int												mVehicleIndex;

	Game::Actors::VehiclePerformanceModifiers		mPerformanceModifiers;
	Game::C_RaceLogicLayer							*mRaceLogicLayerPtr;

	float											mUsingShieldTimer;

	float											mDisableShield;
	bool											mRaceStarted;

	float											mLastResponseTime;

	int												mCurrentFans;
	int												mFansRequired;

private:
	// Access to the multiplayer abilities of the vehicle that owns this component
	Game::C_LevelStateMultiplayerVehicleAbilities*   mpMultiplayerAbilities;
};

}

#endif //#ifndef VLC_FAN_POWER_H
