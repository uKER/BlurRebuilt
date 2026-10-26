#include <stdafx.h>
#include <Core/core.h>
#include "../Misc/NetDebug.h"
#include "NetScoreboards.h"


	

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif 

namespace RacerNet
{
	U32 ScoreboardId(const eScoreboards sb)
	{
		return (U32)( sb + 1 );

		BIZ_ASSERT_FAIL("Requesting invalid scoreboard id");
		return 0;
	}

	void MpBuildScoreboardColumnsToWrite(const eScoreboards sb, const MpScoreboardDataForEvent& data, ScoreboardColumnWriteList& out)
	{
		out.Clear(); 
		Scoreboard::ColumnData temp;

		temp.SetS64(0, data.m_GameRank);
		out.PushBack(temp);

		temp.SetS64(1, data.m_TotalFans);
		out.PushBack(temp);

		temp.SetS64(2, data.m_TotalTimePlayed);
		out.PushBack(temp);

		temp.SetS64(3, data.m_RacePoints);
		out.PushBack(temp);

		temp.SetS64(4, data.m_TopThree);
		out.PushBack(temp);

		temp.SetS64(5, data.m_TotalRaces);
		out.PushBack(temp);

		temp.SetS64(6, data.m_RaceWins);
		out.PushBack(temp);

		temp.SetS64(7, data.m_PowerUpHits);
		out.PushBack(temp);

		temp.SetS64(8, data.m_PowerUpsFired);
		out.PushBack(temp);

		temp.SetS64(9, data.m_CarsWrecked);
		out.PushBack(temp);

		temp.SetS64(10, data.m_LegendLevel);
		out.PushBack(temp);
		
		temp.SetS64(11, data.m_TimeToAchieveLegend);
		out.PushBack(temp);
	}

	void SpBuildScoreboardColumnsToWrite(const eSPScoreboardTypes sb, const SpScoreboardDataForEvent& data, ScoreboardColumnWriteList& out)
	{
		out.Clear(); 
		Scoreboard::ColumnData temp;


		temp.SetS64(0, data.m_TotalFans);
		out.PushBack(temp);

		temp.SetS64(1, data.m_LapTime);
		out.PushBack(temp);

		temp.SetS64(2, data.m_CompTime);
		out.PushBack(temp);

		temp.SetS64(3, data.m_Wrecks);
		out.PushBack(temp);

		temp.SetS64(4, data.m_VehicleId);
		out.PushBack(temp);

		temp.SetS64(5, data.m_Difficulty);
		out.PushBack(temp);
	}

} // end namespace RacerNet


#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
