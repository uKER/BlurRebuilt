#ifndef RACEMODE_MULTIPLAYER_H
#define RACEMODE_MULTIPLAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <Network\Misc\NetDefines.h>
#include <GameLogic\Structures\NetVehicleData.h>
#include <GameLogic\Structures\NetTeamData.h>
#include <GameLogic\Structures\NetGameVars.h>
#include <GameLogic\MultiplayerUtilities.h>
#include <GameLogic\Structures\LevelResult.h>


extern Debug::Bool g_AllowSingleRacer;
extern Debug::Int g_MinRankedPlayers;

static const float FINAL_DELAY_FOR_RESULTS = 5.0f;

namespace NetPlay { class C_Session; class C_PlayerSkillWriter; }
namespace Scoreboard { class  C_Writer; }

namespace Game
{

namespace Actors
{
	class RaceLogicCore;
}
class C_RaceLogicLayer;

class C_RaceModeMultiplayer : public C_RaceModeBase
{
public:
	virtual void								Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	virtual void								Update();

	//	These three methods are called IN THIS ORDER at:
	//	End of loading level,
	virtual void								StartLevel();
	//	Beginning of rolling start (after Waiting For Players),
	virtual void								BeginRollingStart();
	//	Start of race (when all the game logic gets started etc)
	virtual void								StartRace();

	virtual void								UpdateHUD();
	virtual void								SetupFakeResults() {}; // For UI testing
	virtual void								SetupResults();
	virtual void								SetupQuitAndWinResults(lua_State *pState) {};
	virtual void								QuitLevel();

	bool										BuildLobbyWaitingRaceInfoData(LobbyWaitingRaceInfo& raceInfo);

	virtual bool								MessageHandler(const Messaging::C_MessageBase* pMessage);

	virtual bool								ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel);
	virtual bool								ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel);
	virtual void								ForceTimeOutLocalVehicles(E_VEHICLE_STATE timedOutState = E_VEHICLE_STATE_TIMED_OUT);

	void										EnsureValidVehicleStatesSet(); 

	bool										mIsNetworkGame;					//	So we can use this class for split screen games too

protected:

	virtual bool								CheckForEndOfEvent(void);
	virtual void								CheckRaceTimoutTimer(void);
	void										DecrementFinalDelayTimer(void);
	virtual void								ForceEndOfEvent(void);					//	Set any vehicles that haven't reported in as "dropped out"

	virtual void								GetLocalPoints(void);
	virtual bool								SendControlsUpdate(void);

	virtual bool								BuildNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, U32 remoteIndex);
	virtual bool								AddModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex);	//	Add any racemode-specific to the end of the vehicle data net message
	virtual bool								HandleNetMessageBasicVehicleData(RacerNet::C_NetMessage& msg, const U8 senderId);
	virtual bool								HandleModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex);	//	Read any racemode-specific from the end of the vehicle data net message

	virtual void								DisableVehiclesLostDuringLoad(void);

	virtual void								TidyUpVehicleAndSendResults(const U8 p_Index);
	virtual void								VehicleFinished(const U8 p_Index, Game::E_VEHICLE_STATE p_FinishType);
	virtual void								VehicleLeftRace(const U8 p_Index);
	virtual void								EliminateVehicle(U8 p_Index);

	virtual void								UpdateInRaceVoiceRecipients(void);
	bool										PlayersCanVoiceChat(const U8 joinId1, const U8 joinId2);
	
	void										FindPreyAndNemesis( NetVehicleIndexStack &localList, Game::C_LevelResult &levelResult );

	virtual void								AppendVoiceDataToControlsUpdate(RacerNet::C_NetMessage& msg, const U8 joinId);
	virtual void								ExtractVoiceDataFromControlsUpdate(RacerNet::C_NetMessage& msg, const U8 senderId);
	
	virtual void								SetNetworkVehicleHealth(U8 p_Index, S32 p_HealthPoints);
	virtual void								FillVehicleData(const U8 p_Index);
	virtual void								SendNetResultsForVehicle(int p_Index);
	virtual bool								BuildNetResultsForVehicle(RacerNet::C_NetMessage& netMsg, int index);
	virtual bool								HandleNetResultsForVehicle(RacerNet::C_NetMessage& msg, const U8 senderId);
	virtual bool								HandleForceTimeOutMessage(RacerNet::C_NetMessage& msg, const U8 senderId);

	void										UpdateRacePositions();
	void										UpdateVehicleSpotlights();
	
	void										LocalEliminationSurvival();	//	All local vehicles still racing score for surviving elimination.

	NetVehicleData&								VehicleData(U8 p_Index) const;	//	Get live vehicle data for given index
	NetTeamData&								TeamData(U8 p_Index) const;	//	Get live team data for given team index

	bool										mVehicleHasLeft;				//	Flag to handle vehicles that have quit

	Time::C_Time								mRaceStartTime;					//	Time the race started (used to calculate race time).

#ifndef CFG_GOLD
	void										LogDebugNetworkPickupKilled(const E_PICKUP_CAUSE_OF_DEATH causeOfDeath) const;
#endif //#ifndef CFG_GOLD

	/*---------------------------------------------	 MODE-SPECIFIC METHODS	---------------------------------------*/

	//	Virtual methods for the different Multiplayer race modes
	virtual void								MpMode_Update() {};
	virtual void								MpMode_SortResults() {};
	virtual F32									MpMode_GetPrimaryStatValue(U8 vehicleIndex) { return .0f; };

	/*---------------------------------------------	 NET EVENT PROCESSING	---------------------------------------*/

	void										ProcessEvents(U8 p_LocalIndex, U8 p_NumEvents, RacerNet::C_NetMessage& p_Msg);
	//	If the base MP race mode doesn't know how to deal with a net event, it passes it on to the derived processing method.
	virtual void								ProcessEventsModeSpecific(U8 p_EventType, RacerNet::C_NetMessage& p_Msg, bool p_Process) {};

	void										ProcessEvent_DroppedPickupPickedUp(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_RegisterShocks(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_PerkFired(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_PickupKilled(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_PickupPostDelay(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_PickupDroppedOnTrack(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_DecoyDroppedOnTrack(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_InstaPickupFire(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_HitByPerk(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_Ghosted(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_Idle(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_MineMoved(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_Eliminated(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_Repaired(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_ShieldedPerk(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_AggressiveScore(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_SetPowerupType(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_DamageDealt(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_AbilityActivated(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_JudgedPowerupShielded(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_JudgedPowerupHit(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_MineDestroyedByProjectile(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_SetPowerupRespawnDelay(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_EvadedShock(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_HitEveryShock(RacerNet::C_NetMessage& p_Msg, bool p_Process);
	void										ProcessEvent_SilentRunning(RacerNet::C_NetMessage& p_Msg, bool process);
	
	/*----------------------------------------------	RESULTS SORTING	-------------------------------------------*/

	//	Create initial stack of indices sorted by Net Join ID
	void										SortAllNetJoinIds(NetVehicleIndexStack& out) const;

	void										SortByStatistic(NetVehicleIndexStack& out, UI::E_MultiplayerStat stat, bool greatestFirst) const;

	void										CopyFromStackIntoResults(NetVehicleIndexStack& stack, S8& startPos);

	Time::C_Time								mUpdateTimer;			//	Time last network control message was sent.
	Time::C_Time								mRacePositionUpdateTime;	//	Time we last updated the race positions for the HUD

	int											mPostRaceDelayTimer;	// Time before the race finished message is sent so we can sync scoring/challenges etc ... possibly this should be in all modes

	F32											mFinalDelayTimer;		//	Time before results screen should be displayed.
	bool										mFinalDelayTimerActive;
	bool										CheckPickupTypeAggressive(E_PICKUP_TYPE currType ) const;

	/*----------------------------------------------	SPLIT SCREEN	-------------------------------------------*/
	bool										NoHumanPlayersStillActive();
	void										SetUpSplitScreenSpotlights();
	void										ForceSplitScreenRaceFinished();

	/*-----------------------------------------------	TEAMS	---------------------------------------------------*/
	void										SetupTeamSpotlights();
	void										UpdateTeamScores();
	virtual S32									GetVehicleTeamScoreContribution(NetVehicleData& vehicleData) { return 0; };

	/*-----------------------------------------------	Spotlights	---------------------------------------------------*/
	const float									CalculateDesiredSpotlightIntensity(const Game::Actors::C_VehicleLogicActor* pLogicActor) const;

	// MISC
	void										TurnOffSilentRunningOnAllVehicles(); 

private:
	bool										mRaceResultsPosted;
	bool										mFirstPlayerFinished;

};


} // namespace Game

#endif // RACEMODE_MUTLIPLAYER
