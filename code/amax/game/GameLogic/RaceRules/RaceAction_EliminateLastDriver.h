#ifndef RACE_LAP_ACTION_ELIMINATE_LAST_DRIVER_H
#define RACE_LAP_ACTION_ELIMINATE_LAST_DRIVER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceAction_Base.h>
#include <Game\Messaging\GameflowMessages.h>

namespace Game
{
	class C_EliminationTimerStartedMessage : public Messaging::C_MessageDerived<C_EliminationTimerStartedMessage, Game::E_GameflowMessage_EliminationTimeoutStart>
	{
	public:

		C_EliminationTimerStartedMessage(F32 timeout)
		{
			mTimeout = timeout;
		}

		//Variables
	public:
		F32											mTimeout;
	};

	class C_EliminationTimerEndedMessage : public Messaging::C_MessageDerived<C_EliminationTimerEndedMessage, Game::E_GameflowMessage_EliminationTimeoutEnded>
	{
	};

	class RaceAction_EliminateLastDriver : public RaceActionBase
	{
	public:
		RaceAction_EliminateLastDriver();

		void PerformAction(int carIdx);
		RaceModeActionType GetType() { return E_Action_EliminateLastDriver; };

		void Serialise( Sys::BinarySerialiserAutoEndian& stream )		{ RaceActionBase::Serialise(stream); };
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )	{ RaceActionBase::Deserialise(stream); };

		virtual void Init();
		virtual void Update(float t); 
		int mNumDrivers;

	private:
		bool    mElminationTimerModeActive;
		bool	mEliminationAlreadyHappenedThisFrame;
		int		mEliminatedThisFrame;
	};

} // namespace Game

#endif // RACE_LAP_ACTION_ELIMINATE_LAST_DRIVER_H