// ***************************************************************
//  NetPartyNonce   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	defines a specific unique nonce structure that can be 
//			used to represent a specific party session.
// ***************************************************************

#ifndef __NET_PARTY_NONCE_H__
#define __NET_PARTY_NONCE_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/Nonce.h>

namespace RacerNet
{
	/// Created by the party host so all players joining the session can say they are part of a specific party.
	/** When the host wants his party to join a session he will create one of these and distribute it to the party peers  along with
	 *	the session to join. When the peers attemp to join they pass this data to the session host so he can put them in the same party within the session
	 *	players.
	 */
	struct NetPartyNonce
	{
		void Create(const U64  id)
		{
			m_Id = id;
			m_Nonce.Create();
		}

		bool operator == (const NetPartyNonce& other) const 
		{
			if (other.m_Id != m_Id)
				return false;
			return m_Nonce == other.m_Nonce;
		}

		U64 m_Id;				
		NetPlay::Nonce m_Nonce;
	};

} // end namespace RacerNet

#endif // __NET_PARTY_NONCE_H__
