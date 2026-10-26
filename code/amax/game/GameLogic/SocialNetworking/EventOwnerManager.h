#ifndef EVENT_OWNER_MANAGER_H
#define EVENT_OWNER_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageHandler.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>	// [Blur2PC]
#include <ui/Util/WideString.h>	// [Blur2PC]

namespace Game
{

const int MAX_EVENT_OWNER_EVENTS=120;

typedef struct EventOwnerStats
{
	U32 mEventId;
	U32 mMedals;
	U32 mScore;

}EventOwnerStats;

enum EVENT_OWNER_RESULT
{
	EVENT_OWNER_RESULT_LOSING,
	EVENT_OWNER_RESULT_WINNING,

	EVENT_OWNER_RESULT_NO_SCORE_DRAW,
	EVENT_OWNER_RESULT_SCORE_DRAW,
};

class C_EventOwnerManager : public Messaging::C_MessageHandler
{
public:	
	
	void Init();
	void Update();

	void Clear();

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	static C_EventOwnerManager& Instance(){return s_instance;};

	void ReadFriendsStats();
	void WritePlayersStats();

	EVENT_OWNER_RESULT BeatingFriend(U32 sp_event_id);

	void SetFriend(bdOnlineUserID fr){mFriend=fr;ReadFriendsStats();};

	bdOnlineUserID GetFriend(){return mFriend;};

	const WCHAR *GetScoreString(U32 sp_event_id);

	bool ReadingRivalStats(){return (mReadingStats || mNeedToReadStats);};

	void OwnerStillFriendCheck();

private:
	static C_EventOwnerManager s_instance;

	UI::C_WideString64 mScoreString;


	bdOnlineUserID mFriend;

	Btl::Vector<EventOwnerStats> mFriendsStats;

	bool mNeedToWriteStats;
	
	bool mNeedToReadStats;
	bool mReadingStats;
};

}; // namespace Game

#endif	//EVENT_OWNER_MANAGER_H
