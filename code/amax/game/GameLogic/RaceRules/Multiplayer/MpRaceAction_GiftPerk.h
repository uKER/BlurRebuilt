#ifndef MP_RACE_ACTION_GIFT_PERK_H
#define MP_RACE_ACTION_GIFT_PERK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\Multiplayer\MpRaceActionBase.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>

namespace Util
{
	class C_Random;
}

namespace Game
{

	//	Base race action for gifting perks to vehicles
	class C_MpRaceAction_GiftPerk : public C_MpRaceActionBase
	{
	public:
		C_MpRaceAction_GiftPerk(F32 p_Period, E_PICKUP_TYPE p_Type);
		virtual ~C_MpRaceAction_GiftPerk();

		E_PICKUP_TYPE								ChooseRandomPickup();

	protected:
		E_PICKUP_TYPE								m_PickupType;
		Util::C_Random*								m_RandomGenerator;
	};

	//	Gifts to all local vehicles
	class C_MpRaceAction_GiftPerk_All : public C_MpRaceAction_GiftPerk
	{
	public:
		C_MpRaceAction_GiftPerk_All(F32 p_Period, E_PICKUP_TYPE p_Type):
		  C_MpRaceAction_GiftPerk(p_Period, p_Type) {};

		void PerformAction();
	};

	//	Gifts to a single vehicle
	class C_MpRaceAction_GiftPerk_Random : public C_MpRaceAction_GiftPerk
	{
	public:
		C_MpRaceAction_GiftPerk_Random(F32 p_Period, E_PICKUP_TYPE p_Type):
			C_MpRaceAction_GiftPerk(p_Period, p_Type) {};

		void PerformAction();
	};

	//	Gifts to vehicle in 1st place
	class C_MpRaceAction_GiftPerk_First : public C_MpRaceAction_GiftPerk
	{
	public:
		C_MpRaceAction_GiftPerk_First(F32 p_Period, E_PICKUP_TYPE p_Type):
			C_MpRaceAction_GiftPerk(p_Period, p_Type) {};

		void PerformAction();
	};

	//	Gifts to vehicle in last place
	class C_MpRaceAction_GiftPerk_Last : public C_MpRaceAction_GiftPerk
	{
	public:
		C_MpRaceAction_GiftPerk_Last(F32 p_Period, E_PICKUP_TYPE p_Type):
			C_MpRaceAction_GiftPerk(p_Period, p_Type) {};

		void PerformAction();
	};


}	//	namespace Game

#endif // MP_RACE_ACTION_GIFT_PERK_H