// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The interface to the Content Streaming Service

#ifndef BD_CONTENTSTREAMING_H
#define BD_CONTENTSTREAMING_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdHTTPWrapper/bdHTTPWrapper.h>
#include <bdLobby/bdContentStreaming/bdFileMetaData.h>
#include <bdLobby/bdContentStreaming/bdURL.h>

#define BD_MAX_URL_RESULTS 5

BD_REFERENCE(bdRemoteTask);

class bdGetFilesResult;
class bdRemoteTaskManager;
class bdFileID;
class bdQuotaUsage;
class bdTag;
class bdDownloadInterceptor;
class bdUploadInterceptor;

class bdContentStreaming : public bdLobbyConnectionListener
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

		~bdContentStreaming();

#ifdef BD_PLATFORM_XENON
		
		// Note that init calls XTitleServerCreateEnumerator and XEnumerate in a blocking fashion which
		// could in some cases 
		void init(DWORD lspServiceID);
#endif	
		/// Upload data to the content server
		/// \param fileSlot[in] The slot to use to store the file. Any preexisting file in that slot
		///						will be overwriten.
		/// \param fileData[in] Pointer to data to be uploaded.  The size is limited 
		///						on a per title basis.
		/// \param fileSize[in] Size of data to be uploaded in bytes.
		/// \param filename[in] Pointer to a null terminated string not more than 
		///						BD_MAX_FILE_NAME_LENGTH characters. Note that filenames are not unique 
		///						per user.
		/// \param category[in]	The category to assign to the file being uploaded. 
		///						Use categories when listing files to only return files of the same category.
		/// \param thumbData[in] Pointer to thumbnail data to be uploaded.  The size is limited 
		///						BD_MAX_THUMB_DATA_SIZE (8k) on both the client and server.
		/// \param thumbDataSize[in] Size of thumbnail data to be uploaded in bytes.
		/// \param numTags[in]	Number of tags in tags array to be uploaded, may not be more than 
		///						BD_MAX_NUM_TAGS.
		/// \param tags[in]		Array of bdTag instance for the file being uploaded.
		/// \param fileID[out]  The File ID by which the uploaded file can be uniquely identified within 
		///						a title.
		///
		/// \return		A reference to the task that handles the request. 
		///				BD_NULL is returned if a client error occurred before execution request.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_CONTENTSTREAMING_STORAGE_SPACE_EXCEEDED - if user exceeds space limit.
		///			BD_CONTENTSTREAMING_NUM_FILES_EXCEEDED - if user exceeds file number limit.
		///			BD_CONTENTSTREAMING_UPLOAD_BANDWIDTH_EXCEEDED- if user exceeds bandwidth limit.		
		//			BD_CONTENTSTREAMING_FILENAME_MAX_LENGTH_EXCEEDED
		//				- This error is returned when either filename to be uploaded exceeds BD_MAX_FILE_NAME_LENGTH.
		//			BD_CONTENTSTREAMING_MAX_THUMB_DATA_SIZE_EXCEEDED
		//				- The thumbnail data for an uploaded file cannot exceed BD_MAX_THUMB_DATA_SIZE.

		/// \note pump() MUST be called to complete this method.
		/// \note You can check the progress of the upload operation by calling checkProgress()
		/// \note You can abort the operation by calling abortHTTPOperation()
		/// \note The users quota will be adjusted as follows after a successful remove:
		/// m_usedDailyUploadBW += 'size of file uploaded'
		/// m_usedNumFiles += 1
		/// m_usedStorageSpace += 'size of file uploaded - size of any preexisting file in specified file slot'
		bdRemoteTaskRef upload(const bdUInt16 fileSlot,
							   const void* fileData,
							   const bdUInt32 fileSize,
							   const bdNChar8 *const fileName,
							   const bdUInt16 category,
							   const void* thumbData,
							   const bdUInt32 thumbDataSize,
							   const bdUInt32 numTags,
							   const bdTag tags[],
							   bdFileID *fileID);

		/// Upload data to the content server
		/// As described above, except that this version allows clients to specify a callback handler instead
		/// of requiring a buffer to be passed in.
		/// \param uploadHandler[in] A pointer to an instance of bdUploadInterceptor. handleUpload will be called 
		/// from the upload thread to request data to be uploaded until such time as the full file has been uploaded
		/// To abort the upload simply return 0 from bdUploadInterceptor::handleUpload 
		bdRemoteTaskRef upload(	const bdUInt16 fileSlot,
								bdUploadInterceptor* uploadHandler,
								const bdUInt32 fileSize,
								const bdNChar8 *const fileName,
								const bdUInt16 category,
								const void* thumbData,
								const bdUInt32 thumbDataSize,
								const bdUInt32 numTags,
								const bdTag tags[],
								bdFileID* fileID);

		/// Download user file data (and metadata) from the content server
		/// \param fileID[in]	 ID of file we wish to download.
		/// \param fileData[out] Pointer to a writable area of memory of sufficient size 
		///						 to receive the file data 
		/// \param fileDataSize[in] The size of the data pointed to by fileData.
		/// \param fileMetaData[out]The meta data associated with the file
		/// \param startByte[in]	(Optional) Offset within the file from which to commence downloading 
		///							If set to zero this param will be ignored
		/// \param endByte[in]		(Optional) Offset within the file at which to stop downloading. 
		///							If set to zero this param will be ignored
		/// \note	Do not use the range requests as a mechanism to limit the download rate - use the callback form 
		/// of download to and implement rate limiting within bdDownloadInterceptor.
		/// \note	FileData must contain enough space to download file.
		///
		/// \return		A reference to the task that handles the request. 
		///				BD_NULL is returned if a client error occurred before execution request.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_STREAMING_SERVICE_FILE_NOT_AVAILABLE - if the file is not present 
		/// \note You can use one of two approaches when specifying the file size:  
		/// i) pass in a maximum size to be downloaded, or
		/// ii) obtain the file size exactly by calling getFileMetaDataByID() or listFilesByOwner() 
		/// methods and inspecting the returned bdFileMetaData. You may have already called these 
		/// methods in order to obtain the fileID in the first place.
		/// \note pump() MUST be called to complete this method.
		/// \note You can check the progress of the download operation by calling checkProgress()
		/// \note You can abort the operation by calling abortHTTPOperation()
		/// \note The users quota will be adjusted as follows after a successful remove:
		/// m_usedDailyDownloadBW += 'size of file downloaded' regardless of startByte and endByte
		bdRemoteTaskRef download(const bdUInt64 fileID,
								 void *fileData, 
								 const bdUInt32 fileDataSize,
								 bdFileMetaData *fileMetaData,
								 bdUInt32 startByte = 0,
								 bdUInt32 endByte = 0);

		/// Download user file data (and metadata) from the content server
		/// As described above, except that this version allows clients to specify a callback handler instead
		/// of requiring a buffer to be passed in.
		/// \param downloadHandler[in] A pointer to an instance of bdDownloadInterceptor. handleDownload will be called 
		/// from the download thread every time a chunk of data has been received until the download is aborted or completed.
		/// To abort the download simply return 0 from bdDownloadInterceptor::handleDownload 
		bdRemoteTaskRef download(const bdUInt64 fileID,
								 bdDownloadInterceptor *downloadHandler,
								 bdFileMetaData *fileMetaData,
								 bdUInt32 startByte = 0,
								 bdUInt32 endByte = 0);

		/// Download user file data (and metadata) from the content server from a specified owner and slot.
		/// This method is exposed so that clients can skip the metadata step when they wish to access
		/// a user's file which has a well known fileslot. For example, assume every player has a 
		/// profile picture which is always located in slot 0 and is of max size 80KB. To download 
		/// this profile picture we can simply call this method using fileslot = 0 and passing in a
		/// buffer of size 80KB. One of the advantages of designating particular slots for particular 
		/// files is that files can be downloaded without first querying the server for the filemetadata.
		/// \param ownerID[in]		Owner of file we wish to download.
		/// \param fileSlot[in]		Slot of the file we wish to download.
		/// \param fileData[in]		Pointer to a writable area of memory of sufficient size 
		///							to receive the file data 
		/// \param fileDataSize[in] The size of the data pointed to by fileData.
		/// \param fileMetaData[out] The meta data associated with the file
		/// \param startByte[in]	(Optional) Offset within the file from which to commence downloading 
		///							If set to zero this param will be ignored
		/// \param endByte[in]		(Optional) Offset within the file at which to stop downloading. 
		///							If set to zero this param will be ignored
		/// \note	Do not use the range requests as a mechanism to limit the download rate - use the callback form 
		/// of download to and implement rate limiting within bdDownloadInterceptor.
		/// \note	FileData must contain enough space to download file.
		/// \return		A reference to the task that handles the request. 
		///				BD_NULL is returned if a client error occurred before execution request.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_CONTENTSTREAMING_FILE_NOT_AVAILABLE - if the file is not present 
		/// \note pump() MUST be called to complete this method.
		/// \note You can check the progress of the download operation by calling checkProgress()
		/// \note You can abort the operation by calling abortHTTPOperation()
		/// \note The users quota will be adjusted as follows after a successful remove:
		/// m_usedDailyDownloadBW += 'size of file downloaded' regardless of startByte and endByte
		bdRemoteTaskRef downloadFileBySlot(	const bdOnlineUserID ownerID,
											const bdUInt16 fileSlot,
											void *fileData, 
											const bdUInt32 fileDataSize,
											bdFileMetaData *fileMetaData,
											bdUInt32 startByte = 0,
											bdUInt32 endByte = 0);

		/// Download user file data (and metadata) from the content server
		/// As described above, except that this version allows clients to specify a callback handler instead
		/// of requiring a buffer to be passed in.
		/// \param downloadHandler[in] A pointer to an instance of bdDownloadInterceptor. handleDownload will be called 
		/// from the download thread every time a chunk of data has been received until the download is aborted or completed.
		/// To abort the download simply return 0 from bdDownloadInterceptor::handleDownload 
		bdRemoteTaskRef downloadFileBySlot(	const bdOnlineUserID ownerID,
											const bdUInt16 fileSlot,
											bdDownloadInterceptor *downloadHandler,
											bdFileMetaData *fileMetaData,
											bdUInt32 startByte = 0,
											bdUInt32 endByte = 0);

		/// Download publisher file data (and metadata) from the content server
		/// \param fileID[in]	ID of file we wish to download.
		/// \param fileData[in] Pointer to a writable area of memory of sufficient size 
		///						to receive the file data 
		/// \param fileDataSize[in]	 The size of the data pointed to by fileData.
		/// \param fileMetaData[out] The meta data associated with the file
		/// \param startByte[in]	(Optional) Offset within the file from which to commence downloading 
		///							If set to zero this param will be ignored
		/// \param endByte[in]		(Optional) Offset within the file at which to stop downloading. 
		///							If set to zero this param will be ignored
		/// \note	Do not use the range requests as a mechanism to limit the download rate - use the callback form 
		/// of download to and implement rate limiting within bdDownloadInterceptor.
		/// \note	FileData must contain enough space to download file.
		/// \return		A reference to the task that handles the request. 
		///				BD_NULL is returned if a client error occurred before execution request.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_CONTENTSTREAMING_FILE_NOT_AVAILABLE - if the file is not present 

		/// \note You can check the progress of the download operation by calling checkProgress()
		/// \note You can abort the operation by calling abortHTTPOperation()
		bdRemoteTaskRef downloadPublisherFile(const bdUInt64 fileID,
											  void *fileData, 
											  const bdUInt32 fileDataSize,
											  bdFileMetaData *fileMetaData,
											  bdUInt32 startByte = 0,
											  bdUInt32 endByte = 0);

		/// Download publisher file data (and metadata) from the content server
		/// As described above, except that this version allows clients to specify a callback handler instead
		/// of requiring a buffer to be passed in.
		/// \param downloadHandler[in] A pointer to an instance of bdDownloadInterceptor. handleDownload will be called 
		/// from the download thread every time a chunk of data has been received until the download is aborted or completed.
		/// To abort the download simply return 0 from bdDownloadInterceptor::handleDownload 
		bdRemoteTaskRef downloadPublisherFile(const bdUInt64 fileID,
											  bdDownloadInterceptor *downloadHandler,
											  bdFileMetaData *fileMetaData,
											  bdUInt32 startByte = 0,
											  bdUInt32 endByte = 0);

		/// Download publisher file data (and metadata) from the content server using the 
		/// supplied receipt as proof of purchase.
		/// \param fileID[in]		ID of file we wish to download
		/// \param receipt[in]		A Complete Apple Store Receipt in plain text form
		/// \param receiptSize[in]	The size of the data pointed to by receipt
		/// \param fileData[in]		Pointer to a writable area of memory of sufficient size 
		///							to receive the file data 
		/// \param fileDataSize[in]	The size of the data pointed to by fileData.
		/// \param fileMetaData[out] The meta data associated with the file
		/// \param startByte[in]	(Optional) Offset within the file from which to commence downloading 
		///							If set to zero this param will be ignored
		/// \param endByte[in]		(Optional) Offset within the file at which to stop downloading. 
		///							If set to zero this param will be ignored
		/// \note	Do not use the range requests as a mechanism to limit the download rate - use the callback form 
		/// of download to and implement rate limiting within bdDownloadInterceptor.
		/// \note	FileData must contain enough space to download file.
		/// \return		A reference to the task that handles the request. 
		///				BD_NULL is returned if a client error occurred before execution request.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_CONTENTSTREAMING_FILE_NOT_AVAILABLE - if the file is not present 

		/// \note You can check the progress of the download operation by calling checkProgress()
		/// \note You can abort the operation by calling abortHTTPOperation()
		bdRemoteTaskRef downloadApplePurchasedFile(const bdUInt64 fileID,
												   const void *receipt, 
												   const bdUInt32 receiptSize,
												   void *fileData,
												   const bdUInt32 fileDataSize,
												   bdFileMetaData *fileMetaData,
												   bdUInt32 startByte = 0,
												   bdUInt32 endByte = 0);


		/// Download publisher file data (and metadata) from the content server using the 
		/// supplied receipt as proof of purchase.
		/// As described above, except that this version allows clients to specify a callback handler instead
		/// of requiring a buffer to be passed in.
		/// \param downloadHandler[in] A pointer to an instance of bdDownloadInterceptor. handleDownload will be called 
		/// from the download thread every time a chunk of data has been received until the download is aborted or completed.
		/// To abort the download simply return 0 from bdDownloadInterceptor::handleDownload 
		bdRemoteTaskRef downloadApplePurchasedFile(	const bdUInt64 fileID,
													const void *receipt, 
													const bdUInt32 receiptSize,
													bdDownloadInterceptor *downloadHandler,
													bdFileMetaData *fileMetaData,
													bdUInt32 startByte = 0,
													bdUInt32 endByte = 0);

		/// Delete a file from the current users' storage
		/// \param fileSlot[in] The file slot we wish to make available
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_CONTENTSTREAMING_FILE_NOT_AVAILABLE - if the file was not present to delete
		/// \return		A reference to the task that handles the request. 
		///				BD_NULL is returned if a client error occurred before execution request.
		/// \note pump() MUST be called to complete this method.
		/// \note The users quota will be adjusted as follows after a successful remove:
		/// m_usedStorageSpace -= 'size of file deleted'
		bdRemoteTaskRef removeFile(const bdUInt16 fileSlot);

		/// Get the metadata for a file or files, searching by unique file IDs
		/// \param numFileIDs[in]		The number of FileIDs in fileIDs. This also 
		///								specifies the max number of results to be returned.
		/// \param fileIDs[in]			An array of FileIDs we seek meta data for
		/// \param maxNumResults[in]	
		/// \param fileDescriptor[out]	The results container with a minimum size of numFileIDs
		bdRemoteTaskRef getFileMetaDataByID(const bdUInt numFileIDs,
											const bdUInt64 fileIDs[],
											bdFileMetaData fileDescriptor[]);

		/// Get the thumbnail data for a file. Thumbnail data is stored as a file in bdStorage 
		/// and has a maximum size of BD_MAX_THUMB_DATA_SIZE (8k).
		/// \param numFileIDs[in]			The number of FileIDs in fileIDs
		/// \param fileIDs[in]				An array of FileIDs we seek thumbnails for
		/// \param fileDescriptor[out]		The thumbnail results class. 
		bdRemoteTaskRef getThumbNailData(const bdUInt numFileIDs,
										 const bdEntityID fileIDs[],
										 bdGetFilesResult filesResult[]);

		/// Get metadata for a number of publisher files
		/// \param startDate[in]	UNIX TIMESTAMP (seconds since epoch) specifying the creation date. 
		///							All files (up to maxNumResults) with creation time AFTER startDate will be returned.
		/// \param category[in]			If a non zero value is specified, only files of matching category will be returned.
		///								Categories for publisher files may be specified when uploading via Devzone2.
		/// \param fileMetaData[out]	Array container of size maxNumResults which will be used to 
		///								store the results of this operation (i.e. the file metadatas)
		/// \param maxNumResults[in]	The maximum number of results to be returned
		/// \param offset[in]			Used for paging through publisher files. Instructs the server to ignore
		///								this number of files in the result set
		/// \note files are sorted in reverse chronological order - the most recent file will be at 
		/// index 0 in the result set
		/// \param fileName[in] An optional parameter specifying a specific fileName to filter by. This is will
		/// only search for exact matches of the name, not partial matches.
		bdRemoteTaskRef listAllPublisherFiles(const bdUInt32 startDate,
											  const bdUInt16 category,
											  bdFileMetaData fileMetaData[],
											  const bdUInt16 maxNumResults,
											  const bdUInt16 offset = 0,
											  const bdNChar8 *fileName = BD_NULL);

		/// Get metadata for a number of user files
		/// \param ownerID[in]			Owner of the files to list
		/// \param startDate[in]		UTC Date specifying the date we wish to search for files from
		/// \param category[in]			If a non zero value is specified, only files of matching category will be returned.
		///								Specified the category of a file when uploading.
		/// \param fileMetaData[out]	Array container of size maxNumResults which will be used to 
		///								store the results of this operation (i.e. the file locations)
		/// \param maxNumResults[in]	The maximum number of results to be returned
		/// \param offset[in]			Used for paging through user files. Instructs the server to ignore
		///								this number of files.
		/// \note files are sorted in reverse chronological order - the most recent file will be at 
		/// index 0 in the result set
		/// \param fileName[in] An optional parameter specifying a specific fileName to filter by. This is will
		/// only search for exact matches of the name, not partial matches. fileName is not unique for user files.
		bdRemoteTaskRef listFilesByOwner(const bdOnlineUserID ownerID,
										 const bdUInt32 startDate,
										 const bdUInt16 category,
										 bdFileMetaData fileMetaData[],
										 const bdUInt16 maxNumResults,
										 const bdUInt16 offset = 0,
										 const bdNChar8 *fileName = BD_NULL);


		/// Get the current usage counts for the current user and the absolute bandwidth & storage limits for that user
		/// From this infomation clients can calculate the amount of quota remaining.
		/// \param quotaUsage[out] A class containing the clients current usage.
		/// \note It is recommended that this method is only called once per session 
		/// and that a client side copy is kept up to date after uploads, downloads and 
		/// removes methods are called
		/// \note Unless titles are using 'per user limits' the limits returned for each user will be 
		/// the same. 
		bdRemoteTaskRef getQuotaUsage(bdQuotaUsage *quotaUsage);

		/// Report content.
		/// Clients can use this function to report offensive or inappropriate content.
		/// The server compiles and keeps a list, which can be retrieved via webservice in order of most reported content.
		/// \param fileID[in]	The fileID of the piece of content to report.
		bdRemoteTaskRef reportContent(const bdUInt64 fileID);

		/// Aborts a http operation (upload(), downloadXXX() and remove())
		/// The associated remote task status is set to BD_FAILED
		/// \note This is not entirely immediate, but the the http should exist as soon as possible
		void abortHTTPOperation();

		/// Get the amount of data either downloaded or uploaded since the start of the last HTTP 
		/// operation (in bytes)
		/// applies to: upload(), download() 
		void checkProgress(bdUInt32 &bytesTransfered, 
						   bdFloat32 &dataRate);

		/// Get the most recently recieved http error code
		/// applies to: upload(), download() 
		bdInt32 getLastHTTPError() const;

		/// pump needs to be called in order to complete tasks involving file streaming & http requests
		/// applies to: upload(), downloadXXX() and remove().
		void pump();

#if (defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX)

		/// Toggles whether to output debug text from LibCurl. Debug text is output by default.
		/// \param enable[in] Boolean determining if to output debug text
		void enableVerboseOutput(bdBool enable);

		/// Toggles whether to disable progress statistics within LibCurl. Progress statistics
		/// are enabled by default.
		/// \param enable[in] Boolean determining if to disable progress statistics
		void enableProgressMeter(bdBool enable);

		/// Populates the m_testURLs array with servers for use by LSGTestGeneric
		/// \param URLs[in] Pointer to array of URLs to copy
		/// \param numUrls[in] Number of urls to copy
		void setTestLSPServerAddresses(bdURL* URLs, bdUInt32 numURLs);

#endif

	protected:

		void pumpUploadViaHTTP();
		void pumpDownloadViaHTTP();
		void pumpDeleteViaHTTP();

		bdRemoteTaskRef _preUpload(const bdNChar8 *const filename,
								   const bdUInt16 fileSlot,
								   const bdUInt32 fileSize,
								   const bdUInt16 category,
								   bdURL uploadURLs[]);

		/// Write the metadata for an uploaded file so that it is searchable
		/// \note	Ignoring this step means that no user will be able to access the file. Do not inform the 
		/// user of success until this step has indicated success
		bdRemoteTaskRef _postUpload();

		/// Obtain a URL with embedded authentication info which can be used to download a file via HTTP
		/// \param filename[in] Pointer to a null terminated string not more than 
		///						BD_MAX_FILE_NAME_LENGTH characters. Note that filenames are unique per user.
		///						Uploading a second file of the same name will overwrite any preexisting file
		///						of that name.
		/// \param fileSize[in] Size of data to be uploaded in bytes.
		bdRemoteTaskRef _preDownloadFileBySlot(bdOnlineUserID ownerUID,
											   bdUInt16 fileSlot,
											   bdUInt32 fileSize,
											   bdFileMetaData *fileMetaData);

		bdRemoteTaskRef _preDownloadByFileID(bdUInt64 fileID,
											 bdUInt32 fileSize,
											 bdFileMetaData *fileMetaData);

		bdRemoteTaskRef _preDownloadPublisherFile(bdUInt64 fileID,
												  bdUInt32 fileSize,
												  bdFileMetaData *fileMetaData);

		bdRemoteTaskRef _preDownloadITunesPurchasedFile(bdUInt64 fileID,
														bdUInt32 fileSize,
														const void *receipt, 
														const bdUInt32 receiptSize,
														bdFileMetaData *fileMetaData);

		bdRemoteTaskRef _upload(const bdUInt16 fileSlot,
								const void* fileData,
								bdUploadInterceptor *uploadHandler,
								const bdUInt32 fileSize,
								const bdNChar8 *const fileName,
								const bdUInt16 category,
								const void* thumbData,
								const bdUInt32 thumbDataSize,
								const bdUInt32 numTags,
								const bdTag tags[],
								bdFileID *fileID);

		bdRemoteTaskRef _download(	const bdUInt64 fileID,
									const bdOnlineUserID ownerID,
									const bdUInt16 fileSlot,
									void *fileData, 
									bdUInt32 fileSize,
									bdDownloadInterceptor *downloadHandler,
									bdFileMetaData *fileMetaData,
									bdUInt32 startByte,
									bdUInt32 endByte);

		bdRemoteTaskRef _downloadPublisherFile(	const bdUInt64 fileID,
												void *fileData, 
												bdUInt32 fileDataSize,
												bdDownloadInterceptor *downloadHandler,
												bdFileMetaData *fileMetaData,
												bdUInt32 startByte,
												bdUInt32 endByte);

		bdRemoteTaskRef _downloadApplePurchasedFile(const bdUInt64 fileID,
													const void *receipt, 
													const bdUInt32 receiptSize,
													void *fileData,
													bdUInt32 fileSize,
													bdDownloadInterceptor *downloadHandler,
													bdFileMetaData *fileMetaData,
													bdUInt32 startByte,
													bdUInt32 endByte);

		/// Obtain a URL with embedded authentication info which can be used to delete a file via HTTP
		/// \param filename[in] Pointer to a null terminated string not more than 
		///						BD_MAX_FILE_NAME_LENGTH characters. Note that filenames are unique per user.
		///						Uploading a second file of the same name will overwrite any preexisting file
		///						of that name.
		bdRemoteTaskRef _preDeleteFile(bdUInt16 fileSlot,
									   bdURL deleteURLs[]);

		// state of an combined MMP-Webserver operation
		enum bdStatus
		{
			READY				= 0,
			PRE_HTTP_OPERATION	= 1,			
			HTTP_OPERATION		= 2,		
			POST_HTTP_OPERATION	= 3,
			DONE				= 4,
			FAILED				= 5
		};

		static const char* const bdStatusString[];

		void setState(bdStatus newState, 
					  bdLobbyErrorCode errorCode = BD_NO_ERROR);

		void initUpload(const bdUInt16 fileSlot,
						const void* fileData,
						const bdUInt32 fileSize,
						bdUploadInterceptor* uploadHandler,
						const bdNChar8 *const fileName,
						const bdUInt16 category,
						const void* thumbData,
						const bdUInt32 thumbDataSize,
						const bdUInt32 numTags,
						const bdTag tags[],
						bdFileID *fileID);

		void initDownload(void *fileData, 
						  bdUInt32 fileSize,
						  bdDownloadInterceptor *downloadHandler,
						  bdFileMetaData *fileMetaData,
						  bdUInt32 startByte = 0,
						  bdUInt32 endByte = 0);

		friend class bdLobbyService;

		/// Constructs a storage manager with the given task handler.
		/// Note: never call this constructor directly, use bdLobbyService::getStorage() instead.
		/// \param remoteTaskManager[in] A pointer to the remote task manager that handles
		/// requests and replies
		bdContentStreaming(bdRemoteTaskManager *const remoteTaskManager);

		// state of an combined MMP-Webserver operation
		bdUInt m_currentTaskID;
		bdStatus m_state;

		// state during upload operations
		bdUploadInterceptor* m_uploadHandler;
		bdFileMetaData m_taskData;
		const void *m_uploadData;
		bdFileID *m_uploadFileID;
		bdURL m_URLs[BD_MAX_URL_RESULTS];
		const void *m_thumbData;
		bdUInt32 m_thumbDataSize;
		bdUInt16 m_httpSite;

		// state during download operations
		bdDownloadInterceptor* m_downloadHandler;
		bdFileMetaData *m_downloadMetaData;
		void *m_downloadData;
		bdUInt32 m_downloadDataSize;
		bdUInt32 m_startByte;
		bdUInt32 m_endByte;

		// the remote task to keep the client informed as to the status of the entire operation
		bdRemoteTaskRef m_overallTask;

		// the currently executing MMP operation (not visible to the client)
		bdRemoteTaskRef m_remoteTask; 
		bdHTTPWrapper m_http;
		bdRemoteTaskManager *m_remoteTaskManager;

#if (defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX)

		//For LSG test app only
		//Contains mappings between address to be swapped
		//Format is: Address1,AddressToBeSwappedForAddres1,Address2,AddressToBeSwappedForAddress2,...
		bdURL m_testURLs[BD_MAX_URL_RESULTS * 2];
		//Number of test addresses
		bdUInt32 m_testAddressCount;
		//Flag indicating whether to swap addresses received from LSG with those passed from command line
		bdBool m_useTestAddresses;
		
		/// Swaps the server name received from the LSG with that passed in from the command line
		/// For test use by LSGTestGeneric only!
		/// \param URL[out] URL to be swapped 
		void swapURLInfo(bdNChar8* URL);
#endif
};

const char* const s_bdStatusString[] = {
	"READY",
	"PRE_HTTP_OPERATION",			
	"HTTP_OPERATION",		
	"POST_HTTP_OPERATION",
	"DONE",
	"FAILED"
};

#endif // BD_CONTENTSTREAMING_H

