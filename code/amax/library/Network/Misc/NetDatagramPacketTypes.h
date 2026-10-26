// ***************************************************************
//  NetDatagramPacketTypes   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Lists all the possible datagram packet types we will be 
//			sending.
// ***************************************************************
 

#ifndef __NET_DATAGRAM_PACKET_TYPES_H__
#define __NET_DATAGRAM_PACKET_TYPES_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// Base enum for packet types that will be sent internally or all games will require.
	/** If this is changed make sure you change the network packet version in NetVersion.h
	 *	to make all previous compiles incompatiable with the new code.
	 */
	enum
	{
		eNetMessage_ReliableAcknowlegement = 1,			/**< acknowledgement to receiving a reliable message. */	
		eNetMessage_KeepAlive,							/**< keepalive packet received. */
				
		eNetMessage_JoinRequest,						/**< received request to join the session, only host should get this. */
		eNetMessage_JoinReply,							/**< peer sends this back in response to receiving eNetMessage_JoinRequest. */
		eNetMessage_JoinData,							/**< host sends this back in response to receiving eNetMessage_JoinRequest. */
		eNetMessage_JoinFailure,						/**< host / peer sends this back in response to receiving eNetMessage_JoinRequest. */
		eNetMessage_InitialData,						/**< when a peer successfully joins a session he sends one of these to all other peers, they reply with their data. */
		eNetMessage_NewRaceData,						/**< used in a custom race lobby when the host has changed the race settings and needs to tell everybody else. */
	
		eNetMessage_Leaving,							/**< send by a peer when they leave the session, goes to all other peers. */			
		eNetMessage_DisconnectConsoles,					/**< sent to the host as a request to disconnect or sent by the host as a command to disconnect the specified console(s) */			
	
		eNetMessage_Migrate,							/**< sent by a new host when he wishes the network session to migrate the hosting to himself. */			
		eNetMessage_MigrateMatchingSession,				/**< sent by a new host when the online matchingmaking session is ready to be migrated. */			
		eNetMessage_MigrateMatchingSessionReply,		/**< sent by a peer back to the host when they have migrated their matchmaking session session. */

		eNetMessage_PlayerStates,						/**< list of player states. */
		eNetMessage_VehicleData,						/**< vehicle data for a number of peers. */
		eNetMessage_StartRace,							/**< Start race. */
		eNetMessage_AwardsData,							/**< MP Awards data for this race. */
		eNetMessage_VoteToSkipNextRace,					/**< We weould like to skip the next race. */
		eNetMessage_VoteOnNextRace,						/**< We have voted on a particular race. */
		eNetMessage_VoteResults,						/**< Results from the voting is passed around. */

		eNetMessage_BasicVoice,							/**< Basic unreliable packet that contsins nothing but voice data. */	
		eNetMessage_MuteChange,							/**< Latest voice mute data from a remote peer. */	

		eNetMessage_BasicVehicleData,					/**< Vehicle control data */
		eNetMessage_VehicleFinished,					/**< Vehicle has finished the race */

		eNetMessage_PartyPeerJoinRace,					/**< Party host wants a peer to join a specific race session. */
		eNetMessage_PartyPeerReturnToLobby,				/**< Party host wants a peer to return to the party lobby. */

		eNetMessage_LobbyWaitingRaceInfo,				/**< Race status info sent to players currently waiting in lobby during race-in-progress. */

		eNetMessage_PostRace,							/**< Data that is sent after every race event */

		eNetMessage_LobbyUpdate,						/**< Host sends out when lobby countdown is enabled and how long is remaining */
		eNetMessage_BeginVoting,						/**< Host sends out to instruct eligible players to begin voting on whether to skip the next race */

		eNetMessage_CommunicatorStatusUpdate,			/**< Communicator has been inserted / removed */

		eNetMessage_ForceRaceTimeout,					/**< Host sends out when he wants to force everyone to timeout (To instantly end the race). Only for use in private custom races!*/

		eNetMessage_Max,								
	};

} // end namespace RacerNet

#endif // __NET_DATAGRAM_PACKET_TYPES_H__
