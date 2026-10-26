#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <ContentServer/KeyArchiveManager.h>

#include <Ui/Game/Profiles/ProfileInterface.h>

#include <Network/Core.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

namespace Game
{

Debug::Logger &gKeyArchiveMessageLogger = Debug::GetLogger("Amax.KeyArchive");

C_KeyArchiveManager C_KeyArchiveManager::s_instance;

bool C_KeyArchiveManager::SetUserIndex(U8 index)
{
	if (!SysProfile::C_Manager::Instance().GetDemonwareUserId(index, mDemonWareUserID))
	{
		//BIZ_WARNING_MSG(false,"Could not aquire users demonware ID");
		return false;
	}

	return true;
}

void C_KeyArchiveManager::SetDemonwareId(bdOnlineUserID id)
{
	mDemonWareUserID=id;
}

bool C_KeyArchiveManager::ClearKeyArchiveToWrite()
{
	RacerNet::C_NetServices::Instance().EnterLock();
	mKeysToWrite.Resize(0);
	RacerNet::C_NetServices::Instance().Exitlock();
	return true;
}

bool C_KeyArchiveManager::AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY key, bdInt64 value, bdKeyValuePair::bdArchiveUpdateType update_type)
{
	RacerNet::C_NetServices::Instance().EnterLock();

	if(mKeysToWrite.Size() >= KEY_ARCHIVE_INDEX_KEY_MAX)
	{
		BIZ_ASSERT_FAIL("Cannot add any more Key Archive Pairs");
		RacerNet::C_NetServices::Instance().Exitlock();
		return false;
	}

	if (value==BD_INT64_NAN)
	{
		LOGERROR(gKeyArchiveMessageLogger, "Trying to write a NaN to the KeyArchives! Key: %d", key);
		value=0; // error handling
	}

	bdKeyValuePair pair;

	pair.m_index=(bdUInt16)key;
	pair.m_value=value;
	pair.m_updateType=update_type;

	mKeysToWrite.PushBack(pair);

	RacerNet::C_NetServices::Instance().Exitlock();

	return true;
}

bool C_KeyArchiveManager::WriteKeyArchive()
{
	RacerNet::C_NetServices::Instance().EnterLock();

	if (!mAsyncTask.isNull())
	{
		LOGERROR(g_Log, "C_KeyArchiveManager::WriteKeyArchive(),  A task is still running.");
		RacerNet::C_NetServices::Instance().Exitlock();
		return false;
	}

	bdKeyArchive *key_archive = RacerNet::C_NetServices::Instance().Router().GetKeyArchive();

	bool ok = false;

	if(key_archive)
	{
		#ifndef CFG_GOLD
		for(int i=0; i<mKeysToWrite.Size(); i++)
		{
			BIZ_ASSERT_SPEED(mKeysToWrite[i].m_value!=BD_INT64_NAN);
		}
		#endif
		mAsyncTask=key_archive->write(mDemonWareUserID, false, &(mKeysToWrite[0]), mKeysToWrite.Size());

		// did the write task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gKeyArchiveMessageLogger,"Key Archive - Write Task Started\n");

			mCurrentTaskType=KEY_ARCHIVE_TASK_TYPE_WRITING;
			ok = true;
		}
	}

	RacerNet::C_NetServices::Instance().Exitlock();
	return ok;
}

bool C_KeyArchiveManager::ReadKeyArchive()
{
	RacerNet::C_NetServices::Instance().EnterLock();

	if (!mAsyncTask.isNull())
	{
		RacerNet::C_NetServices::Instance().Exitlock();
		LOGERROR(g_Log, "C_KeyArchiveManager::ReadKeyArchive(),  A task is still running.");
		return false;
	}

	for(int i=0; i<KEY_ARCHIVE_INDEX_KEY_MAX; i++)
	{
		mKeysRead[i].m_value=0;
		mKeysRead[i].m_index=0xffff;
	}
	bdKeyArchive *key_archive = RacerNet::C_NetServices::Instance().Router().GetKeyArchive();

	bool ok = false;
	if (key_archive)
	{
		mAsyncTask=key_archive->readAll(mDemonWareUserID, false, false, &mKeysRead[0], KEY_ARCHIVE_INDEX_KEY_MAX);
		// did the write task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gKeyArchiveMessageLogger,"Key Archive - Read Task Started\n");

			mCurrentTaskType=KEY_ARCHIVE_TASK_TYPE_READING;
			ok = true;
		}
	}
	RacerNet::C_NetServices::Instance().Exitlock();

	return ok;
}

bool C_KeyArchiveManager::ReadKeyArchiveSubset(int start, int size)
{
	RacerNet::C_NetServices::Instance().EnterLock();

	if (!mAsyncTask.isNull())
	{
		RacerNet::C_NetServices::Instance().Exitlock();
		LOGERROR(g_Log, "C_KeyArchiveManager::ReadKeyArchiveSubset(), A task is still running.");
		return false;
	}

	mSubsetCount=mSubsetStart=0;

	for(int i=0; i<size; i++)
	{
		mKeysRead[i].m_value=0;
		mKeysRead[i].m_index=start+i;
	}
	bdKeyArchive *key_archive = RacerNet::C_NetServices::Instance().Router().GetKeyArchive();

	bool ok = false;
	if (key_archive)
	{
		mAsyncTask=key_archive->read(mDemonWareUserID, false, false, &mKeysRead[0], size);
		// did the write task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gKeyArchiveMessageLogger,"Key Archive - Read Task Started\n");

			mSubsetStart=start;
			mSubsetCount=size;

			mCurrentTaskType=KEY_ARCHIVE_TASK_TYPE_READING_SUBSET;
			ok = true;
		}
	}
	RacerNet::C_NetServices::Instance().Exitlock();

	return ok;
}

void C_KeyArchiveManager::Update()
{
	RacerNet::C_NetServices::Instance().EnterLock();

	if(mCurrentTaskType != KEY_ARCHIVE_TASK_TYPE_NOTHING)
	{
		bool error=false;
		bool still_going=PumpCurrentTask(&error);

		if(!still_going)
		{
			mLastRequestFailed=error;
			mCurrentTaskType=KEY_ARCHIVE_TASK_TYPE_NOTHING;
		}
	}
	RacerNet::C_NetServices::Instance().Exitlock();
}

bool C_KeyArchiveManager::PumpCurrentTask(bool *error)
{
	RacerNet::C_NetServices::Instance().EnterLock();
    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    BIZ_ASSERT(services.IsInitialised());

	bdKeyArchive *key_archive = RacerNet::C_NetServices::Instance().Router().GetKeyArchive();

    if (key_archive)
    {
        *error = false;
		NetPlay::DemonwareRemoteTask::eTaskState taskState=NetPlay::DemonwareRemoteTask::CheckState(mAsyncTask);
        switch (taskState)
        {
			case NetPlay::DemonwareRemoteTask::ePending:
				{
					RacerNet::C_NetServices::Instance().Exitlock();
					return true;
				}
				break;

			case NetPlay::DemonwareRemoteTask::eDone:
				if (mAsyncTask->getErrorCode() == BD_NO_ERROR)
				{
					// successful
					LOGINFO( gKeyArchiveMessageLogger,"Key Archive - Task Completed and Successful\n");

					//task is successful so do whatever we need to do with the results, this must also be called before KillCurrentTask()
					FinishCurrentTask();
					*error = false;
				}
				else
				{
					// failed
					LOGINFO( gKeyArchiveMessageLogger,"Key Archive - Task Completed but Failed\n");

					*error = true;
				}
				break;

			default:
				*error = true;
				break;
        }
    }
	else
	{
		*error = true;
	}

    // close async task to release demonware resources.
	KillCurrentTaskInternal();
	RacerNet::C_NetServices::Instance().Exitlock();

	return false;
}

void C_KeyArchiveManager::FinishCurrentTask()
{
	switch(mCurrentTaskType)
	{
		case KEY_ARCHIVE_TASK_TYPE_NOTHING:
			assert(!"Key Archive - Finished a Task that has been set to TYPE_NOTHING");
			break;

		case KEY_ARCHIVE_TASK_TYPE_WRITING:
			LOGINFO( gKeyArchiveMessageLogger,"Key Archive - All Pairs written\n");
			break;
		case KEY_ARCHIVE_TASK_TYPE_READING:
			ProcessReadKeys();
			LOGINFO( gKeyArchiveMessageLogger,"Key Archive - All Pairs read\n");
			break;

		case KEY_ARCHIVE_TASK_TYPE_READING_SUBSET:
			ProcessReadKeysSubset();
			LOGINFO( gKeyArchiveMessageLogger,"Key Archive - All Subset Pairs read\n");
			break;

		default:
			assert(!"Key Archive - Finished a Task that has been set to something unknown\n");
			break;
	}
}

void C_KeyArchiveManager::KillCurrentTask()
{
	if (mCurrentTaskType!=KEY_ARCHIVE_TASK_TYPE_NOTHING)
	{
		mLastRequestFailed=true;
	}
	KillCurrentTaskInternal();
}

void C_KeyArchiveManager::KillCurrentTaskInternal()
{
	RacerNet::C_NetServices::Instance().EnterLock();
	LOGINFO( gKeyArchiveMessageLogger,"Key Archive - Killing Any Current Task\n");

	mCurrentTaskType=KEY_ARCHIVE_TASK_TYPE_NOTHING;
	
	NetPlay::DemonwareRemoteTask::Close(mAsyncTask);

	RacerNet::C_NetServices::Instance().Exitlock();
}

const KEY_ARCHIVE_TASK_TYPE C_KeyArchiveManager::GetCurrentTaskType() const
{
	return mCurrentTaskType;
}

bool C_KeyArchiveManager::GetLastRequestFailed() const
{
	return mLastRequestFailed;
}

void C_KeyArchiveManager::ProcessReadKeys()
{
	for(int i=0; i<KEY_ARCHIVE_INDEX_KEY_MAX; i++)
	{
		mKeyBuffer[i]=0;
	}

	for(int i=0; i<KEY_ARCHIVE_INDEX_KEY_MAX; i++)
	{
		int key=mKeysRead[i].m_index;
		if (key==0xffff)
		{
			break;
		}

		mKeyBuffer[key]=mKeysRead[i].m_value;
	}
}

void C_KeyArchiveManager::ProcessReadKeysSubset()
{
	//clear out the subset
	for(int i=0; i<mSubsetCount; i++)
	{
		mKeyBuffer[mSubsetStart+i]=0;
	}

	//copy out the sub set into the buffer
	for(int i=0; i<mSubsetCount; i++)
	{
		int key=mKeysRead[i].m_index;
		mKeyBuffer[key]=mKeysRead[i].m_value;
	}
}

}; // namespace Game
