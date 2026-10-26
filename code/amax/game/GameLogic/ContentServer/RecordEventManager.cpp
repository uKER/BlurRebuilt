#include "stdafx.h"
#include <ContentServer/RecordEventManager.h>
#include <Network/Core.h>
#include "misc_shared/compression/deflate.h"

Debug::Logger &gRecordEventMessageLogger = Debug::GetLogger("Amax.RecordEvent");

namespace Game
{

extern Debug::Logger &gKeyArchiveMessageLogger;

C_RecordEventManager C_RecordEventManager::s_instance;

bool C_RecordEventManager::WriteEventRecord(const char *str, EVENT_RECORD_CATEGORY category)
{
	RacerNet::C_NetServices::Instance().EnterLock();

	bdTitleUtilities* title_utilities = RacerNet::C_NetServices::Instance().Router().GetTitleUtilities();

	bool ok = false;
	if (title_utilities)
	{
		KillCurrentTask();

		mAsyncTask=title_utilities->recordEvent(str,strlen(str) + 1,category);

		// did the write task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gKeyArchiveMessageLogger,"Event Record - Write Task Started\n");

			mCurrentTaskType=EVENT_RECORD_TASK_TYPE_WRITING;
			ok = false;
		}
	}

	RacerNet::C_NetServices::Instance().Exitlock();
	return ok;
}

bool C_RecordEventManager::WriteEventRecordBin(void *data, int size, EVENT_RECORD_CATEGORY category)
{
	RacerNet::C_NetServices::Instance().EnterLock();

	bdTitleUtilities* title_utilities = RacerNet::C_NetServices::Instance().Router().GetTitleUtilities();

	bool ok=false;
	if (title_utilities)
	{
		KillCurrentTask();

		// Create our buffer, move it forward so we leave space to write the uncompressed size later
		U32 bufferSize = (size + size / 50 + 128) + 4;
		U8 *outputBuffer = new U8[bufferSize];
		outputBuffer += 4;
		U32 outSize = 0;

		// Compress the data
		Compression::C_Deflate deflate;
		deflate.Create();
		deflate.SetPolicy(0);
		deflate.Start((U8*)data, size, outputBuffer, bufferSize);
		deflate.Finish(outSize);
		deflate.Destroy();

		// Write the uncompressed size to the final buffer
		outputBuffer -= 4;
		memcpy(outputBuffer, &outSize, 4);
		
		mAsyncTask = title_utilities->recordEventBin(outputBuffer, outSize + 4, category);

		delete [] outputBuffer;

		// did the write task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gKeyArchiveMessageLogger,"Event Record - Write Task Started\n");

			mCurrentTaskType=EVENT_RECORD_TASK_TYPE_WRITING;
			ok = true;
		}
	}
	RacerNet::C_NetServices::Instance().Exitlock();
	return ok;
}

void C_RecordEventManager::Update()
{
	RacerNet::C_NetServices::Instance().EnterLock();

	if(mCurrentTaskType != EVENT_RECORD_TASK_TYPE_NOTHING)
	{
		if(mCurrentTaskType == EVENT_RECORD_TASK_TYPE_WRITING)
		{
			bool error;
			bool still_going=PumpCurrentTask(&error);

			if(!still_going)
			{
				if(error)
				{
					//handle error here if needed
				}

				mCurrentTaskType=EVENT_RECORD_TASK_TYPE_NOTHING;
			}
		}
	}
	RacerNet::C_NetServices::Instance().Exitlock();
}

bool C_RecordEventManager::PumpCurrentTask(bool *error)
{
    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());

	bdTitleUtilities* title_utilities = services.Router().GetTitleUtilities();

    if (title_utilities)
    {
        *error = false;
        switch (NetPlay::DemonwareRemoteTask::CheckState(mAsyncTask))
        {
			case NetPlay::DemonwareRemoteTask::ePending:
				{
					return true;
				}
				break;

			case NetPlay::DemonwareRemoteTask::eDone:
				if (mAsyncTask->getErrorCode() == BD_NO_ERROR)
				{
					// successful
					LOGINFO( gRecordEventMessageLogger,"Event Record - Task Completed and Successful\n");

					//task is successful so do whatever we need to do with the results, this must also be called before KillCurrentTask()
					FinishCurrentTask();
					*error = false;
				}
				else
				{
					// failed
					LOGINFO( gRecordEventMessageLogger,"Event Record - Task Completed but Failed\n");

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
	KillCurrentTask();

	return false;
}

void C_RecordEventManager::FinishCurrentTask()
{
	switch(mCurrentTaskType)
	{
		case EVENT_RECORD_TASK_TYPE_NOTHING:
			assert(!"Event Record - Finished a Task that has been set to TYPE_NOTHING");
			break;

		case EVENT_RECORD_TASK_TYPE_WRITING:
			LOGINFO( gRecordEventMessageLogger,"Event Record - written\n");
			break;

		default:
			assert(!"Event Record - Finished a Task that has been set to something unknown\n");
			break;
	}
}

void C_RecordEventManager::KillCurrentTask()
{
	RacerNet::C_NetServices::Instance().EnterLock();

	LOGINFO( gRecordEventMessageLogger,"Event Record - Killing Any Current Task\n");

	mCurrentTaskType=EVENT_RECORD_TASK_TYPE_NOTHING;
	
	NetPlay::DemonwareRemoteTask::Close(mAsyncTask);

	RacerNet::C_NetServices::Instance().Exitlock();
}

}; // namespace Game
