#ifndef RACEMODE_MULTIPLAYER_ELIMINATION_H
#define RACEMODE_MULTIPLAYER_ELIMINATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeMultiplayer.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>

namespace Game
{
class C_LevelState;

class C_RaceModeMultiplayer_Elimination : public C_RaceModeMultiplayer
{
public:
	void										Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	void										MpMode_Update();
	void										MpMode_SortResults();

	void										StartLevel();

	F32											GetRaceTimeout() const { return m_EliminationTimer; }
	int											GetEliminationStartPos();

	bool										MessageHandler(const Messaging::C_MessageBase* pMessage);

private:

	bool										AddModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex);	//	Add any racemode-specific to the end of the vehicle data net message
	bool										HandleModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex);	//	Read any racemode-specific from the end of the vehicle data net message

	void										UpdateEliminationPoints();	//	Calculates fresh elimination points for all local vehicles
	void										UpdateEliminationOrder(NetVehicleIndexStack& racers);	//	Sorts the currently-racing vehicles in elimination order
	//	Calculates which vehicle should be eliminated (only carried out by host)
	void										InitiateElimination();
	void										DropShockDome();
	void										RandomiseShockPosition(Maths::HVec3& pos);
	void										RewardSurvivingVehicles();

	//	Overriding the base call in VehicleFinished()
	void										EliminateVehicle(U8 p_Index);

	F32											m_EliminationTimer;				//	Time until next Elimination (if applicable)
	F32											m_RemainingBarrageTimer;		//	Time remaining of the current barrage
	F32											m_TimeToNextShotInBarrage;		//	Time until next "shot" in the current barrage period
	F32											m_EliminationPeriod;			//	Time between Eliminations (if applicable)
	S32											m_EliminationNumber;
	bool										m_BarrageInProgress;
	UI::E_EliminationEvent						m_EliminationEvent;				//	Describes when we Eliminate vehicles
	UI::E_EliminationType						m_EliminationType;

	U8											m_EliminationLapCounter;		//	Used to determine lap-crossing that triggers elimination in E_EliminationPeriod_LapX
	U8											m_CurrentBarrageTargetIndex;

	Util::C_Random								m_Random;

	NetVehicleIndexStack						m_EliminationPositions;

	/*********************************************	ELIMINATION POINTS CALCULATORS	*********************************/
	F32											GetEliminationPoints(U8 p_VehicleIndex);

};


} // namespace Game


#endif	//	RACEMODE_MULTIPLAYER_RACING_H