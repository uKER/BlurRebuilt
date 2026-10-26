// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_TITLE_UTILITIES_H
#define BD_TITLE_UTILITIES_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdTitleUtilities/bdOnlineUserInfo.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdTitleStats;
class bdVerifyString;
class bdTimeStamp;

#define BD_VERIFY_STRING		(1u)
#define BD_GET_TITLE_STATS		(2u)
#define BD_RECORD_EVENT			(3u)
//#define BD_RECORD_IP			(4u)	This feature was implemented in bdDML instead of here.
#define BD_RECORD_EVENT_BIN		(5u)
#define BD_GET_SERVER_TIME		(6u)
#define BD_ARE_USERS_ONLINE		(7u)



/// Collection of various utility functions 
class bdTitleUtilities
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdTitleUtilities(bdRemoteTaskManager *const remoteTaskManager);

		/// Destructor
		~bdTitleUtilities();
	

		/// Determines whether the given string passes the vulgarity filter. 
		/// \param str[in] : Specifies the string to be verified
		/// \param length[in] : Specifies the length of the string.
		/// \param verified[out] : A pointer to an instance of bdVerifyString which will contain
		///						   - true if the string passed the profanity filter
		///						   - false if the string did not pass the profanity filter
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef verifyString(const bdNChar8 *const str,
									 const bdUInt length,
									 bdVerifyString *verified);
		
		/// Records an 'event' for a specific user, at the current time.
		/// The event is a string, which is opaque to the server and can be used 
		/// for later analysis.
		/// \param eventStr[in] : String which will be written to the server log table.
		///						  Use this to describe the event which is being logged
		///						  on the client.
		/// \param length[in] :   Specifies the length of the string.
		///						  The default max length is defined server side, and is 1024.
		///						  Contact DemonWare support if you wish to change the max string length.
		///						  See BD_EVENT_DESC_TRUNCATED in bdLobby\bdLobbyErrorCodes.h.
		/// \param category[in] : Specifies an optional category that this string belongs in
		///						  Use this to differentiate among different types of recorded events
		///						  Default category is 0
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef recordEvent(const bdNChar8 *const eventStr,
									const bdUInt length,
									bdUInt category = 0);

		/// Records an 'event' for a specific user, at the current time.
		/// The event is a BLOB, which is opaque to the server and can be used 
		/// for later analysis.
		/// \param eventBlob[in] : Blob which will be written to the server log table.
		///						  Use this to describe the event which is being logged
		///						  on the client.
		/// \param length[in] :   Specifies the length of the string.
		///						  The default max length is defined server side, and is 1024.
		///						  Contact DemonWare support if you wish to change the max string length.
		///						  See BD_EVENT_DESC_TRUNCATED in bdLobby\bdLobbyErrorCodes.h.
		/// \param category[in] : Specifies an optional category that this blob belongs in
		///						  Use this to differentiate among different types of recorded events
		///						  Default category is 0
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef recordEventBin(const void* const eventBlob,
									   const bdUInt length,
                                       bdUInt category = 0);


		/// Retrieves general usage statistics for this title, specifically: 
		/// - current number of online sessions 
		/// - total users that have created accounts
		/// - current number of online users
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When this task completes, the results are contained in the
		///			supplied bdTitleStats class.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef getTitleStats(bdTitleStats *titleStats);

		/// Gets the online status for each of the specified users.
		/// \param results[in|out]		:	A pointer to an array of bdOnlineUserInfo objects,
		///								to specify which user IDs to retrieve online status for AND 
		///								to store the results for the query when the task has
		///								completed. The size of the array has to match the size
		///								as specified by numUsers.
		/// \param numUsers[in]:	The number of bdOnlineUserInfos in the array \a results.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When this task completes, the results are contained in the
		///			supplied bdOnlineUserInfo array.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.  
		bdRemoteTaskRef areUsersOnline(bdOnlineUserInfo results[], const bdUInt numUsers);
		/// Retrieves the server time (UTC) in seconds since the epoch.
		/// \param timeStamp[in]: Pointer to the result timestamp.
		/// \return	A reference to the task handle that should be pumped to completion
		bdRemoteTaskRef getServerTime(bdTimeStamp *timeStamp);

	protected:

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;



};

#endif //BD_TITLE_UTILITIES_H

