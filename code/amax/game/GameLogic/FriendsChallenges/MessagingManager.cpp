#include "stdafx.h"

#include <FriendsChallenges/MessagingManager.h>

#include <Ui/Game/Profiles/ProfileInterface.h>

#include <Network/Core.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\FriendsChallenges\FriendChallenges.h>

Debug::Logger &gMessagingMessageLogger = Debug::GetLogger("Amax.Messaging");

namespace Game
{

C_MessagingManager C_MessagingManager::s_instance;

bool C_MessagingManager::SendMessage(bdOnlineUserID ids[], int num_ids, void *message, int message_size)
{
    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());

	bdMessaging* messaging = services.Router().GetMessaging();
		
	KillCurrentTask();

    if (messaging)
    {
		mAsyncTask = messaging->sendMail(message,message_size,ids,num_ids);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gMessagingMessageLogger,"Messaging - Sending Message\n");
			mCurrentTaskType=MESSAGING_TASK_TYPE_SENDING_MESSAGE;
			return true;
		}

	}

	return false;
}

bool C_MessagingManager::GetMail(bool silent)
{
	if(mCurrentTaskType != MESSAGING_TASK_TYPE_NOTHING)
		return true;

    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());

	bdMessaging* messaging = services.Router().GetMessaging();
		
	KillCurrentTask();

	mMailHeadersRetrieved=0;
	mSilent=silent;

    if (messaging)
    {
		mAsyncTask = messaging->getMailHeaders(mMailHeaders,0,MAX_MESSAGING_MAILS);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gMessagingMessageLogger,"Messaging - Getting Mail Headers\n");
			mCurrentTaskType=MESSAGING_TASK_TYPE_GETTING_HEADERS;
			return true;
		}
	}

	return false;
}


bool C_MessagingManager::GetMailMessages()
{
    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());

	bdMessaging* messaging = services.Router().GetMessaging();
		
	KillCurrentTask();

    if (messaging)
    {
		for(U32 a=0;a<mMailHeadersRetrieved;a++)
		{
			mMailIDs[a]=mMailHeaders[a].m_mailID;

			U32 size=mMailHeaders[a].m_size;

			if(mMailBodies[a].m_mailBody)
			{
				delete[] (U8 *)mMailBodies[a].m_mailBody;
			}

			mMailBodies[a].m_mailID=0;
			mMailBodies[a].m_size=size;
			mMailBodies[a].m_mailBody = new U8[size];
		}

		mAsyncTask = messaging->getMails(mMailIDs, mMailHeadersRetrieved, mMailBodies, false);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gMessagingMessageLogger,"Messaging - Getting Mail Bodies\n");
			mCurrentTaskType=MESSAGING_TASK_TYPE_GETTING_MESSAGES;
			return true;
		}
	}

	return false;
}

bool C_MessagingManager::DeleteMail(int index)
{
    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());

	bdMessaging* messaging = services.Router().GetMessaging();
		
	KillCurrentTask();

    if (messaging)
    {
		bdUInt64 ids[1] = {mMailIDs[index]};
		
		mDeletingIndex=index;
		mAsyncTask = messaging->deleteMail(ids,1);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gMessagingMessageLogger,"Messaging - Delete Mail\n");
			mCurrentTaskType=MESSAGING_TASK_TYPE_DELETING_MESSAGE;
			return true;
		}
	}

	return false;
}

void C_MessagingManager::Update()
{
	if(mCurrentTaskType == MESSAGING_TASK_TYPE_NOTHING)
		return;

	if(mSilent)
	{
		if(mCurrentTaskType == MESSAGING_TASK_TYPE_GETTING_HEADERS || mCurrentTaskType == MESSAGING_TASK_TYPE_GETTING_MESSAGES)
		{
			int error;
			bool still_going=PumpCurrentTask(&error);

			if(!still_going)
			{
				if(error)
				{
					//handle error here if needed
				}

				mCurrentTaskType=MESSAGING_TASK_TYPE_NOTHING;
			}
		}
	}
}

bool C_MessagingManager::PumpCurrentTask(int *error)
{
    RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
    assert(services.IsInitialised());

	bdMessaging* messaging = services.Router().GetMessaging();

    if (messaging)
    {
        *error = BD_NO_ERROR;
        switch (NetPlay::DemonwareRemoteTask::CheckState(mAsyncTask))
        {
			case NetPlay::DemonwareRemoteTask::ePending:
				{
					return true;
				}
				break;

			case NetPlay::DemonwareRemoteTask::eFailed:
				{
					int e=mAsyncTask ? mAsyncTask->getErrorCode() : BD_HANDLE_TASK_FAILED;
					*error = e;
					
					// close async task to release demonware resources.
					KillCurrentTask();
					
					LOGINFO( gMessagingMessageLogger,"Messaging - Task Failed (%d)\n", e);
					return false;
				}
				break;

			case NetPlay::DemonwareRemoteTask::eDone:
				if (mAsyncTask->getErrorCode() == BD_NO_ERROR)
				{
			        *error = BD_NO_ERROR;

					if(mCurrentTaskType==MESSAGING_TASK_TYPE_GETTING_HEADERS)
					{
						mMailHeadersRetrieved=mAsyncTask->getNumResults();
						mTotalMailHeadersOnServer=mAsyncTask->getTotalNumResults();

						if(mMailHeadersRetrieved)
						{
							bool ok=GetMailMessages();

							if(!ok)
							{
								// close async task to release demonware resources.
								KillCurrentTask();

								*error = BD_HANDLE_TASK_FAILED;
								return false;
							}
						
							return true;
						}
						else
						{
							// close async task to release demonware resources.
							KillCurrentTask();
							return false;
						}
					}
					else if(mCurrentTaskType==MESSAGING_TASK_TYPE_GETTING_MESSAGES)
					{
						mNewMailSinceLastGet=false;

						// close async task to release demonware resources.
						KillCurrentTask();

						for(U32 a=0; a<mMailHeadersRetrieved; a++)
						{
							FriendChallenge *fc=(FriendChallenge *)mMailBodies[a].m_mailBody;

							LOGINFO( gMessagingMessageLogger,"Messaging - Downloaded %d %S\n", mMailBodies[a].m_mailID, fc->mName);
						}
					}
					else if(mCurrentTaskType==MESSAGING_TASK_TYPE_DELETING_MESSAGE)
					{
						// close async task to release demonware resources.
						KillCurrentTask();

						//remove the memory of the deleted one
						if(mMailBodies[mDeletingIndex].m_mailBody)
						{
							delete[] (U8 *)mMailBodies[mDeletingIndex].m_mailBody;
							mMailBodies[mDeletingIndex].m_mailBody=NULL;
						}

						//if its removed from the server, then remove it from the list and shuffle the others up
						mMailHeadersRetrieved--;
						for(U32 a=mDeletingIndex; a < mMailHeadersRetrieved && a < MAX_MESSAGING_MAILS; a++)
						{
							mMailBodies[a]=mMailBodies[a+1];
							mMailHeaders[a]=mMailHeaders[a+1];
							mMailIDs[a]=mMailIDs[a+1];
						}

						//clear out the last one as its a copy of the penultimate one
						mMailIDs[mMailHeadersRetrieved]=0;
						mMailHeaders[mMailHeadersRetrieved].m_mailID=0;
						mMailBodies[mMailHeadersRetrieved].m_mailBody=NULL;
						mMailBodies[mMailHeadersRetrieved].m_mailID=0;
						mMailBodies[mMailHeadersRetrieved].m_size=0;
					}
					else if(mCurrentTaskType == MESSAGING_TASK_TYPE_SENDING_MESSAGE)
					{
						// close async task to release demonware resources.
						KillCurrentTask();

						LOGINFO( gMessagingMessageLogger,"Messaging - Message Sent\n");
					}

					// successful
					LOGINFO( gMessagingMessageLogger,"Messaging - Task Completed and Successful\n");
					return false;
				}
				else
				{
					// failed
					int e=mAsyncTask ? mAsyncTask->getErrorCode() : BD_HANDLE_TASK_FAILED;
					*error = e;

					// close async task to release demonware resources.
					KillCurrentTask();

					LOGINFO( gMessagingMessageLogger,"Messaging - Task Completed but Failed (%d)\n", e);
					return false;
				}
				break;

			default:
				{
					*error = BD_HANDLE_TASK_FAILED;
				}
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

void C_MessagingManager::KillCurrentTask()
{
	LOGINFO( gMessagingMessageLogger,"Messaging - Killing Any Current Task\n");

	mCurrentTaskType=MESSAGING_TASK_TYPE_NOTHING;
	
	NetPlay::DemonwareRemoteTask::Close(mAsyncTask);
}

}; // namespace Game
