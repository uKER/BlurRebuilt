// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE:

BD_LOBBY_ERROR_CODES_BEGIN

	// Indicates a successful operation.
	BD_LOBBY_ERROR_CODE(BD_NO_ERROR, 					0)

	//
	// Client-side-only errors
	//

	/// Maximum number of concurrent remote tasks exceeded
	BD_LOBBY_ERROR_CODE(BD_TOO_MANY_TASKS, 				1)

	/// Not connected to the Lobby Service
	BD_LOBBY_ERROR_CODE(BD_NOT_CONNECTED,  				2)

	/// Sending of message failed
	BD_LOBBY_ERROR_CODE(BD_SEND_FAILED,    				3)

	/// The handling of the task failed
	BD_LOBBY_ERROR_CODE(BD_HANDLE_TASK_FAILED,			4)

	/// Failed to start a task
	BD_LOBBY_ERROR_CODE(BD_START_TASK_FAILED,			5)

	//
	// General lobby service gateway errors
	//

	/// Disambiguation: BD_LOBBY_ERROR_CODE(100)
	/// This error code has 2 different meanings.
	/// 1) When returned by the LSG layer it represents:
 	///		The result returned by the lobby server exceeds the client's buffer size
	///		The default size of the receive buffer used by bdLobbyService is 64K.
	///		Larger buffer sizes can be specified using bdLobbyService::connect().
	/// 2) When returned by the python layer it represents:
	///		The function parameters supplied were malformed.
	/// \note There is no way to tell what layer is reporting the error.
	///       For further discussion on ongoing efforts to resolve
	///	      this please refer to the DemonWare bdBugTracker(tm).
	BD_LOBBY_ERROR_CODE(BD_RESULT_EXCEEDS_BUFFER_SIZE,	100)

	/// Access to requested resource denied.
	BD_LOBBY_ERROR_CODE(BD_ACCESS_DENIED,				101)

	/// General Exception. Please contact Demonware support
	BD_LOBBY_ERROR_CODE(BD_EXCEPTION_IN_DB,				102)

	/// General Exception. Please contact Demonware support
	BD_LOBBY_ERROR_CODE(BD_MALFORMED_TASK_HEADER,		103)

	/// Tried to read/write to a non-existent row in the DB.
	BD_LOBBY_ERROR_CODE(BD_INVALID_ROW,					104)

	/// An empty list of IDs was sent to the server in a request.
	BD_LOBBY_ERROR_CODE(BD_EMPTY_ARG_LIST,				105)

	/// The parameters passed to the Lobby Service are not valid.
    /// Please ensure that you are serializing the correct number and types
	/// of arguments for any customized services
	BD_LOBBY_ERROR_CODE(BD_PARAM_PARSE_ERROR,	 		106)

	/// Param of incorrect type sent. For example sending a bdInt32 to a task
	/// that expects a bdUInt32.
	BD_LOBBY_ERROR_CODE(BD_PARAM_MISMATCHED_TYPE,		107)

	/// Service not available for this titleID. Please contact Demonware support
	BD_LOBBY_ERROR_CODE(BD_SERVICE_NOT_AVAILABLE, 		108)

	/// The connection to the Authentication Service failed
	BD_LOBBY_ERROR_CODE(BD_CONNECTION_RESET,		 	109)

	/// LSG rejected the authentication ticket, because it was out of date, or invalid.
	BD_LOBBY_ERROR_CODE(BD_INVALID_USER_ID,			 	110)

	/// LSG does not understand this client's protocol version. Can indicate a mismatch
	/// between versions of LSG and client code.
	BD_LOBBY_ERROR_CODE(BD_LOBBY_PROTOCOL_VERSION_FAILURE, 111)

	/// LSG encountered an unexpected error. Please contact DemonWare support if the error
	/// persists.
	BD_LOBBY_ERROR_CODE(BD_LOBBY_INTERNAL_FAILURE,		112)

	/// Protocol error occurred.
	BD_LOBBY_ERROR_CODE(BD_LOBBY_PROTOCOL_ERROR,		113)


	//
	// Teams service errors
	//

	/// No outstanding proposal exists for team membership for this user
	/// Returned by bdTeams::acceptMembership or bdTeams::rejectMembership
	/// This can be caused by supplying incorrect parameters or when an existing
	/// proposal has been withdrawn using bdTeams::withdrawProposal
	BD_LOBBY_ERROR_CODE(BD_MEMBER_NO_PROPOSAL,			300)

	/// The team name supplied is already in use
	/// Returned by bdTeams::createTeam or bdTeams::updateTeam
	BD_LOBBY_ERROR_CODE(BD_TEAMNAME_ALREADY_EXISTS,		301)

	/// The maximum number of team memberships for this titleID has been reached.
	/// By default the limit is set to 3.
	/// Returned by bdTeams::createTeam or bdTeams::acceptMembership
	/// This value can be altered by contacting Demonware support.
	BD_LOBBY_ERROR_CODE(BD_MAX_TEAM_MEMBERSHIPS_LIMITED,	302)

	/// The maximum number of team ownerships for this titleID has been reached.
	/// By default the limit is set to 3.
	/// Returned by bdTeams::createTeam or bdTeams::leave
	/// This value can be altered by contacting Demonware support.
	BD_LOBBY_ERROR_CODE(BD_MAX_TEAM_OWNERSHIPS_LIMITED,	303)

	/// Operation not permitted by this user
	/// Returned by bdTeams::leaveTeam, bdTeams::getPrivateInfo, 
	/// bdTeams::getPrivateMemberInfo, & bdTeams::sendTeamMemberMessage
	BD_LOBBY_ERROR_CODE(BD_NOT_A_TEAM_MEMBER,			304)

	/// Incorrect TeamID supplied
	/// Returned by bdTeams::updateTeam bdTeams::promoteMemberToAdmin bdTeams::demoteMember
	/// bdTeams::kickMember bdTeams::leaveTeam bdTeams::proposeMembership
	/// bdTeams::rejectMembership bdTeams::acceptMembership
	BD_LOBBY_ERROR_CODE(BD_INVALID_TEAM_ID,				305)

	/// Incorrect Team Name supplied
	/// Returned by bdTeams::getTeamID
	BD_LOBBY_ERROR_CODE(BD_INVALID_TEAM_NAME,			306)

	/// Operation not permitted by this user
	/// Returned by bdTeams::promoteMemberToAdmin, bdTeams::demoteMember
	BD_LOBBY_ERROR_CODE(BD_NOT_A_TEAM_OWNER,			307)

	/// Operation not permitted by this user
	/// Returned by bdTeams::proposeMembership bdTeams::sendTeamAdminMessage
	BD_LOBBY_ERROR_CODE(BD_NOT_AN_ADMIN_OR_OWNER,		308)

	/// A membership proposal already exists for this user
	/// Returned by bdTeams::proposeMembership
	BD_LOBBY_ERROR_CODE(BD_MEMBER_PROPOSAL_EXISTS,		309)

	/// The proposed user is already a team member
	/// Returned by bdTeams::proposeMembership
	BD_LOBBY_ERROR_CODE(BD_MEMBER_EXISTS,				310)

	/// The team is full and no additional members can join.
	/// The option can be configured. Default: 100 members.
	BD_LOBBY_ERROR_CODE(BD_TEAM_FULL,					311)

	/// The team name is vulgar.
	/// This is returned if a team name fails a server-side
	/// vulgarity test, during team creation/updation.
	BD_LOBBY_ERROR_CODE(BD_VULGAR_TEAM_NAME,			312)

	/// The user is blocked by otherUser.
	/// This is returned if you proposeMembership to a user who
	/// has blocked you, etc...
	BD_LOBBY_ERROR_CODE(BD_TEAM_USERID_BANNED,				313)

	/// The team has no members online.
	/// This is returned if you send an instant message to a team
	/// that has no users online so the message is not delivered.
	BD_LOBBY_ERROR_CODE(BD_TEAM_EMPTY,					314)

	//
	// Stats service errors
	//

	/// Invalid Leaderboard ID supplied
	BD_LOBBY_ERROR_CODE(BD_INVALID_LEADERBOARD_ID,				400)

	/// Stats Values and data do not match.
	BD_LOBBY_ERROR_CODE(BD_INVALID_STATS_SET,					401)

	/// An empty stats set was submitted
	BD_LOBBY_ERROR_CODE(BD_EMPTY_STATS_SET_IGNORED,				403)

	/// An attempt was made to write stats directly to an
	/// arbitrated leaderboard.
	BD_LOBBY_ERROR_CODE(BD_NO_DIRECT_ACCESS_TO_ARBITRATED_LBS,	404)

	/// Permission to write stats was denied, only dedicated servers
	/// can write to dedicated leaderboards
	BD_LOBBY_ERROR_CODE(BD_STATS_WRITE_PERMISSION_DENIED,		405)

	/// If the write type does not make sense for one or more of
	/// the data types. [E.g. write type add on a string !]
	BD_LOBBY_ERROR_CODE(BD_STATS_WRITE_TYPE_DATA_TYPE_MISMATCH,	406)

	/// The user ID has no valid statistics set
	BD_LOBBY_ERROR_CODE(BD_NO_STATS_FOR_USER,					407)

	/// The leaderboard specified is un-ranked and the 
	/// task is not permitted on an un-ranked leaderboard
	BD_LOBBY_ERROR_CODE(BD_INVALID_ACCESS_TO_UNRANKED_LB,		408)


	//
	// More LSG errors
	//

	/// Title ID supplied is not configured on the LSG.
	BD_LOBBY_ERROR_CODE(BD_INVALID_TITLE_ID,			500)


	//
	// MessageChannel chat service errors
	//

	/// Attempted to retrieve a mail that does not exist.
	BD_LOBBY_ERROR_CODE(BD_MESSAGING_INVALID_MAIL_ID,	600)

	/// User tried to block themselves.
	BD_LOBBY_ERROR_CODE(BD_SELF_BLOCK_NOT_ALLOWED,		601)

	//
	// Authentication service errors.
	//

	/// No error occurred.
	/// This is returned when the authentication operation succeeded.
	BD_LOBBY_ERROR_CODE(BD_AUTH_NO_ERROR,					700)

	/// The authentication request was malformed.
	/// This is returned when the authentication service could not parse the
	/// request correctly.
	/// This may occur if an incorrect version of the client code is used
	/// to access the authentication service.
	BD_LOBBY_ERROR_CODE(BD_AUTH_BAD_REQUEST,				701)

	/// A generic authentication service error.
	/// Please contact DemonWare support.
	BD_LOBBY_ERROR_CODE(BD_AUTH_SERVER_CONFIG_ERROR,		702)

	/// The authentication service did not recognize the title ID that was
	/// used to access it with.
	BD_LOBBY_ERROR_CODE(BD_AUTH_BAD_TITLE_ID,				703)

	/// The authentication service did not recognize the user ID, username, or
	/// license ID that was used.
	/// The most common cause of this is an unknown username being passed to
	/// bdAuthService::authorizeAccount.
	BD_LOBBY_ERROR_CODE(BD_AUTH_BAD_ACCOUNT,				704)

	/// An attempt was made to perform an authentication operation that is
	/// not configured for the given title ID.
	/// For example, it is not possible to use licenses with the authentication
	/// service if they are not configured for the title ID. Similarly, use
	/// of bdAuthService::authorizeAnonymous requires special configuration.
	BD_LOBBY_ERROR_CODE(BD_AUTH_ILLEGAL_OPERATION,			705)

	/// An incorrect license code was passed to the
	/// bdAuthService::authorizeDedicatedHost function.
	BD_LOBBY_ERROR_CODE(BD_AUTH_INCORRECT_LICENSE_CODE,		706)

	/// The username that was passed to bdAuthService::createAccount already exists.
	BD_LOBBY_ERROR_CODE(BD_AUTH_CREATE_USERNAME_EXISTS,		707)

	/// The username that was passed to bdAuthService::createAccount had an
	/// invalid format.
	/// By default, usernames must be between 6 and 20 characters long, and only
	/// contain alphanumeric characters and the characters ".-_".
	/// This can be modified on a per-title ID basis, in consultation with
	/// DemonWare support. If you would like to modify this, please submit a
	/// regular expression for the format you require. The default regular
	/// expression is as follows:  '^([A-Za-z0-9_\\.\\-]{6,20})$'
	BD_LOBBY_ERROR_CODE(BD_AUTH_CREATE_USERNAME_ILLEGAL,	708)

	/// The username that was passed to bdAuthService::createAccount did not
	/// pass the authentication service vulgarity filter.
	/// The authentication service performs a sub-string match for common
	/// vulgarities on the username.
	/// This can be modified on a per-title ID basis, in consultation with
	/// DemonWare support. To obtain the default vulgarity file, please contact
	/// DemonWare support.
	BD_LOBBY_ERROR_CODE(BD_AUTH_CREATE_USERNAME_VULGAR,		709)

	/// A call to bdAuthService::createAccount failed because the maximum
	/// number of accounts for the given license code already exist.
	/// This is specific to titles that implement license management.
	/// The default maximum number of accounts per license code is 4.
	/// This can be modified on a per-title ID basis, in consultation with
	/// DemonWare support.
	BD_LOBBY_ERROR_CODE(BD_AUTH_CREATE_MAX_ACC_EXCEEDED,	710)

	/// An attempt to migrate accounts from one title ID to another failed
	/// because the title IDs are not configured to support migration.
	/// Account migration is a feature available to titles that implement
	/// license management (account sharing is available for those that don't).
	/// Account migration is only available between title IDs that have been
	/// explicitly configured to support migration of each other's accounts.
	BD_LOBBY_ERROR_CODE(BD_AUTH_MIGRATE_NOT_SUPPORTED,		711)

	/// An attempt was made to access a title ID that has been disabled on the
	/// authentication service.
	/// This will occur when the online service for a title ID has been
	/// discontinued.
	BD_LOBBY_ERROR_CODE(BD_AUTH_TITLE_DISABLED,				712)

	/// A license code that was used to perform an authentication operation has
	/// expired and cannot be used.
	/// This error may occur if license management is being used and time-limited
	/// licenses has been provided to users, for example, with time-limited demo
	/// versions of the game.
	BD_LOBBY_ERROR_CODE(BD_AUTH_ACCOUNT_EXPIRED,			713)

	/// A license code that was used to perform an authentication operation has
	/// been locked and cannot be used.
	BD_LOBBY_ERROR_CODE(BD_AUTH_ACCOUNT_LOCKED,				714)

	/// A generic authentication service error.
	/// This may occur if an incorrect version of the client code is used
	/// to access the authentication service.
	/// Please contact DemonWare support.
	BD_LOBBY_ERROR_CODE(BD_AUTH_UNKNOWN_ERROR,				715)

	/// The password that was passed to bdAuthService::authorizeAccount is
	/// incorrect.
	BD_LOBBY_ERROR_CODE(BD_AUTH_INCORRECT_PASSWORD,			716)

	/// The IP address from which the authorizeDedicatedHost() request was issued
	/// is not in the permitted range configured for the given license code.
	BD_LOBBY_ERROR_CODE(BD_AUTH_IP_NOT_IN_ALLOWED_RANGE,	717)

	/// Verification of the token issued by the Nintendo Authentication Service
	/// failed.
	BD_LOBBY_ERROR_CODE(BD_AUTH_WII_TOKEN_VERIFICATION_FAILED,	718)

	/// Authentication of a Wii client failed. This error occurs if the authentication
	/// server wasn't able to process the authentication request.
	BD_LOBBY_ERROR_CODE(BD_AUTH_WII_AUTHENTICATION_FAILED,	719)

	/// The maximum number of dedicated license code (cd-keys) has already been allocated
	/// to the requesting IP address.
	BD_LOBBY_ERROR_CODE(BD_AUTH_IP_KEY_LIMIT_REACHED, 720)

	//
	// Profile service errors
	//

	// Returned by get(Public/Private)Info
	BD_LOBBY_ERROR_CODE(BD_NO_PROFILE_INFO_EXISTS,	800)

	//
	// Friends service errors
	//

	/// No outstanding friendship proposal exists for this user
	/// Returned by bdFriends::acceptProposal bdFriends::rejectProposal
	BD_LOBBY_ERROR_CODE(BD_FRIENDSHIP_NOT_REQUSTED,		900)

	/// The specified user is not a friend
	/// Returned by bdFriends::removeFriend, bdFriends::sendFriendMessage
	BD_LOBBY_ERROR_CODE(BD_NOT_A_FRIEND,				901)

	/// User attempted to form friendship with themselves
	/// Returned by bdFriends::proposeFriendship (mommy says you can't play with yourself)
	BD_LOBBY_ERROR_CODE(BD_SELF_FRIENDSHIP_NOT_ALLOWED,	903)

	/// Friendship already exists
	/// Returned by bdFriends::proposeFriendship bdFriends::acceptProposal
	BD_LOBBY_ERROR_CODE(BD_FRIENDSHIP_EXISTS,			904)

	/// Friendship proposal already exists
	/// Returned by bdFriends::proposeFriendship
	BD_LOBBY_ERROR_CODE(BD_PENDING_FRIENDSHIP_EXISTS,	905)

	/// Operation blocked by the recipient
	/// Returned by bdFriends::proposeFriendship, bdFriends::sendFriendMessage
	BD_LOBBY_ERROR_CODE(BD_USERID_BANNED,				906)

	/// The user has the max amount of friends allowed. No additional friendships can be made.
	/// The option can be configured. Default: 100 friends.
	BD_LOBBY_ERROR_CODE(BD_FRIENDS_FULL,				908)

	// Returned when you attempt to get rich presence for an invalid ID or there is no rich
	// presence data
	BD_LOBBY_ERROR_CODE(BD_FRIENDS_NO_RICH_PRESENCE,	909)

	/// The data used when setting rich presence was larger than the maximum permitted size
	///  which is 1024 bytes.
	BD_LOBBY_ERROR_CODE(BD_RICH_PRESENCE_TOO_LARGE,		910)

	//
	// Storage service errors
	//

	/// The file specified does not exist
	/// Returned by bdStorage::deleteFile bdStorage::getFile
	BD_LOBBY_ERROR_CODE(BD_NO_FILE,					1000)

	/// User attempted to download a private file of another user
	/// Returned by bdStorage::getFile
	BD_LOBBY_ERROR_CODE(BD_PERMISSION_DENIED,		1001)

	/// Uploaded file exceeds the maximum size allowed 
	/// Returned by bdStorage::uploadFile
	BD_LOBBY_ERROR_CODE(BD_FILESIZE_LIMIT_EXCEEDED,	1002)

	/// The supplied filename exceeds the maximum size allowed 
	/// Returned by bdStorage::uploadFile
	/// Returned by bdContentStreaming::upload
	BD_LOBBY_ERROR_CODE(BD_FILENAME_MAX_LENGTH_EXCEEDED,	1003)

	//
	// Channel chat service errors
	//

	/// The specified channel does not exist
	/// Returned by bdChannelChat::subscribeToChannel bdChannelChat::sendChannelMessage
	/// bdChannelChat::sendWhisperMessage
	BD_LOBBY_ERROR_CODE(BD_CHANNEL_DOES_NOT_EXIST,		1101)

	/// User already subscribed to this channel
	/// Returned by bdChannelChat::subscribeToChannel
	BD_LOBBY_ERROR_CODE(BD_CHANNEL_ALREADY_SUBSCRIBED,	1102)

	/// User not subscribed to this channel
	/// Returned by bdChannelChat::unsubscribeToChannel bdChannelChat::sendChannelMessage
	/// bdChannelChat::sendWhisperMessage bdChannelChat::getChannelMembers
	BD_LOBBY_ERROR_CODE(BD_CHANNEL_NOT_SUBSCRIBED,		1103)

	/// Channel has exceeded maximum number of members.
	/// By default this is set to 100, but the limit can be altered by
	/// contacting Demonware support.
	/// Returned by bdChannelChat::subscribeToChannel
	BD_LOBBY_ERROR_CODE(BD_CHANNEL_FULL,				1104)

	/// User has exceeded the maximum number of channel subscriptions.
	/// By default this is set to 5, but the limit can be altered by
	/// contacting Demonware support.
	/// Returned by bdChannelChat::subscribeToChannel
	BD_LOBBY_ERROR_CODE(BD_CHANNEL_SUBSCRIPTIONS_FULL,	1105)

	/// Users are not allowed whisper to themselves!
	BD_LOBBY_ERROR_CODE(BD_CHANNEL_NO_SELF_WHISPERING,	1106)

	//
	// Title Utilities service errors
	//

	/// The event descriptor provided is greater than the maxEventDescLength
	/// defined for this title and has been truncated to the set maxEventDescLength.
	/// maxEventDescLength can be changed on a per title basis, contact support
	/// if you wish to change this. The default value is 1024.
	BD_LOBBY_ERROR_CODE(BD_EVENT_DESC_TRUNCATED,		1200)

	//
	// Content Unlock service errors
	//

	/// Generic content unlock error 
	BD_LOBBY_ERROR_CODE(BD_CONTENT_UNLOCK_UNKNOWN_ERROR,		1300)

	/// Error reported if the unlock key passed up is not a valid one
	BD_LOBBY_ERROR_CODE(BD_UNLOCK_KEY_INVALID,					1301)

	/// Error reported if the unlock key passed up has been used by someone else already
	BD_LOBBY_ERROR_CODE(BD_UNLOCK_KEY_ALREADY_USED_UP,			1302)


	//
	// Key Archive service errors
	//

	// TODO: add accurate comment.
	BD_LOBBY_ERROR_CODE(BD_KEY_ARCHIVE_INVALID_WRITE_TYPE,		  1500)

	//
	// Bandwidth test errors
	//

	BD_LOBBY_ERROR_CODE(BD_BANDWIDTH_TEST_TRY_AGAIN,			1810)

	BD_LOBBY_ERROR_CODE(BD_BANDWIDTH_TEST_STILL_IN_PROGRESS,	1811)

	BD_LOBBY_ERROR_CODE(BD_BANDWIDTH_TEST_NOT_PROGRESS,			1812)

	// Client side only
	BD_LOBBY_ERROR_CODE(BD_BANDWIDTH_TEST_SOCKET_ERROR,			1813)


	//
	// Arbitration Service errors
	//

	// Writing to an arbitrated session with an invalid sessions nonce
	BD_LOBBY_ERROR_CODE(BD_INVALID_SESSION_NONCE,				1901)

	// Failed to start an arbitrated sessions
	BD_LOBBY_ERROR_CODE(BD_ARBITRATION_FAILURE,					1903)

	// The user who submitted stats for an arbitrated session was not previously registered
	BD_LOBBY_ERROR_CODE(BD_ARBITRATION_USER_NOT_REGISTERED,		1905)

	// The leaderboard was not configured to support arbitration
	BD_LOBBY_ERROR_CODE(BD_ARBITRATION_NOT_CONFIGURED, 1906)

	//
	// Content Streaming errors
	//

	// Returned when the file metadata is not found.
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_FILE_NOT_AVAILABLE,			2000)

	// The user has used all of their storage space on the server, 
	// and does not have the quota for the requested file upload.
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_STORAGE_SPACE_EXCEEDED,		2001)

	// The user has already uploaded the maximum number of files allowed
	// and does not have the quota for another file upload.
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_NUM_FILES_EXCEEDED,			2002)

	// The user has already used their daily upload bandwidth 
	// and does not have the quota for another file upload.
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_UPLOAD_BANDWIDTH_EXCEEDED,	2003)

	// This error is returned when either filename to be uploaded exceeds BD_MAX_FILE_NAME_LENGTH
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_FILENAME_MAX_LENGTH_EXCEEDED,		2004)

	// The thumbnail data for an uploaded file cannot exceed BD_MAX_THUMB_DATA_SIZE.
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_MAX_THUMB_DATA_SIZE_EXCEEDED,		2005)

	// Returned when the user has used their download bandwidth for the day.
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_DOWNLOAD_BANDWIDTH_EXCEEDED,		2006)
	
	// The client specifies download buffer size with param fileDataSize. 
	// If the file to download is larger than fileData size the task will return this error.
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_NOT_ENOUGH_DOWNLOAD_BUFFER_SPACE,	2007)
	
	// The content server hosting the file is not configured on the server
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_SERVER_NOT_CONFIGURED,	2008)
	
	// The apple store receipt used did not match the item requested
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_APPLE_RECEIPT_FILENAME_MISMATCH,	2012)
	
	// Not technically an MMP error but represents any http error
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_HTTP_ERROR,					2020)

	// Returned by upload/download in the case of init error. 
	BD_LOBBY_ERROR_CODE(BD_CONTENTSTREAMING_FAILED_TO_START_HTTP,		2021)
	

	//
	// bdVoteRank service errors
	//

	// Returned by bdVoteRank if submitRating is called with an empty bdRatingInfo array.
	BD_LOBBY_ERROR_CODE(BD_VOTERANK_ERROR_EMPTY_RATING_SUBMISSION,		2030)

	// Only BD_MAX_NUM_VOTES are allowed per submitRating call, if exceeded this error is returned.
	BD_LOBBY_ERROR_CODE(BD_VOTERANK_ERROR_MAX_VOTES_EXCEEDED,			2031)

	// All ratings submitted were invalid. Ratings must be within:
	// BD_VOTE_RANK_MIN_RATING <= rating >= BD_VOTE_RANK_MAX_RATING
	BD_LOBBY_ERROR_CODE(BD_VOTERANK_ERROR_INVALID_RATING,				2032)

	//
	// bdTags service errors
	//
	// Only BD_MAX_NUM_TAGS are allowed per piece of content. If exceeded this error is returned.
	BD_LOBBY_ERROR_CODE(BD_MAX_NUM_TAGS_EXCEEDED,						2040)

	// Matchmaking service errors
	//

	/// Invalid queryID supplied.
	BD_LOBBY_ERROR_CODE(BD_INVALID_QUERY_ID,			2100)

	/// Invalid session specified
	BD_LOBBY_ERROR_CODE(BD_NO_ENTRY_TO_UPDATE,			2101)

	/// An invitation to a game session already existed
	BD_LOBBY_ERROR_CODE(BD_SESSION_INVITE_EXISTS,		2102)

	/// The session ID used was invalid.
	BD_LOBBY_ERROR_CODE(BD_INVALID_SESSION_ID,			2103)

	/// An attachment sent with a session invite was too large.
	BD_LOBBY_ERROR_CODE(BD_ATTACHMENT_TOO_LARGE,		2104)

	/// An invalid group ID was used or requested.
	BD_LOBBY_ERROR_CODE(BD_INVALID_GROUP_ID,		2800)


	//
	// Twitter service errors
	//

	// Authorization failed (invalid account or password).
	BD_LOBBY_ERROR_CODE(BD_TWITTER_AUTH_ATTEMPT_FAILED,		3501)

	// The user is no longer authorized (re-authorization required).
	BD_LOBBY_ERROR_CODE(BD_TWITTER_AUTH_TOKEN_INVALID,		3502)

	// The Twitter user has reached his or her update limit, and so the post
	// did not succeed.
	BD_LOBBY_ERROR_CODE(BD_TWITTER_UPDATE_LIMIT_REACHED,	3503)

	// Occurs when
	//   a) The Twitter servers return unavailable,
	//   b) We fail to connect to the Twitter servers,
	//   c) The bdTwitterService is not enabled on our servers.
	BD_LOBBY_ERROR_CODE(BD_TWITTER_UNAVAILABLE,				3504)

	// An unexpected Twitter error has occurred, contact DemonWare for
	// support.
	BD_LOBBY_ERROR_CODE(BD_TWITTER_ERROR,					3505)

	// We succeeded in connecting to Twitter, but the request timed out-- the
	// request may or may not have succeeded.
	// NOTE: This can also be returned by bdTwitterService::register.  If so,
	//   the request failed and you can safely retry.
	// NOTE: If this is returned by bdTwitterService::post, retrying might
	//   result in a duplicate post (but only if a different message is posted
	//   in between retries).
	BD_LOBBY_ERROR_CODE(BD_TWITTER_TIMED_OUT,				3506)

	// This error indicates that the twitter service has been purpopsfully disabled for the user
	BD_LOBBY_ERROR_CODE(BD_TWITTER_DISABLED_FOR_USER,				3507)


	//
	// Facebook service errors
	//

	// Authorization failed (invalid account or password).
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_AUTH_ATTEMPT_FAILED,	3601)

	// The user is no longer authorized (re-authorization required).
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_AUTH_TOKEN_INVALID,		3602)

	// For bdFacebookService::uploadPhoto, the provided bdFileID does not
	// exist.
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_PHOTO_DOES_NOT_EXIST,	3603)

	// For bdFacebookService::uploadPhoto, the file referenced by bdFileID
	// is not valid.  See
	// http://wiki.developers.facebook.com/index.php/Photos.upload for the
	// supported image types.
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_PHOTO_INVALID,			3604)

	// For bdFacebookService::uploadPhoto, the album is full.
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_PHOTO_ALBUM_FULL,		3605)

	// Occurs when
	//   a) The Facebook servers return unavailable,
	//   b) We fail to connect to the Facebook servers,
	//   c) The bdFacebookService is not enabled on our servers.
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_UNAVAILABLE,			3606)

	// An unexpected Facebook error has occurred, contact DemonWare for
	// support.
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_ERROR,					3607)

	// We succeeded in connecting to Facebook, but the request timed out-- the
	// request may or may not have succeeded.
	// NOTE: This can also be returned by bdFacebookService::register.  If so,
	//   the request failed and you can safely retry.
	// NOTE: If this is returned by bdFacebookService::post, retrying might
	//   (and oftentimes will) result in a duplicate post
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_TIMED_OUT,				3608)

	// This error indicates that the facebook service has been purposefully disabled for the user
	BD_LOBBY_ERROR_CODE(BD_FACEBOOK_DISABLED_FOR_USER,				3609)


	//
	// Apple push notification service errors
	//

	// This error is raised when you try to send to a user that hasn't registered, or has uninstalled the app.
	BD_LOBBY_ERROR_CODE(BD_APNS_USER_NOT_REGISTERED,		3700)

	// This error is raised when you specify a payload that is invalid.
	BD_LOBBY_ERROR_CODE(BD_APNS_INVALID_PAYLOAD,			3701)

BD_LOBBY_ERROR_CODES_END(BD_MAX_ERROR_CODE)
