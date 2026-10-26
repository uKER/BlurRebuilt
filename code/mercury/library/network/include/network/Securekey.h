// ***************************************************************
//  Securekey   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Security information for network game sessions.
// ***************************************************************

#ifndef NETWORK_SECURE_KEY_H
#define NETWORK_SECURE_KEY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdCore.h>
#include <bdCore/bdSocket/bdSecurityKey.h> 
#include <bdCore/bdSocket/bdSecurityID.h>

namespace NetPlay
{
	/// wrapper class for holding a nonce that is used in arbitrated online game sessions.
	struct ArbitrationNonce
	{
		ArbitrationNonce() : m_ArbitrationNonce(0)
		{}
		U64		m_ArbitrationNonce; // only used in internet enabled sessions..
	};

	/// basically a wrapper structure for holding security information regarding a network session.
	struct SessionSecurity : ArbitrationNonce
	{
		SessionSecurity() 
		{}

#if defined (LIVE_ON_WINDOWS)
		XNKID	m_Id;
		XNKEY	m_Key;
#else
		// demonware security structures.
		bdSecurityID	m_Id;
		bdSecurityKey	m_Key;
#endif
	};

	/// passed arround when migrating session to new owner.
	struct MigrationInfo
	{
#if defined(LIVE_ON_WINDOWS)
		MigrationInfo() 
		{}
		XSESSION_INFO m_SessionInfo;
#else
		bdSecurityID	m_Id;
		bdSecurityKey	m_Key;
#endif
	};
}



#endif


