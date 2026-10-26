#ifndef BIZ_NP_FRIEND_STORAGE
#define BIZ_NP_FRIEND_STORAGE

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/List.h>
#include <Btl/allocators/Pool.h>
#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/GameInvite.h>
#include <Network/Systemheaders.h>
#include <Network/Friend.h>
#include <Network/Presence.h>
#include "bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h"


#if defined (PLATFORM_PS3)
namespace SysProfile 
{	
	// this class will be used internally by the system profile manager to cache the players friends list while connected
	// to the PS3 network platform.
	class C_NpFriend : public Btl::ListElementDouble<C_NpFriend>
	{
	public:
		C_NpFriend()
		{Clear();}
		virtual ~C_NpFriend()
		{};

		void Clear() 
		{ 
			m_State = NetPlay::eFriendOffline; 
			m_GameInvite.m_Active = false;

			// default rich presence;
			NetPlay::RichPresence rp;
			m_RichPresence = rp;
		}
		void Destroy(void) {Clear();}		// this is called then the object is removed from the linked list, required for ListElementDouble to function.

		NetPlay::eFriendState	m_State;				// is friend playing our game
		BaseProfile				m_BaseProfile;			// holds name, NpId or bdOnlineUserId
		GameInvite				m_GameInvite;			// have we been sent an invite from this friend.
		NetPlay::RichPresence	m_RichPresence;			// rich presence info for this friend.
	};

	// to make things easier to read !!
	typedef Btl::List<C_NpFriend>						NpFriendsList;
	typedef Btl::StaticPool<C_NpFriend, MAX_FRIENDS>	NpFriendsPool;
}
#endif // PLATFORM_PS3

#endif //BIZ_NP_FRIEND_STORAGE
