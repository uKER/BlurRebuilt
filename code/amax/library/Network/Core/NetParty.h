// ***************************************************************
//  NetParty   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	handles online party session processing.
// ***************************************************************

#ifndef __NET_PARTY_H__
#define __NET_PARTY_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <Systemprofile/SystemProfile.h>
#include <bdSocket/bdAddrHandle.h>
#include "../Misc/NetJoinErrors.h"
#include "../Misc/NetMigrationStates.h"
#include "NetJoinHelper.h"
#include "../Misc/NetQoS.h"
#include "../Misc/NetPartyNonce.h"

// forward declerations
namespace NetPlay { struct SessionSecurity; class C_Session; struct SessionCommandNotification; struct SlotConfig; }
namespace SysProfile {struct BaseProfile; }

namespace RacerNet
{
	// forward declerations
	class C_NetPlayerManager;
	class C_NetRouterBase;
	class C_NetMessage;
	struct DatagramReliablePacketHeader;
	struct DatagramUnReliablePacketHeader;
	class C_NetSessionKeyTracker;
	class C_NetCreationHelper;
	class C_NetPlayer;
	struct NetJoinRaceFromPartyData;

	/// main class for controlling the party network channel.
	class C_NetParty : public C_NetJoinHelper
	{
	public:

		// so we know when players have been added / removed from the matching server
		typedef enum
		{
			ePartyMatchingStatePlayersRemoved = 0,	// server command to remove players has completed.
			ePartyMatchingStatePlayersRemoving,		// async task to remove players has started.
			ePartyMatchingStatePlayersAdded,		// players have been added to the matching server.
		}ePartyMatchingState;

		typedef Btl::Vector<SysProfile::UserIdentifier, Btl::VectorParameters_Static<30> > MatchingPlayersPartyList;

		C_NetParty();
		virtual ~C_NetParty();

		/// Call once after creating object to initialise any dynamic resources.
		/**	@param ppRouter pointer to the socket router object address to use as transport.
		 *	@param ppPlayerManager pointer to the player manager object address.
		 *	@param ppPartyMatchmaking pointer to the matchmaking manager address. 
		 *	@return true if dynamic resources were allocated correctly.
		 */
		bool Initialise(C_NetRouterBase** ppRouter, C_NetPlayerManager **ppPlayerManager, NetPlay::C_Session** ppPartyMatchmaking);

		/// Have we allocated the dynamic resources by calling Initialise() yet.
		/** @return true if resources have been allocated.
		 */
		bool IsInitialised(void) const { return m_Initialised; }

		/// Is the network race session actually running.
		/** @return true if the race session is running or false if it's idle. 
 		 */
		bool IsRunning(void) const { return m_Running; }

		/// Only when no players have been added to the matching server is it deemed to be idle.
		/** xbox360 says a  player can only be in 1 presence session at a time so we need to remove them from this party
		 *	session before we can add them into the race session.
		 */
		bool IsMatchingServerIdle(void) const { return ePartyMatchingStatePlayersRemoved == m_MatchingPlayerCommandState ? true : false; }

		/// Has an error occurred that we can't recover from.
		bool BigError(void) const { return m_BigError; }

		/// Call when we no longer need any of the resources allocated.
		/** @see Initialise()
		 */
		void Shutdown();

		/// call once per frame to update tasks
		/** @param frameTime how long has passed since last called this function.
		 *	@param raceSessionRunning is the race session currently active or not.
		 */
		void Poll(const F32 frameTimeInSeconds, const bool raceSessionRunning);

		/// This totally resets the internal data to default.
		/** If we are conencted we will disconnect and destroy any online session(s) that may be active.
		 */
		void DisconnectAndClear(void);

		/// Attempt to create an Online session.
		/**	@param helper C_NetCreationHelper object that has local player info for an online game
		 *	@return true if the net session was created.
		 */
		bool CreateOnlineSession(C_NetCreationHelper& helper);

		/// Attempt to join an Online server.
		/**	@param helper C_NetCreationHelper object that has been created with for a lan game.
		 *	@param security the secure key data thet the host will have returned in the server data.
		 *	@param pHostCommonAddress the serialised common address of the host so we can generate his secure ip address.
		 *	@param hostConsoleId the console Id of the host, this will be returned in the sesion data.
		 *	@param hostName the name of the host who we are joining. this will come via the QoS data.
		 *	@return true if the join process was started or false if not.
		 */
		bool JoinOnlineSession(C_NetCreationHelper& helper, NetPlay::SessionSecurity& security, const U8* pHostCommonAddress, const U64 hostConsoleId, const SysProfile::BaseProfile& hostName);

		/// Call upon entry to the party lobby.
		/**	Modifies the local player data to enable voice and updates player states when returning from a race session.
		 */
		void EnterLobby(void);

		/// Party host can enable and disable matching Qos when entering / leaving the lobby
		/** When using demonware the QoS can only be applied to a single matching session, so as we transition
		 *	into the race session we must remove the Qos listener from the partys esison. When returning to the party lobby
		 * the host can re enable the matching Qos on this matchinhg session
		 */
		void EnableMatchingQoS(const bool enabled);

		/// Access to the join helper object.
		/** Make sure the object has been initialised before using this function.
		 * @see IsInitialised()
		 */
		C_NetJoinHelper& JoinHelper(void) {return *this; }

		/// receive a reliable network message on the party channel.
		/** @param msg received reliable message data.
		 *	@param msgType the type of message that the data relates to from ..\Misc\NetDatagramPacketTypes.h.
		 *	@param senderId the id of the player within the party channel who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveReliable(C_NetMessage& msg, const U8 msgType, const U8 senderId);

		/// receive an unreliable network message on the partsy channel.
		/** @param msg received message data.
		 *	@param msgType the type of message that the data relates to from ..\Misc\NetDatagramPacketTypes.h.
		 *	@param senderId the id of the player within the party channel who sent this message.
	 	 *	@param addr the network address of the peer who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveUnReliable(C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, bdAddrHandleRef& addr);

		/// Receive a join session request.
		/** @param msg received message data.
		 *	@param hdr the packet header that contains some of the remote players data.
		 *	@param addr the network address of the peer who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveJoinRequest(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, bdAddrHandleRef& addr);

		/// Receive a joijn failure response.
		/** @param msg received message data.
		 *	@param hdr the packet header that contains some of the remote players data.
		 *	@param addr the network address of the peer who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveJoinFailure(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, bdAddrHandleRef& addr);

		/// Receive a join session reply from the host.
		/** @param msg received message data.
		 *	@param hdr the packet header that contains some of the remote players data.
		 *	@param addr the network address of the host who sent this message.
		 *	@param senderId the join reference of the player who sent this reply.
		 *	@return true if the message was processed.
		 */
		bool ReceiveJoinReply(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr, bdAddrHandleRef& addr, const U8 senderId);

		/// Inform the session that this local player has modified their communicator mute data.
		/** Session is responsible for enabling / disabling voice comms because of mute functionality.
		 *	When a remote player is muted no voice should be sent to them to conserve bandwidth.
		 */
		void VoiceMuteChanged(void);

		/// Send a message to all connected peers containing the latest local player states relating to this session.
		void SendLocalPlayerStatesMessage(void);

		/// attempt to build the data we need to to enable rich presence join in progress to function.
		/**	Only used on PS3, PC since xbox360 handles this automatically.
		 *	@param sessionId matching session key if join in progress is available
		 *	@return true if this session should allow join in progress for friends.
		 */
		bool JoinInProgressAvailable(bdSecurityID& sessionId);

		/// attempt to build the data we need to send a game invite if possible.
		/** @param sessionSec returned session security that is sent as the invite. Only valid if the function returns true
		 *	@return true if we can actually invite players into the session or false if not
		 */
		bool CanInvite(NetPlay::SessionSecurity& sessionSec);

		/// Are we already in the session denoted by the supplied bdSecurityId
		/** Primarily used when accpting a game invite, as its no point leaving the session only to return later.
		 *	@param sessionId the unique session identifter to compare against.
		 *	@return true if we are already in the session or false if we are not.
		 */
		bool InSession(const bdSecurityID& sessionId);

		/// Used to determine if the party matching session is in a state where we can create our race session.
		/** On xbox360 players can only be in 1 presence session at a time, so when we transition from the party
		 *	to the race we remove the players from the party matching session so they can then go into the race matching
		 *	session. Removing the players from the matching session is asynchronous hence why we have to wait until its completed.
		 */
		bool MatchingSessionFullyDeActivated(void);

		/// Get the data structure that party peers use to join races sent from the party host.
		/** @param out structure to be filled in if the party is running.
		 *	@return true if the structure contains valid data on function exit, if the party is not active false will be returned and the structure should not be referenced.
		 */
		bool GetJoinRaceFromPartyData(NetJoinRaceFromPartyData& out);
			
		/// Overwrite the internal structure with the one supplied.
		/** Can be used when we wish yto clear the internal data.
		 */
		void SetJoinRaceFromPartyData(const NetJoinRaceFromPartyData& in);

		/// Has the party host requested we return to the party lobby.
		/** @param clear if true then we also clear the flag after returning it.
		 *	@return true if we should return to the lobby or false if not.
		 */
		bool ReceivedReturnToLobbyRequest(const bool clear);

		/// Attempt to send a correctly configured set of data to peers so they can join the specified race session.
		/** Only the party host should be calling this.
		 *	@param data correctly configured data relating to the session we wish the peers to join.
		 */
		void SendPeerJoinRaceSessionMessage(const NetJoinRaceFromPartyData& data);

		/// Attempt to send a message that instructs all party peers to return to the party lobby.
		/** Only The party host should be calling this function.
		 */
		void SendPeerReturnToLobbyMessage(void);

		/// Generates dlc settings based on the content that all players in the party have access to.
		/** For a content pack to be useble all playres in then party must have access to it. 
		 */
		U32 GenerateAvailableDlc(void);

		/// Send When we have inserted / removed the communicator.
		void SendCommunicatorStatusChangedMessage(void);

	private:
		C_NetParty(const C_NetParty& original);				// private copy constructor
		C_NetParty& operator = (const C_NetParty rhs);		// private un coded assignment operator

		// run disconnection logic
		void PollDisconnects(const F32 frameTimeInSeconds);

		// continue the join procedure if active.
		void PollJoin(void);

		// unregister security keys that are no longer in use.
		void PollSecurityKeyTracker(void);

		// update who we need to send voice to.
		void PollVoice(void);

		// send a join failed message to the return address
		void SendJoinFailed(const eNetJoinSessionErrors reason, const bdAddrHandleRef& returnAddr);

		// attempt to sent the reliable message to any peer that is fully connected. 
		void SendReliableMessage(C_NetMessage& msg, const U8 messageId);

		// creates local players as network players. 
		bool CreateLocalPlayers(C_NetCreationHelper& helper, const bool hostingSession);

		// registers any players that have voice ability that are not already registered.
		void SetupRemoteVoice(void);

		// if hosting this will set the Qos data onto the matchingsession
		void SetupQoS(void);

		// tracks what players need adding / removing from the matchmaking session
		void PollMatchingServer(const bool raceSessionRunning);

		// destroys all local and remote network players.
		void DestroyAllPlayers(void);

		// destroy a single network player.
		void DestroyPlayer(const U8 joinId, const bool showNotification = false);

		// removes players from out buffers so PollMatchingPlayers() can update the server with the correct info.
		void RemoveMatchingPlayer(const SysProfile::UserIdentifier& plr);

		// start the process of host migration, race session and online matchmaking session.
		void StartHostMigration(void);
		bool ReceiveMigrateMessage(C_NetMessage& msg, const U8 senderId);

		// tracks the migration of the matching server
		bool PollMatchingServerMigration(const F32 frameTimeInSeconds);

		// destroy all players at a given console id.
		void DestroyConsole(const U64& consoleId);

		// receive a message from a console who is leaving the session.
		bool ReceiveLeavingMessage(C_NetMessage& msg, const U8 senderId);

		// receive message from peer with updated voice mute data.
		bool ReceiveMuteChangeMessage(C_NetMessage& msg, const U8 senderId);
		
		// receive a player states change message from a peer.
		bool ReceivePlayerStatesMessage(C_NetMessage& msg, const U8 senderId);

		// receive a message from a peer with a request we disconnect the supplied console(s)
		bool ReceiveDisconnectConsolesMessage(C_NetMessage& msg, const U8 senderId);

		// receive the full join data message from the host.
		bool ReceiveJoinDataMessage(C_NetMessage& msg, const U8 senderId);

		// initial data messages are swapped when a new peer completes the join process.
		void SendInitialDataMessage(C_NetPlayer* pRecipient);
		bool ReceiveInitialDataMessage(C_NetMessage& msg, const U8 senderId);

		// hosts ends this when he wants peers to start their matching session migration 
		void SendMigrateMatchingSessionMessage(void);
		bool ReceiveMigrateMatchingSessionMessage(C_NetMessage& msg, const U8 senderId);

		// hosts ends this when he wants peers to start their matching session migration 
		void SendMigrateMatchingSessionReplyMessage(void);
		bool ReceiveMigrateMatchingSessionReplyMessage(C_NetMessage& msg, const U8 senderId);

		// build our initial join request message. 
		bool BuildJoinRequestMessage(C_NetMessage& msg, C_NetPlayer* pRecipient);

		// host wants us to try and join the supplied race session.
		bool ReceivePeerJoinRaceSessionMessage(C_NetMessage& msg, const U8 senderId);

		// host wants us to return to party lobby.
		bool ReceivePeerReturnToLobbyMessage(C_NetMessage& msg, const U8 senderId);

		// peer has changed communicator status
		bool ReceiveCommunicatorStatusChangedMessage(C_NetMessage& msg, const U8 senderId);

	private:
		bool m_Initialised;
		bool m_Running;
		bool m_BigError;
		bool m_VoiceEnabled;
		bool m_Migrating;
		bool m_ReturnToLobby;
		bool m_QoSEnabled;
		F32 m_DisconnectResendMsgTimer;
		NetQoSParty m_QoS;
		C_NetPlayerManager** m_ppPlayerMgr;
		C_NetRouterBase** m_ppRouter;
		NetPlay::C_Session** m_ppPartyMatchmaking;
		C_NetSessionKeyTracker* m_pSessionKeyTracker;
		MatchingPlayersPartyList m_MatchingToRemove;
		eNetMigrationStates	m_MatchingMigrationState;
		NetPlay::SessionCommandNotification* m_pMatchingMigrationNotification;
		U32 m_MatchingMigrationNotificationId;
		F32 m_MatchingMigrationTimer;
		ePartyMatchingState m_MatchingPlayerCommandState;
		NetPlay::SessionCommandNotification* m_pMatchingRemovePlayersNotification;
		U32 m_MatchingRemovePlayersNotificationId;
		NetJoinRaceFromPartyData*	m_pJoinRaceFromPartyData;
	};

}// end namespace RacerNet

#endif //__NET_PARTY_H__

