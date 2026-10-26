#ifndef FRIENDS_MANAGER_H
#define FRIENDS_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network\FriendEnumerator.h>
#include <misc_shared/Messaging/MessageHandler.h>

namespace Game
{

const int MAX_FRIEND_MANAGER_FRIENDS=101;

struct FriendManager_Friend
{
	SysProfile::BaseProfile mProfile;
	int mID;
};

class C_FriendsManager : public Messaging::C_MessageHandler
{
public:	
	
	void Init();
	void StartEnumeration();
	void Update();

	void Clear();
	bool IsEnumerating(){return mEnumerating;};

	static C_FriendsManager& Instance(){return s_instance;};

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	int GetNumFriends(){return mFriends.Size();};
	
	//this should only be used for loops etc, use ID if you want to find the actual friend
	SysProfile::BaseProfile *GetFriendProfileFromIndex(int index){return &mFriends[index].mProfile;};
	int GetFriendIDFromIndex(int index){return mFriends[index].mID;};
	
	SysProfile::BaseProfile *GetFriendProfileFromID(int id);
	SysProfile::BaseProfile *GetFriendProfileFromOnlineID(bdOnlineUserID id);

	void AddFriend(SysProfile::BaseProfile prof);

private:
	static C_FriendsManager s_instance;

	NetPlay::FriendProfile						mEnumeratedFriends[MAX_FRIEND_MANAGER_FRIENDS];

	NetPlay::C_FriendsEnumerator				mFriendsEnumerator;
	bool										mEnumerating;

	Btl::Vector<FriendManager_Friend>			mFriends;
	int											mCurrentID;

};

}; // namespace Game

#endif