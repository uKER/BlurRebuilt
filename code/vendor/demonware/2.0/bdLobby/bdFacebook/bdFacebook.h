// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_FACEBOOK_H
#define BD_FACEBOOK_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdFacebook/bdFacebookIsRegistered.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;


#define BD_FACEBOOK_REGISTER		(1u)
#define BD_FACEBOOK_POST			(2u)
#define BD_FACEBOOK_UNREGISTER		(3u)
#define BD_FACEBOOK_UPLOAD_PHOTO	(4u)
#define BD_FACEBOOK_IS_REGISTERED	(5u)


#define BD_FACEBOOK_MAX_ACCOUNT_LENGTH		(1024u)
#define BD_FACEBOOK_MAX_PASSWORD_LENGTH		(64u)
#define BD_FACEBOOK_MAX_MESSAGE_LENGTH		(255u) // http://wiki.developers.facebook.com/index.php/Status.set
#define BD_FACEBOOK_MAX_DESCRIPTION_LENGTH	(4096u)
#define BD_FACEBOOK_MAX_IMAGE_URL_LENGTH	(1024u)


/// bdFacebook service
class bdFacebook
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdFacebook(bdRemoteTaskManager *const remoteTaskManager);

		/// Destructor
		~bdFacebook();

		/// Registers a Facebook account.
		/// \param accountName[in]:	The account name of the Facebook account.
		///							Must not be longer than
		///							BD_FACEBOOK_MAX_ACCOUNT_LENGTH.
		/// \param password[in]:	The password of the Facebook account.  Must
		///							not be longer than
		///							BD_FACEBOOK_MAX_PASSWORD_LENGTH.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		///			BD_FACEBOOK_AUTH_FAILED - authorization failed
		bdRemoteTaskRef registerAccount(const bdNChar8 *const accountName,
										const bdNChar8 *const password);

		/// Posts a message to a user's (registered) Facebook account.
		/// \param message[in]:		The message to post (appears where
		///							facebook status message is).  Must not be
		///							longer than BD_FACEBOOK_MAX_MESSAGE_LENGTH.
		/// \param name[in]:		The name (title) of the story.  Must not be
		///							longer than BD_FACEBOOK_MAX_MESSAGE_LENGTH.
		/// \param caption[in]:		Caption of the story.  Must not be longer
		///							than BD_FACEBOOK_MAX_MESSAGE_LENGTH.
		/// \param description[in]:	Descriptive text about the story.  Must not
		///							be longer than
		///							BD_FACEBOOK_MAX_DESCRIPTION_LENGTH.
		/// \param imageUrl[in]:	URL of an image for the story.  Must not be
		///							longer than
		///							BD_FACEBOOK_MAX_IMAGE_URL_LENGTH.
		/// \param imageHref[in]:	HREF for the image.  Cannot be longer than
		///							BD_FACEBOOK_MAX_IMAGE_URL_LENGTH.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	For more details on the parameters, see
		///			http://wiki.developers.facebook.com/index.php/Stream.publish.
		/// \note	All parameters are optional.  Specify BD_NULL to omit.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		///			BD_FACEBOOK_AUTH_FAILED - user is no longer authorized
		bdRemoteTaskRef post(const bdNChar8 *const message,
							 const bdNChar8 *const name,
							 const bdNChar8 *const caption,
							 const bdNChar8 *const description,
							 const bdNChar8 *const imageUrl,
							 const bdNChar8 *const imageHref);

		/// Unregisters a Facebook account.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		bdRemoteTaskRef unregisterAccount();

		/// Uploads a photo to a user's (registered) Facebook account.
		/// \param fileID[in]:	The ID of the (content server) file to upload.
		/// \param caption[in]:	The caption for the photo.  Must not be longer
		///						than BD_FACEBOOK_MAX_MESSAGE_LENGTH.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	The caption parameter is optional.  Use BD_NULL to omit.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		///			BD_FACEBOOK_AUTH_FAILED - user is no longer authorized
		bdRemoteTaskRef uploadPhoto(const bdUInt64 fileID,
									const bdNChar8 *const caption);

		/// Checks whether the user has a registered Facebook account.
		/// \param isRegistered[out]:	A bdFacebookIsRegistered which will
		///								hold the result upon completion.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success
		bdRemoteTaskRef isRegistered(bdFacebookIsRegistered *const isRegistered);

	protected:

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif //BD_FACEBOOK_H

