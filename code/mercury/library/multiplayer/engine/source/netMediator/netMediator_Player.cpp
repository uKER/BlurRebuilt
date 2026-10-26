
#include "stdafx.h"
#include "netMediator_Player.h"
#include "netStream/netStream.h"
#include "ui/util/WideString.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netMediation
{
	// player info (host -> peers).
	// information about a player in the party / game.
	Player::Player()
	: m_active(false)
	, m_localIndex(-1)
	, m_slot(-1)
	, m_skill(0)
	, m_headset(false)
	{
		UI::C_TextStringID str("MPS_NULL");
		UI::WideStrFormat(m_profile.m_Name, sizeof(m_profile.m_Name)/2, str.String());
	}

	bool Player::notActive() const
	{
		return (m_profile.m_State == SysProfile::eProfileStateNotActive);
	}

	bool Player::blagged() const
	{
		return (m_profile.m_State == SysProfile::eProfileStateBlagged);
	}

	bool Player::offline() const
	{
		return (m_profile.m_State == SysProfile::eProfileStateOffline);
	}

	bool Player::online() const
	{
		return (m_profile.m_State == SysProfile::eProfileStateOnline);
	}

	bool Player::isGuest() const
	{
		return (m_profile.m_IsGuest);
	}

	bool Player::serialise(class netStream* stream)
	{
		m_connGuid.serialise(stream);
		m_guid.serialise(stream);
		stream->writeType(m_active);
		stream->writeType(m_localIndex);
		stream->writeType(m_profile);
		stream->writeType(m_skill);
		stream->writeType(m_slot);
		stream->writeType(m_headset);
		return stream->ok();
	}

	bool Player::deserialise(class netStream* stream)
	{
		m_connGuid.deserialise(stream);
		m_guid.deserialise(stream);
		stream->readType(m_active);
		stream->readType(m_localIndex);
		stream->readType(m_profile);
		stream->readType(m_skill);
		stream->readType(m_slot);
		stream->readType(m_headset);
		return stream->ok();
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
