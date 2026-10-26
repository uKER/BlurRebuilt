#ifndef RECORD_EVENT_MANAGER_H
#define RECORD_EVENT_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/DemonwareTask.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

#include <bdlobby\bdtitleutilities\bdTitleUtilities.h>

namespace Game
{

enum EVENT_RECORD_TASK_TYPE
{
	EVENT_RECORD_TASK_TYPE_NOTHING,
	
	EVENT_RECORD_TASK_TYPE_WRITING,

	EVENT_RECORD_TASK_TYPE_MAX,
};

enum EVENT_RECORD_CATEGORY
{
	EVENT_RECORD_CATEGORY_RACELOG = 0,

	EVENT_RECORD_CATEGORY_DATA_GATHERING_COMBINED_EVENTS = 31,

	EVENT_RECORD_CATEGORY_SORCERY_INFORMATION = 100,

	EVENT_RECORD_CATEGORY_TWITTER_AUTHENTICATION = 1000,
	EVENT_RECORD_CATEGORY_TWITTER_MESSAGE = 1001,

	EVENT_RECORD_CATEGORY_MAX,
};


class C_RecordEventManager
{
public:
	
	C_RecordEventManager()
	{
		mCurrentTaskType=EVENT_RECORD_TASK_TYPE_NOTHING;
	};
	
	//must be called before all usage of the title utilities
	bool IsActive(){return RacerNet::C_NetServices::Instance().Router().GetTitleUtilities() ? true : false;};

	//per frame update function
	void Update();

	//write an event record to the server
	bool WriteEventRecord(const char *str, EVENT_RECORD_CATEGORY category);

	//write a binary event record to the server
	bool WriteEventRecordBin(void *data, int size, EVENT_RECORD_CATEGORY category);

	// is busy
	bool IsBusy(){return mCurrentTaskType != EVENT_RECORD_TASK_TYPE_NOTHING;};

	//kill the current task
	void KillCurrentTask();

	//return the instance
	static C_RecordEventManager& Instance(){return s_instance;};

private:
	static C_RecordEventManager s_instance;
	
	EVENT_RECORD_TASK_TYPE mCurrentTaskType;

	// used for tracking reads and writes
	bdRemoteTaskRef mAsyncTask;

	//finish the current task
	void FinishCurrentTask();

	// pump the current task, returns false when finished, errors are returned by passing in a bool pointer (this will turn to an enum soon)
	bool PumpCurrentTask(bool *error);
};


} // namespace Game

#endif // RECORD_EVENT_MANAGER_H
