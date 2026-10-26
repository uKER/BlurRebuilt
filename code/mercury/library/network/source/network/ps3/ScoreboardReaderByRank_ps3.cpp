#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/ScoreboardReaderByRank.h>
#include <Network/ScoreboardStructures.h>
#include <Network/DemonwareStats.h>
#include <Network/DemonwareTask.h>
#include <systemprofile/SystemProfile.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdStats/bdStats.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include "bdLobby/bdLobbyCommon/bdAuthCommon.h"


namespace Scoreboard
{
	// constructor
	C_ReaderByRank::C_ReaderByRank() :	m_Initialised(false), m_Reading(false), m_MaxScoreboardsToReadFrom(1), m_NumScoreboardsToQuery(0), 
		m_pRequriredScoreboards(NULL)
	{
	}


	// destructor
	C_ReaderByRank::~C_ReaderByRank()
	{
		CleanUp();
	}


	// create dynamic resources.
	bool C_ReaderByRank::Initialise(void)
	{
		if (m_Initialised)
			return true;

		assert(m_MaxScoreboardsToReadFrom == 1);
		m_pRequriredScoreboards = reinterpret_cast<void*> (new C_DemonwareReader);

		if (m_pRequriredScoreboards)
			m_Initialised = true;
		else
			CleanUp();

		return m_Initialised;
	}


	// add a scoreboard to the list of what needs to be queried.
	bool C_ReaderByRank::AddScoreboard(const U32 scoreboardId, const ColumnIdentifierList& columns, const ColumnTypeList& columnTypes, const bool flushCache)
	{
		if (!m_Initialised)
			return false;

		assert(!m_Reading);
		if (m_Reading)
			return false;

		if (flushCache)
		{
			// kill any prevous scoreboard data.
			StopAndCleanup(); 
			m_NumScoreboardsToQuery = 0;
		}

		// can we actually query anymore scoreboards
		if (m_NumScoreboardsToQuery >= m_MaxScoreboardsToReadFrom)
		{
			LOGERROR(NetPlay::g_Log, "Trying to read from too many scoreboards with 1 server call !\n");
			return false;
		}

		// configire reader with column types and scoreboard Id that we will query.
		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		pReader->Flush();
		pReader->Configure(columnTypes, scoreboardId);
		m_NumScoreboardsToQuery += 1;

		return true;
	}


	// private function to de allocate all dynamic cesources
	void C_ReaderByRank::CleanUp(void)
	{
		StopAndCleanup();

		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (pReader)
		{
			delete pReader;
			m_pRequriredScoreboards = NULL;
		}

		m_NumScoreboardsToQuery = 0;
		m_Initialised = false;
		m_Reading = false;
	}


	// request scoreboard data starting at a specific rank.
	bool C_ReaderByRank::StartReadByRank(const U32 titleId, const U32 startRank, const U32 maxRowsToRetrieve)
	{
		if (!m_Initialised)
			return false;

		// stop any old server request
		StopAndCleanup();

		if (0 == m_NumScoreboardsToQuery || 0 == maxRowsToRetrieve)
			return false;

		const U32 r = (startRank == 0) ? 1 : startRank; // ranks start at 1 not 0.

		// configure reader with the number of rows we will request. dynamic allocation.
		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (!pReader->Configure(maxRowsToRetrieve))
		{
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

		// start server request.
		LOGINFO(NetPlay::g_Log, "C_ReaderByRank::StartReadByRank() - Starting enumeration. Scoreboard:%d    Columns per row: %d", pReader->m_ScoreboardId, pReader->m_ColumnTypes.Size());

		m_AsyncTask = pStatsService->readStatsByRank(pReader->m_ScoreboardId, r, pReader->Data(), maxRowsToRetrieve);

		m_Reading = NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask);
		if (!m_Reading)
			StopAndCleanup();

		return m_Reading;
	}


	// start a read request based on a users pivot point in then scoreboard.
	bool C_ReaderByRank::StartReadByPivot(const U32 titleId, const SysProfile::UserIdentifier& userPivot, const U32 maxRowsToRetrieve)
	{
		if (!m_Initialised)
			return false;

		// stop any old server request
		StopAndCleanup();

		if (0 == m_NumScoreboardsToQuery || 0 == maxRowsToRetrieve)
			return false;

		// configure reader with the number of rows we will request. dynamic allocation.
		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (!pReader->Configure(maxRowsToRetrieve))
		{
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

		// create a user id based on the supplied sceNpId. this will be used as the pivot for the returned results.
		const bdOnlineUserID userId = bdAuthUtility::getUserID(userPivot.m_Uid.handle.data);	

		// start server request.
		LOGINFO(NetPlay::g_Log, "C_ReaderByRank::StartReadByPivot() - Starting enumeration. Scoreboard:%d    Columns per row: %d", pReader->m_ScoreboardId, pReader->m_ColumnTypes.Size());

		m_AsyncTask = pStatsService->readStatsByPivot(pReader->m_ScoreboardId, userId, pReader->Data(), maxRowsToRetrieve);

		m_Reading = NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask);

		if (!m_Reading)
			StopAndCleanup();

		return m_Reading;
	}


	// continue asynchronous read operation.
	bool C_ReaderByRank::Continue(bool& errorOccurred)
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
		case NetPlay::DemonwareRemoteTask::eDone:
			errorOccurred = false;
			break;

		default:
			errorOccurred = true;
			break;
		}

		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);

		if (!errorOccurred)
		{
			pReader->m_NumRowsReturned = m_AsyncTask->getNumResults();
			pReader->m_TotalScoreboardEnteries = m_AsyncTask->getTotalNumResults();
			NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);
			m_Reading = false;
		}
		else
		{
			LOGWARN(NetPlay::g_Log, "C_ReaderByRank::ContinueRead() - Failed to enumerate scoreboard %d Code %d", pReader->m_ScoreboardId, m_AsyncTask->getErrorCode());
			StopAndCleanup();
		}

		return true;
	}


	// extract row data from scoreboard read results..
	U32 C_ReaderByRank::ExtractRows(const U32 scoreboardId, RowData* pOutput, const U32 maxRowsToExtract, U32& scoreboardSize)
	{
		scoreboardSize  = 0;

		if (!m_Initialised)
			return 0;

		assert(pOutput);
		assert(maxRowsToExtract);
		if (NULL == pOutput || 0 == maxRowsToExtract)
			return 0;

		// make sure we are not still reading the data from the server.
		if (m_Reading)
		{
			LOGERROR(NetPlay::g_Log, "can't extract scoreboard rows when data is still being read from the server!\n");
			return 0;
		}

		U32 rowCount = 0;	

		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (pReader->m_NumRowsReturned > 0)
		{
			if (scoreboardId == pReader->m_ScoreboardId)
			{
				// how many users are currently on this scoreboard
				scoreboardSize = pReader->m_TotalScoreboardEnteries;

				// how many rows do we need to give back to the game
				rowCount = BD_MIN(maxRowsToExtract , pReader->m_NumRowsReturned);

				if (pReader->m_NumRowsReturned > maxRowsToExtract)
					LOGWARN(NetPlay::g_Log, "Scoreboard results contain more rows than you can extract!\n");

				// extract all rows.
				for (U32 i=0; i< rowCount; i++)
				{
					const C_DemonwareRow& rowIn = pReader->m_pRows[i];
					RowData& rowOut = pOutput[i];

					// set scoreboard identifier.
					rowOut.m_ScoreboardId =	pReader->m_ScoreboardId;

					// set name
					for(int i = 0; i < SYSTEM_PROFILE_NAME_LENGTH; i ++)
						rowOut.m_Userdata.m_Name[i] = (WCHAR)rowIn.m_entityName[i];
					rowOut.m_Userdata.m_Name[SYSTEM_PROFILE_NAME_LENGTH-1] = 0;

					// no sceNpId returned via demonware..
					memset(&rowOut.m_Userdata.m_Uid, 0, sizeof(rowOut.m_Userdata.m_Uid));
		
					// can't be a guest if on a scoreboard.
					rowOut.m_Userdata.m_IsGuest =	false;

					// set entity id.
					rowOut.m_EntityId =	rowIn.m_entityID;

					// set rank.
					rowOut.m_Rank =	rowIn.m_rank;

					// set rating.
					rowOut.m_Rating	= rowIn.m_rating;

					// set column data.
					rowOut.m_Columns = rowIn.m_ColumnData;
				}
			}
			else
			{
				LOGWARN(NetPlay::g_Log, "C_ReaderByRank::ExtractRows() -> requested scoreboard (%d) is not the same as what was originally enumerated (%d) !.", scoreboardId, pReader->m_ScoreboardId);
			}
		}

		return rowCount;
	}


	// stop asynchronous task and release release restlts buffer.
	void C_ReaderByRank::StopAndCleanup(void)
	{
		if (!m_Initialised)
			return;

		// kill async task
		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		// kill enumeration data.
		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (pReader)
			pReader->Flush();

		m_Reading = false;
	}
}// end namespace	
