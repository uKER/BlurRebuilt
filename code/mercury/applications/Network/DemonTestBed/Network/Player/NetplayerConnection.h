// ***************************************************************
//  NetplayerConnection   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	tracks a players connection via a single security association.
// ***************************************************************

#ifndef __NET_PLAYER_CONNECTION_H__
#define __NET_PLAYER_CONNECTION_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "NetPlayerStates.h"
#include "../Core/NetTimer.h"
#include <bdSocket/bdAddrHandle.h>
#include <bdCore/bdSocket/bdCommonAddr.h>

namespace RacerNet
{
	/// Defines a network player.
	/** This structure will be used to track party and game session data for a single network player.
	 */
	class C_NetPlayerConnection 
	{
	public:
		C_NetPlayerConnection();
		virtual ~C_NetPlayerConnection();

		/// Do we have a full connection to the player.
		/** NAT negotiation will have completed if the connection is complete.
		 *	Don't call this for local players since they won't have a connection to themselves..
		 */
		bool IsConnected(void) const;

		/// Disconnect the address and reset the connection.
		/** @param unregisterAddress true if we wish to unregister the address.
		 */
		void Disconnect(const bool unregisterAddress);

	public:
		eNetPlayerState	m_PlayerState;						// players state within this connection.
		bool			m_DropRequested;					// player has been flagged to be dropped from this connection.
		bool			m_PublicPlayer;						// if online does this player take a public slot on the server ?
		bool			m_Host;								// is this player flagged as the host on this connetcion.
		bool			m_AddedToOnlineSession;				// has this player been added to the online session for this connection.
		C_NetTimer		m_TimeSinceLastReceivedKeepAlive;	// how long has passed since we received a packet from this player.
		float			m_TimeSinceLastSentUnReliable;		// how long has it been since we sent any type of unreliable message to this player. this is used internally to send periodic keepalive packets 
		float			m_KeepAliveFrequency;				// how often do we wish to send keepalives, can scale this back to lower bandwidth if required.
		bdAddrHandleRef	m_Address;							// if player is remote this will hold the network address otherwise it will be a null reference for local players.
		U8				m_JoinId;							// player unique join reference within this connection.
	};

}// end namespace RacerNet

#endif // __NET_PLAYER_CONNECTION_H__
