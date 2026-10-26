// ***************************************************************
//  NetJoinStates   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Lists all the states required to join a network session.
// ***************************************************************

#ifndef __NET_JOIN_STATES_H__
#define __NET_JOIN_STATES_H__
        
#ifdef _MSC_VER  
#pragma once       
#endif           
          
namespace RacerNet
{
	/// List of possible states a client can be in while joining a session
	typedef enum
	{
		eJoinStateNotActive = 0,				// not currently joinings a session
		eJoinStateStartCreatingPeerSession,		// Xbox live uses sesion API which means we must create a peer sesion before joining..
		eJoinStateCreatingPeerSession,			// wait for xbox live session to be created
		eJoinStateWaitingForHostReply,			// join request send and awiting response from host
		eJoinStateHandshaking,					// received initial data from host and handshaking with session players
		eJoinStateFailed,						// oops something went wrong while joining.
		eJoinStateComplete,						// join has completed successfully
	}eNetJoinSessionStates;

}	// end namespace RacerNet

#endif    // __NET_JOIN_STATES_H__