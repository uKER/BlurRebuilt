
#include "stdafx.h"
#include "netLeaderboardReadQuery.h"
#include "netLeaderboardReadResult.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLeaderboardReadQueryData::netLeaderboardReadQueryData(int leaderboardId, int gameModeId)
: m_leaderboardId(leaderboardId)
, m_gameModeId(gameModeId)
{}

bool netLeaderboardReadQueryData::toColumns(Scoreboard::ColumnIdentifierList& columns, Scoreboard::ColumnTypeList& columnTypes)
{
	columns.Clear();
	columnTypes.Clear();
	bdLinkedList<Column>::Position pos = m_columns.getHeadPosition();
	while (pos)
	{
		const Column& c = m_columns.forward(pos);
		columns.PushBack(c.m_id);
		columnTypes.PushBack((Scoreboard::eColumnDataType)(c.m_format));
	}
	return true;
}

void netLeaderboardReadQueryData::clear()
{
	m_columns.clear();
}

netLeaderboardReadQueryData::Column::Column(U32 id, int format)
: m_id(id)
, m_format(format)
{}

void netLeaderboardReadQueryData::add(U32 id, int format) 
{ 
	m_columns.addTail(Column(id, format)); 
}

netLeaderboardReadQuery::netLeaderboardReadQuery()
{
	clear();
	m_batchSize = 5;
	m_batchWaitTime = (5 * 100 * 1000);
}

netLeaderboardReadQuery::~netLeaderboardReadQuery()
{}

void netLeaderboardReadQuery::clear()
{
	m_leaderboards.clear();
	m_users.clear();
	m_titleId = netPlatform::Core::instance()->titleId();
	m_state = STATE_NOT_SET;
	m_type = MODE_NOT_SET;
	m_rankStart = 0;
	m_rankLimit = 0;
	m_pivotRange = 0;
}

void netLeaderboardReadQuery::addUser(SysProfile::UserIdentifier user)
{
	bdLinkedList<SysProfile::UserIdentifier>::Position pos = m_users.getHeadPosition();
	while(pos)
	{
		SysProfile::UserIdentifier u = m_users.forward(pos);
		
		netUserId uid = netPlatform::Core::instance()->getUserId(u);
		netUserId userid = netPlatform::Core::instance()->getUserId(user);
		
		if(uid == userid)
			return;
	}
	m_users.addTail(user);
}


void netLeaderboardReadQuery::addColumn(int leaderboardId, U32 columnId, int format)
{
	bdLinkedList<netLeaderboardReadQueryDataRef>::Position pos = m_leaderboards.getHeadPosition();
	while(pos)
	{
		netLeaderboardReadQueryDataRef leaderboard = m_leaderboards.forward(pos);
		if(leaderboard->m_leaderboardId == leaderboardId)
		{
			leaderboard->add(columnId, format);
			return;
		}		
	}

	netLeaderboardReadQueryDataRef leaderboard = new netLeaderboardReadQueryData(leaderboardId);
	leaderboard->add(columnId, format);
}

void netLeaderboardReadQuery::setRankSearch(const int* scoreboards, int scoreboardsCount, int start, int limit)
{
	m_type		= MODE_RANK;
	m_rankStart = start;
	m_rankLimit = limit;

	m_users.clear();
	m_profile = SysProfile::FullProfile();
	
	m_leaderboards.clear();
	for(int i = 0; i < scoreboardsCount; i ++)
	{
		m_leaderboards.addTail(new netLeaderboardReadQueryData(scoreboards[i]));
		addDefaultColumns(scoreboards[i]);
	}
}

void netLeaderboardReadQuery::setPivotSearch(const int* scoreboards, int scoreboardsCount, const SysProfile::UserIdentifier& pivot, int range)
{
	m_type			= MODE_PIVOT;
	m_pivot			= pivot;
	m_pivotRange	= range;

	m_users.clear();
	
	m_leaderboards.clear();
	for(int i = 0; i < scoreboardsCount; i ++)
	{
		m_leaderboards.addTail(new netLeaderboardReadQueryData(scoreboards[i]));
		addDefaultColumns(scoreboards[i]);
	}
}

void netLeaderboardReadQuery::setUserSearch(const int* scoreboards, int scoreboardsCount, const SysProfile::UserIdentifier* users, int count)
{
	m_type = MODE_USERS;
	
	m_users.clear();
	m_profile = SysProfile::FullProfile();

	for(int i = 0; i < count; i ++)
		m_users.addTail(users[i]);

	m_leaderboards.clear();
	for(int i = 0; i < scoreboardsCount; i ++)
	{
		m_leaderboards.addTail(new netLeaderboardReadQueryData(scoreboards[i]));
		addDefaultColumns(scoreboards[i]);
	}
}

void  netLeaderboardReadQuery::setUserSearch(const int* scoreboards, int scoreboardsCount, const SysProfile::FullProfile& profile)
{
	m_type = MODE_USERS;
	
	m_users.clear();
	m_profile = profile;
	
	m_leaderboards.clear();
	for(int i = 0; i < scoreboardsCount; i ++)
	{
		m_leaderboards.addTail(new netLeaderboardReadQueryData(scoreboards[i]));
		addDefaultColumns(scoreboards[i]);
	}
}
	
void netLeaderboardReadQuery::setSkillSearch(const int* scoreboards, const int* gameModes, int scoreboardsCount, const SysProfile::UserIdentifier* users, int userCount)
{
	m_type = MODE_SKILL;

	m_users.clear();
	m_profile = SysProfile::FullProfile();

	for(int i = 0; i < userCount; i ++)
		m_users.addTail(users[i]);

	m_leaderboards.clear();
	for(int i = 0; i < scoreboardsCount; i ++)
	{
		m_leaderboards.addTail(new netLeaderboardReadQueryData(scoreboards[i], gameModes[i]));
		addDefaultColumns(scoreboards[i]);
	}
}

int netLeaderboardReadQuery::rowCount()
{
	switch(m_type)
	{
	default: 
	case MODE_NOT_SET:
		assert(!"invalid leaderboard search type.");
		return 0;
	case MODE_PIVOT:
		return m_pivotRange;
	case MODE_USERS:
		return m_users.getSize();
	case MODE_RANK:
		return (m_rankLimit - m_rankStart) + 1; 
	case MODE_SKILL:
		return m_users.getSize();
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

