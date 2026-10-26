#ifndef RACE_LAP_ACTION_BASE_H
#define RACE_LAP_ACTION_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{
	enum RACE_ACTION_STRING_TYPE
	{
		RACE_ACTION_STRING_TYPE_HUD_TARGET,
		RACE_ACTION_STRING_TYPE_HUD_CURRENT,
		RACE_ACTION_STRING_TYPE_HUD_BONUS,
		RACE_ACTION_STRING_TYPE_RESULTS,	
	};


	enum RaceModeActionType
	{
		E_Action_EliminateLastDriver,
		E_Action_TimeAttackDisplay,
		E_Action_ManageGhosts,
		E_Action_EliminateHitByPerks,
	};

	class RaceActionBase
	{
	public:
		void SetTimer(float t) { mCurrentTimer = mTimer = t; };

		virtual void PerformAction(int carIdx) {};
		virtual RaceModeActionType GetType() = 0;

		virtual void	GetActionString(const U32 idx, UI::C_WideString512& str, const RACE_ACTION_STRING_TYPE stringType) const {};
	
		virtual void	Serialise( Sys::BinarySerialiserAutoEndian& stream ) {};
		virtual void	Deserialise( Sys::BinaryDeserialiserAutoEndian& stream ) {};

		virtual void	Init(){};
		virtual void	Update(float t) {};
		float mTimer;
		float mCurrentTimer;
	};

} // namespace Game

#endif // RACE_LAP_ACTION_BASE_H