// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The interface to the Tag Service

#ifndef BD_TAGS_H
#define BD_TAGS_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdContentStreaming/bdFileID.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdTags/bdTag.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdTagQueryParams;
class bdTagsArray;

#define BD_MAX_NUM_TAGS (20u)

/// The client-side API for writing, reading, deleting and searching content server tags.
/// A "tag" is an instance of the bdTag class, which is a pair of uints:
/// primary tag, secondary tag.
/// The primary tag is intended for use as a category (i.e. colour) 
/// and the secondary tag is the actual value in that category (i.e. blue).
///
/// Each object uploaded to the content server (i.e. an entityID) may only have BD_MAX_NUM_TAGS assigned to it.
class bdTags
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		~bdTags();

		/// Get all tags for each entityID
		///	\param numEntityIDs[in] The number of entityIDs in the bdEntityID array.
		///							This also  specifies the max number of results to be returned.
		/// \param entitiyIDs[in]	Array of bdEntityIDs (most likely fileIDs from the 
		///							content server bdFile.m_fileID) to get tags for.
		/// \param tagResults[in]	Array of results. Each array index contains tags 
		///							for a particular requested entityID.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef getTagsForEntityIDs(const bdUInt32 numEntityIDs,
											const bdEntityID entityIDs[],
											bdTagsArray tagResults[]);
		
		/// Set the tags for an already uploaded file.
		/// \param entityID[in]	The entityID (most likely a fileID from the 
		///							content server bdUploadedFile.fileID) to set the tags for.
		///	\param numTags[in]	The number of tags in the bdTags array.
		/// \param tags[in]		The tags to apply to that entityID.
		///						
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		///			BD_MAX_NUM_TAGS_EXCEEDED - if writing this set of tags will make the 
		///			maximum number of of tags allowed per file.
		bdRemoteTaskRef setTagsForEntityID(const bdEntityID entityID,
										   const bdUInt numTags,
										   const bdTag tags[]);

		/// Removes the specified tags from the specified entityID.
		/// Tags which do not exist for that entityID but are specified here are ignored on the server.
		/// \param entityID[in]	The entityID (most likely a fileID from the 
		///							content server bdUploadedFile.fileID) to remove set the tags from.
		///	\param numTags[in]	The number of tags in the bdTags array.
		/// \param tags[in]		The tags to remove to that entityID.
		///						
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef removeTagsForEntityID(const bdEntityID entityID,
											  const bdUInt numTags,
											  const bdTag tags[]);

		/// Removes all tags for the specified entityID.
		/// \param entityID[in]	The entityID (most likely a fileID from the 
		///						content server bdUploadedFile.fileID) to remove all the tags from.
		///						
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef removeAllTagsForEntityID(const bdEntityID entityID);

		/// Search for a list of files which have a subset of tags, matching all specified tags.
		/// Tag's are returned ordered by "primaryTag descending, secondaryTag descending".
		///
		/// \param startIndex[in]	 Offset into the result set. Used for paging through large result sets.
		///							 Instructs the server to ignore this number of files at the start of your 
		///							 query results.
		/// \param maxNumResults[in] The maximum number of results to be returned.
		///	\param numTags[in]		 The number of tags in the bdTags array.
		/// \param tags[in]			 An array of bdTag specifying the tags to search for.
		/// \param results[out]		 Results class which will contain the fileIDs found by the tag search.
		///		
		/// \return	The A BD_PENDING bdRemoteTask
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef searchByTags(const bdUInt startIndex,
                                     const bdUInt maxNumResults,
									 const bdUInt numTags,
									 const bdTag tags[],
									 bdFileID results[]);

protected:

		friend class bdLobbyService;

		bdTags(bdRemoteTaskManager *const remoteTaskManager);

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;
};

#endif // BD_TAGS_H
