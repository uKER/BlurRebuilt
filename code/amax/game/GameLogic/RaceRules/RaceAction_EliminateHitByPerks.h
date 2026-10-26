#ifndef RACE_LAP_ACTION_ELIMINATE_HIT_BY_PERKS_H
#define RACE_LAP_ACTION_ELIMINATE_HIT_BY_PERKS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceAction_Base.h>

namespace Game
{

	class RaceAction_EliminateHitByPerks : public RaceActionBase
	{
	public:
		RaceAction_EliminateHitByPerks();

		void									PerformAction(int carIdx);
		RaceModeActionType						GetType() { return E_Action_EliminateHitByPerks; };

		void									GetActionString(const U32 vehicleIndex, UI::C_WideString512& str, const RACE_ACTION_STRING_TYPE stringType) const;

		void									Serialise( Sys::BinarySerialiserAutoEndian& stream )		{ RaceActionBase::Serialise(stream); };
		void									Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )	{ RaceActionBase::Deserialise(stream); };

		void									SetNumberOfLives(const int lives);
		void									AddPickup(const enum E_PICKUP_TYPE ePickupType);
		void									SetFirerID(const int firerId);
			
	private:
		const int								UpdateNumberOfHitsFromAllVehicles(const int receivingVehicleIndex) const;
		const int								UpdateNumberOfHits(const int vehicleIndex, const Actors::C_VehicleLogicActor* pLogicActor) const;

		void									CachePointers();


	//Variables
	private:
		Btl::Vector<E_PICKUP_TYPE>				mPickups;
		int										mNumLives;
		U32										mFirerID;

		C_RaceLogicLayer*						m_pLogicLayer;
	};

} // namespace Game

#endif // RACE_LAP_ACTION_ELIMINATE_HIT_BY_PERKS_H