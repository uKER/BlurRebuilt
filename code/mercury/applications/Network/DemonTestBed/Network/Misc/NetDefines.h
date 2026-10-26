// ***************************************************************
//  NetDefines   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	generic defines for the network systems.
// ***************************************************************

#ifndef __NET_DEFINES_H__
#define __NET_DEFINES_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Core.h>
#include <btl/Containers/vector.h>
#include <btl/Containers/list.h>
#include <btl/Allocators/pool.h>
#include <bdCore/bdSocket/bdSecurityID.h>

namespace RacerNet
{
	#ifndef CFG_GOLD
	// define this to disable player dropouts from the game
	//#define NO_DROPOUTS		
	#endif

	// when trying to join a game we chack network tarnsport versions, a difference means the client can't join.
	// this needs be changed for patches if we need make significant changes to the network transport.
	const U32 NET_PACKET_VERSION = (0xF000001);

	// maximum number of vehicles allowed in network races, can be lower than what the single player uses but not lareger.
	const U8 NET_MAX_VEHICLES = (20);

	// how many players can be on a single console.
#if !defined (PLATFORM_PC)
	const U8 NET_MAX_LOCAL_PLAYERS = (2);
#else
	const U8 NET_MAX_LOCAL_PLAYERS = (1);
#endif

	// how many seperate teams can we have in network race.
	const U8 NET_MAX_TEAMS = (2);

	// maximum number of players we can allow in a party, can never be greater then NET_MAX_VEHICLES.
	const U8 NET_MAX_PARTY_MEMBERS = NET_MAX_VEHICLES;

	// how many seconds do we wait while joining the session before we say connection cant be established.
	const U8 NET_MAX_HANDSHAKE_TIME = (15);

	// how many seconds the host wait before dropping a peer
	const U8 NET_HOST_DROPOUT_TIMER = (4);

	// how often do we wish to send keepalive packets to remote peers, under normal conditions.
	const float	NET_KEEPALIVE_NORMAL_FREQUENCY (0.3f);

	// how many seconds does the peer wait before dropping another peer or informing the host.
	const U8 NET_PEER_DROPOUT_TIMER = (6);

	// how big will each remote players inbox be.
	const U32 NET_MAX_INBOX_SIZE = (1024*6);

	// how big will each remote players inbox be.
	const U32 NET_MAX_OUTBOX_SIZE = (1024*12);

	// defines the lowest bandwidth threshold we recognise for online play.
	const U16 NET_MINIMUM_BANDWIDTH_KBPS (64);

	// list of player identifiers for a particular side.
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<NET_MAX_VEHICLES/2> > NetRaceTeamMembers;

	// list of players seperated into teams.
	typedef Btl::Vector<NetRaceTeamMembers, Btl::VectorParameters_Static<NET_MAX_TEAMS> > NetRaceTeams;

	// list of join references in a single stack.
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<NET_MAX_VEHICLES> > NetJoinReferences;

	// list of security keys Ids so we can track what is active.
	typedef Btl::Vector<bdSecurityID, Btl::VectorParameters_Static<NET_MAX_VEHICLES> > SecurityKeyIds;

	// defines how lan session enumeration will filter out other platforms and versions.
#if defined (PLATFORM_XENON)
	const U32 AMAX_LAN_ID =  (0x0000001A);
#elif defined (PLATFORM_PS3)
	const U32 AMAX_LAN_ID =  (0x0000001B);
#elif defined (PLATFORM_PC)
	const U32 AMAX_LAN_ID =  (0x0000001C);
#else
#endif

} // end namespace RacerNet

#endif // __NET_DEFINES_H__
