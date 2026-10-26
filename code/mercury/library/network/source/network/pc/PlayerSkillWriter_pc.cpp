#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include <Network/PlayerSkillWriter.h>
#include <Network/PlayerSkill.h>
#include <Network/Session.h>
#include <Network/DemonwareTask.h>
#include <Systemprofile/SystemProfile.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdLobby/bdMatchMaking/bdPerformanceValue.h>

namespace NetPlay
{
	C_PlayerSkillWriter::C_PlayerSkillWriter() :	m_Initialised(false), m_Writing(false), m_MaxUsersToWrite(0), m_GameModeId(0),
													m_CurrentIndex(0), m_pWriteCache(NULL)
	{
	}

	C_PlayerSkillWriter::~C_PlayerSkillWriter()
	{
		CleanUp();
	}

	bool C_PlayerSkillWriter::Initialise(const U32 maxUsersToWrite)
	{
		if (m_Initialised)
			return true;

		if (0 == maxUsersToWrite)
			return false; // not valid.

		m_MaxUsersToWrite = maxUsersToWrite;
		m_pWriteCache = new bdPerformanceValue[m_MaxUsersToWrite];
		if (m_pWriteCache)
			m_Initialised =true;
		else
			CleanUp();

		return m_Initialised;
	}

	bool C_PlayerSkillWriter::Start(const C_Session& session, const PlayerSkillSubmission* pSkillData, const U32 numUsers, const U32 gameMode)
	{
		if (!m_Initialised)
			return false;

		if (IsWriting())
		{
			LOGWARN(g_Log, "C_PlayerSkillWriter::Start() - can't start new write until prevoius read request finishes.");
			return false;
		}

		if (NULL == pSkillData || 0 == numUsers)
		{
			LOGERROR(g_Log, "C_PlayerSkillWriter::Start() - invalid parameters passed.");
			return false;
		}

		if (numUsers > m_MaxUsersToWrite)
		{
			LOGERROR(g_Log, "C_PlayerSkillWriter::Start() - trying to write too many players, increase the value passed into Initialise() to allocate more storage.");
			return false;
		}

		// validate passed in data,m can't write skill for guest players.
		for (U32 i=0; i<numUsers; i++)
		{
			if (pSkillData[i].m_User.m_IsGuest)
			{
				LOGERROR(g_Log, "C_PlayerSkillWriter::Start() - We can't write skill data for guest players!");
				return false;
			}
		}

		// cleanup any previous data. if applicable.
		StopAndCleanup();

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
			bdMatchMaking* matchmaking = lobbyService->getMatchMaking();
			if (matchmaking)
			{
				// build data to write..
				for (U32 i=0; i<numUsers; i++)
				{
					m_pWriteCache[i].m_entityID = pSkillData[i].m_User.m_Uid;
					m_pWriteCache[i].m_performanceValue = pSkillData[i].m_Rating;
				}

				m_GameModeId = gameMode;
				m_AsyncTask = matchmaking->submitPerformance(m_GameModeId, m_pWriteCache, numUsers);

				if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
				{
					// server write started successfully.
					m_CurrentIndex = 0;
					m_Writing = true;
					return true;
				}
			}
		}

		StopAndCleanup();
		return false;
	}

	bool C_PlayerSkillWriter::Continue(bool& errorOccurred)
	{
		errorOccurred = true;
		if (!m_Initialised)
			return true;

		if (!IsWriting())
			return true;

		bool error = false;
		
		switch (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return false;
			break;

		case NetPlay::DemonwareRemoteTask::eFailed:
			error = true;
			break;

		default:
			break;
		}

		if (error)
		{
			LOGWARN(g_Log, "C_PlayerSkillWriter::Continue() - failed to write skill data. error code %d", m_AsyncTask->getErrorCode());
		}

		StopAndCleanup();
		errorOccurred = error;
		return true;
	}

	void C_PlayerSkillWriter::StopAndCleanup(void)
	{
		if (!m_Initialised)
			return;

		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		m_Writing = false;
		m_GameModeId = 0;
		m_CurrentIndex = 0;
	}

	void C_PlayerSkillWriter::CleanUp(void)
	{
		StopAndCleanup();

		if (m_pWriteCache)
			delete [] m_pWriteCache;
		m_pWriteCache = NULL;

		m_MaxUsersToWrite = 0;
		m_Initialised = 0;
	}
}