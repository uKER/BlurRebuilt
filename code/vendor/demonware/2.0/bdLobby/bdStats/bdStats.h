// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:	The client-side API for writing, deleting and reading user or team
//			statistics.

#ifndef BD_STATS_H
#define BD_STATS_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdMatchMaking/bdSessionID.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdStatsInfo;
class bdArbitratedStatsInfo;
class bdSessionNonce;

// Stats operations
#define BD_STAT_WRITE						(1u)
#define BD_STAT_DELETE						(2u)
#define BD_STAT_READ_BY_ENTITY_ID			(3u)
#define	BD_STAT_READ_LEADERBOARD_BY_RANK	(4u)
#define	BD_STAT_READ_LEADERBOARD_BY_PIVOT	(5u)
#define	BD_STAT_READ_LEADERBOARD_BY_RATING	(6u)

#define BD_STAT_START_ARBITRATED_SESSION	(1u)
#define BD_STAT_WRITE_ARBITRATED_STATS		(2u)

#define BD_STAT_DELETE_FILE_STATS			(1u)
#define BD_STAT_DELETE_FILE_AND_STATS		(2u)


/// The client-side API for writing, reading and deleting user or team statistics.
/// The statistics service manages an arbitrary number of leaderboards for a
/// title. A user or team can have entries in multiple leaderboards. Users or
/// teams are generically referred to as an \a entity or the purposes of this class.
/// Each entry has a compulsory \a rating attribute, on which the leaderboard
/// is ranked.
/// Each leaderboard can also define an arbitrary number of custom attributes for
/// each entry, which can be used to store user or team information relevant to the
/// leaderboard, and which are returned in the result data for queries of that
/// leaderboard. Custom attributes can be configured for a title through WebIMP.
class bdStats
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdWriteType
		{
			STAT_WRITE_REPLACE = 0,
			STAT_WRITE_ADD,
			STAT_WRITE_MAX,
			STAT_WRITE_MIN,
			
			STAT_WRITE_REPLACE_WHEN_RATING_INCREASE,
			STAT_WRITE_ADD_WHEN_RATING_INCREASE,
			STAT_WRITE_MAX_WHEN_RATING_INCREASE,
			STAT_WRITE_MIN_WHEN_RATING_INCREASE

		};

		~bdStats();

		/// Submit a list of leaderboard entries to the statistics service. 
		/// Dedicated servers may use this function to write statistics for multiple 
		/// users and/or teams to multiple leaderboards. If some of the userIDs / teamIDs
		/// provided are invalid (don't exist), entries for them will be ignored, but all
		/// other entries will be written as usual.
		///
		/// Ordinary users may use this function to write their own statistics, or statistics
		/// for teams they are members of. Attempts to provide userIDs of other users,
		/// wrong teamIDs, etc. will result in the entire write being denied.
		///
		/// \param stats[in]			Array of pointers to bdStatsInfo objects.
		/// \param numStats[in]			Size of the stats array
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	This function can be used to write across multiple leaderboards. Because different
		///			leaderboards may have different custom attributes, this function takes an array
		///			of POINTERS to bdStatsInfo objects. For each leaderboard create a subclass of
		///			bdStatsInfo to match the custom attributes. Each element of the stats array then needs
		///			to be initialized to point to an instance of the appropriate subclass.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success, accompanied by a bdWriteMultiStatsResult.
		///			BD_INVALID_STATS_SET - when some of the stats submissions were incorrectly
		///			initialized; when bdStatsInfo objects provided don't match the MM+ database
		///			configuration.
		///			BD_ACCESS_DENIED - when an ordinary user attempts to write for other users, or
		///			for teams that the user is not part of; when attempting to write to arbitrated
		///			leaderboards; when an ordinary user attempts to write to a dedicated leaderboard.
		bdRemoteTaskRef writeStats(	bdStatsInfo* stats[],
									const bdUInt numStats);


		/// Read entries from a specified leaderboard, based on ranked position.
		/// \param leaderBoardID[in]	The ID of the leaderboard to read entries
		///								from.
		/// \param firstRank [in]		The highest rank for which stats entries
		///								should be returned.
		///								A value of one specifies the top of the
		///								leaderboard.
		/// \param stats[out]			Pointer to an array of bdStatsInfo objects in which the result set is stored.
		///								The size of the array has to match the value specified in maxResults.
		/// \param maxResults [in]		The maximum number of stats entries to return.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard.
		bdRemoteTaskRef readStatsByRank(const bdLeaderBoardID leaderBoardID,
										const bdUInt64 firstRank,
										bdStatsInfo stats[],
										const bdUInt32 maxResults);

		/// Read entries from a specified leaderboard, based on ranked position.
		/// \param leaderBoardID[in]	The ID of the leaderboard to read entries
		///								from.
		/// \param firstRank [in]		The highest rank for which stats entries
		///								should be returned.
		///								A value of one specifies the top of the
		///								leaderboard.
		/// \param stats[out]			Array of pointers to bdStatsInfo objects in which the result set is stored.
		///								The size of the array has to match the value specified in maxResults, and each
		///								element in the array has to point to a valid bdStatsInfo object or subclasses thereof.
		/// \param maxResults [in]		The maximum number of stats entries to return.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard.
		bdRemoteTaskRef readStatsByRank(const bdLeaderBoardID leaderBoardID,
										const bdUInt64 firstRank,
										bdStatsInfo* stats[],
										const bdUInt32 maxResults);
	
	
		/// Read entries from a specified leaderboard, using a specified user ID or
		///	team ID as a pivot. The operation will attempt to return entries
		///	ranked above and below the specified ID, with the specified ID
		///	at the center rank.
		/// \param leaderBoardID[in]	The ID of the leaderboard to read entries
		///								from.
		/// \param entityID [in]		EntityID of the user or team to use as a
		///								center-point for a range of stats to be returned.
		/// \param stats[out]			Pointer to an array of bdStatsInfo objects in which the result set is stored.
		///								The size of the array has to match the value specified in maxResults.
		/// \param maxResults [in]		The maximum number of stats entries to return.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard.
		bdRemoteTaskRef readStatsByPivot(	const bdLeaderBoardID leaderBoardID,
											const bdEntityID entityID,
											bdStatsInfo stats[],
											const bdUInt32 maxResults);

		/// Read entries from a specified leaderboard, using a specified user ID or
		///	team ID as a pivot. The operation will attempt to return entries
		///	ranked above and below the specified ID, with the specified ID
		///	at the center rank.
		/// \param leaderBoardID[in]	The ID of the leaderboard to read entries
		///								from.
		/// \param entityID [in]		EntityID of the user or team to use as a
		///								center-point for a range of stats to be returned.
		/// \param stats[out]			Array of pointers to bdStatsInfo objects in which the result set is stored.
		///								The size of the array has to match the value specified in maxResults, and each
		///								element in the array has to point to a valid bdStatsInfo object or subclasses thereof.
		/// \param maxResults [in]		The maximum number of stats entries to return.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard.
		bdRemoteTaskRef readStatsByPivot(	const bdLeaderBoardID leaderBoardID,
											const bdEntityID entityID,
											bdStatsInfo* stats[],
											const bdUInt32 maxResults);

		/// Read entries from a specified leaderboard, based on rating.
		/// used to read a specified number of entries, starting at a specified rating.
		/// \param leaderBoardID[in]	The ID of the leaderboard to read entries
		///								from.
		/// \param rating [in]		The highest rating for which stats entries
		///								should be returned.
		/// \param stats[out]			Pointer to an array of bdStatsInfo objects in which the result set is stored.
		///								The size of the array has to match the value specified in maxResults.
		/// \param maxResults [in]		The maximum number of stats entries to return.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When this task completes, the results will be set in passed-in stats array.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard.
		bdRemoteTaskRef readStatsByRating(	const bdLeaderBoardID leaderBoardID,
											const bdInt64 rating,
											bdStatsInfo stats[],
											const bdUInt32 maxResults);

		/// Read entries from a specified leaderboard, based on rating.
		/// used to read a specified number of entries, starting at a specified rating.
		/// \param leaderBoardID[in]	The ID of the leaderboard to read entries
		///								from.
		/// \param rating [in]		The highest rating for which stats entries
		///								should be returned.
		/// \param stats[out]			Array of pointers to bdStatsInfo objects in which the result set is stored.
		///								The size of the array has to match the value specified in maxResults, and each
		///								element in the array has to point to a valid bdStatsInfo object or subclasses thereof.
		/// \param maxResults [in]		The maximum number of stats entries to return.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When this task completes, the results will be set in passed-in stats array.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard.
		bdRemoteTaskRef readStatsByRating(	const bdLeaderBoardID leaderBoardID,
											const bdInt64 rating,
											bdStatsInfo* stats[],
											const bdUInt32 maxResults);

		/// Read entries from a specified leaderboard, based on an array of user
		/// IDs or teamIDs.
		/// \param leaderBoardID[in]	The ID of the leaderboard to read from.
		/// \param entityIDs[in]		An array of team IDs or user IDs, whose
		///								stats will be read.
		/// \param numEntityIDs[in]		Size of the entityIDs array.
		/// \param stats[out]			A pointer to a bdStatsInfo array in which the result set will be stored.
		///								The size of the array has to match the size specified in numEntityIDs.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard.
		bdRemoteTaskRef readStatsByEntityID(const bdLeaderBoardID leaderBoardID,
											const bdEntityID entityIDs[],
											const bdUInt numEntityIDs,
											bdStatsInfo stats[]);

		/// Read entries from a specified leaderboard, based on an array of user
		/// IDs or teamIDs.
		/// \param leaderBoardID[in]	The ID of the leaderboard to read from.
		/// \param entityIDs[in]		An array of team IDs or user IDs, whose
		///								stats will be read.
		/// \param numEntityIDs[in]		Size of the entityIDs array.
		/// \param stats[out]			Array of pointers to bdStatsInfo objects in which the result set is stored.
		///								The size of the array has to match the value specified in maxResults, and each
		///								element in the array has to point to a valid bdStatsInfo object or subclasses thereof.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard.
		bdRemoteTaskRef readStatsByEntityID(const bdLeaderBoardID leaderBoardID,
											const bdEntityID entityIDs[],
											const bdUInt numEntityIDs,
											bdStatsInfo* stats[]);

		/// Starts an arbitrated session on the server, one is allowed per bdSecurityID.
		/// This returns a nonce value which is valid for this arbitrated session.
		/// Can be re-called to re-retrieve arbitratedSessionNonce.
		/// \param sessionID[in]	The session ID that identifies the matchmaking session.
		/// \param sessionNonce[out]	Pointer to store the session nonce returned by this task.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef startArbitratedSession (const bdSessionID sessionID,
												bdSessionNonce *const sessionNonce);

		/// Writes a set of statistics to a single leaderboard for all users in a single arbitrated stats session.
		/// \param leaderboardID[in] The leaderboard ID for this arbitrated statistics set.
		/// \param sessionID[in]	The MatchMaking session ID that was used to start 
		///							the Arbitrated Stats session.
		/// \param nonce[in]		The nonce returned by startArbitratedSession() for the
		///							corresponding bdSecurity.
		/// \param stats[in]		Pointer to an array of bdArbitratedStatsInfo objects which contains the players leaderboard entries.
		/// \param numStats[in]		The size of the stats array.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if the leaderboard specified does not exist.
		///			BD_INVALID_STATS_SET - if the parameters in stats do not match the
		///			schema of the specified leaderboard.
		bdRemoteTaskRef writeArbitratedStats (const bdUInt32 leaderboardID,
											  const bdSessionID sessionID,
                                              const bdSessionNonce *const arbitrationNonce,
                                              bdArbitratedStatsInfo *stats[],
                                              const bdUInt32 numStats);


		/// Delete an entry from a leaderboard.
		/// This can be used to reset an entry, since the stats will be recreated
		/// the next time an entry is written to this leaderboard.
		/// \param leaderBoardID[in]	The ID of the leaderboard to delete stats from
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard or not
		///			a valid teams leaderboard.
		bdRemoteTaskRef deleteStats(const bdLeaderBoardID leaderBoardID);

		/// Delete an entry from a file leaderboard.
		/// \param fileID[in]	The file ID of the leaderboard entry to delete stats from
		/// \param leaderBoardID[in]	The ID of the leaderboard to delete stats from
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_INVALID_LEADERBOARD_ID - if leaderBoardID is not a valid leaderboard or not
		///			a valid file leaderboard.
		bdRemoteTaskRef deleteFileStats(const bdUInt64 fileID, const bdLeaderBoardID leaderBoardID);

		/// Delete a file and its corresponding entry from a list of leaderboards.
		/// \param fileID[in]	The file ID of the leaderboard entry to delete stats from
		/// \param numLeaderboardsIDs[in]	The number if leaderboards
		/// \param leaderBoardIDs[in]	List of leaderboard ids to delete stats from
		bdRemoteTaskRef deleteFileAndStats(const bdUInt64 fileID, const bdUInt numLeaderboardsIDs, const bdLeaderBoardID *const leaderBoardIDs);
	protected:

		friend class bdLobbyService;

		bdStats(bdRemoteTaskManager *const remoteTaskManager);

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif // BD_STATS_H
