#ifndef GAMEMODE_SPLITSCREEN_H
#define GAMEMODE_SPLITSCREEN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameModes/GameModeMultiplayer.h>
#include <GameLogic/SplitScreen/SplitScreenScoring.h>
#include <GameLogic/GameData/GameEventData.h>

#include "GameLogic/GameData/MultiplayerData.h"

// Forward decls
namespace Game
{
	class C_LevelState; 
	class C_LevelResult;
}
struct AmaxSplitScreenEvent;

// Main class declaration
namespace Game
{

enum E_NUM_SPLITSCREEN_PLAYERS
{
	E_NUM_SPLITSCREEN_PLAYERS_MIN = 1,
	E_NUM_SPLITSCREEN_PLAYERS_MAX = 4,
	E_NUM_SPLITSCREEN_TOTAL_PLAYERS_MAX = 20,
};


class C_GameModeSplitScreen : public C_GameModeMultiPlayer
{
public:
	C_GameModeSplitScreen();

	//void SetupRace(void *data);
	void StartRace();
	void InterpretRaceResults();

	void ResetScoring();
	const SplitScreenScoring::Scores& GetTotalScores() const;
	const SplitScreenScoring::Scores& GetScoresForRace() const;

	const SplitScreenScoring::RaceResults& GetResultsForRace() const;

	const SplitScreenScoring::Scores& GetTotalFans() const;
	const SplitScreenScoring::Scores& GetFansForRace() const;

	const int GetScoreForRace(const int viewportIndex) const;
	const int GetTotalScore(const int viewportIndex) const;

	const int GetFansForRace(const int viewportIndex) const;
	const int GetTotalFans(const int viewportIndex) const;

	const float GetTotalRaceTime() const;
	const int GetTotalWins(const int viewportIndex) const;
	const int GetTotalRaces() const;

	S32 GetNumberOfPlayers() const { return (S32)m_NumLocalPlayers; }

private:
	void UpdateScores(const Game::C_LevelResult& results, const Game::C_LevelState& levelState);

	void OnSetupRace( lua_State* pState );
	void OnSetupLevelState( lua_State* pState, C_LevelState& levelState );

	const int CalculateNumberOfAIToAdd(const C_LevelState& levelState ) const;
	void PadAI( C_LevelState& levelState, const int noAIToAdd );		//CONST FAIL - Can't be const as eventually it calls Random() which needs to update its seed...sorry :-[
	void CreateAIPaddingVehicle(const VehicleList& validVehicles, C_LevelState& levelState, const int levelStateCarIndex);		//CONST FAIL - Can't be const as eventually it calls Random() which needs to update its seed...sorry :-[


private:
	SplitScreenScoring	m_Scoring;
	S8					m_NumLocalPlayers;
	float				m_TotalPlayTime;
};


} // namespace Game

#endif // GAMEMODE_SPLITSCREEN_H
