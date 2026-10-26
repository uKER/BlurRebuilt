// ***************************************************************
//  NetRaceSettings   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	defines the currect race setting for the network race session.
// ***************************************************************

#ifndef __NET_RACE_SETTINGS_H__
#define __NET_RACE_SETTINGS_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// Holds all the settings for the current network race.
	/** This structure is replicated onto the peers at join time, and when it changes in custom race mode.
	 */
	struct NetRaceSettings
	{
		NetRaceSettings()
		{Clear();}

		void Clear(void)
		{
		}
	};

} // end namespace RacerNet

#endif // __NET_RACE_SETTINGS_H__
