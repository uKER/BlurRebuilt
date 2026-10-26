#ifndef CONTENT_SERVER_MANAGER_H
#define CONTENT_SERVER_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/DemonwareTask.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdLobby\bdVoteRank\bdVoteRank.h>
#include <bdLobby/bdVoteRank/bdRatingInfo.h>
#include <bdLobby\bdStats\bdStatsInfo.h>
#include <bdLobby\bdContentStreaming\bdContentStreamingStatsInfo.h>
#include <bdLobby\bdStorage\bdGetFilesResult.h>
#include <bdLobby\bdRemoteTaskManager\bdRemoteTask.h>

namespace Game
{

const int MAX_CONTENT_SERVER_SEARCH_RESULTS=11;
const int MAX_CONTENT_SERVER_VOTES=10;
const int MAX_CONTENT_SERVER_LEADERBOARD_ROWS=20;

const int CONTENT_SERVER_LEADERBOARD_VOTES_START=50023;
const int CONTENT_SERVER_LEADERBOARD_DOWNLOADS_START=50026;

const int MAX_CONTENT_SERVER_PHOTO_VEHICLE_TAGS=5;
const int MAX_CONTENT_SERVER_PHOTO_POWERUP_TAGS=5;

enum CONTENT_SERVER_TYPE
{
	CONTENT_SERVER_TYPE_PHOTO = 1,
	CONTENT_SERVER_TYPE_TWITTER,
	CONTENT_SERVER_TYPE_GROUP,
	CONTENT_SERVER_TYPE_EVENT,

	CONTENT_SERVER_TYPE_MAX,
};

enum CONTENT_SERVER_GROUP_TAGS
{
	CONTENT_SERVER_TAGS_GROUP_CONTENT_TYPE,
};

enum CONTENT_SERVER_EVENT_TAGS
{
	CONTENT_SERVER_TAGS_EVENT_CONTENT_TYPE,
};

enum CONTENT_SERVER_PHOTO_TAGS
{
	CONTENT_SERVER_TAGS_PHOTO_CONTENT_TYPE,
	CONTENT_SERVER_TAGS_LOCATION,
	CONTENT_SERVER_TAGS_VEHICLE,
	CONTENT_SERVER_TAGS_POWERUP,

	CONTENT_SERVER_TAGS_PHOTO_MAX,
};

enum CONTENT_SERVER_GAME_TYPE
{
	CONTENT_SERVER_GAME_TYPE_SINGLE_PLAYER,
	CONTENT_SERVER_GAME_TYPE_MULTI_PLAYER,
};

enum CONTENT_SERVER_TASK_TYPE
{
	CONTENT_SERVER_TASK_TYPE_NOTHING,
	
	CONTENT_SERVER_TASK_TYPE_UPLOAD,
	CONTENT_SERVER_TASK_TYPE_DOWNLOAD,
	CONTENT_SERVER_TASK_TYPE_DOWNLOAD_MULTI,

	CONTENT_SERVER_TASK_TYPE_SEARCH_BY_TAGS,
	CONTENT_SERVER_TASK_TYPE_LIST_FILE_BY_OWNER,
	CONTENT_SERVER_TASK_TYPE_LIST_ALL_PUBLISHER_FILES,

	CONTENT_SERVER_TASK_TYPE_GET_META_DATA,

	CONTENT_SERVER_TASK_TYPE_REMOVE_FILE,

	CONTENT_SERVER_TASK_TYPE_VOTE,

	CONTENT_SERVER_TASK_TYPE_GET_LEADERBOARD,

	CONTENT_SERVER_TASK_TYPE_MAX,
};

enum CONTENT_SERVER_GROUP_TAG
{
};

enum CONTENT_SERVER_EVENT_TAG
{
	CONTENT_SERVER_EVENT_TAG_GROUP_ID,
};

class csVotingStatsInfo : public bdContentStreamingStatsInfo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		csVotingStatsInfo();		

		virtual bdUInt sizeOf();

		virtual void serialize(bdByteBuffer &buffer) const;
		virtual bdBool deserialize(bdByteBufferRef buffer);

		bdUInt32 m_avgVoteValue;
		bdUInt32 m_totalVotes;
};


//Manager for the new Demonware content server. Note that it is set up to only do one thing at once.
class C_ContentServerManager
{
public:
	
	C_ContentServerManager()
	{
		mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_NOTHING;
		mNumReturnedSearchResults = 0;
		mNumDownloadedFiles = 0;
		mCurrentDownloadIndex = 0;

		mDownloadBufferPtr = 0;		
		mDownloadBufferStride = 0;	
		mDownloadBufferSize = 0;	

		mKeepAutoPumping = false;
	};

	// startup / shutdown content streaming functionality
	// must be called before any content streaming can occur.
	bool Enable(const bool e);

	// used to auto-pump current task (if any)
	void Update();

	//set file name for upload, or for searches
	void SetFileName(const char *fn);

	//set slot for upload, or for searches
	void SetSlot(int slot);
	
	// set tags for upload or searching
	void ClearTags();
	void AddTag(bdTag tag);

	//sets the user index for the current user of the content server
	bool SetUserIndex(U8 index);


	//upload data, uses above Name and Tags
	bool UploadData(void *filedata, U32 filesize, void *thumbdata, U32 thumbsize, CONTENT_SERVER_TYPE type);

	// download a file from the content server
	bool DownloadData(bdUInt64 id, void *data, U32 size);

	bool DownloadData( const U8* pData, const U32 uStride, const U32 uSize );

	bool DownloadThumbnailData( const U8* pData, const U32 uStride, const U32 uSize );

	// pump the current task, returns false when finished, errors are returned by passing in a bool pointer (this will turn to an enum soon)
	bool PumpCurrentTask(int *error);


	//uses the above SetTags as input
	bool SearchByTags();

	// starts a search to list all users files, 'search_for_specific_file' will use the above SetFileName and only return that file
	bool ListFileByOwner(U32 from_date, CONTENT_SERVER_TYPE type,bool search_for_specific_file=false);

	// starts a search to find all the publisher files, 'search_for_specific_file' will use the above SetFileName and only return that file
	bool ListAllPublisherFiles(U32 from_date, CONTENT_SERVER_TYPE type, bool search_for_specific_file);

	//gets the meta data for a file, uses above SetFileName
	bool GetMetaDataFromName();

	//gets the meta data for an ID
	bool GetMetaDataFromID(bdFileID id);

	bool GetMetaDataFromID( const bdUInt numFileIDs, const bdUInt64 fileIDs[] );

	//remove a file from the content server, uses SetFileName
	bool RemoveFile();

	//kill the current task
	void KillCurrentTask();

	// set a time out for async network tasks
	void SetTimeOut(float seconds);

	//get the number of search results
	int GetNumSearchResults(){return mNumReturnedSearchResults;};

	int GetNumDownloadedFiles(){return mNumDownloadedFiles;};

	//get the search result data
	bdFileID GetSearchResultFileID(int n);

	//get the search result data
	bdFileMetaData *GetMetaData(int n);

	//get the meta data from the previous search
	bool GetMetaDataFromPreviousSearch();

	bdUInt64 GetUploadFileID() const {return mUploadedFileID.m_fileID;}
	bdUInt64 GetDownloadFileID() const {return mDownloadedFileID;}

	//VOTE RANK
	//vote on a piece of content
	bool Vote(bdEntityID id, U8 vote, bool manualVote = false);


	//LEADERBOARDS
	//get a top 20 leaderboard for votes or downloads
	bool GetLeaderboard(bool votes, CONTENT_SERVER_TYPE type);

	//get the number of leaderboard rows
	int GetNumLeaderboardRows(){return mLeaderboardRowsReturned;};

	//get the search result data
	bdContentStreamingStatsInfo *GetLeaderboardRow(int r);
	csVotingStatsInfo *GetVotingLeaderboardRow(int r);

	//UPLOAD/DOWNLOAD UI HELPERS
	int GetPercentageFinished(){return Maths::Clamp(int(((float)mDataSent/(float)mDataSize)*100),0,100);};
	float GetDataRate(){return mDataRate;};

	void GetCurrentTaskInfo(bool& working, int& errorType);

	//return the instance
	static C_ContentServerManager& Instance(){return s_instance;};

private:
	//finish the current task
	void FinishCurrentTask();

private:
	
	static C_ContentServerManager s_instance;

	//stores the current task type, so we can deal with the results effectively
	CONTENT_SERVER_TASK_TYPE mCurrentTaskType;

	// used for tracking the upload process
	bdRemoteTaskRef mAsyncTask;

	U32 mNumReturnedSearchResults;
	U32 mNumDownloadedFiles;
	U32 mCurrentDownloadIndex;

	//user information
	U8 mUserIndex;
	bdOnlineUserID mDemonWareUserID;

	//file name and slot, used in upload/download and searches
	char mFileName[BD_MAX_FILE_NAME_LENGTH];
	int mSlot;

	U8 gthumbnaildata[MAX_CONTENT_SERVER_SEARCH_RESULTS][BD_MAX_THUMB_DATA_SIZE];
	
	//tags, used in uploads and searches
	bdTag mTags[BD_MAX_NUM_TAGS];
	U32 mNumTags;

	// returned when the upload completes.
	bdFileID mUploadedFileID;
	
	// returned from download
	bdUInt64 mDownloadedFileID;
	bdFileMetaData mDownloadMetaData;

	// Filled out from metadata download
	bdEntityID mThumbnailFileIDs[MAX_CONTENT_SERVER_SEARCH_RESULTS];
	// return from thumbnail download
	bdGetFilesResult mThumbnailFilesResult[MAX_CONTENT_SERVER_SEARCH_RESULTS];

	// storage for returned search results
	bdFileID mSearchResults[MAX_CONTENT_SERVER_SEARCH_RESULTS];
	bdFileMetaData mMetaDataResults[MAX_CONTENT_SERVER_SEARCH_RESULTS];
	bdUInt64 mMetaDataIDs[MAX_CONTENT_SERVER_SEARCH_RESULTS];

	//download / upload stats
	U32 mDataSent;
	float mDataRate;
	U32 mDataSize;

	//voting stuff
	bdRatingInfo mVoteInfo[MAX_CONTENT_SERVER_VOTES];
	bool mKeepAutoPumping;
	int mAutoPumpError;

	//leaderboard stuff
	U32 mLeaderboardID;
	U32 mLeaderboardRowsReturned;
	bdContentStreamingStatsInfo mLeaderboardRows[MAX_CONTENT_SERVER_LEADERBOARD_ROWS];
	csVotingStatsInfo mLeaderboardVotingRows[MAX_CONTENT_SERVER_LEADERBOARD_ROWS];

	// User supplied when requesting multiplayer downloads
	const U8* mDownloadBufferPtr;	// Pointer to supplied download buffer
	U32 mDownloadBufferStride;		// Stride of data
	U32 mDownloadBufferSize;		// Total size of data
};


} // namespace Game

#endif // CONTENT_SERVER_MANAGER_H