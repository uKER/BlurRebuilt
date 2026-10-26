#ifndef VLC_MP_IDLEKICKCONTROLLER_H
#define VLC_MP_IDLEKICKCONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/GameActors/Components/GameComponents.h>

namespace Game
{
	class C_RaceLogicLayer;
	class C_LevelState;

	namespace Actors
	{
		class C_VehicleLogicActor;
		class RaceLogicCore;
	}
}

namespace Components
{

class BaseMessageHandler;

class I_VehicleComponent_MP_IdleKickController_CreateInfo: public I_GameComponentCreator
{
public:
	I_VehicleComponent_MP_IdleKickController_CreateInfo() {}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_MP_IdleKickController;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_MP_IdleKickController;}

	Game::C_RaceLogicLayer		*mRaceLogicLayerPtr;
	int							mVehicleIndex;
};

class C_VehicleComponent_MP_IdleKickController : public I_GameActorComponent
{
public:

	C_VehicleComponent_MP_IdleKickController();
	~C_VehicleComponent_MP_IdleKickController();

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_MP_IdleKickController;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_MP_IdleKickController;} // from Component::I_GameComponent


	static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_MP_IdleKickController;}
	virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_MP_IdleKickController;} // from Component::I_GameComponent

	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

protected:
	virtual bool									CreateImpl();
	void											SetHandler(BaseMessageHandler* pHandler);

	void											Update();

	void											KickPlayerFromGame(); 

	bool											CheckForControllerInput(const U8 controlNum) const;

	F32												mDeltaTime;
	F32												mIdleTimer;

	BaseMessageHandler*								mMessageHandler;
	int												mVehicleIndex;
	Game::C_RaceLogicLayer*							mRaceLogicLayerPtr;
	Game::Actors::RaceLogicCore*					mRaceLogicCorePtr; 
	Game::C_NetGameVars*							mNetGameVars;

	bool											mRaceStarted;
	bool											mPlayerImmuneToKick; 
	float											mListenDelayTimer;
	float											mTotalTimeListening;

	// Ghosting due to inactivity
	bool											mPlayerCurrentlyGhosted;
	F32												mGhostInactiveTimeLimit;
	F32												mTimeSinceLastControllerInput;

	void											HandlePlayerGhosted(Game::Actors::C_VehicleLogicActor* pLogicActor);
	void											HandlePlayerNotGhosted(Game::Actors::C_VehicleLogicActor* pLogicActor);

private:
	struct IdleKickControllerParameters
	{
		void										Initialise()
		{
			mMotorMashTimeout = mNonMotorMashTimeout = 0.f;
		}
		F32											mMotorMashTimeout;
		F32											mNonMotorMashTimeout;
	};

	void											CacheParameters();

#ifndef CFG_GOLD
	void											UpdateDebugFunctionality();
#endif

};

} // namespace Components


#endif // VLC_MP_IDLEKICKCONTROLLER_H
