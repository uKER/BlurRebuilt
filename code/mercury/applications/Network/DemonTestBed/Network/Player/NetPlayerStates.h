// ***************************************************************
//  NetPlayerStates   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Defines the states that network players ca be.
// ***************************************************************

#ifndef __NET_PLAYER_STATES_H__
#define __NET_PLAYER_STATES_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// List of different states network players can be in.
	typedef enum
	{
		// generic states sued on race and party sessions..
		eNetPlayerStateUnSet =	0,				// not a valid state !
		eNetPlayerStateJoining,					// In join procedure..
		eNetPlayerStateIdle,					// doing nowt in the lobby.

		// specific states used only in race sessions
		eNetPlayerStateChangingSettings,		// changing the game settings
		eNetPlayerStateChangingCar,				// changing selected car..
		eNetPlayerStateLoadingRace,				// loading race
		eNetPlayerStateLoadingUi,				// loading the frontend..

		// sepcific states used only in party sessions.
		eNetPlayerStatePartyBusy,				// party player is currently busy
		eNetPlayerStatePartySyncJoinRaceStart,	// party player is waiting to join a race session
		eNetPlayerStatePartyJoinRace,			// party player in the process of joining a race session
		eNetPlayerStatePartySyncJoinRaceEnd,	// party player has joined the race session and is awaiting the remaining party members to get to this state before transitioning to lobby
		eNetPlayerStatePartyJoinRaceFailed,		// party player failed to join the race (all other party members should leave race now)
		eNetPlayerStatePartyJoinRaceSuccessful,	// party player has joined the race...
		eNetPlayerStatePartyRaceCreated,		// party host has created a race and is awaiting party peers to join

		eNetPlayerStatePartySyncBeforeDisband,	// waiting for everybody to enter this state before players disband to join new party

		eNetPlayerStatePartySyncMatchingStart,	// waiting for all player to enter online career matching screen..
		eNetPlayerStatePartySyncMatching,		// actually matching..
		
		// Any state below here means player is in race..
		eNetPlayerStateWaitingForOthers,		// waiting for other players to load..
		eNetPlayerStateRacing,					// racing
		eNetPlayerStateViewingResults,			// viewing results
		eNetPlayerStateSpectating,				// spectating a race..
		eNetPlayerStateMax
	}eNetPlayerState;

} // end namespace RacerNet

#endif //__NET_PLAYER_STATES_H__
