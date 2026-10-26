// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdVoteRank/bdVoteRank.h>
#include <bdLobby/bdVoteRank/bdRatingInfo.h>

#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>

#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>

#define TYPE_CHECKED true

bdVoteRank::bdVoteRank(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}

bdVoteRank::~bdVoteRank()
{
	m_remoteTaskManager = BD_NULL;
}


bdRemoteTaskRef bdVoteRank::submitRating(bdRatingInfo ratings[],
										const bdUInt numRatings)
{
	BD_ASSERT(BD_MAX_NUM_VOTES >= numRatings, "Max Vote Submission Exceeded\n");
	
	bdRemoteTaskRef task = BD_NULL;

	bdUInt taskSize = BD_TASK_OVERHEAD;
	for (bdUInt i=0; i<numRatings; i++)
	{
		taskSize += ratings[i].sizeOf();
	}
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_VOTERANK_SERVICE, BD_VOTERANK_TASK_SUBMIT_RATING);

	for (bdUInt i=0; i<numRatings; i++)
	{
		ratings[i].serialize(buffer);
	}

	m_remoteTaskManager->startTask(task, buffer);
	return task;	
}
