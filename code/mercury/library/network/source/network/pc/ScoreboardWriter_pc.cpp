#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/ScoreboardWriter.h>
#include <Network/ScoreboardStructures.h>
#include <Network/Session.h>
#include <Network/DemonwareTask.h>
#include <Network/DemonwareStats.h>
#include <SystemProfile/SystemProfile.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdStats/bdStats.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

namespace Scoreboard
{
	// constructor
	C_Writer::C_Writer() :	m_Initialised(false), m_Writing(false), m_MaxScoreboards(0), m_MaxColumns(0), m_NumScoreboardsToWrite(0), 
							m_NumColumnsToWrite(0), m_pScoreboardData(NULL)
	{
	}


	// destructor
	C_Writer::~C_Writer()
	{
		CleanUp();
	}


	// initialise system and allocate dynamic resources.
	bool C_Writer::Initialise(const U32 maxScoreboardsToWrite, const U32 maxColumns)
	{
		if (m_Initialised)
			return true;

		m_MaxScoreboards = maxScoreboardsToWrite;
		m_MaxColumns = maxColumns; // this wont be used on demonware.

		m_pScoreboardData = reinterpret_cast<void*> (new C_DemonwareWriter[m_MaxScoreboards]);

		if (m_pScoreboardData)
			m_Initialised = true;
		else
			CleanUp();

		return m_Initialised;
	}


	// stop any scoreboard write and reset internal data to default.
	bool C_Writer::StopAndCleanup(void)
	{
		if (!m_Initialised)
			return false;

		// close async task if its active.
		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		// clear any old scoreboard data that is in our structures.
		C_DemonwareWriter* pScoreboardData = reinterpret_cast<C_DemonwareWriter*> (m_pScoreboardData);
		if (pScoreboardData)
		{
			for (U32 i=0; i< m_MaxScoreboards; i++)
				pScoreboardData[i].Flush();
		}

		// setup data ready for new write 
		m_NumScoreboardsToWrite = 0;
		m_NumColumnsToWrite = 0;
		m_Writing = false;

		return true;
	}


	// Queue some scoreboard data for writing.
	bool C_Writer::Queue(const U32 scoreboardId, const S64 rating, const eWriteType writeType, const ColumnData* pColumnData, const U32 numColumns)
	{
		if (!m_Initialised)
			return false;

		if (m_Writing)
		{
			LOGERROR(NetPlay::g_Log, "Can't Queue Scoreboard data when still writing old data to server !!\n");
			return false; // cant Queue Data when writing current data
		}

		if (numColumns)
		{
			assert(pColumnData);
			if (NULL == pColumnData)
				return false;
		}

		if (m_NumScoreboardsToWrite == m_MaxScoreboards)
		{
			LOGERROR(NetPlay::g_Log, "Trying to write to more scoreboards than was originally allocated !!\n");
			return false;
		}

		if (numColumns > MAX_SERVER_SCOREBOARD_COLUMNS)
		{
			LOGERROR(NetPlay::g_Log, "Trying to write to more columns than is possible !!\n");
			return false;
		}

		C_DemonwareWriter* pScoreboardData = reinterpret_cast<C_DemonwareWriter*> (m_pScoreboardData);

		// allocate and configure new scoreboard data structure
		C_DemonwareWriter& newScoreboardData = pScoreboardData[m_NumScoreboardsToWrite++];
		newScoreboardData.Init(rating, scoreboardId, writeType);
		for (U32 i=0; i<numColumns; i++)
			newScoreboardData.AddColumn(pColumnData[i]);

		return true;
	}


	// start the asynchronous write procedure
	bool C_Writer::Start(const SysProfile::UserIdentifier& user, const NetPlay::C_Session& session)
	{
		if (!m_Initialised)
			return false;

		if (m_Writing)
		{
			LOGERROR(NetPlay::g_Log, "Trying to write to scoreboard when previous write not finished. !!\n");
			return false;
		}

		if (0 == m_NumScoreboardsToWrite)
		{
			LOGWARN(NetPlay::g_Log, "Trying to write to scoreboard no data has been Queued !!\n");
			return false;
		}

		if (user.m_IsGuest)
		{
			LOGWARN(NetPlay::g_Log, "Trying to write to scoreboard for a guest player.. !!\n");
			StopAndCleanup();
			return false;
		}

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();

		if (NULL == lobbyService || net->getStatus() != bdNetImpl::BD_NET_DONE || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			StopAndCleanup();
			return false;
		}

		bdStats* pStatsService = lobbyService->getStats();
		if (NULL == pStatsService)
		{
			StopAndCleanup();
			return false;

		}

		bdStatsInfo *stats[20];

		if (m_NumScoreboardsToWrite > 20)
		{
			LOGERROR(NetPlay::g_Log, "Scoreboard::C_Writer::StartWrite() - to many scoreboards being written, only %d bdStatsSubmission structures defined", 20);
			return false;
		}

		// create demonware user
		const bdOnlineUserID dwUserId = user.m_Uid;

		C_DemonwareWriter* pScoreboardData = reinterpret_cast<C_DemonwareWriter*> (m_pScoreboardData);

		// configure stats submission structures.
		for (U32 i=0; i<m_NumScoreboardsToWrite; i++)
		{
			C_DemonwareWriter* pData = &pScoreboardData[i];

			// demonware user id for this player, should be a local player
			pData->m_entityID = (dwUserId); 

			stats[i] = pData;
		}

		m_AsyncTask = pStatsService->writeStats(stats, m_NumScoreboardsToWrite);

		if (!NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
		{
			StopAndCleanup();
			return false;
		}

		m_Writing = true;
		return true;
	}


	// update the asynchronous write and monitor for completion.
	bool C_Writer::Continue(bool& errorOccurred)
	{
		errorOccurred = true;

		if (!m_Initialised)
			return true;

		if (!m_Writing) 
			return true;

		switch (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return false;
			break;
		case NetPlay::DemonwareRemoteTask::eDone:
			errorOccurred = false;
			break;

		default:
			errorOccurred = true;
			break;
		}

		if (errorOccurred)
		{
			LOGWARN(NetPlay::g_Log, "Scoreboard::C_Writer - failed to write stats Error Code %d", m_AsyncTask->getErrorCode());
		}

		StopAndCleanup();
		return true;
	}


	// private function used for cleaning up resources..
	void C_Writer::CleanUp(void)
	{
		StopAndCleanup();

		C_DemonwareWriter* pScoreboardData = reinterpret_cast<C_DemonwareWriter*> (m_pScoreboardData);
		if (pScoreboardData)
		{
			delete [] pScoreboardData;
			m_pScoreboardData = NULL;
		}

		m_Initialised = false;
		m_MaxScoreboards = 0;
		m_MaxColumns = 0;
		m_NumScoreboardsToWrite = 0;
		m_NumColumnsToWrite = 0;
		m_Writing = false;
	}

}// end namespace
