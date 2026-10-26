// ***************************************************************
//  NetDefines   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Specifies the version of the network transport.
//			Changing this version makes differing builds of the game
//			incompatible.
// ***************************************************************

#ifndef __NET_VERSION_H__
#define __NET_VERSION_H__

#ifdef _MSC_VER
#pragma once
#endif

#ifndef CFG_GOLD
	enum eNetBuildVersion
	{
		eNetBuildVersion_Master				= 0x0A000060,
		eNetBuildVersion_BizarreQA			= 0x0B000060,
		eNetBuildVersion_BizarreMpBalancer	= 0x0C000060,
		eNetBuildVersion_ActivisionQA		= 0x0D000060,
		eNetBuildVersion_2010Testing		= 0x0E000060,
		eNetBuildVersion_BCMpSession		= 0x0F000060,
		eNetBuildVersion_GamesComSysLink	= 0x1A000060,
		eNetBuildVersion_Misc1				= 0x1B000060,
		eNetBuildVersion_Misc2				= 0x1C000060,
	};
#endif

namespace RacerNet
{

#ifdef CFG_GOLD
	// when trying to join a game we check network transport versions, a difference means the client can't join.
	// this needs be changed for patches if we need make significant changes to the network transport.
	const U32 NET_PACKET_VERSION = (0x0F000001);
#endif

	// defines used when comparing net versions.
	typedef enum
	{
		eNetVersionSame = 0,			// versions are the same.
		eNetVersionOlder,				// We have an older version that what we are comparing against.
		eNetVersionNewer,				// We have a newer version that what we are comparing against.
		eNetVersionUnknown,				// unable to make comparison.
	}eNetVersion;

} // end namespace RacerNet

#endif // __NET_VERSION_H__
