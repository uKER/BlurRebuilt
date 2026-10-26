
#include "stdafx.h"
#include "./netLeaderboardReaderByUsers.h"
#include "./netLeaderboardReadResult.h"
#include "netDebug/netDebug.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLeaderboardReaderByUsers::netLeaderboardReaderByUsers() 
: netLeaderboardReader(netLeaderboardReadQuery::MODE_USERS)
{ 
	Initialise(); 
}

bool netLeaderboardReaderByUsers::stopAndCleanup()
{
	StopAndCleanup();
	return true;
}

bool netLeaderboardReaderByUsers::startReading()
{
	// sanity check
	assert(m_query->m_type == netLeaderboardReadQuery::MODE_USERS);
	
	StopAndCleanup();

	if(m_nextBatch == NULL)
		return false;

	// watermarck the first scoreboard we are reading from the list
	m_currentBatch = m_nextBatch;
	if(m_currentBatch == NULL) return false;
	int count=0;
	
	// queue leaderboards
	bool flush = true;

	// how many scoreboardss we can add per call.
	int batchSize = m_query->m_batchSize;
	
	// there is a platform limit.
	if(batchSize > (int)Scoreboard::C_ReaderByUsers::MaxScoreboardsToReadFrom()) 
		batchSize = (int)Scoreboard::C_ReaderByUsers::MaxScoreboardsToReadFrom();

	// batch calls
	while ((m_nextBatch) && (count < batchSize))
	{
		count++;
		netLeaderboardReadQueryDataRef lb = m_query->m_leaderboards.forward(m_nextBatch);
	
		int	id = lb->m_leaderboardId;
		
		// setup the columns
		Scoreboard::ColumnIdentifierList columns;
		Scoreboard::ColumnTypeList		 columnTypes;
		if(!lb->toColumns(columns, columnTypes))
			return false;
		
		LOGINFO(netDebug::g_Log, "<netLeaderboardReaderByRank> adding scoreboard(%d)...", id);

		if(!AddScoreboard(id, columns, columnTypes, flush))
			return false;
		
		flush = false;
	}

	// cache users
	int usersCount = m_query->m_users.getSize();
	SysProfile::UserIdentifier* users = new SysProfile::UserIdentifier[usersCount];
	bdLinkedList<SysProfile::UserIdentifier>::Position p = m_query->m_users.getHeadPosition();
	int i = 0;
	while (p)
	{
		users[i] = m_query->m_users.forward(p);
		i++;
	}
	// set users
	if(!SetUsers(users, usersCount))
	{
		delete[] users;
		return false;
	}	

	delete[] users;
	return Start(m_query->m_titleId);
}

bool netLeaderboardReaderByUsers::continueReading(bool& error)
{
	error = false;
	bool finished = Continue(error);
	if(!finished) return true;
	if(error) return false;

	if(m_currentBatch == NULL)
		return false;
		
	// setup the data
	bdLinkedList<netLeaderboardReadQueryDataRef>::Position iterator = m_currentBatch;
	while (iterator != m_nextBatch)
	{
		netLeaderboardReadQueryDataRef lb = m_query->m_leaderboards.forward(iterator);

		int	id = lb->m_leaderboardId;
		netLeaderboardReadResultsRef results	= m_query->m_results;
		netLeaderboardReadDataRef leaderboard	= results->getLeaderboard(id);
		if(leaderboard.isNull()) leaderboard	= results->createLeaderboard(id);
	
		int rowCount = m_query->rowCount();
		Scoreboard::RowData* rows = new Scoreboard::RowData[rowCount];
	
		int rowsExtracted = ExtractRows(id, rows, rowCount);
		leaderboard->m_totalEntries = rowCount;
		assert(rowsExtracted <= rowCount);
		for(int i = 0; i < rowsExtracted; i ++)
		{
			leaderboard->updateRow(&rows[i]);
		}
		delete[] rows;
	}
	return false;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

