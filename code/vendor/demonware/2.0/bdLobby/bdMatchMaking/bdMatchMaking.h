// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE : Advertise and discover game session(s) on Matchmaking+.

#ifndef BD_MATCH_MAKING_H
#define BD_MATCH_MAKING_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdMatchMaking/bdSessionID.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdMatchMakingInfo;
class bdSessionParams;
class bdPerformanceValue;
class bdSessionInvite;

// MatchMaking Operations
#define BD_MM_CREATE_SESSION			(1u)
#define BD_MM_UPDATE_SESSION			(2u)
#define BD_MM_DELETE_SESSION			(3u)
#define BD_MM_FIND_SESSION_BY_ID		(4u)
#define BD_MM_FIND_SESSIONS				(5u)
#define BD_MM_NOTIFY_JOIN				(6u)
#define BD_MM_NOTIFY_LEAVE				(7u)
#define BD_MM_INVITE_TO_SESSION			(8u)
#define BD_MM_SUBMIT_PERFORMANCE		(9u)
#define BD_MM_GET_PERFORMANCE			(10u)
#define BD_MM_GET_SESSION_INVITES		(11u)

/// Advertise and discover game session(s) on Matchmaking+.
class bdMatchMaking
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Destructor
		~bdMatchMaking();

		/// Advertise a game session.
		/// \param sessionParams[in]    A pointer to an instance of 
		///								bdMatchMakingInfo that contains session 
		///								specific parameters. The implementation
		///								of this class must match the custom 
		///								parameters that have been configured for
		///								your title ID on Matchmaking+.
		/// \param sessionID[out]		After successful creation, sessionID will
		///								contain the session ID of the advertised game session
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When this task completes, sessionID  is updated to contain  
		///         the session ID that has been assigned to this session. 
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_PARAM_MISMATCHED_TYPE - if the parameters specified in params
		///			do not match the schema of the onlinegames table.
		bdRemoteTaskRef createSession(bdMatchMakingInfo *sessionParams,
									  bdSessionID *const sessionID);

		/// Update an existing session.
		/// \param session ID[in]		The session ID of the session to be updated.
		/// \param sessionInfo[in]		A pointer to an instance of 
		///								bdMatchMakingInfo that contains session 
		///								specific parameters. The implementation
		///								of this class must match the custom 
		///								parameters that have been configured for
		///								your title ID on Matchmaking+. 
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_PARAM_MISMATCHED_TYPE - if the parameters specified in params
		///			do not match the schema of the onlinegames table.
		///			BD_NO_ENTRY_TO_UPDATE - if no online game was found with a
		///			session ID matching the one specified in sessionInfo.
		bdRemoteTaskRef updateSession(const bdSessionID& sessionID,
									  bdMatchMakingInfo *const sessionInfo);

		/// Stop advertising a session.
		/// \param sessionID[in]		The session ID of the session to be 
		///								deleted.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef deleteSession(const bdSessionID& sessionID);

		/// Find the details of the online session with a given session ID.
		/// \param sessionID[in]	The session ID of the session to be found.
		/// \param sessionInfo[out] A pointer to bdMatchMakingInfo or derived class
		///							which will contain the advertised session information
		///							for the specified session ID 
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef findSessionFromID(const bdSessionID& sessionID,
										  bdMatchMakingInfo *const sessionInfo);
	

		/// Find any online sessions that match the specified parameters.
		/// \param queryID[in]		 Specifies the custom query to be performed.
		///	\param startIndex[in]	 Offset into the result set. Used for paging through large result sets.
		/// \param maxNumResults[in] Maximum number of results to be returned by the server.
		/// \param sessionParams[in] Custom session attributes for the query.
		///	\param results[out]	     A pointer to an array of bdMatchMakingInfo objects or
		///							 derived classes thereof, which will contain the result set
		///						     for the query when the task has completed. The size of the array
		///						     has to match the size as specified by maxNumResults.				
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_QUERY_ID - if the queryID specified in params does not
		///			exist.
		///			BD_PARAM_PARSE_ERROR - if the parameters specified in the custom
		///			query are not found in the schema for the onlinegames table. 
		bdRemoteTaskRef findSessions(const bdUInt queryID,
									 const bdUInt startIndex,
                                     const bdUInt maxNumResults,
									 bdSessionParams *sessionParams,
									 bdMatchMakingInfo results[]);

		/// Invite players to a game session. The invited players will receive
		/// an onSessionInvite() event in case they are online, or as soon as they go online and
		/// the session is still in progress.
		/// If the current user has already invited any of the players in \a userIDs to the
		/// game session identified by \a sessionID, the call will fail with a session invite exists error.
		/// \params sessionID[in]	The session ID of the game session
		/// \params userIDs[in]		An array of user IDs of the invited players
		/// \params numUsers[in]	The number of players to be invited
		/// \params attachment[in]  Optional attachment to be sent to the invited players
		/// \params attachmentSize[in] Size of the attachment in bytes.
		/// \note	Invited users will receive an onSessionInvite() event.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_SESSION_INVITE_EXISTS -if the current user has already invited any of the players 
		///			in \a userIDs to the game session identified by \a sessionID.
		bdRemoteTaskRef inviteToSession(const bdSessionID &sessionID,
										const bdOnlineUserID userIDs[],
										const bdUInt numUsers,
										const void *const attachment = BD_NULL,
										const bdUInt attachmentSize = 0);
		
		/// Register a list of users as playing in a session owned by the caller.
		/// This function (along with notifyLeave) serves dual purpose: protection against
		/// users sharing their accounts and/or license keys; and to update the average
		/// performance of players in a session when using performance based matchmaking.
		///
		/// This function may only be used by the session host (the host that called \a createSession),
		/// otherwise MM+ will return an access denied error.
		///
		/// When a user registered with this function disconnects from MM+ servers for any reason
		/// the session host is sent a \a bdMMPlayerDisconnectMessage push message. Recommended
		/// behavior is then to disconnect the user from the session locally, as the user may be
		/// engaging in cheating by trying to use the same user account on multiple game instances,
		/// or by sharing the license code.
		/// \param sessionID[in]	The session ID of the session to which users are being registered.
		/// \param numUsers[in]		Number of users in the \a users array. Must be greater than zero.
		/// \param users[in]		An array of \a bdOnlineUserID objects to be registered as playing the
		///							session.
		/// \return					A pointer to the task handle that should be pumped to completion,
		///							or BD_NULL if the operation could not be performed.
		/// \note	If a user is already marked on MM+ servers as disconnected, the notification message
		///			will be sent immediately. Due to race condition notification messages may arrive
		///			before the task started by this function terminates. Occasionally more than one message
		///			may be delivered for the same user.
		/// \note	Avoid calling this function when a user connects to the server, as this may result in
		///			high frequency of calls. Instead, maintain a list of users connected but not yet registered,
		///			and call this function with that list every 10 seconds or so.
		/// \note	Ensure that your server code does not allow two participants to play under the same user ID.
		bdRemoteTaskRef notifyJoin(	const bdSessionID& sessionID,
									const bdOnlineUserID *const users,
									const bdUInt numUsers);

		/// Notify MM+ servers that a list of users is no longer playing in a session owned by the caller.
		/// This function should be used to notify MM+ when users registered with \a notifyJoin disconnects
		/// from the game session. This is also used in conjunction with performance based matchmaking to
		/// update the average performance of an advertised game session.
		/// This function may only be used by the session host (the host that called \a advertiseSession),
		/// otherwise MM+ will return an access denied error.
		/// \param sessionID[in]	The session ID of the session to which users are being registered.
		/// \param numUsers[in]		Number of users in the \a users array. Must be greater than zero.
		/// \param users[in]		An array of \a bdOnlineUserID objects to be registered as having left the
		///							session.
		/// \return					A pointer to the task handle that should be pumped to completion,
		///							or BD_NULL if the operation could not be performed.
		/// \note	Due to race conditions notification messages may still arrive soon after this call terminates.
		/// \note	Avoid calling this function on every user disconnect, as this may result in high MM+ server load.
		///			Instead, batch the calls in a manner similar to \a notifyJoin.
		bdRemoteTaskRef notifyLeave(const bdSessionID& sessionID,
									const bdOnlineUserID *const users,
									const bdUInt numUsers);

		/// Record the performance of players or teams. 
		/// This function is used by the session host to record the outcome of a round or tournament.
		/// Each player is assigned a placing value representing their performance in the match.
		/// Players are then matched with players of similar skill. Players can have different performance
		/// values for each game type.
		/// \param gameType[in]	The type of the game as it was advertised in createSession()
		/// \param performanceValues[in] Array of performance values
		/// \param numValues[in]	Number of values to be submitted.
		/// \return					A reference to the task handle that should be pumped to completion,
		///							or BD_NULL if the operation could not be performed.
		bdRemoteTaskRef submitPerformance(const bdUInt gameType, 
										  bdPerformanceValue *performanceValues, 
										  const bdUInt numValues);

		/// Fetch a group of users' bdPerformanceValue from the server.
		/// \param entityIDs[in]	A pointer to an array of bdEntityID whose 
		///							bdPerformanceValue should be retrieved.
		/// \param numEntityIDs[in]	The number of elements in the \a entityIDs array.
		/// \param gameType[in]		The type of the game as it was advertised in createSession().
		/// \param results[out]		A pointer to an array of bdPerformanceValue objects,
		///							which will contain the result set for the query 
		///						    when the task has completed. The size of the array
		///						    has to match the size as specified by numEntityIDs.
		/// \return					A reference to the task handle that should be pumped to completion,
		///							or BD_NULL if the operation could not be performed.
		/// \note					When complete, the returned bdRemoteTask will specify:
		///							BD_NO_ERROR - on success. 
		///							BD_INVALID_USER_ID - if any of the user IDs specified in params are
		///							invalid, or are duplicates.
		bdRemoteTaskRef getPerformanceValues(	const bdEntityID *const entityIDs,
												const bdUInt numEntityIDs, 
												const bdUInt gameType,
												bdPerformanceValue results[]);

		/// Retrieve current session invites for the user.
		/// \param maxNumResults[in]	Maximum number of results to return.
		/// \param offset[in]	An offset into the invite list.
		/// \param results[out]	A pointer to an array of bdSessionInvite objects,
		///						which will contain the result set for the query
		///						when the task has completed. The size of the array
		///						has to match the size as specified by maxNumResults.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef getSessionInvites(	const bdUInt32 startIndex,
											const bdUInt32 maxNumResults,
											bdSessionInvite results[]);


	protected:

		friend class bdLobbyService;

		/// Constructor
		/// \param remoteTaskManager[in]
		bdMatchMaking(bdRemoteTaskManager *const remoteTaskManager);

		bdRemoteTaskRef startNotifyTask(const bdSessionID& sessionID,
										const bdOnlineUserID* users,
										const bdUInt numUsers,
										const bdUByte8 taskType);

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif // BD_MATCH_MAKING_H
