#ifndef NETFRIENDS_H
#define NETFRIENDS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netEnums/netEnums.h"
#include "netTime/netTime.h"

#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/GameInvite.h>
#include <SystemProfile/SystemProfileManager.h>
#include <Network/Friend.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

class netFriend: public NetPlay::FriendProfile
{
public:
};

BD_REFERENCE(netFriendEnumerator);
class netFriendEnumerator: public NetPlay::C_FriendsEnumerator, public bdReferencable
{
public:
};

BD_REFERENCE(netFriends);
class netFriends: public bdReferencable
{
public:
	enum
	{
		STATE_IDLE,
		STATE_ENUMERATING,
	};
	enum
	{
		STATUS_NOT_SET=-1,
		STATUS_ERROR,
		STATUS_ABORTED,
		STATUS_OK,
	};


	netFriends();
	~netFriends();

	bool enumerateFriends(const SysProfile::FullProfile& profile);
	void update();
	void clear();
	
	bool enumerating() const;
	bool started() const;
	bool success() const;
	bool failed() const;
	
	int getFriendsCount() const { return m_friendsCount; }
	netFriend* getFriend(int i);
	netFriend* findUser(const netUserId& id);


	int						m_status;
	int						m_state;
	netFriendEnumeratorRef	m_enumerator;
	netFriend				m_friends[NET_MAX_FRIENDS];
	int						m_friendsCount;
};

#endif //NETFRIENDS_H
