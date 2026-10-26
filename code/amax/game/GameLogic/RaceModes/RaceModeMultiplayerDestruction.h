#ifndef RACEMODE_MULTIPLAYER_DESTRUCTION_H
#define RACEMODE_MULTIPLAYER_DESTRUCTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeMultiplayer.h>

struct MotorMashScoringValues;

namespace Game
{

namespace Actors
{
	class C_VehicleLogicActor;
};

class C_RaceModeMultiplayer_Destruction : public C_RaceModeMultiplayer
{

	typedef C_RaceModeMultiplayer PARENTCLASS;

public:
	void										Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	void										MpMode_Update();
	void										MpMode_SortResults();

	F32											GetRaceTimeout() const;

	void										StartLevel();

	bool										MessageHandler(const Messaging::C_MessageBase* pMessage);

private:

	bool										AddModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex);	//	Add any racemode-specific to the end of the vehicle data net message
	bool										HandleModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex);	//	Read any racemode-specific from the end of the vehicle data net message

	void										UpdateLocalData();

	const S32									CalculateVehicleOnTrackPoints(Game::Actors::C_VehicleLogicActor* pLogicActor) const;

	F32											MpMode_GetPrimaryStatValue(U8 vehicleIndex);

	void										CheckTimeOut();
	void										CheckScoreLimit();

	void										CheckForTiedScores(NetVehicleIndexStack& vehiclesRacingSortedByOnTrackPoints) const;	
	void										ResolveTies(NetVehicleIndexStack& vehiclesRacingSortedByOnTrackPoints, NetVehicleIndexStack& vehiclesTied, const int firstTiedArrayIndex) const;

	S32											GetVehicleTeamScoreContribution(NetVehicleData& vehicleData);

	void										VehicleFinished(const U8 p_Index, Game::E_VEHICLE_STATE p_FinishType);

	F32											m_UiTimeOutTimerNext;	

	/*----------------------------------------------	Rivals	-------------------------------------------*/
public:
	F32											GetRivalsCommencementDelayTimer() const {return mRivalsCommencementDelayTimer;}
	F32											GetRivalsCountdownDelayTimer() const {return mRivalsCountdownDelayTimer;}
	bool										GetRivalsModeEnabled() const {return mRivalsModeEnabled;}
	bool										GetInitialRivalsAssigned() const {return mInitialRivalsAssigned;}
	bool										GetRivalsCountdownDelayActive() const {return mRivalsCountdownDelayActive;}


private:
	F32											mRivalsCommencementDelayTimer; 
	F32											mRivalsCountdownDelayTimer;	 
	bool										mRivalsModeEnabled;
	bool										mInitialRivalsAssigned; 
	bool										mBlockScoringMessageSent;
	bool										mRivalsCountdownDelayActive;

	void										HandleInitialRivalConfig(float frameTime);		

};

} // namespace Game

#endif // RACEMODE_MULTIPLAYER_DESTRUCTION_H
