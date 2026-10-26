#ifndef VLC_POINTSSYSTEM_H
#define VLC_POINTSSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <game\GameActors\Components\GameComponents.h>
#include <GameLogic\Structures\VehicleRaceStatistics.h>

#include <GameLogic\Layers\RaceLogicLayer.h>

namespace Game
{
	class ScoringRuleBase;
}

namespace Components
{
	class BaseMessageHandler;

	class I_VehicleComponent_PointsSystem_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_PointsSystem_CreateInfo() {}

		static Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Points;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_VehicleComponent_Points;}

		Actor::C_ActorHandle	mParentActorHandle;
		int						mVehicleIndex;
		Game::C_RaceLogicLayer*	mpRaceLogicLayer;
	};

	class C_VehicleComponent_PointsSystem : public I_GameActorComponent
	{
	public:

		static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_Points;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_Points;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_Points;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_Points;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		F32												GetCurrentPoints() { return mCurrentPoints; }

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		U8												mVehicleIndex;
		Game::C_RaceLogicLayer*							mRaceLogicLayerPtr;

		bool											mEnabled;

		Btl::Vector<int>								mScoringRules; // Index into MP rules data
		float											mCurrentPoints;
	};

}

#endif //#ifndef VLC_POINTSSYSTEM_H
