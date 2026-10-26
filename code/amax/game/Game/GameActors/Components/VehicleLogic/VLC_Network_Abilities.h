#ifndef VLC_NETWORK_ABILITIES_H
#define VLC_NETWORK_ABILITIES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Abilities.h>

#include "misc_shared/Curve/HermiteCurve.h"  //Used for the silent running cloak effects

struct VehicleScoringInfo;
struct MultiplayerConfig;
namespace Game
{
	namespace Actors
	{
		class RaceLogicCore;
	};
};

namespace Components
{
	class BaseMessageHandler;

	class I_VehicleComponent_Network_Abilities_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_Network_Abilities_CreateInfo() {}

		static Component::C_ComponentTypeId			ClassType() {return Components::E_VehicleComponent_MP_Network_Abilities;}
		virtual Component::C_ComponentTypeId		GetComponentTypeID() const {return Components::E_VehicleComponent_MP_Network_Abilities;}

		Actor::C_ActorHandle						mParentActorHandle;
		int											mVehicleIndex;
		Game::C_LevelState							*m_pLevelState;
		VehicleScoringInfo							*m_pScoringInfo;
		Game::Actors::RaceLogicCore					*m_pRLC;
		MultiplayerConfig							*m_pConfig;

		Game::Actors::C_VehicleLogicActor			*m_pVehicleLogicActor;
		Game::Actors::C_VehicleActor				*m_pVehicleGraphicalActor;


		const Game::C_MultiplayerAbility			*m_pSilentRunningAbility;
		AmaxVehicle									*m_pXtAmaxVehicle;
	};

	class C_VehicleComponent_Network_Abilities : public C_VehicleComponent_Abilities
	{
	public:
		static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_VehicleComponent_Abilities;}
		virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_VehicleComponent_Abilities;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId	ClassType() {return Components::E_VehicleComponent_MP_Network_Abilities;}
		virtual Component::C_ComponentInterfaceId	GetComponentTypeID() const {return Components::E_VehicleComponent_MP_Network_Abilities;} // from Component::I_GameComponent

		void										Update();

	protected:
		virtual bool								CreateImpl();
		void										SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }
		const bool									ShouldBeSilentlyRunning() const OVERRIDE;
		void										SetDesiredVehicleAlphas() OVERRIDE;

	private:
		BaseMessageHandler*							mMessageHandler;
	};

}

#endif //#ifndef VLC_NETWORK_ABILITIES_H
