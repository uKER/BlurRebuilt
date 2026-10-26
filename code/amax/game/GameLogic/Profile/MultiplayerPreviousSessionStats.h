#ifndef MULTIPLAYER_PREVIOUS_SESSION_STATS_H
#define MULTIPLAYER_PREVIOUS_SESSION_STATS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

namespace Game
{

	//	Used for the "Previously on Blur..." recap when you sign in
	class C_MultiplayerPreviousSessionStats
	{
	public:
		//	Stuff we serialise/deserialise for the PoB stats
		//#if defined(PLATFORM_XENON)
		U64										m_LastNemesis;
		U64										m_LastPrey;
		//#endif
		S32										m_Races;
		S32										m_Wins;
		U32										m_NumRacePoints;
		S32										m_FansEarned;
		S32										m_RanksGained;

		UI::E_MULTIPLAYER_CHALLENGE_TYPE		m_LastChallengeCompleted;
		U8										m_LastChallengeCompletedLevel;
		U32										m_LastChallengeCarId;

		//	Temp stuff we just use to calculate the PoB stats

		S32										m_PrevRaces;
		S32										m_PrevWins;
		U32										m_PrevRacePoints;
		S32										m_PrevFans;
		S32										m_PrevRank;

		void									Clear();
		void									GetInitialStatValues();
		void									CalculateDeltaValues();

		void									Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void									Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );
		
		void									LegendUpResetStats();
	};



} // namespace Game




#endif // MULTIPLAYER_PREVIOUS_SESSION_STATS_H