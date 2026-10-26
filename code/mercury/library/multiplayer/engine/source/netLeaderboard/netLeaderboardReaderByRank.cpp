
#include "stdafx.h"
#include "./netLeaderboardReaderByRank.h"
#include "./netLeaderboardReadResult.h"
#include "netDebug/netDebug.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLeaderboardReaderByRank::netLeaderboardReaderByRank() 
: netLeaderboardReader(netLeaderboardReadQuery::MODE_RANK)
{ 
	Initialise(); 
}

bool netLeaderboardReaderByRank::stopAndCleanup()
{
	StopAndCleanup();
	return true;
}

bool netLeaderboardReaderByRank::startReading()
{
	// sanity check
	assert(	m_query->m_type == netLeaderboardReadQuery::MODE_PIVOT ||
			m_query->m_type == netLeaderboardReadQuery::MODE_RANK);

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
	if(batchSize > (int) Scoreboard::C_ReaderByRank::MaxScoreboardsToReadFrom()) 
		batchSize = (int) Scoreboard::C_ReaderByRank::MaxScoreboardsToReadFrom();

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
	
	switch(m_query->m_type)
	{
	default:
		{
			assert(!"invalid search type.");
			return false;
		}
	case netLeaderboardReadQuery::MODE_PIVOT:
		{
			// start read
			return StartReadByPivot(m_query->m_titleId, m_query->m_pivot, m_query->m_pivotRange);
		}
	case netLeaderboardReadQuery::MODE_RANK:
		{
			// start read
			return StartReadByRank(m_query->m_titleId, m_query->m_rankStart, m_query->m_rankLimit - m_query->m_rankStart);
		}
	}
	return false;
}

bool netLeaderboardReaderByRank::continueReading(bool& error)
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
		netLeaderboardReadResultsRef results = m_query->m_results;
		netLeaderboardReadDataRef leaderboard = results->getLeaderboard(id);
		if(leaderboard.isNull()) leaderboard = results->createLeaderboard(id);
	
		int rowCount = m_query->rowCount();
		
		MEMORY_TRACKER_PUSH("network.multiplayer");
		Scoreboard::RowData* rows = new Scoreboard::RowData[rowCount];
		
		U32 totalEntries;
		int rowsExtracted = ExtractRows(id, rows, rowCount, totalEntries);
		assert(rowsExtracted <= rowCount);
		leaderboard->m_totalEntries = totalEntries;
		for(int i = 0; i < rowsExtracted; i ++)
		{
			leaderboard->updateRow(&rows[i]);
		}
		delete[] rows;
		MEMORY_TRACKER_POP("network.multiplayer");
	}
	
	error = false;
	return false;
}


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

