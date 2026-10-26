// ***************************************************************
//  NetRace   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	handles online and lan race session processing.
// ***************************************************************

#ifndef __NET_RACE_H__
#define __NET_RACE_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <Systemprofile/SystemProfile.h>
#include <Network/Nonce.h>

#include "../Misc/NetQoS.h"
#include <bdSocket/bdAddrHandle.h>
#include "../Misc/NetJoinErrors.h"
#include "../Misc/NetMigrationStates.h"

#include "NetJoinHelper.h"
#include "NetTeamBalancer.h"

// forward declerations
namespace NetPlay { struct SessionSecurity; class C_Session; struct SessionCommandNotification; struct SlotConfig; }
namespace SysProfile { struct BaseProfile; }

namespace RacerNet
{
	// forward declerations
	class C_NetPlayerManager;
	class C_NetRouterBase;
	class C_NetMessage;
	struct DatagramReliablePacketHeader;
	struct DatagramUnReliablePacketHeader;
	struct NetRaceSettings;
	struct NetRaceDlcSettings;
	class C_LanDiscoveryManager;
	class C_NetSessionKeyTracker;
	class C_NetCreationHelper;
	class C_NetPlayer;
	class C_NetRaceModeListener;
	struct NetMatchingServerData;
	struct NetJoinRaceFromPartyData;

	/// main class for controlling the race network channel.
	class C_NetRace : public C_NetJoinHelper
	{
	public:
		
		typedef Btl::Vector<SysProfile::UserIdentifier, Btl::VectorParameters_Static<30> > MatchingPlayersRaceList;
		typedef Btl::Vector<U8, Btl::VectorParameters_Static<20> > TeamPlayerList;

		C_NetRace();
		virtual ~C_NetRace();

		/// Call once after creating object to initialise any dynamic resources.
		/**	@param ppRouter pointer to the socket router object address to use as transport.
		 *	@param ppPlayerManager pointer to the player manager object address.
		 *	Pparam ppRaceMatchmaking pointer to the matchmaking manager address.  
		 *	@return true if drnamic resources were allocated correctly.
		*/
		bool Initialise(C_NetRouterBase** ppRouter, C_NetPlayerManager** ppPlayerManager, NetPlay::C_Session** ppRaceMatchmaking);

		/// Have we allocated the dynamic resources by calling Initialise() yet.
		/**	@return true if resources have been allocated.
		 */
		bool IsInitialised(void) const { return m_Initialised; }

		/// Is the network race session actually running.
		/**	@return true if the race session is running or false if it's idle. 
		 */
		bool IsRunning(void) const { return m_Running; }

		/// Is this network session online enabled.
		bool IsOnline(void) const { return m_OnlineMode; }

		/// Are we in the process of host migration.
		bool IsMigrating(void) const { return m_Migrating; }

		// this means that a new host is being picked, remote or local player
		bool IsSessionMigrating(void) const { return m_SessionIsMigrating; }

		/// Has an error occurred that we can't recover from.
		bool BigError(void) const { return m_BigError; }

		/// Call when we no longer need any of the resources allocated.
		/**	@see Initialise()
		 */
		void Shutdown();

		/// call once per frame to update tasks
		/**	@param frameTime how long has passed since last called this function.
		 */
		void Poll(const F32 frameTimeInSeconds);

		/// This totally resets the internal data to default.
		/**	If we are conencted we will disconnect and destroy any online session(s) that may be active.
		 */
		void DisconnectAndClear(void);

		/// Are voice services currently running on the race channel.
		/** Voice is only activated once a player enters the lobby.
		 *	@return true if the voice services are running so we can start evaluating where to send voice.
		 */
		bool VoiceServicesRunning(void) const { return m_VoiceEnabled; }

		/// Attempt to create a Lan server.
		/**	Game mode should have already been set from lua, race settings should have also be 
		 *	set in the current network game mode.
		 *	@param helper C_NetCreationHelper object that has local player info for a lan game.
		 *	@return true if the lan server was created successfully.
		 */
		bool CreateLanSession(C_NetCreationHelper& helper);

		/// Attempt to join a Lan server.
		/**	@param helper C_NetCreationHelper object that has been created with for a lan game.
		 *	@param security the secure key data thet the host will have returned in the server data.
		 *	@param pHostCommonAddress the serialised common address of the host so we can generate his secure ip address.
		 *	@param hostConsoleId the console Id of the host, this will be returned in the sesion data.
		 *	@return true if the join process was started or false if not.
		 */
		bool JoinLanSession(C_NetCreationHelper& helper, NetPlay::SessionSecurity& security, const U8* pHostCommonAddress, const U64 hostConsoleId);

		/// Attempt to create an Online session.
		/**	Game mode should have already been set from lua, race settings should have also be 
		 *	set in the current network game mode. Online matching session must be created and running
		 *	@param helper C_NetCreationHelper object that has local player info for an online game
		 *	@return true if the net session was created.
		 */
		bool CreateOnlineSession(C_NetCreationHelper& helper);

		/// Attempt to join an Online server.
		/**	@param helper C_NetCreationHelper object that has been created with for a lan game.
		 *	@param security the secure key data thet the host will have returned in the server data.
		 *	@param pHostCommonAddress the serialised common address of the host so we can generate his secure ip address.
		 *	@param hostConsoleId the console Id of the host, this will be returned in the sesion data.
		 *	@param hostName the name of the host who we are joioning, will come via the Qos Data
		 *	@return true if the join process was started or false if not.
		*/
		bool JoinOnlineSession(C_NetCreationHelper& helper, NetPlay::SessionSecurity& security, const U8* pHostCommonAddress, const U64 hostConsoleId, const SysProfile::BaseProfile& hostName);

		/// Call upon entry to the race lobby.
		/**	Modifies the local player data to enable voice and updates player states when returning from a race.
		 */
		void EnterLobby(void);

		/// Access to the lan discovery manager;
		/**	Make sure the object has been initialised before using this function.
		 *	@see IsInitialised()
		 */
		C_LanDiscoveryManager& LanDiscoveryManager(void) {return *m_pLanDiscoveryMgr;}

		/// Access to the join helper object.
		/**	Make sure the object has been initialised before using this function.
		 *	@see IsInitialised()
		 */
		C_NetJoinHelper& JoinHelper(void) {return *this;}

		/// receive a reliable network message on the race channel.
		/**	@param msg received reliable message data.
		 *	@param msgType the type of message that the data relates to from ..\Misc\NetDatagramPacketTypes.h.
		 *	@param senderId the id of the player within the race channel who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveReliable(C_NetMessage& msg, const U8 msgType, const U8 senderId);

		/// receive an unreliable network message on the race channel.
		/**	@param msg received message data.
		 *	@param msgType the type of message that the data relates to from ..\Misc\NetDatagramPacketTypes.h.
		 *	@param senderId the id of the player within the race channel who sent this message.
		 *	@param addr the network address of the peer who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveUnReliable(C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, bdAddrHandleRef& addr);

		/// Receive a join session request.
		/**	@param msg received message data.
		 *	@param hdr the packet header that contains some of the remote players data.
		 *	@param addr the network address of the peer who sent this message.
		 *	@return true if the message was processed.
		*/
		bool ReceiveJoinRequest(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, bdAddrHandleRef& addr);

		/// Receive a joijn failure response.
		/**	@param msg received message data.
		 *	@param hdr the packet header that contains some of the remote players data.
		 *	@param addr the network address of the peer who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveJoinFailure(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, bdAddrHandleRef& addr);

		/// Receive a join session reply from the host.
		/**	@param msg received message data.
		 *	@param hdr the packet header that contains some of the remote players data.
		 *	@param addr the network address of the host who sent this message.
		 *	@param senderId the join reference of the player who sent this reply message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveJoinReply(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr,  bdAddrHandleRef& addr, const U8 senderId);

		/// Inform the session that this local player has modified their communicator mute data.
		/**	Session is responsible for enabling / disabling voice comms because of mute functionality.
		 *	When a remote player is muted no voice should be sent to them to conserve bandwidth.
		 */
		void VoiceMuteChanged(void);

		/// Send a message to all connected peers containing the latest local player states relating to this session.
		void SendLocalPlayerStatesMessage(void);

		///	Send new race settings to all peers
		void SendNewRaceDataMessage(void);

		/// Parse the connected peer data and build up what content can be used and what content is in use.
		/** @param content will be written into internally based on peer data. 
		 */
		void GenerateDlcSettings(NetRaceDlcSettings& content);

		/// Validate a vehicle against the current race settings and dlc settings
		/**	@param xtVehicleId sorcery if of the vehicle to test.
		 *	@return true if the vehicle is valid, false if not.
		 */
		bool ValidateVehicle(const U32 xtVehicleId);

		/// Check for any players who are still in the state of loading the race, and disconnect them from our game.
		/** Only called if the time we've been waiting for other players to be ready exceeds some limit
		 */
		void DisconnectIdleLoadingPlayers(void);

		/// Attempt to register a lisener so the race logic gets informed of certain actions.
		/** @param pListener pointer to a class that has been derrived from the listener.
		 */
		void RegisterRaceLogicListener(C_NetRaceModeListener* pListener);

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

		/// set if we are in the race or in the ui.
		/** used for small things like Qos.  
		 *	@param flag true if we whink we are in a race or false if not.
		 */
		void FlagInRace(const bool flag) {m_InRace = (m_Running == true) ? flag : false;}

		/// Call externally to force an update of the matching server data.
		/** Any players that are flagged to be added or removed will be sent to the server
		 *	@param forceParamChange if the game settings have changed in anyway then set this to true to
		 *	update the matching parameters on the server.
		 */
		void UpdateMatchingServer(const bool forceParamChange) { PollMatchingServer(forceParamChange, 1.0f/30.0f); }

		/// build ste data structure thats required for party peers to join this race session.
		/** @param out upon exit this should contain all the data requied for a player to join this session without 
		 *	going through a matching query.
		 *	@param partyHostConsoleId the console Id of the party host who is to monitor the join attempt.
		 *	@return true if the data within the ouitput structure should be considered valid.
		 */
		bool BuildPartyPeerJoinSessionData(NetJoinRaceFromPartyData& out, const U64& partyHostConsoleId);

		/// Called at the end of every event afte the results have been calculated.
		/** This is used to transfer any updated player profile data around to peers. 
		 */
		void PostRace(void);

		/// Send When we have inserted / removed the communicator.
		void SendCommunicatorStatusChangedMessage(void);

		/// Returns the team data helper
		C_NetTeamBalancer& GetTeamBalancer(void) { return m_TeamBalancer; }

		// Force everybody in the race to timeout (Instant finish race)
		void ForceRaceTimeout();

		// Returns true if all active players are currently in the lobby
		bool PlayersInLobby(void);

	private:
		C_NetRace(const C_NetRace& original);				// private copy constructor
		C_NetRace& operator = (const C_NetRace rhs);		// private un coded assignment operator
		
		// run disconnection logic
		void PollDisconnects(const F32 frameTimeInSeconds);

		// continue the join procedure if active.
		void PollJoin(void);

		// unregister security keys that are no longer in use.
		void PollSecurityKeyTracker(void);

		// update who we need to send voice to.
		void PollVoice(void);

		// update and balance the teams
		void PollTeamBalancing(void);

		// attempt to sent the reliable message to any peer that is fully connected. 
		void SendReliableMessage(C_NetMessage& msg, const U8 messageId);

		// creates local players as network players. 
		bool CreateLocalPlayers(C_NetCreationHelper& helper, const bool hostingSession);

		// advertises the game on the lan.
		void AdvertiseLanGame(void);

		// get the current security information for the network game
		bool GetSecurity(NetPlay::SessionSecurity& security);

		// gets the current race settings from the active game mode.
		void GetCurrentRaceSettings(NetRaceSettings& current);

		// updates the game mode controller with the latest race settings
		void SetCurrentRaceSettings(NetRaceSettings& current);

		// destroy all players at a given console id.
		void DestroyConsole(const U64& consoleId);

		// destroys all local and remote network players.
		void DestroyAllPlayers(void);

		// destroy a single network player.
		void DestroyPlayer(const U8 joinId, const bool showNotification = false);

		// build our initial join request message. 
		bool BuildJoinRequestMessage(C_NetMessage& msg, C_NetPlayer* pRecipient);

		// receive the full join data message from the host.
		bool ReceiveJoinDataMessage(C_NetMessage& msg, const U8 senderId);

		//	receive new race settings from the host.
		bool ReceiveNewRaceDataMessage(C_NetMessage& msg, const U8 senderId);

		// initial data messages are swapped when a new peer completes the join process.
		void SendInitialDataMessage(C_NetPlayer* pRecipient);
		bool ReceiveInitialDataMessage(C_NetMessage& msg, const U8 senderId);

		// receive a player states change message from a peer.
		bool ReceivePlayerStatesMessage(C_NetMessage& msg, const U8 senderId);

		// receive a message from a console who is leaving the session.
		bool ReceiveLeavingMessage(C_NetMessage& msg, const U8 senderId);

		// receive a message from a peer with a request we disconnect the supplied console(s)
		bool ReceiveDisconnectConsolesMessage(C_NetMessage& msg, const U8 senderId);

		// receive message from peer with updated voice mute data.
		bool ReceiveMuteChangeMessage(C_NetMessage& msg, const U8 senderId);

		// party of the session migration process.
		void CheckForNewHost(void);

		// start the process of host migration, race session and online matchmaking session.
		void StartHostMigration(void);

		// receive a message from a peer with a request we promote him to host.
		bool ReceiveMigrateMessage(C_NetMessage& msg, const U8 senderId);

		// registers any players that have voice ability that are not already registered.
		void SetupRemoteVoice(void);

		// updates the QoS data (if modified) that is tagged on to our online matching queries.
		void PollMatchingQoS(void);

		// tracks what players need adding / removing from the matchmaking session
		void PollMatchingServer(const bool forceParamChange, const F32 frameTimeinSeconds);
		
		// removes players from out buffers so PollMatchingPlayers() can update the server with the correct info.
		void RemoveMatchingPlayer(const SysProfile::UserIdentifier& plr);

		// tracks the migration of the matching server
		bool PollMatchingServerMigration(const F32 frameTimeInSeconds);

		// hosts ends this when he wants peers to start their matching session migration 
		void SendMigrateMatchingSessionMessage(void);

		// peer receives this when host wants him to migrate his matching session
		bool ReceiveMigrateMatchingSessionMessage(C_NetMessage& msg, const U8 senderId);

		// hosts ends this when he wants peers to start their matching session migration 
		void SendMigrateMatchingSessionReplyMessage(void);

		// host receives this when peer has finished migrating their matching session.
		bool ReceiveMigrateMatchingSessionReplyMessage(C_NetMessage& msg, const U8 senderId);

		// generates the matching server parameters from the current game settings.
		void BuildMatchingServerData(const NetRaceSettings& settings, NetMatchingServerData& outData, const U8 numPlayersInSession);

		// whilst racing in a championship are joins still allwoed
		bool CheckIfChampionshipJoinsAllowed(void);

		// peer has changed communicator status
		bool ReceiveCommunicatorStatusChangedMessage(C_NetMessage& msg, const U8 senderId);

		// we are in race.
		bool InRace(void) const { return m_InRace; }
		
	private:
		bool m_Initialised;
		bool m_OnlineMode;
		bool m_Running;
		bool m_Migrating;
		bool m_SessionIsMigrating;
		bool m_VoiceEnabled;
		bool m_InRace;
		bool m_BigError;
		bool m_ForceMatchingServerModify;
		bool m_NewDisconnects;
		S32	m_LocalMatchingSessionManagerIndex;
		F32 m_DisconnectResendMsgTimer;
		NetQoSRace m_QoS;
		C_NetPlayerManager** m_ppPlayerMgr;
		C_NetRouterBase** m_ppRouter;
		NetPlay::C_Session** m_ppRaceMatchmaking;
		C_LanDiscoveryManager* m_pLanDiscoveryMgr;
		C_NetSessionKeyTracker* m_pSessionKeyTracker;
		NetPlay::SessionSecurity* m_pLanSecurity;
		C_NetRaceModeListener* m_pRaceModeListener;
		MatchingPlayersRaceList m_MatchingToRemove;
		NetPlay::SessionCommandNotification* m_pMatchingMigrationNotification;
		U32 m_MatchingMigrationNotificationId;
		eNetMigrationStates	m_MatchingMigrationState;
		F32 m_MatchingMigrationTimer;
		F32 m_MatchingInitialDelayTimer;

		bool				m_BalanceTeams;
		C_NetTeamBalancer	m_TeamBalancer;
	};

}// end namespace RacerNet

#endif //__NET_RACE_H__
