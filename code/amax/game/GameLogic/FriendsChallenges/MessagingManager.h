#ifndef MESSAGING_MANAGER_H
#define MESSAGING_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/DemonwareTask.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

#include <bdLobby\bdMessaging\bdMessaging.h>
#include <bdLobby\bdMessaging\bdMailHeader.h>
#include <bdLobby\bdMessaging\bdMailBody.h>

namespace Game
{

enum MESSAGING_TASK_TYPE
{
	MESSAGING_TASK_TYPE_NOTHING,
	MESSAGING_TASK_TYPE_GETTING_HEADERS,
	MESSAGING_TASK_TYPE_GETTING_MESSAGES,

	MESSAGING_TASK_TYPE_SENDING_MESSAGE,

	MESSAGING_TASK_TYPE_DELETING_MESSAGE,

	MESSAGING_TASK_TYPE_MAX,
};

const int MAX_MESSAGING_MAILS=100;

class C_MessagingManager
{
public:
	
	C_MessagingManager()
	{
		mCurrentTaskType=MESSAGING_TASK_TYPE_NOTHING;
		mSilent=false;
		mNewMailSinceLastGet=false;

		for(int a=0;a<MAX_MESSAGING_MAILS;a++)
		{
			mMailBodies[a].m_mailBody=NULL;
		}
	};
	
	//per frame update function
	void Update();

	bool PumpCurrentTask(int *error);

	void KillCurrentTask();

	bool SendMessage(bdOnlineUserID ids[], int num_ids, void *message, int message_size);
	bool GetMail(bool silent=false);

	bool DeleteMail(int index);

	//return the instance
	static C_MessagingManager& Instance(){return s_instance;};

	U32 GetNumberOfMails(){return mMailHeadersRetrieved;};
	
	bdMailBody *GetMailBody(int index){return &mMailBodies[index];};
	bdMailHeader *GetMailHeader(int index){return &mMailHeaders[index];};

	bool MailStillOnServer(){return (mTotalMailHeadersOnServer > mMailHeadersRetrieved);};

	void NewMail(){mNewMailSinceLastGet=true;};
	bool IsThereNewMail(){return mNewMailSinceLastGet;};

	MESSAGING_TASK_TYPE GetCurrentTask()const{return mCurrentTaskType;}

private:
	
	static C_MessagingManager s_instance;
	
	bool GetMailMessages();

	MESSAGING_TASK_TYPE mCurrentTaskType;

	// used for tracking reads and writes
	bdRemoteTaskRef mAsyncTask;

	U32 mMailHeadersRetrieved;
	U32 mTotalMailHeadersOnServer;

	U32 mDeletingIndex;

	bool mSilent;

	bdMailHeader	mMailHeaders[MAX_MESSAGING_MAILS];
	bdMailBody		mMailBodies[MAX_MESSAGING_MAILS];
	bdUInt64		mMailIDs[MAX_MESSAGING_MAILS];

	bool mNewMailSinceLastGet;
};


} // namespace Game

#endif // LINK_CODE_MANAGER_H