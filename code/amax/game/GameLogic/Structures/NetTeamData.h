#ifndef NET_TEAM_DATA_H
#define NET_TEAM_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <Network\Misc\NetDefines.h>

namespace Game
{

struct NetTeamData
{
	NetTeamData()
		: m_TeamScore(0) {}

	void Clear(bool clearAll)
	{
		m_TeamScore = 0;
		m_RacerOrder.Clear();
	}

	Btl::Vector<S8, Btl::VectorParameters_Static<RacerNet::NET_MAX_VEHICLES > > m_RacerOrder;
	S32		m_TeamScore;
};

typedef Btl::Vector<NetTeamData, Btl::VectorParameters_Array<RacerNet::NET_MAX_TEAMS> > TeamsData;

} // namespace Game

#endif // NET_TEAM_DATA_H