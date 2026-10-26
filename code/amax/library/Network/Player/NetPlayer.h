// ***************************************************************
//  NetPlayer   version:  1.0   ·  date: 04/14/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	holds all data for a single network player, local or 
// remote.
// ***************************************************************

#ifndef __NET_PLAYER_H__
#define __NET_PLAYER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "../Misc/NetDefines.h"
#include "NetplayerConnection.h"
#include "NetPlayerVoice.h"
#include "Data/NetPlayerPartyData.h"
#include "Data/NetPlayerRaceData.h"
#include "Data/NetPlayerProfileData.h"
#include <systemprofile/systemprofile.h>
#include <bdNet/bdNet.h>
#include <bdSocket/bdAddrHandle.h>
#include <bdCore/bdSocket/bdCommonAddr.h>
#include <Network/Nonce.h>

namespace RacerNet
{
	// forward declerations
	class C_NetMessageReliableOutBox;
	class C_NetMessageReliableInBox;
	class C_NetMessage;
	class C_NetRouterBase;

	/// Defines a network player.
	/** This structure will be used to track party and game session data for a single network player.
	 */
	class C_NetPlayer : public Btl::ListElementDouble<C_NetPlayer>
	{
	public:
		friend class C_NetPlayerManager;
		friend class C_NetServices;

		C_NetPlayer();
		virtual ~C_NetPlayer();

		/// This is called by the BTL functions when a player is removed from the linked list.
		/** The game code MUST NOT call this function.
		 */
		void Destroy(void);

		/// Is this player local.
		/** @return true if this player is local to the console who calls it.
		 */
		bool LocalPlayer(void) const {return m_ControllerPort == -1 ? false : true;}

		/// Is this player the primary player on his own console.
		/** @return true if this player is the primary player on his own console.
		 */
		bool PrimaryPlayer(void) const {return 0 == m_LocalIndex;}

		/// test if a specific connection is active for this player.
		/** @param partyConnection true if we wish to test the party session or false if we wish to test the race session.
		 *	@return true if the player has a conenction to the specified session.
		 */
		bool ConnectionActive(const bool partyConnection);

		/// Attempt to send this remote player a reliable message.
		/** This should only be used to send reliable messages, unreliable messages will not be added. Only try to send messages to
		 *	remote primary players or message won't be added to send queue.
		 *	@param messageId the Identifier for this particular network message to the recipient can decode it correctly.
		 *	@param partyMessage should be true if the message we are creating will be sent on the party channel.
		 *	@param msg correctly configured reliable message data that we wish to send. 
		 *	@param localSenderId the identifier for the local player who is sending this message so the recipient knows who it originally came from.
		 *	@param initialConnection should be false for all other messages than join reply messages.
		 */
		bool SendReliable(C_NetMessage& msg, const U8 messageId, const bool partyMessage, const U8 localSenderId, const bool initialConnection = false);

		/// Attempt to send an unreliable message to a remote peer.
		/** This should only be used to send unreliable messages, reliable messages will not be added. Only try to send messages to
		 *	remote primary players or message won't be added to send queue.
		 *	@param msg correctly configured unreliable message to send.
		 *	@param partyMessage true if the message is to be sent on the party channel.
		 *	@param router socket router base class thet will actially send the message.
		 *	@return true if the message was sent.
		 */
		bool SendUnReliable(C_NetMessage& msg, const bool partyMessage, C_NetRouterBase& router);

		/// This will attempt to remove specific messages from the outgoing queue if any exist.
		/** This should only be used when a player is in both the race and party session but disconnects from one of them.
		 *	@param partyMessages true if we wish to remove all party messages or false if we wish to remove all race messages.
		 */
		void RemovePendingReliableMessages(const bool partyMessages);

		/// Flush any voice data that is currently queued up to go to this player.
		void FlushOutgoingVoice(void);

		/// Attempt to add some voice communicator data to this players outgoing voice queue.
		/**	Only remote player are setup with outgoing voice queues. If the internal buffer does not have enough space to copy all the data
		 *	then none of it will be copied.
		 *	@param localNetPlayerIndex this is the index of the local player within the player manager class, this is not a controller index.
		 *	@param pBuffer pointer to the voice data to copy into the outgoing queue.
		 *	@param bufferSize number of bytes to copy.
		 *	@return true if the data was successfully copied into the buffer.
		 */
		bool AppendOutgoingVoice(const U8 localNetPlayerIndex, const U8* pBuffer, const U32 bufferSize);

		/// Is their any voice queued up to send to this remote player.
		/** @return number of voice bytes ready to send to peer.
		 */
		U32 VoiceSize(void) const;

		/// How many voice bytes are queued up for a specific local player index.
		/** @param localPlayerIndex the index (not controller port) of the local player.
		*	@return number of voice bytes ready to send to peer by the local player.
		*/
		U32 VoiceSize(const U8 localPlayerIndex) const;

		/// Attempts to write any voice data that is buffered up to the message.
		/** message should have already been put in unencrypted mode..
		 *	game logic must check that there is enough space left in the message to hold all buffered voice that needs to be sent.
		 *	if their is insufficient space this function will fail.
		 *	@return true if the vocie data was copied into the network message. 
		 */
		bool WriteOutgoingVoiceToMessage(C_NetMessage& msg);

		/// Is the player flagged as being in the race.
		bool IsInRace(void);

		/// Can this player be quick muted in the lobby.
		bool CanQuickMute(void) const;

		/// Can this network player use user created content.
		bool CanUseUserCreatedContent(void) const {return m_ProfileData.m_UserCreatedContentAvailable == 0 ? false : true;}

	public:
		U8	m_CommonAddress[BD_COMMON_ADDR_SERIALIZED_SIZE];			// holds this players common network address in serialised format.
		bool m_VoiceBanned;												// is this player voice banned by the system, no voice allowed if so.
		bool m_HasCommunicator;											// does this player currently have a communicator connected.
		U32	m_DlcMask;													// bits determine which dlc packs are installed for this player.
		U64	m_ConsoleId;												// used MAC address to generate a console identifier
		float m_IsSpeaking;												// is the player curently speaking. 0.0f means not speaking.
		U16	m_UpStreamKbps;												// how much upstream bandwidth this player has in kbps
		U16	m_DownStreamKbps;											// how much downstream bandwidth this playre ahs in kbps
		U8	m_LocalIndex;												// only valid on the local console to track multiple players ont he same console.
		S8	m_ControllerPort;											// controller port if local player, -1 denotes remote player.

		SysProfile::BaseProfile	m_User;									// username + platform specific identifier.	
		
		C_NetPlayerConnection		m_PartyConnection;					// party connection data
		NetPlayerPartyGameData		m_PartyData;						// game logic data for the party session

		C_NetPlayerConnection		m_RaceConnection;					// race connection data.
		NetPlayerRaceGameData		m_RaceData;							// game logic data for the race session.

		NetPlayerProfileGameData	m_ProfileData;						// game profile data that is shared between players, shared between party and race sessions.
		NetPlay::Nonce				m_PlayerNonce;						// unique id for my party or myself

		Btl::BitArray16				m_VoiceEncodeArray;					// for remote players, this defines which local players are sending voice to this remote player.
		U8							m_QuickMute;						// for remote players this defines which local players have them quick muted (bit field).  

	private:
		C_NetMessageReliableInBox*	m_pInBox;							// incoming mesasges sent by this player
		C_NetMessageReliableOutBox*	m_pOutBox;							// outgoing messages waiting to be sent to this player by us.
		NetLocalVoiceArray*			m_pVoice;							// outgoing voice waiting to be sent.
	};

	/// defines a linked list for use with the network player object
	typedef Btl::List<C_NetPlayer> NetPlayerList;

	/// defines a pool to allocate network players from.
	typedef Btl::StaticPool<C_NetPlayer, NET_MAX_VEHICLES> NetPlayerPool;

}// end namespace RacerNet

#endif // __NET_PLAYER_H__
