// ***************************************************************
//  NetPlayerPartyData   version:  1.0   ·  date: 08/19/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Holds all game logic specific data related to the 
//			network party session.
// ***************************************************************

#ifndef __NET_PLAYER_PARTY_DATA_H__
#define __NET_PLAYER_PARTY_DATA_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// Holds any game logic data that is specific to a single network player in the party session.
	struct NetPlayerPartyGameData
	{
		NetPlayerPartyGameData() { Flush(); }

		void Flush(void)
		{
		}
	};
} // end namespace RacerNet

#endif //__NET_PLAYER_PARTY_DATA_H__
