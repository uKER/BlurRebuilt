// ***************************************************************
//  Friends   version:  1.0   ·  date: 02/05/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Friends structures and defines used when enumerating
//			and adding/removing friends.
// ***************************************************************

#ifndef __FRIEND_H__
#define __FRIEND_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Systemprofile/SystemProfile.h>
#include <Systemprofile/GameInvite.h>
#include <Network/Presence.h>

namespace NetPlay
{
	/// enum that describes all the states a friend can be in.
	typedef enum
	{
		eFriendOffline = 0,						/**< can't have any interaction with friend. */
		eFriendOnlineButPlayingAnotherGame,		/**< on ps3 we can't invite this friend as they are in a different context. */
		eFriendOnline,							/**< can send invites to friend. */
		eFriendRequestReceived,					/**< this player wants to be your friend. you will need to reply back to accept or decline the request. PC only. */
		eFriendRequestSent,						/**< you have sent this player a friend request that has yet to be accepted or declined. PC only.*/
	}eFriendState;

	/// Structure to hold the detailed information about an individual friend.
	/** This is only really valid on PS3 / PC where we need to render our own friends list.
	 *	If used on Xbox360 then the game invite / presence field(s) won't be valid since these come through the 
	 *	systemprofile manager via the guide. 
	 */
	struct FriendProfile
	{
		FriendProfile() : m_CanInvite(false), m_FriendState(eFriendOffline)
		{
			m_GameInvite.m_Active = false;
			NetPlay::RichPresence p;
			m_RichPresence = p;
		}
		bool					m_CanInvite;
		eFriendState			m_FriendState;
		SysProfile::BaseProfile	m_Profile;
		SysProfile::GameInvite	m_GameInvite;
		NetPlay::RichPresence	m_RichPresence;
	};

	/// Structure to hold the detailed information about a player who you wish to be on your friends list or have received a request from this player..
	/** This is only really valid on PC where we need to manage friends requests manually, on the other platforms friend management is all done 
	 *	through the system software.
	 */
	struct PendingFriendProfile 
	{
		PendingFriendProfile() : m_FriendState(eFriendOffline)
		{}
		eFriendState			m_FriendState;
		SysProfile::BaseProfile	m_Profile;
	};
}

#endif


