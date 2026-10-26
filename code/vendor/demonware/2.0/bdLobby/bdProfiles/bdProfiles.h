// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Client side interface to the profiles service. 

#ifndef BD_PROFILES_H
#define BD_PROFILES_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyErrorCode.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdProfileInfo;


// Profiles operations
#define BD_PROFILE_GET_PUBLIC_INFO			(1u)
#define BD_PROFILE_GET_PRIVATE_INFO			(2u)
#define BD_PROFILE_SET_PUBLIC_INFO			(3u)
#define BD_PROFILE_SET_PRIVATE_INFO			(4u)
#define BD_PROFILE_DELETE					(5u)

 
/// Create/update/delete and read public and private user profiles. The profiles
/// service is intended as a storage convenience for non-statistical player 
/// information.
class bdProfiles
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS		

		~bdProfiles();

		/// Set/update the public profile info for a specified user. If no profile
		/// exists for this user this function will create one.
		/// \param profileInfo[in]		An instance of a class derived from 
		///								bdProfileInfo that represents the public 
		///								profile info.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_PROFILE_INFO_EXISTS - if no public profile info has been 
		///			defined for this user.
		///			BD_PARAM_MISMATCHED_TYPE - the parameters serialized in profileInfo
		///			did match the types defined for this title.
		bdRemoteTaskRef setPublicInfo(bdProfileInfo *profileInfo);

		/// Set/update the private profile info for a specified local user. If no profile
		/// exists for this user this function will create one. Private information can only 
		/// be updated by the owner of the profile.
		/// \param profileInfo[in]		An instance of a class derived from 
		///								bdProfileInfo that represents the private 
		///								profile info.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_PROFILE_INFO_EXISTS - if no private profile info has been 
		///			defined for this user.
		///			BD_PARAM_MISMATCHED_TYPE - the parameters serialized in profileInfo
		///			did match the types defined for this title.
		bdRemoteTaskRef setPrivateInfo(bdProfileInfo *profileInfo);

		/// Retrieve the public profile info for the specified users.
		/// \param userIDs[in]			An array of userIDs for which the profiles are requested.
		/// \param publicProfiles[out]  An array of public profile infos in which the results are stored.
		///								The size of the array has to match the size specified in numProfiles.
		/// \param numProfiles[in]		The number of profiles to be retrieved.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_PROFILE_INFO_EXISTS - if no public profile info has been 
		///			defined for this user.
		bdRemoteTaskRef getPublicInfos(	const bdOnlineUserID userIDs[], 
										bdProfileInfo publicProfiles[], 
										const bdUInt numProfiles);
		
	
		/// Retrieve the private profile info for the local user.
		/// Private profile information can only be accessed by the owner of the profile.
		/// \param privateProfile[out]	A pointer to an instance storing the private profile info.
		///								This class should be a subclass of bdProfileInfo to store the
		///								custom attributes specified for the private profile.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_PROFILE_INFO_EXISTS - if no private profile info has been 
		///			defined for this user.
		bdRemoteTaskRef getPrivateInfo(bdProfileInfo *const privateProfile); 

		/// Delete a profile for a specified local user.
		/// Profiles can only be deleted by the owner of the profile.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef deleteProfile();


	protected:

		friend class bdLobbyService;

		/// Constructs a profiles manager with the given task handler.
		/// Note: never call this constructor directly, use bdLobbyService::getProfiles() instead.
		/// \param remoteTaskManager[in] A pointer to the remote task manager that handles
		/// requests and replies
		bdProfiles(bdRemoteTaskManager *const remoteTaskManager);

		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif // BD_PROFILES_H
