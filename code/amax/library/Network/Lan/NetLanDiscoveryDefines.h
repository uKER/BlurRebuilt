// ***************************************************************
//  NetLanDiscoveryDefines   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	defines structures used when enumerating LAN games.
// ***************************************************************
 

#ifndef __NET_LAN_DEFINES_H__
#define __NET_LAN_DEFINES_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Core.h>
#include <Network/Nonce.h>
#include <Network/Securekey.h>
#include <Systemprofile/SystemProfile.h>
#include <Btl/Containers/vector.h>
#include <bdCore/bdSocket/bdCommonAddr.h>
#include "../Misc/NetRaceSettingsQos.h"

namespace RacerNet
{
	#if defined (PLATFORM_PS3)
		const U32 AMAX_LAN_DISCOVERY_PORT =  (50000);
	#elif defined (PLATFORM_PC)
		const U32 AMAX_LAN_DISCOVERY_PORT =  (50001);
	#else
	#endif

	typedef enum
	{
		eLanDiscoveryPacketType_Request = 1,
		eLanDiscoveryPacketType_Reply,
	}eLanDiscoveryPacketType;

	/// header data that all lan discovery packets will use.
	struct LanDiscoveryHeader
	{
		U32	m_PacketVersion;
		U32 m_TitleId;
		eLanDiscoveryPacketType m_PacketType;
		NetPlay::Nonce m_Nonce;
	};

	/// packet that get's send when we broadcast that we wist to look for lan games
	struct LanDiscoveryRequest : LanDiscoveryHeader
	{
		U32 m_NumPlayersToJoin;
	};

	/// packet that we get back from a host when we broadcast for active lan games.
	struct LanDiscoveryReply : LanDiscoveryHeader
	{
		U64	m_ConsoleId;
		SysProfile::BaseProfile m_HostProfile;
		NetPlay::SessionSecurity m_SessionSecurity;
		U8 m_HostCommonAddress[BD_COMMON_ADDR_SERIALIZED_SIZE];	
		U8 m_MaxSlots;
		U8 m_SlotsTaken;
		NetRaceSettingsQos m_RaceQos;
	};

	/// for tracking repllies when we wish to enumerate lan sessions.
	typedef Btl::Vector<LanDiscoveryReply, Btl::VectorParameters_Static<10> > LanDiscoveryReplies;

} // end namespace RacerNet

#endif // __NET_LAN_DEFINES_H__
