#ifndef VLC_DRAFTING_H
#define VLC_DRAFTING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/GameActors/Vehicles/VehiclePerformanceModifiers.h>
#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <GameLogic\Layers\RaceLogicLayer.h>

namespace Components
{
	enum E_DRAFTING_SFX_MODE
	{
		E_DRAFTING_SFX_MODE_INACTIVE,
		E_DRAFTING_SFX_MODE_FADEIN,
		E_DRAFTING_SFX_MODE_LOOPING,
		E_DRAFTING_SFX_MODE_FADEOUT,
	};

	class BaseMessageHandler;

	class I_VehicleComponent_Drafting_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_Drafting_CreateInfo() {}

		static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Drafting;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Drafting;}

		Game::C_RaceLogicLayer		*mRaceLogicLayerPtr;
		int							mVehicleIndex;
	};

	class C_VehicleComponent_Drafting : public I_GameActorComponent
	{
	public:

		static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Drafting;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Drafting;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Drafting;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Drafting;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		~C_VehicleComponent_Drafting();

		void											Update();

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

		void											HandleSFX();
		void											SwitchToSFXMode(E_DRAFTING_SFX_MODE mode);

		E_DRAFTING_SFX_MODE								mCurrentSFXMode;

		BaseMessageHandler*								mMessageHandler;
		int												mVehicleIndex;

		Game::C_RaceLogicLayer							*mRaceLogicLayerPtr;

		float											mActiveTimer;
		float											mBehindTimer;

		SpecialFX::I_SpecialFXInstance*					mCurrentSFX;
		float											mSFXTimer;
		bool											mSFXRunning;

		bool											mEnabled;
	};

}

#endif //#ifndef VLC_DRAFTING_H
