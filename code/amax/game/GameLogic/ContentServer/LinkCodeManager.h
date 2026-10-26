#ifndef LINK_CODE_MANAGER_H
#define LINK_CODE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/DemonwareTask.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

#include <bdLobby\bdLinkCode\bdLinkCode.h>

namespace Game
{

const int MAX_LINK_CODE_CATEGORIES=1;

enum LINK_CODE_TASK_TYPE
{
	LINK_CODE_TASK_TYPE_NOTHING,
	LINK_CODE_TASK_TYPE_GETTING,

	LINK_CODE_TASK_TYPE_MAX,
};

class C_LinkCodeManager
{
public:
	
	C_LinkCodeManager()
	{
		mCurrentTaskType=LINK_CODE_TASK_TYPE_NOTHING;
	};
	
	//per frame update function
	void Update();

	bool SetUserIndex(U8 index);

	void GetLinkCode();

	bool PumpCurrentTask(bool *error);

	void KillCurrentTask();

	//return the instance
	static C_LinkCodeManager& Instance(){return s_instance;};

private:
	
	static C_LinkCodeManager s_instance;
	
	//user information
	U8 mUserIndex;
	bdOnlineUserID mDemonWareUserID;

	LINK_CODE_TASK_TYPE mCurrentTaskType;

	// used for tracking reads and writes
	bdRemoteTaskRef mAsyncTask;

	bdLink mLink;
	bdUInt32 mCategories[MAX_LINK_CODE_CATEGORIES];
};


} // namespace Game

#endif // LINK_CODE_MANAGER_H