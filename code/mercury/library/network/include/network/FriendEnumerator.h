// ***************************************************************
//  Friends   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Friends list enumeration.
// ***************************************************************

#ifndef FRIEND_ENUMERATOR_H
#define FRIEND_ENUMERATOR_H 

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/Friend.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdFriends/bdFriendInfo.h>
#include <Network/DemonwareTask.h>
#include <Network/SystemHeaders.h>

namespace NetPlay
{
	// forward declarations..
	struct	FullProfile;
	class	MT::Lock;
	class	C_AsyncTask;	 

	/// A wrapper for receiving friends data from the servers.
	class C_FriendsEnumerator
	{
	public:
		explicit C_FriendsEnumerator(); 
		virtual ~C_FriendsEnumerator(); 

		/// Create all dynamic objects, only needs to be called once prior to first use.
		/** @return true if all dynamic objects were created or false if error occurred.
		 */
		bool Initialise(void);

		/// Try and start asynchronous friend enumeration for a specific local player.
		/**	@see ContinueEnumeration().
		 *	@param inUser the local user who wants to enumerate his friends.
		 *	@param quickEnumeration if true then only online friends will be returned, if false then both online and offline
		 *	friends will be enumerated. Note : retrieval of offline friends is very slow on PS3 since the presence data can't be used.
		 *	It takes about 20ms to enumerate a single offline friend so this scales badly the more friends a user has.
		 *	@return true if successful or false if enumeration failed to start.
		 */
		bool StartEnumeration(const SysProfile::FullProfile& inUser, const bool quickEnumeration = true);

		/// Continue the asynchronous process of enumerating friends.
		/**	@see StartEnumeration().
		 *	@return true when enumeration finished.
		 */
		bool ContinueEnumeration(bool& errorOccured);

		/// forces any active friends enumeration to terminate.
		void StopEnumeration(void);

		/// Once enumerated we can extract the friends out so the game can use them. This function only extracts the basic info on a friend.
		/**	@param pOutput external buffer where the friends information will be copied to.
		 *	@param maxOutput number of friends structures pOutput is ca[able of holding.	
		 *	@param fullFriendsOnly if true then we only return friends who are not pending invite.
		 *	@return the number of friends copied to the output buffer. 
		 */
		U32 ExtractFriends(SysProfile::UserIdentifier* pOutput, const U32 maxOutput, const bool fullFriendsOnly = false);

		/// Once enumerated we can extract the friends out so the game can use them. This function extracts more information relating to each friend.
		/**	@param pOutput external buffer where the friends information will be copied to.
		 *	@param maxOutput number of friends structures pOutput is ca[able of holding.	
		 *	@param fullFriendsOnly if true then we only return friends who are not pending invite.
		 *	@return the number of friends copied to the output buffer.
		 */
		U32 ExtractFriends(FriendProfile* pOutput, const U32 maxOutput, const bool fullFriendsOnly = false);

	private:
		C_FriendsEnumerator(const C_FriendsEnumerator& original);				// private copy constructor
		C_FriendsEnumerator& operator = (const C_FriendsEnumerator rhs);		// private un coded assignment operator

		void Cleanup(void);

	private:
		bool					m_Initialised;
		bool					m_Enumerating;
		bool					m_QuickEnumeration;
		U32						m_NumFriendsEnumerated;
		S32						m_UserIndex;
#if defined(PLATFORM_PS3)
		bool					m_RequestStarted;
#elif defined(PLATFORM_PC)
		bdRemoteTaskRef		m_AsyncTask;
		bdFriendInfo		m_Friends[MAX_FRIENDS];
#endif
	};
}

#endif


