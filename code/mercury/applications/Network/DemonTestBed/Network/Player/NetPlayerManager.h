// ***************************************************************
//  NetPlayerManager   version:  1.0   ·  date: 04/14/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	tracking of network players in all sessions.
// ***************************************************************

#ifndef __NET_PLAYER_MANAGER_H__
#define __NET_PLAYER_MANAGER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "../Misc/NetDefines.h"
#include "NetPlayer.h"
#include "../Message/NetMessageReliableInBox.h"
#include "../Message/NetMessageReliableOutBox.h"

namespace RacerNet
{
	// forward declerations.
	class C_NetRouterBase;

	/// Used for tracking local and remote network players.
	class C_NetPlayerManager
	{
	public:

		/// different possibilities for the session host.
		typedef enum
		{
			eHostCheckStateNoneFound = 0,	/**< no host has been found. */
			eHostCheckStateLocal,			/**< local Player is flagged as the host. */
			eHostCheckStateRemote,			/**< remote player is flagged as the host. */
		}eHostCheckState;

		C_NetPlayerManager();
		virtual ~C_NetPlayerManager();

		/// Call once to initialise the internal data.
		bool Initialise(void);

		/// Allocate a network player structure.
		/** @param requiresMessageQueue true if the player requires reliable message boxes. Only remote primary players actually require these.
		 *	@return pointer to allocated player or null if allocation failed.
		 */
		C_NetPlayer* AllocatePlayer(bool requiresMessageQueue);

		/// Destroy a network players connection.
		/** This wil close any party or race connections the player may have and release the player object for reallocation.
		 *	@param pPlr pointer to the network player to be deallocated.
		 */
		void DeallocatePlayer(C_NetPlayer *pPlr);

		/// Call once per frame to pump the network message queues.
		/** This function is also responsible for sending out keepalive packets to remote peers.
		 *	@param frameTimeInSeconds how many seconds habe passed since the last update.
		 *	@param router if we need to send a keepalive then we will use this to do so.
		 */
		void Poll(const float frameTimeInSeconds, C_NetRouterBase& router);

		/// Call when we no longer need the internal data.
		/** @see Initialise()
		 */
		void Shutdown(void);

		/// get the join id of the local primary player for specified session.
		/** This is mainly used when sending / replying to reliable messages since they are only ever sent by / to the primary players on each 
		 *	console.
		 *	@param partySession true if we wish to get the join id for the party session or false to get the race session id.
		 *	@return join id of the local primary player.
		 */
		U8 LocalPrimaryJoinId(bool partySession) const;

		/// Check to make sure the local primary join id has been assigned.
		/** It may be possible to get join requests from other peers before we actually hear back from the host with our join approved
		 *	message so we must make sure we have a valid join id before replying to these messages. If we don't yet have our join id then 
		 *	we just ignore the message as the peer will resend it after a short amount of time, hopefully by then we will have received our join id from thre host.
		 *	@param partySession true if we wish to query the local join Ids of the party session or false to query the race session.
		 *	@return true if the local players join ids have been assigned.
		 */
		bool ValidateLocalJoinId(const bool partySession) const;

		/// Check the availability of the host within the specified session.
		/** Only tests to see if their is a player flagged as being the host within the specified session.
		 *	@param partySession true if we wish to test the party session or false if we wish to test the race session.
		 *	@return a state from eHostCheckState that defines what the host state is.
		 */
		eHostCheckState CheckHost(const bool partySession) const;

		/// Is the specified player the session host.
		/** Tests the specified session to see if the supplied player id is flagged as being the current host.
		 *	@param hostJoinId specified player id to test for host.
		 *	@param partySession true if we wish to test the party session or false if we wish to test the race session.
		 *	@return true if the supplied player is the host or false if now.
		 */
		bool ValidateHost(const U8 hostJoinId, const bool partySession) const;

		/// generate a new join reference for a specified session.
		/** this will be unique for all players in the specified session.
		 *	@param partySession true if we wish to create the reference for the party session or false to create a race one.
		 */
		U8 GenerateJoinReference(const bool partySession);

		/// How many players are currently active in the specified session.
		/** Don't use this with the Index() function, this should only be used to ascertain the number of players connected to
		 *	a specific session.
		 *	@return Number of network players currently being tracked in the specified session.
		 */
		U8	Size(const bool partySession) const;

		/// How many players are currently active in any session.
		/** @return Number of network players currently being tracked in both sessions.
		 */
		U8	TotalSize(void) const {return m_PlayerList.Size();}

		/// How many local players are currently active.
		/**	@return Number of local players currently being tracked.
		 */
		U8 LocalSize(void) const;

		/// Get access to a network player at a specific index.
		/**	@see TotalSize()
		 *	@param index index into out player linked list.
		 *	@return pointer to required player or NULL is player was not found.
		 */
		C_NetPlayer* Index(const U8 index);

		/// Attempt to find a specfic player.
		/** @param joinId the unique join identifier assigned by the host.
		 *	@param partySession true if we wish th search the party player or false to search the race players.
		 *	the player could have different joinIds for the party and race session.
		 *	@return pointer to required player or NULL is player was not found.
		 */
		C_NetPlayer* FindFromJoinId(const U8 joinId, const bool partySession);

		/// Attempt to find a specfic player.
		/** @param consoleId the specific console identifier where this player resides.
		 *	@param index the index of the player on the console.
		 *	the player could have different joinIds for the party and race session.
		 *	@return pointer to required player or NULL is player was not found.
		 */
		C_NetPlayer* FindFromConsoleId(const U64 consoleId, const U8 index);

		/// Attempt to find a local player.
		/** @param controllerindex the controller port of the local player.
		 *	the player could have different joinIds for the party and race session.
		 *	@return pointer to required player or NULL is player was not found.
		 */
		C_NetPlayer* FindLocal(const U8 controllerIndex);

		/// Check to see if a specific console exists in a specific session.
		/** @param consoleId the identifier of the remote console to compare against.
		 *	@return true if the specified console is already added into our player list.
		 */
		bool ConsoleExists(const U64 consoleId) const;

		/// update the keep alive data for the players on the same console as the supplied player.
		/** @param joinId remote player who we have received a packet from.
		 *	@param partySession did we receive this packet on the party session or the race session.
		 *	@param latestAddress the latest network that we have received data from for the specified player.
		 */
		void UpdateKeepAliveForPlayer(const U8 joinId, const bool partySession, bdAddrHandleRef& latestAddress);

	private:
		bool					m_Initialised;
		U8						m_PartyJoinReferenceCounter;
		U8						m_RaceJoinReferenceCounter;

		NetPlayerList			m_PlayerList;
		NetPlayerPool			m_PlayerPool;

		NetMessageInBoxArray	m_InBoxes;
		NetMessageOutBoxArray	m_OutBoxes;
	};

}// end namespace RacerNet

#endif // __NET_PLAYER_MANAGER_H__
