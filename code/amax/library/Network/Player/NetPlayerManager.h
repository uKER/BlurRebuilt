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
#include "NetPlayerVoice.h"
#include "../Message/NetMessageReliableInBox.h"
#include "../Message/NetMessageReliableOutBox.h"
#include "../Router/NetRouterDefines.h"

namespace SysProfile { struct BaseProfile; }

namespace RacerNet
{
	// forward declerations.
	class C_NetRouterBase;
	class C_NetworkMessage;

	/// defines which system is in charge of defining which player get sent voice and actually sending it.
	typedef enum
	{
		eVoiceControlStateNone = 0,			/**< voice not currently active. */
		eVoiceControlStatePartyConnection,	/**< party session is responsible for voice transmission. */
		eVoiceControlStateRaceConnection,	/**< race session is responsible for voice transmission. */
		eVoiceControlStateGamePlay,			/**< game play in progress so that takes control of voice. */
	}eVoiceControlState;

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

		/// Flush internal data relating to a specific session.
		/** Any players that still exist on the specified session will not be deallocated, the 
		 *	game code must do that first.
		 *	@param partySession true to reference the party session or false to reference the race session data.
		 */
		void Flush(const bool partySession);

		/// Tests if any players on the specified connection are currently flagged as handshaking.
		/**	@param partySession true to reference the party session or false to reference the race session. . 
		 *	@return true if any player on the specified connection is still in the process of handshaking.
		 */
		bool AnyPlayersHandshaking(const bool partySession);


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

		/// Find the host player for a given session.
		/** @param partySession true if we wish to find the party host or false to find the race session host.
		 *	@return host player or NULL if no host was found for the required session.
		 */
		C_NetPlayer* HostPlayer(const bool partySession);

		/// Force a speific player to become the host.
		/**	Used in host migration when connection to the curent host is lost.
		 *	@param joinId the join id of the new player thst is taking over hosting responsibility.
		 *	@param partySession true if the host relates to the party session or false if its the party session.
		 */
		void PromoteToHost(const U8 joinId, const bool partySession);

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

		/// Can be used to parse an incoming player states message.
		/** @param msg network mesasge we have received.
		 *	@param senderId the joinId of the remote peer who sent us this message
		 *	@param partySession true if the mesasge relates to party players states otherwise false for race player states.
		 */
		void ReceivePlayerStatesMessage(C_NetMessage& msg, const U8 senderId, const bool partySession); 

		/// Sets the current voice control state.
		/** @param state requested new state.
		 */
		void SetVoiceControlState(const eVoiceControlState state);

		/// Get the current voice control state.
		eVoiceControlState GetVoiceControlState(void) const {return m_VoiceControlState;}

		/// Tests to see if we need to send any queued voice.
		/** @param maxTime the max time we need to wain inbetween sending voice updates.
		 *	@return true if the time since last voice send request is greater then maxTime.
		 */
		bool VoiceTransferRequired(const float maxTime);

		/// Sends any voice thet is queued up for remote players
		/** Should only be used when not in game play, when in gameplay we must tag the voice data onto outgoing game updates
		 *	to minimize bandwidth.
		 *	@param msg the network message we should use to send the voice data to peers.
		 *	@param router reference to our router to actually send the voice message.
		 */
		void SendBasicVoiceMessage(C_NetMessage& msg, C_NetRouterBase& router);

		/// Sends any voice that is queued to a specific remote player.
		/** @param msg the network message we should use to send the voice data to peers.
		 *	@param router reference to our router to actually send the voice message.
		 *	@param joinId the join identifier for the specific remote player
		 *	@param partySession true if we should use the party session connection or false if we should use the race connection.
		*/
		void SendBasicVoiceMessageToPlayer(C_NetMessage& msg, C_NetRouterBase& router, const U8 joinId, const bool partySession);

		/// Receive a basic voice message from a peer.
		/** @param msg network mesasge we have received.
		 *	@param senderId the joinId of the remote peer who sent us this message
		 *	@param partySession true if the mesasge relates to party players states otherwise false for race player states.
		 */
		void ReceiveBasicVoiceMessage(C_NetMessage& msg, const U8 senderId, const bool partySession); 

		/// Attempt to build a list of remote players that the specified local player has fully muted.
		/**	This list references the persistant mute list mainteined by the console, not the quick mute list that can be used
		 *	from the lobby.
		 *	@param localNetIndex index of the local player to reference, this is not a controller index.
		 *	@param out NetMuteListJoinReferences that will be configured based on what players in the specified session have been muted.
		 *	@param partySession true to check party session for muted players or false to check race session.
		 */
		void BuildMuteList(const U8 localNetIndex, NetMuteListJoinReferences& out, const bool partySession);

		/// Receive a mutelist from a remote peer.
		/**	If any local player is listed within this mutelist then we will update the communicator data so
		 *	the local player knows not to send voice data to that remote player anymore.
		 *	@param senderJoinId the join reference of the remote player who this mute list belongs to.
		 *	@param in the mute list data for the sender.
		 *	@param partySession true to check party session for muted players or false to check race session.
		 */
		void ReceiveMuteList(const U8 senderJoinId, const NetMuteListJoinReferences& in, const bool partySession);

		/// Host calls this when he has migrated his session and has issued the peers to start their migration.
		/**	Host will check the peers states to see when migration is complete for all players.
		 *	This is only used when migrating the matching session, since migrating the network session is quick.
		 *	@param partySession true to update the state for the party peers or false for the race session peers.
		 */
		void SetPeersToMigrationPhase(const bool partySession);

		/// Host calls this upon receiving a reply from a peer that they have completed migration.
		/** @param joinId the unique join id of the primary peer who has finished migration.
		 *	@param partySession true to update the state for the party peers or false for the race session peers.		
		 */
		void SetPeerMigrationComplete(const U8 joinId, const bool partySession);

		/// Host can check if any peers are still migrating.
		/** @see SetPeersToMigrationPhase()
		 *	This is only used when migrating the matching session, since migrating the network session is quick.
		 *	@param partySession true to update the state for the party peers or false for the race session peers.
		 *	@return true if peers on the spectifed session are still falgged as migrating.
		 */
		bool AnyPeersStillMigrating(const bool partySession);

		/// Called by the host if the peer migration is taking too long.
		/** Any peers that have yet to reply will be flagged for disconnection.
		 *	@param partySession true to update the state for the party peers or false for the race session peers.
		 */
		void PeerMigrationTimeout(const bool partySession);

		/// Local players can quickly mute / unmute remote players without going into the consoles operating system.
		/** This is a temporary mute as when the player leaves the mute info will be lost. Mostly for PS3-PC although it can be used on Xbox360.
		 *	@param controllerIndex the user index of the local player who initiated the mute / unmute request
		 *	@param remoteJoinReference the join reference of the remote player who we will be muting / unm uting.
		 *	@param partySession does the remoteJoinReference vcome from the party or race session.
		 *	@param toMute true if we wish to mute the remote player or false to unmute.
		 */
		void QuickMuteToggle(const U8 controllerIndex, const U8 remoteJoinReference, const bool partySession);

		/// What is the most restrictive nat type in use in the specified session.
		/**	@param partySession check the party session yes or no.
		 *	@return the nat type of the most restrictive player.
		 */
		eNatType GetMostRestrictiveNatType(const bool partySession);

		/// Compare the Nat type of the poining player aginst current peers.
		/** Game logic can ban a player fom attempting the join if it wishes.
		 *	@param partySession check the party session yes or no.
		 *	@param joiningNatType the nat type of the joining player.
		 *	@param out if the joining NAT is invalid this will hold the name of the player who its incompatiable with.
		 *	@return true if the joining nat is compatiable with the current peers
		 */
		bool CheckAgainstMostRestrictiveNat(const bool partySession, const U8 joiningNatType, SysProfile::BaseProfile& out);

		// check voice privilege for all connected profiles.
		bool CheckLowestVoicePrivileges();

	private:
		bool					m_Initialised;
		eVoiceControlState		m_VoiceControlState;
		float					m_VoiceSendTimer;
		U8						m_PartyJoinReferenceCounter;
		U8						m_RaceJoinReferenceCounter;

		NetPlayerList			m_PlayerList;
		NetPlayerPool			m_PlayerPool;

		NetMessageInBoxArray	m_InBoxes;
		NetMessageOutBoxArray	m_OutBoxes;

		NetVoiceArray			m_Voice;
	};

}// end namespace RacerNet

#endif // __NET_PLAYER_MANAGER_H__
