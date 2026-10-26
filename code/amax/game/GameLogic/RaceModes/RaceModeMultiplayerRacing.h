#ifndef RACEMODE_MULTIPLAYER_RACING_H
#define RACEMODE_MULTIPLAYER_RACING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeMultiplayer.h>
#include <GameLogic\GameModes\GameModeDefines.h>

struct MultiplayerTimeoutSettings;

namespace Game
{

class C_RaceModeMultiplayer_Racing : public C_RaceModeMultiplayer
{
public:
	void										Initialise(Game::C_LevelState *pLevelState) OVERRIDE;

	void										MpMode_Update();
	void										MpMode_SortResults();

	void										StartLevel();

	F32											GetRaceTimeout() const;
	const F32									GetInitialTimeoutTime() const;

	bool										MessageHandler(const Messaging::C_MessageBase* pMessage);									

protected:

	enum MULTIPLAYER_TIMEOUT_TYPE
	{
		MULTIPLAYER_TIMEOUT_TYPE_NONE = 0,
		MULTIPLAYER_TIMEOUT_TYPE_VISIBLE,
		MULTIPLAYER_TIMEOUT_TYPE_HIDDEN,
		MULTIPLAYER_TIMEOUT_TYPE_FAILSAFE,
		MULTIPLAYER_TIMEOUT_TYPE_MAX_RACE_TIME,
	};

	bool										AddModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex);	//	Add any racemode-specific to the end of the vehicle data net message
	bool										HandleModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex);	//	Read any racemode-specific from the end of the vehicle data net message

	void										CheckRaceTimoutTimer(void);
	bool										UpdateRacingTimers(void);
	void										UpdateLogicPositions(void);
	void										CheckLaps();
	void										CheckTime();
	void										CheckForSplitscreenElimination(); 
	void										CheckAllHumansFinished(int vehIdx);

	F32											MpMode_GetPrimaryStatValue(U8 vehicleIndex);

	bool										AllNotIdleVehiclesFinished() const;

	S32											GetVehicleTeamScoreContribution(NetVehicleData& vehicleData);

	UI::E_MpFinishType							m_FinishType;				//	Reason for finishing the race.

	F32											m_EliminationTimeOutTimer;
	F32											m_TimeOutTimer;					//	Time other vehicles have left to complete the event before being awarded DNF
	bool										m_TimeOutTimerActive;
	E_GameMode									m_GameMode;

	bool										m_AllowTimeouts;
	const MultiplayerTimeoutSettings*			m_TimeoutSettings;
	MULTIPLAYER_TIMEOUT_TYPE					m_TimeoutType;
	F32											m_InitialTimeoutTime;
};


} // namespace Game


#endif	//	RACEMODE_MULTIPLAYER_RACING_H
