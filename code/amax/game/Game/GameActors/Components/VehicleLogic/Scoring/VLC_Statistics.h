#ifndef VLC_STATISTICS_H
#define VLC_STATISTICS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <Game\GameActors\Components\GameComponents.h>

#include <GameLogic\Layers\RaceLogicLayer.h>

//Forward declarations
namespace Game
{
	namespace Actors
	{
		class C_VehicleActor;
	}
}

namespace Components
{
	class C_VehicleComponent_NewScoring;
}

struct VehicleRaceStatistics;
//Forward declarations


namespace Components
{
	class BaseMessageHandler;

	class I_VehicleComponent_Statistics_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_Statistics_CreateInfo() :	mVehicleIndex(INVALID_VEHICLE_IDX), 
														mpRaceLogicLayer(0), 
														mpStatistics(0) {}

		static Component::C_ComponentTypeId				ClassType() {return E_VehicleComponent_Statistics;}
		virtual Component::C_ComponentTypeId			GetComponentTypeID() const {return E_VehicleComponent_Statistics;}

		int												mVehicleIndex;
		::Game::C_RaceLogicLayer*						mpRaceLogicLayer;
		VehicleRaceStatistics*							mpStatistics;
	};

	class C_VehicleComponent_Statistics : public I_GameActorComponent
	{
	public:

		static Component::C_ComponentInterfaceId		InterfaceType() {return E_IF_VehicleComponent_Statistics;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return E_IF_VehicleComponent_Statistics;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return E_VehicleComponent_Statistics;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return E_VehicleComponent_Statistics;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		void											Update();

		static const bool								OffensivePerk(const enum E_PICKUP_TYPE ePerkType, const enum E_PICKUP_SUB_TYPE eSubType, const bool fired);

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }


		void											SetVehicleOvertakerStatistics(int racePos, int otherVehIdx);
		void											SetVehicleOvertakeeStatistics(int racePos);
		virtual void									SetRaceCompletedStatistics();
		void											CheckWreckRaceTime();

		void											CheckVehicleImpactSpecialMessages(const Game::Actors::C_VehicleActor* pVehActor, const Actor::C_Actor* pOtherActor);

	private:
	
		void											HandleMyDeath(const bool suicided);
		void											HandleSomeoneElsesDeath(U8 aboutVehicleIndex);

	protected:

		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		U8												mVehicleIndex;
		::Game::C_RaceLogicLayer*						mRaceLogicLayerPtr;

		bool											mEnabled;

		E_PICKUP_TYPE									mLastFiredPerk;
		U8												mRacePositionWhenFired;
		float											mFiredPerkRecentlyTimer;

		float											mLastOvertakeTime;
		float											mNitroTimer;
		bool											mNitroDouble;
		int												mNoOvertaken;
		U32												mLastFiredID;

		Btl::Set<U32>									mPerksHit;
		bool											mWasJustDrifting;

	private:

		VehicleRaceStatistics*							m_pStatistics;
		Components::C_VehicleComponent_NewScoring*		m_pScoring;
		void											HandleMotorMashWreckScoreCalculation(Game::Actors::C_VehicleLogicActor* pLogicActor,const bool wasRivalAttack, int& basePoints, int& bonusPoints);

#ifndef CFG_GOLD
		void											UpdateDebugValues();
#endif //#ifndef CFG_GOLD
	};

}

#endif //#ifndef VLC_STATISTICS_H
