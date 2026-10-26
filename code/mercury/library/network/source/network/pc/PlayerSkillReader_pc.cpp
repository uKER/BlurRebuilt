#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include <Network/PlayerSkillReader.h>
#include <Network/DemonwareTask.h>
#include <Network/PlayerSkill.h>
#include <Systemprofile/SystemProfile.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdLobby/bdMatchMaking/bdPerformanceValue.h>
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>


namespace NetPlay
{
	C_PlayerSkillReader::C_PlayerSkillReader() :	m_Initialised(false), m_Reading(false), m_MaxUsersToQuery(0),
													m_ResultsValid(false), m_GameModeId(0),
													m_pEntities(NULL), m_pResults(NULL), m_NumResults(0) 
	{
	}

	C_PlayerSkillReader::~C_PlayerSkillReader()
	{
		CleanUp();
	}

	bool C_PlayerSkillReader::Initialise(const U32 maxUsersToQuery)
	{
		if (m_Initialised)
			return true;

		m_MaxUsersToQuery = maxUsersToQuery;

		m_pEntities = new bdEntityID[m_MaxUsersToQuery];
		m_pResults = new bdPerformanceValue[m_MaxUsersToQuery];

		if (m_pEntities && m_pResults)
			m_Initialised = true;
		else
			CleanUp();

		return m_Initialised;
	}

	bool C_PlayerSkillReader::Start(const SysProfile::UserIdentifier* pUsers, const U32 numUsers, const U32 gameMode)
	{
		if (!m_Initialised)
			return false;

		if (IsReading())
		{
			LOGWARN(g_Log, "C_PlayerSkillReader::Start() - can't start new read until prevoius read request finishes.");
			return false;
		}

		if (NULL == pUsers || 0 == numUsers)
		{
			LOGERROR(g_Log, "C_PlayerSkillReader::Start() - invalid parameters passed");
			return false;
		}

		if (numUsers > m_MaxUsersToQuery)
		{
			LOGWARN(g_Log, "C_PlayerSkillReader::Start() - too many users passed, need to up the value passed to Initialise().");
			return false;
		}

		// cleanup any previous data.
		StopAndCleanup();

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
			bdMatchMaking* matchmaking = lobbyService->getMatchMaking();
			if (matchmaking)
			{
				m_GameModeId = gameMode;
					
				// translate user profiles into entity ids..
				for (U32 i=0; i<numUsers; i++)
					m_pEntities[i] = pUsers[i].m_Uid;

				m_AsyncTask = matchmaking->getPerformanceValues(m_pEntities, numUsers, gameMode, m_pResults);

				if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
					m_Reading = true;
				else
					StopAndCleanup();
			}
			else
				StopAndCleanup();
		}
		return m_Reading;
	}

	bool C_PlayerSkillReader::Continue(bool& errorOccurred)
	{
		errorOccurred = true;
		if (!m_Initialised)
			return true;

		if (!m_Reading)
			return true;

		switch (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return false;
			break;
		case NetPlay::DemonwareRemoteTask::eFailed:
			StopAndCleanup();
			break;

		default:
			m_NumResults = m_AsyncTask->getNumResults();
			errorOccurred = false;
			m_ResultsValid = true;
			break;
		}

		m_Reading = false;
		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);
		
		return true;
	}

	bool C_PlayerSkillReader::GetSkillFromResults(const SysProfile::UserIdentifier& user, PlayerSkill& skill)
	{
		PlayerSkill defaultSkill;
		skill = defaultSkill;
		if (!m_Initialised)
			return false;

		if (IsReading())
		{
			LOGWARN(g_Log, "C_PlayerSkillReader::GetSkillFromResults() - can't get results while read still active.");
			return false;
		}

		if (!m_ResultsValid)
		{
			LOGWARN(g_Log, "C_PlayerSkillReader::GetSkillFromResults() - can't get results as there is no server data enumerated. assuming default skill.");
			return true;
		}

		for (U32 i=0; i<m_NumResults; i++)
		{
			if (user.m_Uid == m_pResults[i].m_entityID)
			{
				skill.m_Skill = m_pResults[i].m_performanceValue;
				return true;
			}
		}
	
		return true;
	}

	void C_PlayerSkillReader::StopAndCleanup(void)
	{
		if (!m_Initialised)
			return;

		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		m_GameModeId = 0;
		m_ResultsValid = false;
		m_Reading = false;
		m_NumResults = 0;
	}

	void C_PlayerSkillReader::CleanUp(void)
	{
		StopAndCleanup();

		if (m_pResults)
			delete [] m_pResults;
		m_pResults = NULL;

		if (m_pEntities)
			delete [] m_pEntities;
		m_pEntities = NULL;

		m_GameModeId = 0;
		m_ResultsValid = false;
		m_Reading = false;
		m_MaxUsersToQuery = 0;
		m_Initialised = false;
	}
}
