// ***************************************************************
//  NetScoreboards  version:  1.0   ·  date: 03/16/2009
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Helper functions for dealing with platform scoreboards.
// ***************************************************************

#ifndef __NET_SCOREBOARDS_H__
#define __NET_SCOREBOARDS_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <Network/ScoreboardStructures.h>

namespace RacerNet
{
	// makes the code easier to read.
	typedef Btl::Vector<Scoreboard::ColumnData, Btl::VectorParameters_Static<20> > ScoreboardColumnWriteList;

	// defines all the multiplayer scoreboards we reference.
	typedef enum 
	{		
		eMpScoreboardTotalWins = 0,			// points scoreboard
		eMpScoreboardPerkRatio,				// powerups scoreboard
		eMpScoreboardTotalFans,				// fans scoreboard
		eMpScoreboardLegends,				// Legend level scoreboard
		eMpScoreboardLobby,					// Lobby scoreboard
		eSP_Level_1_A,		eSP_Level_2_A,	eSP_Level_3_A,	eSP_Level_4_A,	
		eSP_Level_5_A,		eSP_Level_6_A,	eSP_Level_7_A,	eSP_Level_8_A,	
		eSP_Level_9_A,		eSP_Level_10_A,	eSP_Level_11_A,	eSP_Level_12_A,	
		eSP_Level_13_A,		eSP_Level_14_A,	eSP_Level_15_A,	eSP_Level_16_A,	
		eSP_Level_17_A,		eSP_Level_18_A,	eSP_Level_19_A,	eSP_Level_20_A,	
		eSP_Level_21_A,		eSP_Level_22_A,	eSP_Level_23_A,	eSP_Level_24_A,	
		eSP_Level_25_A,		eSP_Level_26_A,	eSP_Level_27_A,	eSP_Level_28_A,	
		eSP_Level_29_A,		eSP_Level_30_A,	eSP_Level_31_A,	eSP_Level_32_A,	
		eSP_Level_33_A,		eSP_Level_34_A,	eSP_Level_35_A,	eSP_Level_36_A,	
		eSP_Level_37_A,		eSP_Level_38_A,	eSP_Level_39_A,	eSP_Level_40_A,	
		eSP_Level_41_A,		eSP_Level_42_A,	eSP_Level_43_A,	eSP_Level_44_A,	
		eSP_Level_45_A,		eSP_Level_46_A,	eSP_Level_47_A,	eSP_Level_48_A,	
		eSP_Level_49_A,		eSP_Level_50_A,	eSP_Level_51_A,	eSP_Level_52_A,	
		eSP_Level_53_A,		eSP_Level_54_A,	eSP_Level_55_A,	eSP_Level_56_A,	
		eSP_Level_57_A,		eSP_Level_58_A,	eSP_Level_59_A,	eSP_Level_60_A,	
		eSP_Level_61_A,		eSP_Level_62_A,	eSP_Level_63_A,	eSP_Level_64_A,	
		eSP_Level_65_A,		eSP_Level_66_A,	eSP_Level_67_A,	eSP_Level_68_A,	
		eSP_Level_69_A,		eSP_Level_70_A,	eSP_Level_71_A,	eSP_Level_72_A,	
		eSP_Level_73_A,		eSP_Level_74_A,	eSP_Level_75_A,	eSP_Level_76_A,	
		eSP_Level_77_A,		eSP_Level_78_A,	eSP_Level_79_A,	eSP_Level_80_A,	
		eSP_Level_81_A,		eSP_Level_82_A,	eSP_Level_83_A,	eSP_Level_84_A,	
		eSP_Level_85_A,		eSP_Level_86_A,	eSP_Level_87_A,	eSP_Level_88_A,	
		eSP_Level_89_A,		eSP_Level_90_A,	
		eMaxScoreboards,
	}eScoreboards;

	/// Gets setup at the end of a multiplayer event.
	/** We only need to track this data on a per event basis as we add the values to
	 *	the current scoreboard data to update it. This is done by the server so we dont have to read the 
	 *	current scoreboard data before writing the new data. 
	 */
	struct MpScoreboardDataForEvent
	{
		MpScoreboardDataForEvent() :	m_GameRank(0), m_TotalFans(0), m_TotalTimePlayed(0), m_RacePoints(0), m_TopThree(0),
										m_TotalRaces(0), m_RaceWins(0), m_PowerUpHits(0), m_PowerUpsFired(0), m_CarsWrecked(0),
										m_LegendLevel(0), m_TimeToAchieveLegend(0), m_TotalFansRating(0), m_RacePointsRating(0),
										m_PowerUpHitsRating(0),	m_LegendLevelRating(0),	m_RaceWinsRating(0) {}
		
		//All MP Leaderboards now have this information
		S64 m_GameRank;
		S64 m_TotalFans;
		S64 m_TotalTimePlayed;
		S64 m_RacePoints;
		S64 m_TopThree;
		S64 m_TotalRaces;
		S64 m_RaceWins;
		S64 m_PowerUpHits;
		S64 m_PowerUpsFired;
		S64 m_CarsWrecked;
		S64 m_LegendLevel;
		S64 m_TimeToAchieveLegend;

		S64 m_TotalFansRating;
		S64 m_RaceWinsRating;
		S64 m_PowerUpHitsRating;
		S64 m_LegendLevelRating;
		S64 m_RacePointsRating;
	};

	/// get the platform identifyer for the specified scoreboard.
	U32 ScoreboardId(const eScoreboards sb);

	/// build the data that gets written to the specicific scoreboard.
	/** @param sb scoreboard that is to be written to.
	 *	@param data corectly configured data that will be sent to the scoreboard.
	 *	@param out list of columns that need to be sent to the specified scoreboard.
	 */
	void MpBuildScoreboardColumnsToWrite(const eScoreboards sb, const MpScoreboardDataForEvent& data, ScoreboardColumnWriteList& out);

	typedef enum
	{
		eSPRacesScoreboard = 0,
		eSPFanDemandsScoreboard,
		eSPCheckpointsScoreboard,
		eSPDestructionScoreboard,
		eSP1on1Scoreboard,
	}eSPScoreboardTypes;

	struct LeaderboardSettings
	{
		eScoreboards			mScoreboardToWriteTo;
		eSPScoreboardTypes		mScoreboardType;
	};

	/// Gets setup at the end of a multiplayer event.
	/** We only need to track this data on a per event basis as we add the values to
	 *	the current scoreboard data to update it. This is done by the server so we dont have to read the 
	 *	current scoreboard data before writing the new data. 
	 */
	struct SpScoreboardDataForEvent
	{
		SpScoreboardDataForEvent() :	m_TotalFans(0), m_LapTime(0), m_CompTime(0),	m_Wrecks(0),
										m_VehicleId(0),	m_Difficulty(0), m_RacesRating(0), m_FanDemandsRating(0),
										m_CheckpointsRating(0),	m_DestructionRating(0), m_1on1Rating(0)	{}
		
		//All SP Leaderboards now have this information
		S64 m_TotalFans;
		S64 m_LapTime;
		S64 m_CompTime;
		S64 m_Wrecks;
		S64 m_VehicleId;
		S64 m_Difficulty;

		S64 m_RacesRating;
		S64 m_FanDemandsRating;
		S64 m_CheckpointsRating;
		S64 m_DestructionRating;
		S64 m_1on1Rating;
	};

	/// build the data that gets written to the specicific scoreboard.
	/** @param sb scoreboard that is to be written to.
	 *	@param data corectly configured data that will be sent to the scoreboard.
	 *	@param out list of columns that need to be sent to the specified scoreboard.
	 */
	void SpBuildScoreboardColumnsToWrite(const eSPScoreboardTypes sb, const SpScoreboardDataForEvent& data, ScoreboardColumnWriteList& out);

	

}// end namespace RacerNet



#endif //__NET_SCOREBOARDS_H__
