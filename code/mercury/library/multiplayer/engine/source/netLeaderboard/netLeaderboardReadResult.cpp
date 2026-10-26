
#include "stdafx.h"
#include "netLeaderboardReadResult.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLeaderboardReadData::netLeaderboardReadData(int id)
: m_leaderboardId(id)
, m_totalEntries(0)
{}

netLeaderboardReadData::~netLeaderboardReadData()
{}

netRowDataRef netLeaderboardReadData::getRow(int index)
{
	int i = 0;
	bdLinkedList<netRowDataRef>::Position pos = m_entries.getHeadPosition();

	while(pos)
	{
		netRowDataRef entry = m_entries.forward(pos);
		if(i >= index) return entry;
		i++;
	}
	return NULL;	
}

int netLeaderboardReadData::countRows()
{
	int i = 0;
	bdLinkedList<netRowDataRef>::Position pos = m_entries.getHeadPosition();
	while(pos)
	{
		netRowDataRef entry = m_entries.forward(pos);
		i++;
	}
	return i;	
}

netRowDataRef netLeaderboardReadData::findLeader()
{
	bdLinkedList<netRowDataRef>::Position pos = m_entries.getHeadPosition();
	netRowDataRef leader;

	while(pos)
	{
		netRowDataRef entry = m_entries.forward(pos);
		
		// find same user. remove him first.
		if(leader.isNull() || entry->m_Rank < leader->m_Rank)
			leader = entry;
	}
	return leader;
}

netRowDataRef netLeaderboardReadData::findRow(netUserId user)
{
	bdLinkedList<netRowDataRef>::Position pos = m_entries.getHeadPosition();

	while(pos)
	{
		netRowDataRef entry = m_entries.forward(pos);
		
		// find same user. remove him first.
		if(user == entry->m_EntityId)
			return entry;
	}
	return NULL;	
}

void netLeaderboardReadData::updateSkill(const SysProfile::UserIdentifier& user, netUserId userId, NetPlay::PlayerSkill* skill)
{
	netRowDataRef entry = findRow(userId);

	if(entry.isNull())
	{
		entry = new netRowData;
		entry->m_Userdata.m_Uid		 = user.m_Uid;
		entry->m_Userdata.m_IsGuest  = user.m_IsGuest;
		entry->m_EntityId			 = userId;
		m_entries.addTail(entry);
	}

	entry->m_Skill = skill->m_Skill;
		
}

void netLeaderboardReadData::updateRow(Scoreboard::RowData* row)
{
	
	netRowDataRef entry = findRow(row->m_EntityId);
	if(entry.isNull())
	{
		entry = new netRowData;
		entry->m_Userdata = row->m_Userdata;
		m_entries.addTail(entry);
	}

	entry->m_EntityId		= row->m_EntityId;
	entry->m_ScoreboardId	= row->m_ScoreboardId;
	entry->m_Rank			= row->m_Rank;
	entry->m_Rating			= row->m_Rating;
	entry->m_Columns		= row->m_Columns;

	columnIdsToPropertyIds(entry);
}

void netLeaderboardReadData::clear()
{
	m_entries.clear();
	m_totalEntries = 0;
}

netLeaderboardReadResults::netLeaderboardReadResults()
{}

void netLeaderboardReadResults::clear()
{
	m_leaderboards.clear();
}
	
netRowDataRef netLeaderboardReadResults::findRowData(int id, netUserId user)
{
	netLeaderboardReadDataRef leaderboard = getLeaderboard(id);
	if(leaderboard.isNull()) return NULL;
	return leaderboard->findRow(user);
}
	
netRowDataRef netLeaderboardReadResults::getRowData(int id, int index)
{
	netLeaderboardReadDataRef leaderboard = getLeaderboard(id);
	if(leaderboard.isNull()) return NULL;
	return leaderboard->getRow(index);
}

int netLeaderboardReadResults::countLeaderboardEntries(int id)
{
	netLeaderboardReadDataRef leaderboard = getLeaderboard(id);
	if(leaderboard.isNull()) return 0;
	return leaderboard->countRows();
}

netLeaderboardReadDataRef netLeaderboardReadResults::createLeaderboard(int id)
{
	netLeaderboardReadDataRef leaderboard = getLeaderboard(id);
	
	if(leaderboard.notNull())
	{
		assert(!"<netLeaderboardReader> leaderboard already created.");
		return leaderboard;
	}
	
	leaderboard = netLeaderboardReadData::create(id);
	
	m_leaderboards.addTail(leaderboard);		
	return leaderboard;
}
	
netLeaderboardReadDataRef netLeaderboardReadResults::getLeaderboard(int id)
{
	bdLinkedList<netLeaderboardReadDataRef>::Position pos = m_leaderboards.getHeadPosition();

	while(pos)
	{
		netLeaderboardReadDataRef leaderboard = m_leaderboards.forward(pos);

		if(leaderboard.notNull() && (id == leaderboard->m_leaderboardId))
			return leaderboard;
	} 
	return NULL;
}


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

