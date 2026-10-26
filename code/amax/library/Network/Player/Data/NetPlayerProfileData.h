// ***************************************************************
//  NetPlayerProfileData   version:  1.0   ·  date: 04/24/2009
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	game profile data that is shared between players.
// ***************************************************************

#ifndef __NET_PLAYER_PROFILE_DATA_H__
#define __NET_PLAYER_PROFILE_DATA_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// game profile data that is ahared between players.
	/** rank. etc.
	*/ 
	struct NetPlayerSharedProfile
	{
		NetPlayerSharedProfile() { Flush(); }
		void Flush(void)
		{
			m_Rank = 0;
			m_Legend = 0;
		}

		U16 m_RacePointsThisSession;	//	Number of "race points" gained in this network session.

		U8	m_Rank;	// current online rank.
		U8	m_Legend;	// Number the rank wrapped around
		U8	m_WinningStreak;	//	Number of races won in a row
		U8	m_RacesThisSession;
		U8	m_WinsThisSession;
		bool m_BlurDeveloper;	//	Is a Blur developer and so gets a cool little icon by their name
		bool m_HasBeatenBizarre;	// Possesses the "Beat Bizarre" Achievement
	};

	/// Holds any data that we take from the player profile.
	/** rank for xp for example.
	 */
	struct NetPlayerProfileGameData
	{
		NetPlayerProfileGameData() { Flush(); }
		void Flush(void)
		{
			m_Game.Flush();
			m_UserCreatedContentAvailable = 0;
			m_NatType = 1;
		}

		NetPlayerSharedProfile	m_Game;	// game profile defined data.

		U8	m_UserCreatedContentAvailable;	// can this player reference user created content.
		U8	m_NatType;						// specific nat type for this player.
	};
} // end namespace RacerNet

#endif //__NET_PLAYER_PROFILE_DATA_H__
