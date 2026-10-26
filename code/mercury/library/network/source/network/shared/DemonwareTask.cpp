#include <StdAfx.h>
#include <Network/DemonwareTask.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

namespace NetPlay
{	
	namespace DemonwareRemoteTask
	{
		eTaskState CheckState(bdRemoteTaskRef& task)
		{
			eTaskState state = eFailed;
			if (task && task.notNull())
			{
				const bdRemoteTask::bdStatus remoteTaskStatus = task->getStatus();
				switch(remoteTaskStatus)
				{
				case bdRemoteTask::BD_DONE:
					state = eDone;
					break;

				case bdRemoteTask::BD_PENDING:
					state = ePending;
					break;

				default:
					state = eFailed;
					break;
				}
			}

			return state;
		}

		bool IsBusy(bdRemoteTaskRef& task)
		{
			if (CheckState(task) == ePending)
				return true;
			return false;
		}

		void Close(bdRemoteTaskRef& task)
		{
			if (task)
			{
				if (task.notNull())
					task->cancelTask();
				task = BD_NULL;
			}
		}
	}
}
