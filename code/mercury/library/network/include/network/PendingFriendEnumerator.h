// ***************************************************************
//  Friends   version:  1.0   ·  date: 02/05/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Enumerate friends who have been sent / received a friend invite
//			but have yet to accept it. This is PC only.
// ***************************************************************

#ifndef __PENDING_FRIEND_ENUMERATOR_H__
#define __PENDING_FRIEND_ENUMERATOR_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/Friend.h>
#include <Network/SystemHeaders.h>
#include <bdLobby/bdFriends/bdFriendProposal.h>
#include <Network/DemonwareTask.h>

namespace NetPlay
{
	// forward declerations
	class C_DemonwareTask;

	/// Lists friends who have been sent invites but have yet to accept them.
	/** A player will be listed in here for 2 reasons. 
	 *	1: you have sent them a friends request but they are yet to accept / decline it
	 *	2: you have received a friends request but have yet to accept / decline it.
	 *	This is only valid ion PC where we have to manage friends in the game, on PS3 and Xbox360 friends management is
	 *	all done through the system software.
	 */
	class C_PendingFriendsEnumerator
	{
	public:
		explicit C_PendingFriendsEnumerator(); 
		virtual ~C_PendingFriendsEnumerator(); 

		/// describes the 2 different types of pending friend enumeration
		typedef enum
		{
			eModeNoneSet = 0,		/**< no mode is currently set. */
			eModeInviteSent,		/**< enumerate pending friends who I have sent an invite to. */
			eModeInviteReceived,	/**< enumerate pending friends who i have received an invite from.*/
		}ePendingEnumerateMode;

		/// Create all dynamic objects, only needs to be called once prior to first use.
		/** @return true if all dynamic objects were created or false if error occurred.
		 */
		bool Initialise(void);

		/// Try and start asynchronous friend enumeration for a specific local player.
		/**	@see ContinueTask().
		 *	@param inUser local player who wants to enumerate his pending friends.
		 *	@param mode which mode to retrieve pending friends requests from.
		 *	@return true if successful or false if enumeration failed to start.
		 */
		bool StartEnumeration(const SysProfile::FullProfile& inUser, const ePendingEnumerateMode mode);

		/// Start the process of revoking a friend request we have already sent out.
		/** @see ContinueTask().
		 *	If we have sent a player a friends request message we can revoke the request if the recipient has not
		 *	yet accepted our friends request. 
		 *	For this function to work correctly we must have enumerated our prnding friends with the mode set to eModeInviteSent.
		 *	@param toRevoke pending friends profile to revoke.
		 *	@return true if successful or false if revoke failed to start.
		 */
		bool StartRevokeFriendRequest(const PendingFriendProfile& toRevoke);

		/// Start the process of answering a friend request send from another player
		/** @see ContinueTask().
		 *	If we have received a request to join a players friends list then we can accept or decline it. 
		 *	For this function to work correctly we must have enumerated our prnding friends with the mode set to eModeInviteReceived.
		 *	If we accept the invite then this pending friend will move on to your full friends list @see C_FriendEnumerator.
		 *	@param toReply pending friends profile to answer.
		 *	@param accept true if we wish toa ccept the invite or false to decline it.
		 *	@return true if successful or false if answer failed to start.
		 */
		bool StartAnswerFriendRequest(const PendingFriendProfile& toReply, const bool accept);

		/// Continue the asynchronous process of enumerating / replying / revoking pending friend messages.
		/**	@see StartEnumeration().
		 *	@return true when the asynchronous task has finished. check errorOccurred after this.
		 */
		bool ContinueTask(bool& errorOccurred);

		/// forces any active friends enumeration or reply to terminate.
		/** flushes any enumerated friends internally, once called its not possible to reply to a friend request.
		 *	if this is called while replying to a pending friends request then it's possible that that mesage will be lost.
		 */ 
		void Stop(void);

		/// Check to see if there is an asynchronous task in progress.
		/** It's only possible to start 1 asynchronous task.
		 *	@return true if an asynchronous task is curreently active.
		 */
		bool IsBusy(void) const;

		/// Once enumerated we can extract the pending friends out so the game can use them. 
		/**	@param pOutput external buffer where the pending friends information will be copied to.
		*	@param maxOutput number of friends structures pOutput is ca[able of holding.	
		*	@return the number of pending friends copied to the output buffer.
		*/
		U32 ExtractPendingFriends(PendingFriendProfile* pOutput, const U32 maxOutput);

	private:
		typedef enum
		{
			eStatusIdle = 0,		// doing nothing.
			eStatusEnumerating,		// enumerating some pending friends.
			eStatusRevoking,		// revoking a friend invite that we have sent.
			eStatusReplying,		// replying to an invite that we have been sent.
		}eStatus;

		C_PendingFriendsEnumerator(const C_PendingFriendsEnumerator& original);				// private copy constructor
		C_PendingFriendsEnumerator& operator = (const C_PendingFriendsEnumerator rhs);		// private un coded assignment operator
		void Cleanup(void);

	private:
		bool m_Initialised;
		ePendingEnumerateMode m_Mode;

#if defined(PLATFORM_PC)
		bdRemoteTaskRef	m_AsyncTask;
		eStatus m_Status;
		U32 m_NumPendingFriends;
		bdFriendProposal m_PendingFriends[MAX_FRIENDS];
		U8 m_UserIndex;
#endif
	};
}

#endif


