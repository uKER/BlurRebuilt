#include "stdafx.h"
#include "netMediator/netMediator_PeerState.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netMediation
{
	PeerState::ConnectionList::ConnectionList() 
	{ 
		m_peerCount = 0; 
	}


	int PeerState::ConnectionList::qosRating(const netTransport::ConnGuid& guid) const
	{
		for(int i = 0; i < m_peerCount; i ++)
		{
			if(m_peers[i].m_guid == guid)
				return m_peers[i].m_qos;
		}
		return netTransport::CONN_QOS_UNAVAILABLE;
	}

	const netTransport::Peer* PeerState::ConnectionList::peer(const netTransport::ConnGuid& guid) const
	{
		if(guid.isNull())
			return NULL;

		for(int i = 0; i < m_peerCount; i ++)
		{
			if(m_peers[i].m_guid == guid)
				return &(m_peers[i]);
		}
		return NULL;
	}

	const netTransport::Peer* PeerState::ConnectionList::addr(const netTransport::Addr& addr) const
	{
		for(int i = 0; i < m_peerCount; i ++)
		{
			if(m_peers[i].m_addr == addr)
				return &(m_peers[i]);
		}
		return NULL;
	}

	bool PeerState::ConnectionList::serialise(class netStream* stream)
	{
		stream->writeType(m_peerCount);

		for(int i = 0; i < MAX_PEERS; i ++)
		{
			m_peers[i].serialise(stream);
		}
 		return stream->ok();
	}

	bool PeerState::ConnectionList::deserialise(class netStream* stream)
	{
		stream->readType(m_peerCount);

		for(int i = 0; i < MAX_PEERS; i ++)
		{
			m_peers[i].deserialise(stream);
		}
		return stream->ok();
	}

	PeerState::Session::Session()
	{
		m_electionWeight = 0;
		m_electionUnanimous = false;
		m_electionResults = false;
		m_electionCandidate = netTransport::ConnGuid();
		m_NATtype = netPlatform::NAT_TYPE_NOT_SET;

		m_arbitratedPlayerCount = 0;
		memset(&m_migrationInfo, 0, sizeof(m_migrationInfo));
		memset(&m_arbitrationInfo, 0, sizeof(m_arbitrationInfo));
		memset(m_name, 0, sizeof(m_name));
	}

	bool PeerState::Session::serialise(class netStream* stream)
	{
		stream->writeBytes(m_name, sizeof(m_name));
		m_selfGuid.serialise(stream);
		m_hostGuid.serialise(stream);
		m_electionCandidate.serialise(stream);
		stream->writeType(m_electionWeight);
		stream->writeType(m_electionResults);
		stream->writeType(m_electionUnanimous);
		stream->writeType(m_migrationInfo);
		stream->writeType(m_arbitrationInfo);
		stream->writeType(m_arbitratedPlayerCount);
		stream->writeType(m_NATtype);
		return stream->ok();
	}

	bool PeerState::Session::deserialise(class netStream* stream)
	{
		stream->readBytes(m_name, sizeof(m_name));
		m_selfGuid.deserialise(stream);
		m_hostGuid.deserialise(stream);
		m_electionCandidate.deserialise(stream);
		stream->readType(m_electionWeight);
		stream->readType(m_electionResults);
		stream->readType(m_electionUnanimous);
		stream->readType(m_migrationInfo);
		stream->readType(m_arbitrationInfo);
		stream->readType(m_arbitratedPlayerCount);
		stream->readType(m_NATtype);
		return stream->ok();
	}

	PeerState::PeerState()
	{
	}

	bool PeerState::serialise(class netStream* stream)
	{
		m_session.serialise(stream);
		m_connections.serialise(stream);
		return stream->ok();
	}

	bool PeerState::deserialise(class netStream* stream)
	{
		m_session.deserialise(stream);
		m_connections.deserialise(stream);
		return stream->ok();
	}

	PeerStateRender::PeerStateRender(const char* name, netTransport::ConnGuid guid, PeerStateRef state, bool established)
	{
		sprintf_s(m_name, sizeof(m_name), name);
		m_guid = guid;
		m_state = state;
		m_established = established;
	}

	void PeerStateRender::init(const Maths::HVec2& pos, const Maths::HVec2& dir, float rad)
	{
		m_pos = pos;
		m_radius = rad / 10.0f;
		if(m_radius < 8) m_radius = 8;
		if(m_radius > 32) m_radius = 32;

		m_dir = dir;
		m_tan = Maths::HVec2(-dir.Y(), dir.X());
		m_centre = pos + dir * rad;
		m_input = m_centre - m_tan * m_radius;
		m_output = m_centre + m_tan* m_radius;
	}

	#define DEFAULT_COLOUR(alpha) Maths::RGBA((U8)0xff, (U8)0xff, (U8)0xff, (U8)alpha) // white
	#define SELF_COLOUR(alpha) Maths::RGBA((U8)0x00, (U8)0xff, (U8)0x00, (U8)alpha) // green
	#define HOST_COLOUR(alpha) Maths::RGBA((U8)0xff, (U8)0x00, (U8)0x00, (U8)alpha) // red
	#define MIGR_COLOUR(alpha) Maths::RGBA((U8)0xff, (U8)0x00, (U8)0xff, (U8)alpha) // magenta

	void PeerStateRender::renderLink(DbUtil::I_DebugRenderPtr debugRenderPtr, PeerStateRenderRef other, netTransport::ConnGuid selfGuid)
	{
		if(m_state.isNull())
			return;

		/*
		if(other->m_guid == selfGuid)
		{
			colour = SELF_COLOUR(255);
		}
		*/

		int qosRating = (other->m_state.isNull())? netTransport::CONN_QOS_UNAVAILABLE : m_state->m_connections.qosRating(other->m_state->m_session.m_selfGuid);

		float delta = (float)(qosRating - netTransport::CONN_QOS_BAD) / (float)(netTransport::CONN_QOS_GOOD - netTransport::CONN_QOS_BAD);
		if(delta < 0.0f) delta = 0.0f;
		if(delta > 1.0f) delta = 1.0f;
		unsigned char alpha = (unsigned char)(80.0f + delta * (255.0f - 80.0f));

		Maths::RGBA colour = DEFAULT_COLOUR(alpha);

		if(other->m_guid == m_state->m_session.m_hostGuid)
		{
			colour = HOST_COLOUR(alpha);
		}
		if(other->m_guid == m_state->m_session.m_electionCandidate && (m_state->m_session.m_hostGuid != m_state->m_session.m_electionCandidate))
		{
			colour = MIGR_COLOUR(alpha);
		}

		float length = 0.33f;

		const netTransport::Peer* peer = m_state->m_connections.peer(other->m_guid);
		
		if(peer != NULL)
		{
			switch(peer->m_state)
			{
			default:
				{
					length = 0.33f;
					break;
				}
			case netTransport::CONN_STATE_CONNECTING:
			case netTransport::CONN_STATE_RESERVED:
			case netTransport::CONN_STATE_PENDING:
			case netTransport::CONN_STATE_ACCEPTING:
				{
					length = 0.33f;
					break;
				}
			case netTransport::CONN_STATE_ESTABLISHED:
				{
					length = 1.0f;
					break;
				}
			case netTransport::CONN_STATE_UNRESPONSIVE:
				{
					length = 0.75f;
					break;
				}
			case netTransport::CONN_STATE_CLOSING:
			case netTransport::CONN_STATE_CLOSED:
				{
					length = 0.25f;
					break;
				}
			}
		}

		Maths::HVec2 endpoint = m_output + (other->m_input - m_output) * length;

		debugRenderPtr->Draw2dLine(m_output, endpoint, colour);
	}

	void PeerStateRender::render(DbUtil::I_DebugRenderPtr debugRenderPtr, netTransport::ConnGuid selfGuid)
	{
		if(m_state.notNull())
		{
			Maths::HVec2 pt(m_centre + m_dir * (m_radius * 2.0f));
			
			// draw host name.
			char buff[128];
			sprintf_s(buff, sizeof(buff), "%d. %s", m_state->m_session.m_selfGuid.m_guid, m_name);
			debugRenderPtr->Draw2dText(pt, buff, Maths::RGBA(0xffffffff), m_radius * 2.0f);

			bool isSelf = (m_state->m_session.m_selfGuid == selfGuid);
			bool isHost = (m_state->m_session.m_selfGuid == m_state->m_session.m_hostGuid);
			bool isMig  = (m_state->m_session.m_selfGuid == m_state->m_session.m_electionCandidate);

			if(isSelf)
			{
				Maths::HVec2 pt1 = m_centre + m_dir * (m_radius * 0.25f);
				Maths::HVec2 size(m_radius, m_radius);
				debugRenderPtr->Draw2dCircle(pt1, size, SELF_COLOUR(255));
			}

			/*
			if(isHost)
			{
				Maths::HVec2 pt1 = m_centre + m_dir * (m_radius * 0.50f);
				debugRenderPtr->Draw2dCircle(pt1, m_radius, HOST_COLOUR(255));
			}

			if(isMig && (m_state->m_session.m_hostGuid != m_state->m_session.m_electionCandidate))
			{
				Maths::HVec2 pt1 = m_centre + m_dir * (m_radius * 0.75f);
				debugRenderPtr->Draw2dCircle(pt1, m_radius, MIGR_COLOUR(255));
			}
			*/
		}
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
