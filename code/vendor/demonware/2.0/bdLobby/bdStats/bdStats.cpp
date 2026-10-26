// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdStats/bdStats.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdStats/bdStatsInfo.h>
#include <bdLobby/bdStats/bdArbitratedStatsInfo.h>
#include <bdLobby/bdStats/bdSessionNonce.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

#define BD_LOG_LEVEL "stats"
#define TYPE_CHECKED true

bdStats::bdStats(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}

bdStats::~bdStats()
{
	m_remoteTaskManager = BD_NULL;
}

bdRemoteTaskRef bdStats::writeStats(bdStatsInfo *stats[],
									const bdUInt numStats)
{
	bdRemoteTaskRef task = BD_NULL;

	// first iteration: call serialize on an 0-length bytebuffer to count the number of bytes needed
	bdByteBuffer buffer(0, TYPE_CHECKED);
	for (bdUInt i=0; i<numStats; i++)
	{
		stats[i]->serialize(buffer);
	}
	buffer.write(BD_NULL, BD_TASK_OVERHEAD);

	// this allocates the actual amount of bytes needed
	buffer.allocateBuffer();

	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_WRITE);

	// second iteration: write the actual data
	for (bdUInt i=0; i<numStats; i++)
	{
		stats[i]->serialize(buffer);
	}

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}

	return task;
}

bdRemoteTaskRef bdStats::readStatsByRank(	const bdLeaderBoardID leaderBoardID,
											const bdUInt64 firstRank,
											bdStatsInfo stats[],
											const bdUInt32 maxResults)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID)+ sizeof(bdUInt64) + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_READ_LEADERBOARD_BY_RANK);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	wroteParam = wroteParam && buffer.writeUInt64(firstRank);
	wroteParam = wroteParam && buffer.writeUInt32(maxResults);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(stats, maxResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::readStatsByRank(const bdLeaderBoardID leaderBoardID,
                                         const bdUInt64 firstRank,
                                         bdStatsInfo* stats[],
                                         const bdUInt32 maxResults)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID)+ sizeof(bdUInt64) + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_READ_LEADERBOARD_BY_RANK);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	wroteParam = wroteParam && buffer.writeUInt64(firstRank);
	wroteParam = wroteParam && buffer.writeUInt32(maxResults);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResultList((bdTaskResult**)stats, maxResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}
	
bdRemoteTaskRef bdStats::readStatsByPivot(	const bdLeaderBoardID leaderBoardID,
											const bdEntityID entityID,
											bdStatsInfo stats[],
											const bdUInt32 maxResults)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + sizeof(bdEntityID) + sizeof(bdUInt32 ) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_READ_LEADERBOARD_BY_PIVOT);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	wroteParam = wroteParam && buffer.writeUInt64(entityID);
	wroteParam = wroteParam && buffer.writeUInt32(maxResults);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(stats, maxResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::readStatsByPivot(	const bdLeaderBoardID leaderBoardID,
										  const bdEntityID entityID,
										  bdStatsInfo* stats[],
										  const bdUInt32 maxResults)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + sizeof(bdEntityID) + sizeof(bdUInt32 ) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_READ_LEADERBOARD_BY_PIVOT);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	wroteParam = wroteParam && buffer.writeUInt64(entityID);
	wroteParam = wroteParam && buffer.writeUInt32(maxResults);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResultList((bdTaskResult**)stats, maxResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

	
bdRemoteTaskRef bdStats::readStatsByRating(	const bdLeaderBoardID leaderBoardID,
											const bdInt64 rating,
											bdStatsInfo stats[],
											const bdUInt32 maxResults)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + sizeof(bdInt64) + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_READ_LEADERBOARD_BY_RATING);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	wroteParam = wroteParam && buffer.writeInt64(rating);
	wroteParam = wroteParam && buffer.writeUInt32(maxResults);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(stats, maxResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::readStatsByRating(	const bdLeaderBoardID leaderBoardID,
										   const bdInt64 rating,
										   bdStatsInfo* stats[],
										   const bdUInt32 maxResults)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + sizeof(bdInt64) + sizeof(bdUInt32) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_READ_LEADERBOARD_BY_RATING);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	wroteParam = wroteParam && buffer.writeInt64(rating);
	wroteParam = wroteParam && buffer.writeUInt32(maxResults);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResultList((bdTaskResult**)stats, maxResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::readStatsByEntityID(	const bdLeaderBoardID leaderBoardID,
												const bdEntityID entityIDs[],
												const bdUInt numEntityIDs,
												bdStatsInfo stats[])
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1 + numEntityIDs;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + (sizeof(bdEntityID)*numEntityIDs) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_READ_BY_ENTITY_ID);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	for (bdUInt i = 0; i < numEntityIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(entityIDs[i]);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResult(stats, numEntityIDs);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::readStatsByEntityID(	const bdLeaderBoardID leaderBoardID,
											 const bdEntityID entityIDs[],
											 const bdUInt numEntityIDs,
											 bdStatsInfo* stats[])
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1 + numEntityIDs;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + (sizeof(bdEntityID)*numEntityIDs) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_READ_BY_ENTITY_ID);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	for (bdUInt i = 0; i < numEntityIDs; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(entityIDs[i]);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			task->setTaskResultList((bdTaskResult**)stats, numEntityIDs);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::startArbitratedSession (const bdSessionID sessionID,
                                                 bdSessionNonce *sessionNonce)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdSessionID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_ARBITRATION_SERVICE, BD_STAT_START_ARBITRATED_SESSION);

	sessionID.serialize(buffer);
	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		const bdUInt numResults = 1;
		task->setTaskResult(sessionNonce, numResults);
	}

	return task;
}

		
bdRemoteTaskRef bdStats::writeArbitratedStats(const bdUInt32 leaderBoardID,
											  const bdSessionID sessionID,
											  const bdSessionNonce *const sessionNonce,
                                              bdArbitratedStatsInfo *stats[],
											  const bdUInt32 numStats)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 3 + numStats;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdUInt32) + sizeof(bdSessionID) + sizeof(bdSessionNonce) + numParameters;

	// first iteration: call serialize on an 0-length bytebuffer to count the number of bytes needed
	bdByteBuffer buffer(0, TYPE_CHECKED);
	for (bdUInt i=0; i<numStats; i++)
	{
		stats[i]->serialize(buffer);
	}
	buffer.write(BD_NULL, taskSize);

	// this allocates the actual amount of bytes needed
	buffer.allocateBuffer();

	bdRemoteTaskManager::initTaskBuffer(buffer, BD_ARBITRATION_SERVICE, BD_STAT_WRITE_ARBITRATED_STATS);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	sessionID.serialize(buffer);
	sessionNonce->serialize(buffer);
	for (bdUInt32 i=0; i < numStats; i++)
	{	
		stats[i]->serialize(buffer);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::deleteStats(const bdLeaderBoardID leaderBoardID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STATS_SERVICE, BD_STAT_DELETE);

	bdBool wroteParam = buffer.writeUInt32(leaderBoardID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::deleteFileStats(const bdUInt64 fileID, const bdLeaderBoardID leaderBoardID)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FILE_STATS_SERVICE, BD_STAT_DELETE_FILE_STATS);
	bdBool wroteParam = buffer.writeUInt64(fileID);
	wroteParam = wroteParam && buffer.writeUInt32(leaderBoardID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}

bdRemoteTaskRef bdStats::deleteFileAndStats(const bdUInt64 fileID, const bdUInt numLeaderboards, const bdLeaderBoardID *const leaderBoardIDs)
{
	bdRemoteTaskRef task = BD_NULL;
	const bdUInt numParameters = 2 + numLeaderboards;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdLeaderBoardID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_FILE_STATS_SERVICE, BD_STAT_DELETE_FILE_AND_STATS);
	bdBool wroteParam = buffer.writeUInt64(fileID);
	wroteParam = wroteParam && buffer.writeUInt32(numLeaderboards);
	for (bdUInt i = 0; i < numLeaderboards; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt32(leaderBoardIDs[i]);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(task, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return task;
}
