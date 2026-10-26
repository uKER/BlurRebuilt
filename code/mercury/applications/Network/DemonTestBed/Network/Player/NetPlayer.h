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
#include <systemprofile/systemprofile.h>
#include <bdNet/bdNet.h>


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

	public:
		U64	m_ConsoleId;							// used MAC address to generate a console identifier
		U8	m_LocalIndex;							// only valid on the local console to track multiple players ont he same console.
		S32	m_IsSpeaking;							// is the player curently speaking.
		S8	m_ControllerPort;						// controller port if local player, -1 denotes remote player.
		SysProfile::BaseProfile	m_User;				// username + platform specific identifier.	
		C_NetPlayerConnection m_PartyConnection;	// party connection data
		C_NetPlayerConnection m_RaceConnection;		//	race connection data.

	private:
		C_NetMessageReliableInBox*	m_pInBox;
		C_NetMessageReliableOutBox*	m_pOutBox;
	};

	/// defines a linked list for use with the network player object
	typedef Btl::List<C_NetPlayer> NetPlayerList;

	/// defines a pool to allocate network players from.
	typedef Btl::StaticPool<C_NetPlayer, NET_MAX_VEHICLES> NetPlayerPool;

}// end namespace RacerNet

#endif // __NET_PLAYER_H__
