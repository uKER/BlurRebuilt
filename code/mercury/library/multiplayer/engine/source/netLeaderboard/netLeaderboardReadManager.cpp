
#include "stdafx.h"
#include "netLeaderboardReadManager.h"
#include "netPlayerSkillReader.h"
#include "netLeaderboardReaderByRank.h"
#include "netLeaderboardReaderByUsers.h"
#include "netLeaderboardReadResult.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLeaderboardReadManager::netLeaderboardReadManager()
{
	m_joinedResults = new netLeaderboardReadResults();
}

netLeaderboardReadManager::~netLeaderboardReadManager()
{
	stop();
}

netLeaderboardReadResultsRef netLeaderboardReadManager::database()
{
	return m_joinedResults;
}

bool netLeaderboardReadManager::busy() const 
{ 
	return (!m_pendingQueries.isEmpty()) || (!m_searches.isEmpty()); 
}

bool netLeaderboardReadManager::queueQuery(netLeaderboardReadQueryRef query)
{
	query->m_state = netLeaderboardReadQuery::STATE_PENDING;
	m_pendingQueries.addTail(query);
	return true;
}

bool netLeaderboardReadManager::stop()
{
	// join the first game with matching name
	bdLinkedList<netLeaderboardReaderRef>::Position pos = m_searches.getHeadPosition();
	while(pos)
	{
		netLeaderboardReaderRef search = m_searches.forward(pos);
		search->stop();
	}

	// join the first game with matching name
	bdLinkedList<netLeaderboardReadQueryRef>::Position p = m_pendingQueries.getHeadPosition();
	while(p)
	{
		netLeaderboardReadQueryRef query = m_pendingQueries.forward(p);
		query->m_state = netLeaderboardReadQuery::STATE_ABORTED;
	}
	m_pendingQueries.clear();
	return true;
}

bool netLeaderboardReadManager::clear()
{
	stop();
	m_joinedResults->clear();
	return true;
}

bool netLeaderboardReadManager::update()
{
	// join the first game with matching name
	bdLinkedList<netLeaderboardReaderRef>::Position pos = m_searches.getHeadPosition();
	
	while(pos)
	{
		netLeaderboardReaderRef search = m_searches.getAt(pos);

		search->update();
	
		if(!search->busy())
		{
			m_searches.removeAt(pos);
		}
		else
		{
			m_searches.forward(pos);
		}
	}

	int searchCount = m_searches.getSize();
	int maxSearchCount = 1;

	while((searchCount < maxSearchCount) && (!m_pendingQueries.isEmpty()))
	{
		netLeaderboardReadQueryRef query = m_pendingQueries.getHead();
		m_pendingQueries.removeHead();

		if(query->m_results.isNull())
			query->m_results = m_joinedResults;

		netLeaderboardReaderRef reader;
		switch(query->m_type)
		{
		default:
			{
				assert(!"Invalid leaderboard query.");
				query->m_state = netLeaderboardReadQuery::STATE_ERROR;
				break;
			}
		case netLeaderboardReadQuery::MODE_USERS:
			{
				reader = new netLeaderboardReaderByUsers();
				break;
			}
		case netLeaderboardReadQuery::MODE_RANK:
			{
				reader = new netLeaderboardReaderByRank();
				break;
			}
		case netLeaderboardReadQuery::MODE_PIVOT:
			{
				reader = new netLeaderboardReaderByRank();
				break;
			}
		case netLeaderboardReadQuery::MODE_SKILL:
			{
				reader = new netPlayerSkillReader(query->m_users.getSize());
				break;
			}
		}

		if(reader.notNull())
		{
			reader->start(query);
			m_searches.addTail(reader);
			searchCount = m_searches.getSize();
		}				
	}
	return true;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

