// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_TWITTER_H
#define BD_TWITTER_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdTwitter/bdTwitterIsRegistered.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;


#define BD_TWITTER_REGISTER			(1u)
#define BD_TWITTER_POST				(2u)
#define BD_TWITTER_UNREGISTER		(3u)
#define BD_TWITTER_IS_REGISTERED	(4u)


#define BD_TWITTER_MAX_ACCOUNT_LENGTH	(1024u)
#define BD_TWITTER_MAX_PASSWORD_LENGTH	(64u)
#define BD_TWITTER_MAX_MESSAGE_LENGTH	(140u)


/// bdTwitter service
class bdTwitter
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdTwitter(bdRemoteTaskManager *const remoteTaskManager);

		/// Destructor
		~bdTwitter();

		/// Registers a Twitter account.
		/// \param accountName[in]:		The account name of the Twitter account.
		/// \param password[in]:		The password of the Twitter account.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		///			BD_TWITTER_AUTH_FAILED - authorization failed
		bdRemoteTaskRef registerAccount(const bdNChar8 *const accountName,
										const bdNChar8 *const password);

		/// Posts a message to a user's (registered) Twitter account.
		/// \param message[in]:	The message to post.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		///			BD_TWITTER_AUTH_FAILED - user is no longer authorized
		bdRemoteTaskRef post(const bdNChar8 *const message);

		/// Unregisters a Twitter account.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		bdRemoteTaskRef unregisterAccount();

		/// Checks whether the user has a registered Twitter account.
		/// \param isRegistered[out]:	A bdTwitterIsRegistered which will hold 
		///								the result upon completion.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		bdRemoteTaskRef isRegistered(bdTwitterIsRegistered *const isRegistered);


	protected:

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif //BD_TWITTER_H

