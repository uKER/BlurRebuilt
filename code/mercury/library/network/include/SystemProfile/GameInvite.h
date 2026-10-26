#ifndef SYSTEM_GAME_INVITE_H
#define SYSTEM_GAME_INVITE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdCore.h>
#include <bdCore/bdSocket/bdSecurityKey.h> 
#include <bdCore/bdSocket/bdSecurityID.h>

namespace SysProfile
{	
	/// To allow joining of Online game sessions if sent by friends..
	struct GameInvite
	{
		GameInvite() : m_Active(false)
		{}

		bool m_Active;

#if defined(LIVE_ON_WINDOWS)
		XINVITE_INFO	m_InviteData;
#else 
		bdSecurityID	m_Id;
		bdSecurityKey	m_Key;		// no longer used, will be removed.
#endif
	};

}; // namespace SysProfile

#endif
