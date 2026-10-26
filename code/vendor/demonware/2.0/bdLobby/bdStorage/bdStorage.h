// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Client-side interface to the storage service. 

#ifndef BD_STORAGE_MANAGEMENT_H
#define BD_STORAGE_MANAGEMENT_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdStorage/bdFileInfo.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdFileData;
class bdGetFilesResult;

// Storage Operations
#define BD_STORAGE_UPLOAD_FILE			(1u)
#define BD_STORAGE_REMOVE_FILE			(2u)
#define BD_STORAGE_GET_FILE				(3u)
#define BD_STORAGE_GET_FILE_BY_ID		(4u)
#define BD_STORAGE_LISTBYOWNER			(5u)
#define BD_STORAGE_LISTPUBLISHERFILES	(6u)
#define BD_STORAGE_GET_PUBLISHER_FILE	(7u)
#define BD_STORAGE_UPDATE_FILE			(8u)
#define BD_STORAGE_GET_FILES_BY_ID		(9u)


/// Client-side interface to the storage service. Manages uploads and downloads
/// of small files.
/// The storage service enables persistent storage of small binary files. Storage
/// files can be either user-owned, in which case they are uploaded by clients 
/// with this interface, or they are publisher-owned, in which case DemonWare 
/// support should be contacted for more info. 
class bdStorage
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS		

		~bdStorage();

		/// Upload a file to the user storage area.
		/// \param fileName[in]			The name of the file to be stored. If a file with that name already exists
		///								for that user, its contents will be overridden.
		/// \param fileData[in]			A pointer to the actual file data.
		/// \param fileSize[in]			The size of the file in bytes.
		/// \param fileVisibility[in]	The visibility of the file. If set to
		///								bdFileInfo::BD_VISIBLE_PRIVATE, only
		///								the file owner will be able to access the file.
		/// \param ownerID[in]			The bdOnlineUserID of the user who will own the file.  If 0, the current user's
		/// 							ID is used.
		/// \param fileInfo[out] Pointer to the uploaded file info.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_PERMISSION_DENIED - the user is not a dedicated server and attempts to upload a file for
		///				another user.
		bdRemoteTaskRef uploadFile(const bdNChar8 *const fileName,
								   const void *const fileData,
								   const bdUInt fileSize,
								   const bdFileInfo::bdVisibility fileVisibility = bdFileInfo::BD_VISIBLE_PRIVATE,
								   const bdOnlineUserID ownerID = 0,
								   bdFileInfo *const fileInfo = BD_NULL);

		/// Remove a file from storage.
		/// \param fileName[in]	The name of the file to be deleted.
		/// \param ownerID[in]	The bdOnlineUserID of the user who owns the file.  If 0, the current user's ID is used.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_FILE - if a file with that name does not exist.
		///			BD_PERMISSION_DENIED - the user is not a dedicated server and attempts to delete a file owned by
		///				another user.
		bdRemoteTaskRef removeFile(const bdNChar8 *const fileName,
								   const bdOnlineUserID ownerID = 0);

		/// Get a file from storage.
		/// \param fileName[in]	 The name of the file to be retrieved.
		/// \param fileData[out] Pointer to the file data where the downloaded file is stored.
		/// \param ownerUID[in]	 The user ID of the owner of the file (optional). This is used to download
		///						 other user's public files.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_FILE - if a file with that name does not exist.
		///			BD_PERMISSION_DENIED - if this user does not have permission to 
		///			read the specified file.
		bdRemoteTaskRef getFile(const bdNChar8 *const fileName,
							    bdFileData *const fileData,
							    const bdOnlineUserID ownerUID = BD_NULL);	

		/// Get a file from storage.
		/// \param fileID[in]	The file ID of the file to be retrieved. 
		/// \param fileData[out] Pointer to the file data where the downloaded file is stored.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_FILE - if the fileID specified does not represent a file.
		///			BD_PERMISSION_DENIED - if this user does not have permission to 
		///			read the specified file.
		bdRemoteTaskRef getFileByID(const bdUInt64 fileID,
									bdFileData *const fileData);	
		
		/// Get a file from storage.
		/// \param numFileIDs[in]	The number of fileIDs to fetch files for 
		/// \param fileIDs[in]		The fileIDs of the files to fetch.
		/// \param fileData[out]	Pointer to the file datas where the downloaded files can be stored.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_FILE - if the fileID specified does not represent a file.
		///			BD_PERMISSION_DENIED - if this user does not have permission to 
		///			read the specified file.
		bdRemoteTaskRef getFilesByID(const bdUInt32 numFileIDs,
									 const bdUInt64 fileIDs[],
									 bdGetFilesResult filesResult[]);

		/// Get a list of files owned by a specified user. 
		/// \param ownerID[in]			The ID of the user whose files should be
		///								returned. Specify 0 to list the files belonging to the local user.
		/// \param startDate[in]		Restricts the result set to contain only files that have been 
		///								modified after the specified date.
		/// \param fileList[out]		Array to store the relevant file information.
		/// \param maxNumResults[in]	Maximum number of results to return.
		/// \param	offset[in]			An optional offset into the file list.
		/// \param fileName[in]			An optional string with the name of the desired
		///								file. Set to \a BD_NULL for any name. Match is
		///								exact (case-sensitive).
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef listFilesByOwner(const bdOnlineUserID ownerID,
									     const bdUInt32 startDate,
									     bdFileInfo fileList[],
									     const bdUInt16 maxNumResults,
									     const bdUInt16 offset = 0,
									     const bdNChar8 *const fileName = BD_NULL);
		
		/// Get a list of all files owned by the publisher (i.e. non-user files).
		/// \param startDate[in]		A timestamp specifying the oldest file
		///								that should be returned. This is in the
		///								form of seconds since midnight Jan 1 1970, UTC.
		/// \param fileList[out]		A pointer to an array of bdFileInfo objects to store
		///								the resulting file information.
		/// \param maxNumResults[in]	The maximum number of results to return from the server.
		/// \param	offset[in]			An optional offset into the file list.
		/// \param fileName[in]			An optional string with the name of the desired
		///								file. Set to \a BD_NULL for any name. Match is
		///								exact (case-sensitive).
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef listAllPublisherFiles(const bdUInt32 startDate,
											  bdFileInfo fileList[],
											  const bdUInt16 maxNumResults,
											  const bdUInt16 offset = 0,
											  const bdNChar8 *const fileName = BD_NULL);

		/// Update a file given its file ID.
		/// \param fileID[in]	The file ID of the file to be updated. 
		/// \param fileData[in] Pointer to the new file data.
		/// \param fileSize[in] The size of the file data in bytes.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_NO_FILE - if the fileID specified does not represent a file.
		///			BD_PERMISSION_DENIED - if this user does not have permission to write the specified file.
		bdRemoteTaskRef updateFile(const bdUInt64 fileID,
								   const void *fileData,
								   bdUInt fileSize);

		/// Download a file from the publisher storage area
		/// \param fileName[in]		The name of the file to be retrieved
		/// \param fileData[out]	Pointer to the file data where the downloaded file is stored.
		bdRemoteTaskRef getPublisherFile(const bdNChar8 *const fileName,
										 bdFileData *const fileData);


	protected:

		friend class bdLobbyService;
		friend class bdContentStreaming;

		/// Constructs a storage manager with the given task handler.
		/// Note: never call this constructor directly, use bdLobbyService::getStorage() instead.
		/// \param remoteTaskManager[in] A pointer to the remote task manager that handles
		/// requests and replies
		bdStorage(bdRemoteTaskManager *const remoteTaskManager);

		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif // BD_STORAGE_MANAGEMENT_H
