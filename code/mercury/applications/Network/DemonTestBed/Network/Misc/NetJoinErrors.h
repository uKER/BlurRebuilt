// ***************************************************************
//  NetJoinErrors   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	lists all the possible reasons why you could not join
//			a network session.
// ***************************************************************

#ifndef __NET_JOIN_ERRORS_H__
#define __NET_JOIN_ERRORS_H__
        
#ifdef _MSC_VER  
#pragma once       
#endif           
          
namespace RacerNet
{
	// List of possible errors while a player cant join an already running network game.
	typedef enum
	{
		eJoinErrorInvalid = 0,			// Unknown join error
		eJoinErrorNone,					// No error has occurred, we have contecated the host successfully.				
		eJoinErrorNoConnectionToHost,	// Not possible to connect to the Host.
		eJoinErrorHostDiffersOnPeer,	// The host we originally joined with doesnt match this peers host so we can't join.
		eJoinErrorNotEnoughOpenSlots,	// Not enough open player slots to join the game.
		eJoinErrorBannedFromSession,	// The host has banned you from joining.
		eJoinErrorNoCommunications,		// NAT issues mean you can't communicate with everybody who is currently in the game.
		eJoinErrorToManyTeams,			// There is already the full complement of Teams in the lobby.
		eJoinErrorToManyTeamMembers,	// Your team has it's full complement of members.
		eJoinErrorContentMismatch,		// You don't have all the required content to play.
		eJoinErrorRankedMatchStarted,	// Ranked match has already started.
		eJoinErrorPartyClosed,			// Party is closed to joins now ..
		eJoinErrorProfileProblem,		// can't join because we have a gamer profile issue.
	}eNetJoinSessionErrors;

} // end namespace RacerNet

#endif // __NET_JOIN_ERRORS_H__   
