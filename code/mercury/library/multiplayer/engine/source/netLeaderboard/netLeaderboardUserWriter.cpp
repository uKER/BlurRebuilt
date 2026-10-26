
#include "stdafx.h"
#include "netLeaderboardUserWriter.h"
#include "netLeaderboardWriteData.h"
#include "netSession/netSession.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLeaderboardUserWriter::netLeaderboardUserWriter()
{
	m_state = STATE_NOT_SET;
}

netLeaderboardUserWriter::~netLeaderboardUserWriter()
{
}

netLeaderboardWriteDataRef netLeaderboardUserWriter::findLeaderboard(int scoreboardId)
{
	bdLinkedList<netLeaderboardWriteDataRef>::Position pos = m_leaderboards.getHeadPosition();

	while(pos)
	{
		netLeaderboardWriteDataRef leaderboard = m_leaderboards.forward(pos);

		if(leaderboard->m_scoreboardId == scoreboardId)
		{
			return leaderboard;
		}
	}
	return NULL;
}

netLeaderboardWriteDataRef netLeaderboardUserWriter::createLeaderboard(int scoreboardId)
{
	netLeaderboardWriteDataRef leaderboard = new netLeaderboardWriteData(scoreboardId);
	m_leaderboards.addHead(leaderboard);
	return leaderboard;
}

bool netLeaderboardUserWriter::setRating(int scoreboardId, S64 rating)
{
	netLeaderboardWriteDataRef leaderboard = findLeaderboard(scoreboardId);
	
	if(leaderboard.isNull())
		leaderboard = createLeaderboard(scoreboardId);

	leaderboard->m_rating = rating;
	return true;
}


bool netLeaderboardUserWriter::setRowData(int scoreboardId, Scoreboard::RowData* row)
{
	netLeaderboardWriteDataRef leaderboard = findLeaderboard(scoreboardId);
	
	m_user = row->m_Userdata;

	if(leaderboard.isNull())
		leaderboard = createLeaderboard(scoreboardId);

	leaderboard->m_rating = row->m_Rating;

	for(int i = 0; i < row->m_Columns.Size(); i ++)
	{
		leaderboard->setProperty(&(row->m_Columns[i]));
	}
	return true;
}
	
bool netLeaderboardUserWriter::setWriteType(int scoreboardId, Scoreboard::eWriteType writeType)
{
	netLeaderboardWriteDataRef leaderboard = findLeaderboard(scoreboardId);
	
	if(leaderboard.isNull())
		leaderboard = createLeaderboard(scoreboardId);

	leaderboard->m_writeType = writeType;
	return true;
}

bool netLeaderboardUserWriter::setPropertyS32(int scoreboardId, U32 fieldId, S32 value)
{
	netLeaderboardWriteDataRef leaderboard = findLeaderboard(scoreboardId);
	
	if(leaderboard.isNull())
		leaderboard = createLeaderboard(scoreboardId);

	leaderboard->setPropertyS32(fieldId, value);
	return true;
}

bool netLeaderboardUserWriter::setPropertyF32(int scoreboardId, U32 fieldId, F32 value)
{
	netLeaderboardWriteDataRef leaderboard = findLeaderboard(scoreboardId);
	
	if(leaderboard.isNull())
		leaderboard = createLeaderboard(scoreboardId);

	leaderboard->setPropertyF32(fieldId, value);
	return true;
}

bool netLeaderboardUserWriter::setPropertyS64(int scoreboardId, U32 fieldId, S64 value)
{
	netLeaderboardWriteDataRef leaderboard = findLeaderboard(scoreboardId);
	
	if(leaderboard.isNull())
		leaderboard = createLeaderboard(scoreboardId);

	leaderboard->setPropertyS64(fieldId, value);
	return true;
}

bool netLeaderboardUserWriter::setPropertyF64(int scoreboardId, U32 fieldId, F64 value)
{
	netLeaderboardWriteDataRef leaderboard = findLeaderboard(scoreboardId);
	
	if(leaderboard.isNull())
		leaderboard = createLeaderboard(scoreboardId);

	leaderboard->setPropertyF64(fieldId, value);
	return true;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

