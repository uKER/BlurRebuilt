// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Client-side interface to the content unlock service. 

#ifndef BD_CONTENT_UNLOCK_MANAGEMENT_H
#define BD_CONTENT_UNLOCK_MANAGEMENT_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdContentUnlock/bdUnlockContentInfo.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;

BD_REFERENCE(bdConnection);

// Content Unlock Operations
#define BD_CONTENT_UNLOCK_UNLOCK_CONTENT			(1u)
#define BD_CONTENT_UNLOCK_LIST_UNLOCKED_CONTENT		(2u)
#define BD_CONTENT_UNLOCK_LIST_CONTENT_BY_KEY		(3u)

/// Client-side interface to the content unlock service. Manages unlocking of 
/// content and listing of unlocked content.
/// The content unlock service enables persistent storage of unlock keys and
/// associated content. Each unlock key can be used to unlock content only once 
/// and only by one user. 
class bdContentUnlock
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS		

		~bdContentUnlock();

		/// Lists all content unlockable using a license code 
		/// - calls getContent after generating a license ID (unlock key) from the license code
		/// \param licenseCode[in]		The license code for the associated content
		/// \param content[out]			The associated content
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_UNLOCK_KEY_INVALID - The license code does not correspond to a valid unlock key
		///			BD_UNLOCK_KEY_ALREADY_USED_UP - The content associated with this unlock key has 
		///											been unlocked by another user.
		bdRemoteTaskRef listContentByLicenseCode(	const bdNChar8* licenseCode,
													bdUnlockContentInfo *content );

		/// Lists all content unlockable using an unlock key
		/// \param unlockKey[in]		The unlock key for the associated content
		/// \param content[out]			The associated content
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_UNLOCK_KEY_INVALID - The unlock key is not associated with any content
		///			BD_UNLOCK_KEY_ALREADY_USED_UP - The content associated with this unlock key has 
		///											been unlocked by another user.
		bdRemoteTaskRef listContent(	const bdUInt64 unlockKey,
										bdUnlockContentInfo *content );

		/// Unlock a piece of content using a license code 
		/// - calls unlock content after generating a license ID from the license code
		/// \param licenseCode[in]		The license code used to unlock a piece of content
		/// \param unlockedContent[out]	The content that is unlocked
		/// \param hardwareID[in]		The hardware ID - So we can store the original kit that unlocked this content.
		///								If we attempt to unlock content that has previously been unlocked by us, the 
		///								hardwareID passed in here will overwrite the existing one.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_UNLOCK_KEY_INVALID - The unlock key is not associated with any content
		///			BD_UNLOCK_KEY_ALREADY_USED_UP - The content associated with this unlock key has 
		///											been unlocked by another user.
		bdRemoteTaskRef unlockContentByLicenseCode(	const bdNChar8* licenseCode,
													bdUnlockContentInfo *unlockedContent,
													const bdUInt64 hardwareID = 0);


		/// Unlock a piece of content using an unlock key
		/// \param unlockKey[in]		The key used to unlock a piece of content
		/// \param unlockedContent[out]	The content that is unlocked
		/// \param hardwareID[in]		The hardware ID - So we can store the original kit that unlocked this content.
		///								If we attempt to unlock content that has previously been unlocked by us, the 
		///								hardwareID passed in here will overwrite the existing one.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_UNLOCK_KEY_INVALID - The unlock key is not associated with any content
		///			BD_UNLOCK_KEY_ALREADY_USED_UP - The content associated with this unlock key has 
		///											been unlocked by another user.
		bdRemoteTaskRef unlockContent(	const bdUInt64 unlockKey,
										bdUnlockContentInfo *unlockedContent,
										const bdUInt64 hardwareID = 0);

		/// Return a list of content that this user has unlocked
		/// \param contentList[out]		Array to store the unlocked content information.
		/// \param maxNumResults[in]	Maximum number of results to return.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef listUnlockedContent(bdUnlockContentInfo contentList[],
											bdUInt16 maxNumResults);


	protected:

		friend class bdLobbyService;

		/// Constructs a content unlock with the given task handler.
		/// Note: never call this constructor directly, use bdLobbyService::getContentUnlock() instead.
		/// \param remoteTaskManager[in] A pointer to the remote task manager that handles
		/// requests and replies
		bdContentUnlock(bdRemoteTaskManager *const remoteTaskManager);

		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif // BD_CONTENT_UNLOCK_MANAGEMENT_H
