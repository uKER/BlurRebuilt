
#include "stdafx.h"
#include "./netPlayerSkillReader.h"
#include "./netLeaderboardReadResult.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netPlayerSkillReader::netPlayerSkillReader(int playerCount)
: netLeaderboardReader(netLeaderboardReadQuery::MODE_SKILL)
{
	NetPlay::C_PlayerSkillReader::Initialise(playerCount);
}

bool netPlayerSkillReader::stopAndCleanup()
{
	StopAndCleanup();
	return true;
}

bool netPlayerSkillReader::startReading()
{
	// sanity check
	assert(m_query->m_type == netLeaderboardReadQuery::MODE_SKILL);

	// watermarck the first scoreboard we are reading from the list
	m_currentBatch = m_nextBatch;
	if(m_currentBatch == NULL) return false;
		
	StopAndCleanup();
	
	// go to the next the scoreboard to read
	netLeaderboardReadQueryDataRef leaderboard = m_query->m_leaderboards.forward(m_nextBatch);

	LOGINFO(netDebug::g_Log, "<netLeaderboardReaderByRank> adding scoreboard(%d)...", leaderboard->m_leaderboardId);

	// fill in user list
	int usersCount = m_query->m_users.getSize();
	SysProfile::UserIdentifier* users = new SysProfile::UserIdentifier[usersCount];
	bdLinkedList<SysProfile::UserIdentifier>::Position pos = m_query->m_users.getHeadPosition();
	int i = 0;
	while (pos)
	{
		users[i] = m_query->m_users.forward(pos);
		i++;
	}

	int leaderboardId = leaderboard->m_leaderboardId;
	int gameModeId = leaderboard->m_gameModeId;

	// game mode is leaderboard on the xbox
	// as skills are read from scoreboards.
	
	// start read
	if(leaderboard.isNull() || !NetPlay::C_PlayerSkillReader::Start(users, usersCount, gameModeId))
	{
		delete[] users;
		return false;
	}

	delete[] users;
	return true;
};

bool netPlayerSkillReader::continueReading(bool& error)
{
	error = false;
	bool finished = NetPlay::C_PlayerSkillReader::Continue(error);

	if(!finished) return true;
	if(error) return false;

	if(m_currentBatch == NULL)
		return false;
	
	// setup the data
	bdLinkedList<netLeaderboardReadQueryDataRef>::Position iterator = m_currentBatch;
	while (iterator != m_nextBatch)
	{
		netLeaderboardReadQueryDataRef lb = m_query->m_leaderboards.forward(iterator);

		int	id = lb->m_leaderboardId;
		netLeaderboardReadResultsRef results  = m_query->m_results;
		netLeaderboardReadDataRef leaderboard = results->getLeaderboard(id);
		if(leaderboard.isNull()) leaderboard  = results->createLeaderboard(id);
		
		// fill in player ratings
		bdLinkedList<SysProfile::UserIdentifier>::Position pos = m_query->m_users.getHeadPosition();
		while (pos)
		{
			SysProfile::UserIdentifier user = m_query->m_users.forward(pos);

			NetPlay::PlayerSkill userData;
			if(!NetPlay::C_PlayerSkillReader::GetSkillFromResults(user, userData))
			{
				error = true;
				return false;
			}

			// update player skill for that leaderboard
			leaderboard->updateSkill(user, netPlatform::Core::instance()->getUserId(user), &userData);
		#if defined(PLATFORM_PC)
			LOGINFO(netDebug::g_Log, "<netLeaderboardReader> lb(%d) : player(%llu), skill(%d).", id, user.m_Uid, userData.m_Skill);
		#else
			LOGINFO(netDebug::g_Log, "<netLeaderboardReader> lb(%d) : player(%s), skill(%d).", id, user.m_Uid.handle.data, userData.m_Skill);
		#endif
		}

		LOGINFO(netDebug::g_Log, "<netLeaderboardReader> read scoreboard(0x%x)...", id);
	}
	error = false;
	return false;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

