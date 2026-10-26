#ifndef VLC_PLAYERRUBBERBANDING_H
#define VLC_PLAYERRUBBERBANDING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "misc_shared/Curve/HermiteCurve.h"

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

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

	class C_VehicleComponent_PlayerRubberBanding : public I_GameActorComponent
	{
	public:
		C_VehicleComponent_PlayerRubberBanding();
		~C_VehicleComponent_PlayerRubberBanding();

		static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_VehicleComponent_PlayerRubberBanding;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_VehicleComponent_PlayerRubberBanding;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return Components::E_VehicleComponent_PlayerRubberBanding;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return Components::E_VehicleComponent_PlayerRubberBanding;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);
		void											Update();

		void											SetMaxRaceLogicPosition(float maxRaceLogicPosition) {mMaxRaceLogicPosition = maxRaceLogicPosition;}
		void											SetMinRaceLogicPosition(float minRaceLogicPosition) {mMinRaceLogicPosition = minRaceLogicPosition;}
		float											GetMaxRaceLogicPosition() {return mMaxRaceLogicPosition;}
		float											GetMinRaceLogicPosition() {return mMinRaceLogicPosition;}

		static void										SendGroupDataToComponents();

	protected:
		virtual bool									CreateImpl();
		void											Initialise();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }
		
	private:

		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		int												mVehicleIndex;
		float											mRubberBandingModifier; //between -1.f and 1.f

		float											mDesiredRubberBandingModifier;
		bool											mSmoothingToNewModifier;

		bool											mEnabled;

		const float										GetDistanceToBandPoint() const;
		void											ApplyRubberBandingByImpulse(Game::Actors::C_VehicleLogicActor* pActor);
		float											GetImpulseMagnitude() const;

		const float										ComputePlayerRubberBandingModifier() const;
		static float									ComputeDistanceBetweenVehicles(int vehicleIndexA, int vehicleIndexB);

		static float									CalculateTrackLength();
		float											mTrackLength;
		float											mMaxRaceLogicPosition;
		float											mMinRaceLogicPosition;
		float											mOurRaceLogicPosition;

		Game::Actors::RaceLogicCore*					mRLC;
		Game::C_RaceLogicLayer*							mRaceLogicLayer;
		Game::C_LevelState*								mLevelState;

		static const float								s_minimumAcceleratorValue;
		static int										s_componentCount;
		static Curve::C_HermiteCurve					s_impulseMagnitudeGraph;

		struct RacePositionData
		{
			int											vehicleIndex;
			int											racePosition;
		};

		static void										UpdateCurve();

		static int										SortRacePositionsFunc(const void* a, const void* b);

		//to help with debug drawing
		float											GetMaxRaceLogicPositionBetweenZeroAndOne(){ return (mMaxRaceLogicPosition-1.f)/mLevelState->mLaps; } 
		float											GetMinRaceLogicPositionBetweenZeroAndOne(){ return (mMinRaceLogicPosition-1.f)/mLevelState->mLaps; }
		void											DebugDraw();

	};

	class I_VehicleComponent_PlayerRubberBanding_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_PlayerRubberBanding_CreateInfo() {}

		static Component::C_ComponentTypeId				ClassType() {return Components::E_VehicleComponent_PlayerRubberBanding;}
		virtual Component::C_ComponentTypeId			GetComponentTypeID() const {return Components::E_VehicleComponent_PlayerRubberBanding;}

		Actor::C_ActorHandle							mParentActorHandle;
		int												mVehicleIndex;	
		int												mTotalHumanPlayers;

		Game::Actors::RaceLogicCore*					mRLC;
		Game::C_LevelState*								mLevelState;
		Game::C_RaceLogicLayer*							mRaceLogicLayer;
	};

}

#endif //#ifndef VLC_PLAYERRUBBERBANDING_H
