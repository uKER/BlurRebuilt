// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Client-side interface to the voterank service. 

#ifndef BD_VOTERANK_MANAGEMENT_H
#define BD_VOTERANK_MANAGEMENT_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdRatingInfo;

BD_REFERENCE(bdConnection);

// Task IDs
#define BD_VOTERANK_TASK_SUBMIT_RATING (1u)

// #defines
#define BD_MAX_NUM_VOTES (20u)

/// Client-side interface to the voterank service. 
/// Allows submission of ratings.
/// Ratings are stored in leaderboards.  
/// To read ratings, use the bdStats API, supplying the category specific leaderboardID.
/// WebIMP will be updated in future to handle category specification and code generation.
/// Until then, developers must supply a list of categories to DemonWare.
/// An enum (bdCategoryLBID) of category specific leaderboardIDs will then be returned along with an updated WebIMP file.
class bdVoteRank
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS		

		~bdVoteRank();

		/// Submit rating(s) to the service
		/// \param ratings[in]			Array of bdRatingInfo objects to submit.
		///								Rating values are limited to 0-10 inclusive.
		/// \param numRatings           number of ratings submitted
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef submitRating(bdRatingInfo ratings[],
									const bdUInt numRatings);


		/// Looking for a readRating function?
		/// Use the bdLobby\bdStats API to read from bdVoteRank leaderboards directly.
		/// Once leaderboard entries have been obtained, you can get multiple files metadata 
		/// via the bdContentStreaming::getFileMetaDataByID() function.

	protected:

		friend class bdLobbyService;

		/// Constructs a voterank manager with the given task handler.
		/// Note: never call this constructor directly, use bdLobbyService::getVoterank() instead.
		/// \param remoteTaskManager[in] A pointer to the remote task manager that handles
		/// requests and replies
		bdVoteRank(bdRemoteTaskManager *const remoteTaskManager);

		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif // BD_VOTERANK_MANAGEMENT_H
