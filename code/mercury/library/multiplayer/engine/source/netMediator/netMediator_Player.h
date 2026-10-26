#ifndef NETMEDIATOR_PLAYER_H
#define NETMEDIATOR_PLAYER_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netGuid/netGuid.h"
#include "netStream/netSerialise.h"
#include "netTransport/netTransport_ConnGuid.h"
#include <SystemProfile/SystemProfile.h>

namespace netMediation
{
	// player info (host -> peers).
	// information about a player in the party / game.
	class Player: public netSerialisable
	{
	public:
		SysProfile::FullProfile	m_profile;			// profile data
		bool					m_active;			// we active or not?
		char					m_slot;				// player slot in the session
		netGuid					m_guid;				// player id assigned by the host.
		netTransport::ConnGuid	m_connGuid;			// connection the player is attached to.
		char					m_localIndex;		// local index
		int						m_skill;			// skill level (used for matchmaking).
		bool					m_headset;			// has a headset.
		
		Player();

		bool isGuest() const;
		bool notActive() const;
		bool blagged() const;
		bool online() const;
		bool offline() const;
		
		bool   serialise(class netStream* stream);
		bool deserialise(class netStream* stream);
	};
}

#endif //NETMEDIATOR_PLAYER_H