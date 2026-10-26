#include "stdafx.h"

#include <ContentServer/ContentServerManager.h>

#include <SafeString.inl>

#include <Ui/Game/Profiles/ProfileInterface.h>

#include <Network/Core.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

Debug::Logger &gContentServerMessageLogger = Debug::GetLogger("Amax.ContentServer");

Debug::Bool g_debugContentServerInjectGeneralError("ContentServer.InjectError", false);
Debug::Int g_debugContentServerInjectGeneralErrorEnum("ContentServer.InjectErrorEnum", 0);

namespace Game
{

C_ContentServerManager C_ContentServerManager::s_instance;


csVotingStatsInfo::csVotingStatsInfo()
{
}

void csVotingStatsInfo::serialize(bdByteBuffer &buffer) const
{
	// Call up to the base class to serialize the standard statistics info.
	bdContentStreamingStatsInfo::serialize(buffer);

	// Serialize the custom attributes.
	buffer.writeUInt32(m_avgVoteValue);
	buffer.writeUInt32(m_totalVotes);

}

bdBool csVotingStatsInfo::deserialize(bdByteBufferRef buffer)
{
	// Call up to the base class to deserialize the standard matchmaking info.
	bdBool ok = bdContentStreamingStatsInfo::deserialize(buffer);
	ok = ok && buffer->readUInt32(m_avgVoteValue);	
	ok = ok && buffer->readUInt32(m_totalVotes);	
	return ok;
}

bdUInt csVotingStatsInfo::sizeOf()
{
	return sizeof(csVotingStatsInfo);
}


bool C_ContentServerManager::Enable(const bool e)
{
	if (RacerNet::C_NetServices::Instance().IsInitialised())
		return RacerNet::C_NetServices::Instance().Router().EnableContentStreaming(e);
	return false;
}

void C_ContentServerManager::Update()
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		services.EnterLock();
		if(mKeepAutoPumping)
		{
			mKeepAutoPumping = PumpCurrentTask(&mAutoPumpError);
		}
		services.Exitlock();
	}
}

void C_ContentServerManager::SetFileName(const char *fn)
{
	Amax::C_SafeString::StringCopy(mFileName,sizeof(mFileName),fn);
}

void C_ContentServerManager::SetSlot(int slot)
{
	mSlot=slot;
}

void C_ContentServerManager::ClearTags()
{
	mNumTags=0;
}

void C_ContentServerManager::AddTag(bdTag tag)
{
	if(tag.m_priTag == -1 || tag.m_secTag == -1)
	{
		BIZ_WARNING_MSG(false,"Trying to use a -1 tag on the Content Server - Ignoring");
		return;
	}

	mTags[mNumTags]=tag;

	mNumTags++;

	assert(mNumTags <= BD_MAX_NUM_TAGS);	//added a check as we can't use Btl::Vector
}

bool C_ContentServerManager::SetUserIndex(U8 index)
{
	mUserIndex=index;

	if (!SysProfile::C_Manager::Instance().GetDemonwareUserId(mUserIndex, mDemonWareUserID))
	{
		//BIZ_WARNING_MSG(false, "Could not aquire users demonware ID");
		return false;
	}

	return true;
}

bool C_ContentServerManager::UploadData(void *filedata, U32 filesize, void *thumbdata, U32 thumbsize, CONTENT_SERVER_TYPE type)
{
	bool ok=false;

    // close previous async upload task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		mAsyncTask = streaming->upload(mSlot, filedata, filesize, mFileName, type, thumbdata, thumbsize, mNumTags, mTags, &mUploadedFileID);

		// did the upload task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - Upload Task Started\n");

			mDataSize=filesize;
			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_UPLOAD;
			ok=true;
		}
	}

	return ok;
}

bool C_ContentServerManager::DownloadData(bdUInt64 id,void *data,U32 size)
{
	bool ok=false;

    // close previous async upload task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		mDownloadMetaData.reset();
		mAsyncTask = streaming->download(id, data, size, &mDownloadMetaData);

		// did the upload task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - Download Task Started FileID %d, Size %d\n",id,size);

			mDataSize=size;
			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_DOWNLOAD;
			ok=true;
		}
	}

	return ok;
}


bool C_ContentServerManager::DownloadData( const U8* pData, const U32 uStride, const U32 uSize )
{
	BIZ_ASSERT( pData );

	// This gets reset after the download succeeds
	if ( mCurrentDownloadIndex )
		return false;
	
	mDownloadBufferPtr = pData;
	mDownloadBufferStride = uStride;
	mDownloadBufferSize = uSize;

	mNumDownloadedFiles = 0; // reset

    // close previous async upload task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		mDownloadMetaData.reset();
		mAsyncTask = streaming->download(mMetaDataResults[mCurrentDownloadIndex].m_fileID, (void*)mDownloadBufferPtr, mMetaDataResults[mCurrentDownloadIndex].m_fileSize, &mDownloadMetaData );

		// did the upload task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - Download Task Started FileID %d, Size %d\n",mMetaDataResults[mCurrentDownloadIndex].m_fileID, uStride );

			mDataSize = mMetaDataResults[mCurrentDownloadIndex].m_fileSize;
			mCurrentTaskType = CONTENT_SERVER_TASK_TYPE_DOWNLOAD_MULTI;
			return true;
		}
	}

	return false;
}


bool C_ContentServerManager::DownloadThumbnailData( const U8* pData, const U32 uStride, const U32 uSize )
{
	bool ok=false;

	// check its not too small
	if ( uSize < mNumReturnedSearchResults )
	{
		BIZ_ASSERT_MSG( uSize < mNumReturnedSearchResults, "Size must match search results size" );
		return false;
	}

	// Copy meta data details into the results structure so we can request it from the content server
	for( U32 i = 0; i < mNumReturnedSearchResults; i++ )
	{
		mThumbnailFileIDs[i] = mMetaDataResults[i].m_thumbNailFileID;
		mThumbnailFilesResult[i].m_data.m_fileData = (void*)pData;
		mThumbnailFilesResult[i].m_data.m_fileSize = uStride;

		pData += uStride;
	}

    // close previous async upload task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		mAsyncTask = streaming->getThumbNailData( mNumReturnedSearchResults, mThumbnailFileIDs, mThumbnailFilesResult );

		// did the upload task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - DownloadThumbnailData Task Started\n");

			mNumDownloadedFiles = 0;
			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_DOWNLOAD;
			ok=true;
		}
	}

	return ok;
}

bool C_ContentServerManager::PumpCurrentTask(int *error)
{
    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());
	services.EnterLock();

 	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
    if (streaming)
    {
        *error = BD_NO_ERROR;
        switch (NetPlay::DemonwareRemoteTask::CheckState(mAsyncTask))
        {
			case NetPlay::DemonwareRemoteTask::ePending:
				{
					if(mCurrentTaskType == CONTENT_SERVER_TASK_TYPE_DOWNLOAD || CONTENT_SERVER_TASK_TYPE_DOWNLOAD_MULTI || mCurrentTaskType == CONTENT_SERVER_TASK_TYPE_UPLOAD)
					{
						streaming->checkProgress(mDataSent, mDataRate); // num bytes sent/received.
						LOGINFO( gContentServerMessageLogger,"Content Server - Pending (%0.2f / %0.2f)\n",(float)mDataSent/1024.f,(float)mDataRate/1024.f);
					}

					services.Exitlock();
					return true;
				}
				break;

			case NetPlay::DemonwareRemoteTask::eFailed:
				{
					int e=mAsyncTask ? mAsyncTask->getErrorCode() : BD_HANDLE_TASK_FAILED;
					LOGINFO( gContentServerMessageLogger,"Content Server - Task Failed (%d)\n",e);
					*error = e;
					services.Exitlock();
					return false;
				}
				break;

			case NetPlay::DemonwareRemoteTask::eDone:
				if (mAsyncTask->getErrorCode() == BD_NO_ERROR)
				{
					bool finished = true;
					if ( mCurrentTaskType == CONTENT_SERVER_TASK_TYPE_DOWNLOAD_MULTI )
					{
						LOGINFO( gContentServerMessageLogger,"Content Server - File Downloaded - ID %d Size %d\n",mMetaDataResults[mCurrentDownloadIndex].m_fileID,mMetaDataResults[mCurrentDownloadIndex].m_fileSize);					

						mNumDownloadedFiles++;
						mCurrentDownloadIndex++;

						// Continue download?
						if ( mCurrentDownloadIndex < mNumReturnedSearchResults )
						{
							// Start download task
							bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
							if (streaming)
							{
								mDownloadMetaData.reset();
								mAsyncTask = streaming->download(mMetaDataResults[mCurrentDownloadIndex].m_fileID, (void*)(mDownloadBufferPtr += mDownloadBufferStride), mDownloadBufferStride, &mDownloadMetaData);

								// did the upload task start
								if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
								{
									mCurrentTaskType = CONTENT_SERVER_TASK_TYPE_DOWNLOAD_MULTI;
									mDataSize += mMetaDataResults[mCurrentDownloadIndex].m_fileSize;

									services.Exitlock();
									return true;
								}
							}
						}
						else		
							finished = true; // we are done :-)
					}
					
					if ( finished )
					{
						// successful
						LOGINFO( gContentServerMessageLogger,"Content Server - Task Completed and Successful\n");

						//task is successful so do whatever we need to do with the results, this must also be called before KillCurrentTask()
						FinishCurrentTask();
						*error = BD_NO_ERROR;

						#ifndef CFG_GOLD
						if(g_debugContentServerInjectGeneralError)
						{
							g_debugContentServerInjectGeneralError=false;
							*error=g_debugContentServerInjectGeneralErrorEnum;
						}
						#endif
					}
				}
				else
				{
					// Could fail 1 of 10 so skip it
					if ( mCurrentTaskType == CONTENT_SERVER_TASK_TYPE_DOWNLOAD_MULTI )
					{
						mCurrentDownloadIndex++;
						services.Exitlock();
						return true;
					}
					else
					{
						// failed
						int e=mAsyncTask ? mAsyncTask->getErrorCode() : BD_HANDLE_TASK_FAILED;

						LOGINFO( gContentServerMessageLogger,"Content Server - Task Completed but Failed\n");
						mCurrentDownloadIndex = 0;
						
						*error = e;
						services.Exitlock();
						return false;
					}
				}
				break;

			default:
				break;
        }
    }
	else
	{
		*error = BD_HANDLE_TASK_FAILED;
	}

    // close async task to release demonware resources.
	KillCurrentTask();

	services.Exitlock();
	return false;
}

void C_ContentServerManager::FinishCurrentTask()
{
	switch(mCurrentTaskType)
	{
		case CONTENT_SERVER_TASK_TYPE_NOTHING:
			assert(!"Content Server - Finished a Task that has been set to TYPE_NOTHING");
			break;

		case CONTENT_SERVER_TASK_TYPE_UPLOAD:
			LOGINFO( gContentServerMessageLogger,"Content Server - File Uploaded with an ID of %d\n",mUploadedFileID.m_fileID);
			break;
		case CONTENT_SERVER_TASK_TYPE_DOWNLOAD:
			LOGINFO( gContentServerMessageLogger,"Content Server - File Downloaded - ID %d Size %d\n",mDownloadMetaData.m_fileID,mDownloadMetaData.m_fileSize);
			mDownloadedFileID = mDownloadMetaData.m_fileID;
			break;

		case CONTENT_SERVER_TASK_TYPE_DOWNLOAD_MULTI:
			LOGINFO( gContentServerMessageLogger,"Content Server - Files Downloaded - %d Size \n", mNumDownloadedFiles );
			mCurrentDownloadIndex = 0;
			break;

		case CONTENT_SERVER_TASK_TYPE_SEARCH_BY_TAGS:
			mNumReturnedSearchResults=mAsyncTask->getNumResults();
			LOGINFO( gContentServerMessageLogger,"Content Server - Search Returned with %d results\n",mNumReturnedSearchResults);
			for(U32 a=0;a<mNumReturnedSearchResults;a++)
			{
				LOGINFO( gContentServerMessageLogger,"Content Server - File ID %d\n",mSearchResults[a].m_fileID);
			}

			break;

		case CONTENT_SERVER_TASK_TYPE_LIST_ALL_PUBLISHER_FILES:
		case CONTENT_SERVER_TASK_TYPE_LIST_FILE_BY_OWNER:
		case CONTENT_SERVER_TASK_TYPE_GET_META_DATA:
			mNumReturnedSearchResults=mAsyncTask->getNumResults();
			LOGINFO( gContentServerMessageLogger,"Content Server - Meta Data aquired %d results and put them in mMetaDataResults\n",mNumReturnedSearchResults);
			for(U32 a=0;a<mNumReturnedSearchResults;a++)
			{
				LOGINFO( gContentServerMessageLogger,"Content Server - File ID %d Slot %d Name %s Size %d\n",mMetaDataResults[a].m_fileID,mMetaDataResults[a].m_fileSlot,mMetaDataResults[a].m_fileName,mMetaDataResults[a].m_fileSize);
			}
			break;

		case CONTENT_SERVER_TASK_TYPE_REMOVE_FILE:
			LOGINFO( gContentServerMessageLogger,"Content Server - File %s Removed\n",mFileName);
			break;

		case CONTENT_SERVER_TASK_TYPE_VOTE:
			LOGINFO( gContentServerMessageLogger,"Content Server - Thank you for Voting\n");
			break;

		case CONTENT_SERVER_TASK_TYPE_GET_LEADERBOARD:
			mLeaderboardRowsReturned=mAsyncTask->getNumResults();
			LOGINFO( gContentServerMessageLogger,"Content Server - Leaderboard returned %d rows\n",mLeaderboardRowsReturned);
			for(U32 a=0;a<mLeaderboardRowsReturned;a++)
			{
				if(mLeaderboardID < CONTENT_SERVER_LEADERBOARD_DOWNLOADS_START)
					LOGINFO( gContentServerMessageLogger,"Content Server - Row %d ID %d Rating %d\n", a+1, mLeaderboardVotingRows[a].m_entityID, mLeaderboardVotingRows[a].m_rating);
				else
					LOGINFO( gContentServerMessageLogger,"Content Server - Row %d ID %d Rating %d\n", a+1, mLeaderboardRows[a].m_entityID, mLeaderboardRows[a].m_rating);
			}
			break;

		default:
			assert(!"Content Server - Finished a Task that has been set to something unknown\n");
			break;
	}
}

bool C_ContentServerManager::SearchByTags()
{
	bool ok=false;
    
	KillCurrentTask();

	bdTags *tags = RacerNet::C_NetServices::Instance().Router().GetTags();

	if(tags)
	{
		mAsyncTask = tags->searchByTags(0,MAX_CONTENT_SERVER_SEARCH_RESULTS,mNumTags,mTags,mSearchResults);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - Search By Tags Started\n");

			mNumReturnedSearchResults=0;
			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_SEARCH_BY_TAGS;
			ok=true;
		}
	}

	return ok;
}

bool C_ContentServerManager::ListFileByOwner(U32 from_date, CONTENT_SERVER_TYPE type, bool search_for_specific_file)
{
	bool ok=false;
    
	// close previous async task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		for(int a=0;a<MAX_CONTENT_SERVER_SEARCH_RESULTS;a++)
		{
			mMetaDataResults[a].reset();
		}

		if(search_for_specific_file)
		{
			mAsyncTask = streaming->listFilesByOwner(mDemonWareUserID,from_date,type,mMetaDataResults,MAX_CONTENT_SERVER_SEARCH_RESULTS,0,mFileName);
		}
		else
		{
			mAsyncTask = streaming->listFilesByOwner(mDemonWareUserID,from_date,type,mMetaDataResults,MAX_CONTENT_SERVER_SEARCH_RESULTS,0);
		}

		// did the upload start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - List File By Owner Started\n");
			
			mNumReturnedSearchResults=0;
			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_LIST_FILE_BY_OWNER;
			ok=true;
		}
	}

	return ok;
}

bool C_ContentServerManager::ListAllPublisherFiles(U32 from_date, CONTENT_SERVER_TYPE type, bool search_for_specific_file)
{
	bool ok=false;
    
	// close previous async task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		if(search_for_specific_file)
			mAsyncTask = streaming->listAllPublisherFiles(from_date,type,mMetaDataResults,MAX_CONTENT_SERVER_SEARCH_RESULTS,0,mFileName);
		else
			mAsyncTask = streaming->listAllPublisherFiles(from_date,type,mMetaDataResults,MAX_CONTENT_SERVER_SEARCH_RESULTS,0);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - List All Publisher Files Started\n");

			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_LIST_ALL_PUBLISHER_FILES;
			ok=true;
		}
	}

	return ok;
}

bool C_ContentServerManager::GetMetaDataFromName()
{
	bool ok=false;
    
	/*
	// close previous async task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		mAsyncTask = streaming->getFileMetaData(mFileName,&mSearchResults[0]);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - Get Meta Data From Name Started\n");

			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_GET_META_DATA;
			ok=true;
		}
	}
	*/
	return ok;
}

bool C_ContentServerManager::GetMetaDataFromPreviousSearch()
{
	bool ok=false;
    
	// close previous async task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		for(unsigned int a=0;a<mNumReturnedSearchResults;a++)
		{
			mMetaDataIDs[a]=mSearchResults[a].m_fileID;
			mMetaDataResults[a].reset();
		}

		mAsyncTask = streaming->getFileMetaDataByID(mNumReturnedSearchResults, mMetaDataIDs, mMetaDataResults);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - Get Meta Data From Previous Search Started\n");

			mNumReturnedSearchResults=0;
			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_GET_META_DATA;
			ok=true;
		}
	}	

	return ok;
}

bool C_ContentServerManager::GetMetaDataFromID(bdFileID id)
{
	bool ok=false;
    
	// close previous async task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		static bdUInt64 ids[1];
		ids[0]=id.m_fileID;

		mAsyncTask = streaming->getFileMetaDataByID(1, ids, mMetaDataResults);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - Get Meta Data From Name ID Started\n");

			mNumReturnedSearchResults=0;
			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_GET_META_DATA;
			ok=true;
		}
	}	

	return ok;
}


bool C_ContentServerManager::GetMetaDataFromID( const bdUInt numFileIDs, const bdUInt64 fileIDs[] )
{
	bool ok=false;
    
	// close previous async task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		for(unsigned int a=0;a<numFileIDs;a++)
		{
			mMetaDataIDs[a]=fileIDs[a];
		}

		mAsyncTask = streaming->getFileMetaDataByID(numFileIDs, mMetaDataIDs, mMetaDataResults);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - GetMetaDataFromID Started\n");

			mNumReturnedSearchResults=0;
			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_GET_META_DATA;
			ok=true;
		}
	}	

	return ok;
}

bool C_ContentServerManager::RemoveFile()
{
	bool ok=false;

	// close previous async task if active.
    KillCurrentTask();

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		mAsyncTask = streaming->removeFile(0);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gContentServerMessageLogger,"Content Server - Removed File Started\n");

			mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_REMOVE_FILE;
			ok=true;
		}
	}

	return ok;
}

void C_ContentServerManager::SetTimeOut(float seconds)
{
	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
	if (streaming)
	{
		LOGWARN( gContentServerMessageLogger,"Content Server - Timeout no longer implemented.\n");
	}
}

void C_ContentServerManager::KillCurrentTask()
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if(services.IsInitialised())
	{
		services.EnterLock();

		LOGINFO( gContentServerMessageLogger,"Content Server - Killing Any Current Task\n");

		mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_NOTHING;
		NetPlay::DemonwareRemoteTask::Close(mAsyncTask);
		services.Exitlock();
	}
}

bdFileID C_ContentServerManager::GetSearchResultFileID(int n)
{
	assert(n >= 0 && n < (int) mNumReturnedSearchResults);

	return mSearchResults[n];
}

bdFileMetaData *C_ContentServerManager::GetMetaData(int n)
{
	assert(n >= 0 && n < MAX_CONTENT_SERVER_SEARCH_RESULTS);
	
	return &mMetaDataResults[n];
}

bool C_ContentServerManager::Vote(bdEntityID id, U8 vote, bool manualVote)
{
	bool ok=false;

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		services.EnterLock();
		bdVoteRank *vote_rank = RacerNet::C_NetServices::Instance().GetVoteRank();

		// close previous async task if active.
		KillCurrentTask();

		if(vote_rank)
		{
			mVoteInfo[0].m_entityID=id;
			mVoteInfo[0].m_rating=vote;

			mAsyncTask = vote_rank->submitRating(mVoteInfo,1);

			// did the task start
			if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
			{
				LOGINFO( gContentServerMessageLogger,"Content Server - Voting Started\n");

				mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_VOTE;
				ok=true;
			}
		}

		if(manualVote)
			mKeepAutoPumping = true;

		services.Exitlock();
	}
	return ok;
}

bool C_ContentServerManager::GetLeaderboard(bool votes, CONTENT_SERVER_TYPE type)
{
	bool ok=false;

	KillCurrentTask();

	bdNetImpl *const net = bdNet::getInstance();
	bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();

	if (NULL == lobbyService || net->getStatus() != bdNetImpl::BD_NET_DONE || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
	{
		return false;
	}

	bdStats* pStatsService = lobbyService->getStats();
	if (NULL == pStatsService)
	{
		return false;
	}

	if(votes)
	{
		mAsyncTask = pStatsService->readStatsByRank(CONTENT_SERVER_LEADERBOARD_VOTES_START+type, 1, mLeaderboardVotingRows, MAX_CONTENT_SERVER_LEADERBOARD_ROWS);
		mLeaderboardID=CONTENT_SERVER_LEADERBOARD_VOTES_START+type;
	}
	else
	{
		mAsyncTask = pStatsService->readStatsByRank(CONTENT_SERVER_LEADERBOARD_DOWNLOADS_START+type, 1, mLeaderboardRows, MAX_CONTENT_SERVER_LEADERBOARD_ROWS);
		mLeaderboardID=CONTENT_SERVER_LEADERBOARD_DOWNLOADS_START+type;
	}

	// did the task start
	if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
	{
		LOGINFO( gContentServerMessageLogger,"Content Server - Top 20 Leaderboard Started\n");

		mLeaderboardRowsReturned=0;
		mCurrentTaskType=CONTENT_SERVER_TASK_TYPE_GET_LEADERBOARD;
		ok=true;
	}

	return ok;
}

bdContentStreamingStatsInfo *C_ContentServerManager::GetLeaderboardRow(int r)
{
	assert(r >= 0 && r < MAX_CONTENT_SERVER_LEADERBOARD_ROWS);
	
	return &mLeaderboardRows[r];
}

csVotingStatsInfo *C_ContentServerManager::GetVotingLeaderboardRow(int r)
{
	assert(r >= 0 && r < MAX_CONTENT_SERVER_LEADERBOARD_ROWS);
	
	return &mLeaderboardVotingRows[r];
}

void C_ContentServerManager::GetCurrentTaskInfo( bool& working, int& errorType )
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		services.EnterLock();
		working = mKeepAutoPumping;
		errorType = mAutoPumpError;
		services.Exitlock();
	}
}

}; // namespace Game
