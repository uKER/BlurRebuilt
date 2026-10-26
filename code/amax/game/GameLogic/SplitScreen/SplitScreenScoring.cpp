#include <stdafx.h>
#include "SplitScreenScoring.h"
#include <GameLogic/GameModes/GameModeSplitScreen.h>
#include <Game/Game.h>

namespace Game
{

SplitScreenScoring::SplitScreenScoring()
{
	m_noViewports = 1;

	Reset();
}

void SplitScreenScoring::SetNoViewports(const int noViewports)
{
	m_noViewports = noViewports;
}

void SplitScreenScoring::Reset()
{
	for(int viewportIndex = 0; viewportIndex < m_noViewports; viewportIndex++)
	{
		m_totalScores[viewportIndex] = 0;
		m_scoresForRace[viewportIndex] = 0;

		m_fansForRace[viewportIndex] = 0;
		m_totalFans[viewportIndex] = 0;

		m_TotalWins[viewportIndex] = 0;
	}
}

void SplitScreenScoring::RaceFinished(const SplitScreenScoring::RaceResults& racePositionResults, const SplitScreenScoring::RaceResults& fansResults)
{
	int score = 0;
	int fans = 0;
	int racePosition = 0;
	
	bool includesAIs = ( C_Game::GetInstance()->GetLevelState().GetMultiplayerSettings().mAIMode != UI::E_AiOff );

	Game::C_LevelResult& LevelResults = C_Game::GetInstance()->GetLevelResult();

	for(int viewportIndex = 0; viewportIndex < racePositionResults.Size(); ++viewportIndex)
	{
		//The results array stores the race position 
		racePosition = racePositionResults[viewportIndex];
		if(racePosition < 0)
			continue;

		//Work out the score based on the number of viewports and the race position of the player
		score = GetSplitScreenScore(racePosition, includesAIs);
		fans = fansResults[viewportIndex];

		m_RaceResults[viewportIndex] = racePosition;

		//Store the score for this race
		m_scoresForRace[viewportIndex] = score;
		//...and the running total
		m_totalScores[viewportIndex] += score;

		//Store the fans got in this race
		m_fansForRace[viewportIndex] = fans;
		//...add the fans too (to a different array)
		m_totalFans[viewportIndex] += fans;

		//Store the win
		if(racePosition == 1)
		{
			m_TotalWins[viewportIndex]++;
		}
	}
}

const int SplitScreenScoring::GetSplitScreenScore(const int racePosition, const bool includesAIs) const 
{
	MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
	if(pConfig)
	{
		XtHandle<SplitScreenScores>& pScores = pConfig->m_ScoresWithAI;

		if(!includesAIs)
		{
			pScores = pConfig->m_ScoresWithoutAI;
		}

		switch(m_noViewports)
		{
			case 1:
				{
					if(pScores->mOneViewport.count >= racePosition)
					{
						return pScores->mOneViewport[racePosition-1];
					}
				}
				break;
			case 2:
				{
					if(pScores->mTwoViewports.count >= racePosition)
					{
						return pScores->mTwoViewports[racePosition-1];
					}
				}
				break;
			case 3:
				{
					if(pScores->mThreeViewports.count >= racePosition)
					{
						return pScores->mThreeViewports[racePosition-1];
					}
				}
				break;
			case 4:
				{
					if(pScores->mFourViewports.count >= racePosition)
					{
						return pScores->mFourViewports[racePosition-1];
					}
				}
				break;
		}
	}

	return 0;
}

const SplitScreenScoring::Scores& SplitScreenScoring::GetTotalScores() const
{
	return m_totalScores;
}

const SplitScreenScoring::Scores& SplitScreenScoring::GetScoresForRace() const
{
	return m_scoresForRace;
}

const SplitScreenScoring::RaceResults& SplitScreenScoring::GetRaceResulsts() const
{
	return m_RaceResults;
}

const SplitScreenScoring::Scores& SplitScreenScoring::GetTotalFans() const
{
	return m_totalFans;
}

const SplitScreenScoring::Scores& SplitScreenScoring::GetFansForRace() const
{
	return m_fansForRace;
}

const SplitScreenScoring::Scores& SplitScreenScoring::GetTotalWins() const
{
	return m_TotalWins;
}

const int SplitScreenScoring::GetTotalRaces() const
{
	int totalRaces = 0;
	for(int viewportIndex = 0; viewportIndex < m_TotalWins.Size(); ++viewportIndex)
	{
		totalRaces += m_TotalWins[viewportIndex];
	}
	return totalRaces;
}
};