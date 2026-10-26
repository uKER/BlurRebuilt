
#include "stdafx.h"
#include "./netLeaderboardReader.h"
#include "./netLeaderboardReadQuery.h"
#include "netDebug/netDebug.h"
#include "netFriends/netFriends.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLeaderboardReader::netLeaderboardReader(int type)
: m_type(type)
{
	setState(STATE_IDLE);
	setStatus(STATUS_NOT_SET);
}

void netLeaderboardReader::setState(int state)
{
	m_state = state;
	m_step  = 0;
	m_stateWatch.start();
}

bool netLeaderboardReader::clear()
{
	stop();
	setState(STATE_IDLE);
	setStatus(STATUS_NOT_SET);
	m_query = NULL;
	return true;
}

netLeaderboardReader::~netLeaderboardReader()
{
	setState(STATE_IDLE);
	setStatus(STATUS_NOT_SET);
	m_query = NULL;
}

void netLeaderboardReader::setStatus(int status)
{
	m_status = status;

	if(m_query.notNull())
	{
		switch(m_status)
		{
		default:
		case STATUS_NOT_SET:
			break;
		case STATUS_OK:
			m_query->m_state = netLeaderboardReadQuery::STATE_SUCCESS;
			break;
		case STATUS_ERROR:
			m_query->m_state = netLeaderboardReadQuery::STATE_ERROR;
			break;
		case STATUS_ABORTED:
			m_query->m_state = netLeaderboardReadQuery::STATE_ABORTED;
			break;
		}
	
		const char* typeStr;
		switch(m_query->m_type)
		{
		default:
		case netLeaderboardReadQuery::MODE_NOT_SET:
			typeStr = "invalid type.";
			break;
		case netLeaderboardReadQuery::MODE_USERS:
			typeStr = "USERS";
			break;
		case netLeaderboardReadQuery::MODE_RANK:
			typeStr = "RANK";
			break;
		case netLeaderboardReadQuery::MODE_PIVOT:
			typeStr = "PIVOT";
			break;
		case netLeaderboardReadQuery::MODE_SKILL:
			typeStr = "SKILL";
			break;
		}

		switch(m_query->m_state)
		{
		default:
		case netLeaderboardReadQuery::STATE_PENDING:
		case netLeaderboardReadQuery::STATE_SEARCHING:
			//netDebug::logError("<netLeaderboardReader> Ended '%s' search. result='%s'.", typeStr, "invalid result");
			break;
		case netLeaderboardReadQuery::STATE_ABORTED:
			LOGWARN(netDebug::g_Log, "<netLeaderboardReader> Ended '%s' search. result='%s'.", typeStr, "ABORTED");
			break;
		case netLeaderboardReadQuery::STATE_ERROR:
			LOGERROR(netDebug::g_Log, "<netLeaderboardReader> Ended '%s' search. result='%s'.", typeStr, "ERROR");
			break;
		case netLeaderboardReadQuery::STATE_SUCCESS:
			LOGINFO(netDebug::g_Log, "<netLeaderboardReader> Ended '%s' search. result='%s'.", typeStr, "SUCCESS");
			break;
		}
	}
}

bool netLeaderboardReader::update()
{
	switch(m_state)
	{
	// ground zero
	default:
	case STATE_IDLE:
		{
			break;
		}
	// reading stats
	case STATE_ENUMERATING_FRIENDS:
		{
			// sanity check
			if(m_query.isNull())
			{
				assert(false);
				setState(STATE_PAUSING);
				setStatus(STATUS_ERROR);
				return true;
			}

			// first, start.
			if(m_step == 0)
			{
				bool error;
				if(!startFriendsEnumeration(error))
					return true;

				if(error)
				{
					setState(STATE_PAUSING);
					setStatus(STATUS_ERROR);
					return true;
				}

				LOGINFO(netDebug::g_Log, "<netLeaderboardReader> friend enumeration started...");
				m_step++;
			}

			// poll reads
			if(m_step == 1)
			{
				bool error;
				if(continueFriendsEnumeration(error))
					return true;

				// error. dealy a bit then leave with error
				if(error)
				{
					LOGERROR(netDebug::g_Log, "<netLeaderboardReader> friend enumeration error!");
					setState(STATE_PAUSING);
					setStatus(STATUS_ERROR);
					return true;
				}
				else
				{
					LOGINFO(netDebug::g_Log, "<netLeaderboardReader> friend enumeration finished!");
					setState(STATE_READING);
					return true;
				}
			}
		}
	case STATE_READING:
		{
			// sanity check
			if(m_query.isNull())
			{
				assert(false);
				setState(STATE_PAUSING);
				setStatus(STATUS_ERROR);
				return true;
			}
			
			// first, start.
			if(m_step == 0)
			{
				if(!startReading())
				{
					setState(STATE_PAUSING);
					setStatus(STATUS_ERROR);
					return true;
				}
				LOGINFO(netDebug::g_Log, "<netLeaderboardReader> read started...");
				m_step++;
			}
			
			// poll reads
			if(m_step == 1)
			{
				bool error;
				if(!continueReading(error))
				{
					// error. dealy a bit then leave with error
					if(error)
					{
						LOGERROR(netDebug::g_Log, "<netLeaderboardReader> read error!");
						setState(STATE_PAUSING);
						setStatus(STATUS_ERROR);
						return true;
					}
					else
					{
						LOGINFO(netDebug::g_Log, "<netLeaderboardReader> read finished!");

						// no more to do. exit after the pause
						// else, delay a bit and search next batch
						if(m_nextBatch == NULL)
							setStatus(STATUS_OK);
							
						setState(STATE_PAUSING);
						return true;
					}
				}
			}
			break;
		}
	// pause after a read. 
	// at the end of pause, either exit the task, 
	// or read next batch.
	case STATE_PAUSING:
		{
			
			// reads next batch.
			if(m_stateWatch.elapsed(m_query->m_batchWaitTime))
			{
				// we've finished with success / error / aborted
				// go idle.
				if(m_status != STATUS_NOT_SET)
				{
					setState(STATE_IDLE);
				}
				else
				{
					// start a new batchj of scoreboards (only one).
					m_currentBatch = m_nextBatch;

					// sanity checks
					assert(m_currentBatch != NULL);
					assert(m_query.notNull());

					// read next batch
					setState(STATE_READING);
				}
			}
			break;
		}
	}
	return true;
}

bool netLeaderboardReader::stop()
{
	stopAndCleanup();
	
	if(m_state == STATE_READING)
	{
		setState(STATE_PAUSING);
	}
	if(m_status == STATUS_NOT_SET)
	{
		setStatus(STATUS_ABORTED);
	}
	return true;
}

bool netLeaderboardReader::start(netLeaderboardReadQueryRef query)
{
	assert(m_query.isNull());
	assert(query->m_results);
	assert(query->m_state == netLeaderboardReadQuery::STATE_PENDING);
	
	m_query	= query;
	m_query->m_state = netLeaderboardReadQuery::STATE_SEARCHING;
	
	m_currentBatch = m_query->m_leaderboards.getHeadPosition();
	m_nextBatch = m_currentBatch;
	
	const char* typeStr;
	switch(m_query->m_type)
	{
	default:
	case netLeaderboardReadQuery::MODE_NOT_SET:
		typeStr = "invalid type.";
		break;
	case netLeaderboardReadQuery::MODE_USERS:
		typeStr = "USERS";
		break;
	case netLeaderboardReadQuery::MODE_RANK:
		typeStr = "RANK";
		break;
	case netLeaderboardReadQuery::MODE_PIVOT:
		typeStr = "PIVOT";
		break;
	case netLeaderboardReadQuery::MODE_SKILL:
		typeStr = "SKILL";
		break;
	}

	LOGINFO(netDebug::g_Log, "<netLeaderboardReader> search started '%s'...", typeStr);
	
	switch(m_query->m_profile.m_State)
	{
	default:
		{
			setState(STATE_READING);
			setStatus(STATUS_NOT_SET);
			return true;
		}
	case SysProfile::eProfileStateOnline:
		{
			assert(!m_query->m_profile.m_IsGuest);
			setState(STATE_READING);
			//setState(STATE_ENUMERATING_FRIENDS);
			return true;
		}
	}
}

bool netLeaderboardReader::startFriendsEnumeration(bool& error)
{
	if(m_query.isNull())
	{
		error = true;
		return true;
	}
	// start enumeration
	MEMORY_TRACKER_PUSH("network.multiplayer");
	m_friends = new netFriends;
	MEMORY_TRACKER_POP("network.multiplayer");
	
	if(!m_friends->enumerateFriends(m_query->m_profile))
	{
		error = true;
		return true;
	}
	return true;
}
			
bool netLeaderboardReader::continueFriendsEnumeration(bool& error)
{
	if(m_query.isNull())
	{
		m_friends = NULL;
		error = true;
		return false;
	}
	
	// meh.
	if(m_friends.isNull())
	{
		error = true;
		return false;
	}

	m_friends->update();

	// still enumerating
	if(m_friends->enumerating())
		return true;

	if(!m_friends->success())
	{
		m_friends = NULL;
		error = true;
		return false;
	}

	m_query->addUser(m_query->m_profile);
	for(int i = 0; i < m_friends->getFriendsCount(); i ++)
	{
		netFriend* f = m_friends->getFriend(i);
		if(f == NULL) continue;
		m_query->addUser(f->m_Profile);
	}

	m_friends = NULL;
	error = false;
	return false;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

