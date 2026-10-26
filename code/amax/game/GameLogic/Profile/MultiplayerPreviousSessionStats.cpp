#include "stdafx.h"

#include "GameLogic\Profile\MultiplayerPreviousSessionStats.h"
#include <GameLogic/Profile/GameProfileManager.h>

namespace Game
{

void C_MultiplayerPreviousSessionStats::Clear()
{
	m_LastNemesis = m_LastPrey = m_Races = m_Wins = m_NumRacePoints = m_FansEarned = m_RanksGained = m_LastChallengeCompletedLevel = m_LastChallengeCarId = 0;
	m_PrevRaces	= m_PrevWins = m_PrevRacePoints = m_PrevFans = m_PrevRank = 0;

	m_LastChallengeCompleted = UI::E_MULTIPLAYER_CHALLENGE_TYPE_NONE;
}

void C_MultiplayerPreviousSessionStats::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );
	out << m_LastNemesis
		<< m_LastPrey
		<< m_Races
		<< m_Wins
		<< m_NumRacePoints
		<< m_FansEarned
		<< m_RanksGained
		<< (int)m_LastChallengeCompleted
		<< m_LastChallengeCompletedLevel
		<< m_LastChallengeCarId;
}

void C_MultiplayerPreviousSessionStats::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );
	int lastChallengeCompleted = 0;
	in >> m_LastNemesis
		>> m_LastPrey
		>> m_Races
		>> m_Wins
		>> m_NumRacePoints
		>> m_FansEarned
		>> m_RanksGained
		>> lastChallengeCompleted
		>> m_LastChallengeCompletedLevel
		>> m_LastChallengeCarId;

	m_LastChallengeCompleted = (UI::E_MULTIPLAYER_CHALLENGE_TYPE)lastChallengeCompleted;
}

void C_MultiplayerPreviousSessionStats::GetInitialStatValues()
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMpProfile = pGameProfile->GetMultiplayerProfile();

	m_PrevRaces			= pMpProfile->GetMultiplayerStatistics().mNumEventsCompleted;
	m_PrevWins			= pMpProfile->GetMultiplayerStatistics().mNumEventsWon;
	m_PrevRacePoints	= pMpProfile->GetMultiplayerStatistics().mNumRacePoints;
	m_PrevFans			= pMpProfile->GetMultiplayerStatistics().mNumLifetimeFans;
	m_PrevRank			= pMpProfile->GetCurrentLevel();
}

void C_MultiplayerPreviousSessionStats::CalculateDeltaValues()
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMpProfile = pGameProfile->GetMultiplayerProfile();

	m_Races			= pMpProfile->GetMultiplayerStatistics().mNumEventsCompleted - m_PrevRaces;
	m_Wins			= pMpProfile->GetMultiplayerStatistics().mNumEventsWon - m_PrevWins;
	m_NumRacePoints	= pMpProfile->GetMultiplayerStatistics().mNumRacePoints - m_PrevRacePoints;
	m_FansEarned	= pMpProfile->GetMultiplayerStatistics().mNumLifetimeFans - m_PrevFans;
	m_RanksGained	= pMpProfile->GetCurrentLevel() - m_PrevRank;

	m_LastChallengeCompleted = pMpProfile->GetChallengeSystem().GetLastChallengeCompleted();
	m_LastChallengeCompletedLevel = pMpProfile->GetChallengeSystem().GetLastChallengeCompletedLevel();
	m_LastChallengeCarId = pMpProfile->GetChallengeSystem().GetLastChallengeCompletedCarId();

}

void C_MultiplayerPreviousSessionStats::LegendUpResetStats()
{
	m_LastChallengeCompleted = UI::E_MULTIPLAYER_CHALLENGE_TYPE_NONE;
	m_LastChallengeCompletedLevel = 0;
	m_LastChallengeCarId = -1;
}

} // namespace Game