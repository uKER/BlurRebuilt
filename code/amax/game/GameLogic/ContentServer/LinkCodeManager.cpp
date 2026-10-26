#include "stdafx.h"

#include <ContentServer/LinkCodeManager.h>

#include <Ui/Game/Profiles/ProfileInterface.h>

#include <Network/Core.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

#include <GameLogic\Profile\GameProfileManager.h>

Debug::Logger &gLinkCodeMessageLogger = Debug::GetLogger("Amax.LinkCode");

namespace Game
{

C_LinkCodeManager C_LinkCodeManager::s_instance;

bool C_LinkCodeManager::SetUserIndex(U8 index)
{
	mUserIndex=index;

	if (!SysProfile::C_Manager::Instance().GetDemonwareUserId(mUserIndex, mDemonWareUserID))
	{
		//BIZ_WARNING_MSG(false,"Could not aquire users demonware ID");
		return false;
	}

	return true;
}

void C_LinkCodeManager::GetLinkCode()
{
	if(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mWebsiteLinkCode.Length())
		return;

    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());

	bdLinkCode* link_code = services.Router().GetAgoraLink();

    if (link_code)
    {
		mCategories[0]=0;
		mAsyncTask=link_code->getLinkCodes(mDemonWareUserID,mCategories,&mLink,MAX_LINK_CODE_CATEGORIES);

		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gLinkCodeMessageLogger,"Link Code - Getting Started\n");

			mCurrentTaskType=LINK_CODE_TASK_TYPE_GETTING;
		}
	}
}

void C_LinkCodeManager::Update()
{
	if(mCurrentTaskType == LINK_CODE_TASK_TYPE_NOTHING)
		return;

	if(mCurrentTaskType == LINK_CODE_TASK_TYPE_GETTING)
	{
		bool error;
		bool still_going=PumpCurrentTask(&error);

		if(!still_going)
		{
			if(error)
			{
				//handle error here if needed
			}

			mCurrentTaskType=LINK_CODE_TASK_TYPE_NOTHING;
		}
	}
}

bool C_LinkCodeManager::PumpCurrentTask(bool *error)
{
    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());

	bdLinkCode* link_code = services.Router().GetAgoraLink();

    if (link_code)
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
					Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mWebsiteLinkCode = mLink.m_linkCode;

					LOGINFO( gLinkCodeMessageLogger,"Link Code - Task Completed and Successful\n");
					*error = false;
					return false;
				}
				else
				{
					// failed
					LOGINFO( gLinkCodeMessageLogger,"Link Code - Task Completed but Failed\n");
					*error = true;
					return false;
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

void C_LinkCodeManager::KillCurrentTask()
{
	LOGINFO( gLinkCodeMessageLogger,"Link Code - Killing Any Current Task\n");

	mCurrentTaskType=LINK_CODE_TASK_TYPE_NOTHING;
	
	NetPlay::DemonwareRemoteTask::Close(mAsyncTask);
}

}; // namespace Game
