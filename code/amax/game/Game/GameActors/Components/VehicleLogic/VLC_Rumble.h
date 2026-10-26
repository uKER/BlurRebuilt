#ifndef VLC_RUMBLE_H
#define VLC_RUMBLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

namespace Game
{
	class C_RaceLogicLayer;
}

namespace Components
{

	class BaseMessageHandler;

	class I_VehicleComponent_Rumble_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_Rumble_CreateInfo() {}

		static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Rumble;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Rumble;}

		Game::C_RaceLogicLayer		*mRaceLogicLayerPtr;
		int							mVehicleIndex;
	};

	class C_VehicleComponent_Rumble : public I_GameActorComponent
	{
	public:

		static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Rumble;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Rumble;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Rumble;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Rumble;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		~C_VehicleComponent_Rumble();

		void											Update();

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

		BaseMessageHandler*								mMessageHandler;
		int												mVehicleIndex;

		Game::C_RaceLogicLayer							*mRaceLogicLayerPtr;
	

		bool											mShouldUpdate;

		float mDeltaTime;

		float mPerkBuildUpTimer;
		float mPerkBuildUpLimit;

		float mPerkFiredTimer;

		float mPerkHitByTimer;

		float mHeavyLandingTimer;

		float mNitrousTimer;
		float mCollisionTimer;

		float mHandbrakeTimer;

		Btl::Vector<bool> mWheelsOnGround;
		Btl::Vector<U32> mWheelSurface;
	};

}

#endif //#ifndef VLC_RUMBLE_H
