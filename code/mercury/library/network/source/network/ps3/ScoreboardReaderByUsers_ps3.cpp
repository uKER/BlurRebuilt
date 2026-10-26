#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include <Network/ScoreboardReaderByUsers.h>
#include <Network/ScoreboardStructures.h>
#include <Network/DemonwareStats.h>
#include <Network/DemonwareTask.h>
#include <systemprofile/SystemProfile.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdStats/bdStats.h>
#include <bdlobby/bdRemoteTaskManager/bdRemoteTask.h>
#include "bdLobby/bdLobbyCommon/bdAuthCommon.h"

namespace Scoreboard
{
	// constructor
	C_ReaderByUsers::C_ReaderByUsers() :	m_Initialised(false), m_Reading(false), m_MaxScoreboardsToReadFrom(1), m_MaxUsersToQuery(0), 
		m_NumUsersToQuery(0), m_NumScoreboardsToQuery(0), m_pRequiredUsers(NULL), 
		m_pRequriredScoreboards(NULL)
	{
	}


	// destructor
	C_ReaderByUsers::~C_ReaderByUsers()
	{
		CleanUp();
	}


	// alocate all dynamic resources.
	bool C_ReaderByUsers::Initialise(const U32 maxUserstoQuery)
	{
		if (m_Initialised)
			return true;

		m_MaxUsersToQuery = (maxUserstoQuery == 0) ? MAX_FRIENDS+1 : maxUserstoQuery;

		assert(m_MaxScoreboardsToReadFrom == 1); // using demonware we can only query 1 scoreboard with 1 api call.

		// allocate user storage
		m_pRequiredUsers = reinterpret_cast<void*> (new bdOnlineUserID[m_MaxUsersToQuery]);

		// allocate scoreboard enumerator helper
		m_pRequriredScoreboards = reinterpret_cast<void*> (new C_DemonwareReader);

		if (m_pRequiredUsers && m_pRequriredScoreboards)
			m_Initialised = true;
		else
			CleanUp();

		return m_Initialised;
	}


	// set user data before we start reading.
	bool C_ReaderByUsers::SetUsers(const SysProfile::UserIdentifier* pUsers, const U32 numUsers)
	{
		if (!m_Initialised)
			return false;

		// make sure we cant modify players while still requesting server data
		assert(m_Reading == false);
		if (m_Reading)
			return false; 

		// check input data
		assert(pUsers);
		assert(numUsers);
		if(NULL == pUsers || 0 == numUsers)
			return false;

		// make sure we not passing in to many users.
		if (numUsers > m_MaxUsersToQuery)
		{
			LOGERROR(NetPlay::g_Log, "Trying to read too many users in 1 server call!\n");
			return false;
		}

		// convert sceNpId into bdOnlineUserId and store for later.
		bdOnlineUserID* pStorage = reinterpret_cast<bdOnlineUserID*> (m_pRequiredUsers);
		m_NumUsersToQuery = numUsers;
		for (U32 i=0; i< numUsers; i++)
			pStorage[i] = bdAuthUtility::getUserID(pUsers[i].m_Uid.handle.data);;

		return true;
	}


	// add a scoreboard to the list of what needs to be queried.
	bool C_ReaderByUsers::AddScoreboard(const U32 scoreboardId, const ColumnIdentifierList& columns, const ColumnTypeList& columnTypes, const bool flushCache)
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


	// stop async task and release results bufer is active.
	void C_ReaderByUsers::StopAndCleanup(void)
	{
		if (!m_Initialised)
			return;

		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		// kill enumeration data.
		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (pReader)
			pReader->Flush();

		m_Reading = false;
	}


	// try and start requesting data from the scoreboard server
	bool C_ReaderByUsers::Start(const U32 titleId)
	{
		if (!m_Initialised)
			return false;

		// stop any previous read and release results buffer..
		StopAndCleanup();

		// no input data setup so cant query scoreboards..
		if (0 == m_NumScoreboardsToQuery || 0 == m_NumUsersToQuery)
			return false; 

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

		// configure reader with the number of rows we will request. dynamic allocation.
		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (!pReader->Configure(m_NumUsersToQuery))
		{
			StopAndCleanup();
			return false;
		}

		LOGINFO(NetPlay::g_Log, "C_ReaderByUsers::StartRead() - Starting enumeration. Scoreboard:%d    Columns per row: %d", pReader->m_ScoreboardId, pReader->m_ColumnTypes.Size());
		bdOnlineUserID* pUsers = reinterpret_cast<bdOnlineUserID*> (m_pRequiredUsers);

		m_AsyncTask = pStatsService->readStatsByEntityID(pReader->m_ScoreboardId, pUsers, m_NumUsersToQuery, pReader->Data());

		m_Reading = NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask);

		if (!m_Reading)
			StopAndCleanup();

		return m_Reading;
	}


	// continue asynchronous read operation.
	bool C_ReaderByUsers::Continue(bool& errorOccurred)
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
			pReader->m_NumRowsReturned = m_AsyncTask->getNumResults(); // set number of rows received.
			NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);
			m_Reading = false;
		}
		else
		{
			LOGWARN(NetPlay::g_Log, "C_ReaderByUsers::ContinueRead() - Failed to enumerate scoreboard %d", pReader->m_ScoreboardId);
			StopAndCleanup();
		}

		return true;
	}


	// extract row data from scoreboard read results..
	U32 C_ReaderByUsers::ExtractRows(const U32 scoreboardId, RowData* pOutput, const U32 maxRowsToExtract)
	{
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

					// set SceNpId to default since we don't get it back from the server when using demonware.
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
				LOGWARN(NetPlay::g_Log, "C_ReaderByUsers::ExtractRows() -> requested scoreboard (%d) is not the same as what was originally enumerated (%d) !.", scoreboardId, pReader->m_ScoreboardId);
			}
		}

		return rowCount;
	}

	bool C_ReaderByUsers::ExtractRowForUser(const SysProfile::UserIdentifier& user,const U32 scoreboardId, RowData& rowOut)
	{
		if (!m_Initialised)
			return false;

		// make sure we are not still reading the data from the server.
		if (m_Reading)
		{
			LOGERROR(NetPlay::g_Log, "can't extract scoreboard rows when data is still being read from the server!\n");
			return false;
		}

		const bdOnlineUserID userId = bdAuthUtility::getUserID(user.m_Uid.handle.data);

		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (pReader->m_NumRowsReturned > 0)
		{
			if (scoreboardId == pReader->m_ScoreboardId)
			{
				for (U32 i=0; i< pReader->m_NumRowsReturned; i++)
				{
					const C_DemonwareRow& rowIn = pReader->m_pRows[i];

					if (userId != rowIn.m_entityID)
						continue; //not user we are looking for.

					// set scoreboard identifier.
					rowOut.m_ScoreboardId =	pReader->m_ScoreboardId;

					// set name
					for(int i = 0; i < SYSTEM_PROFILE_NAME_LENGTH; i ++)
						rowOut.m_Userdata.m_Name[i] = (WCHAR)rowIn.m_entityName[i];
					rowOut.m_Userdata.m_Name[SYSTEM_PROFILE_NAME_LENGTH-1] = 0;

					// set SceNpId to default since we don't get it back from the server when using demonware.
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

					return true;
				}
			}
			else
			{
				LOGWARN(NetPlay::g_Log, "C_ReaderByUsers::ExtractRowForUser() -> requested scoreboard (%d) is not the same as what was originally enumerated (%d) !.", scoreboardId, pReader->m_ScoreboardId);
			}
		}

		return false;
	}

	// private function to release resources
	void C_ReaderByUsers::CleanUp(void)
	{
		StopAndCleanup();

		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		bdOnlineUserID *pUsers = reinterpret_cast<bdOnlineUserID*> (m_pRequiredUsers);
		if (pUsers)
			delete [] pUsers;
		m_pRequiredUsers = NULL;

		C_DemonwareReader* pReader = reinterpret_cast<C_DemonwareReader*> (m_pRequriredScoreboards);
		if (pReader)
			delete pReader;
		m_pRequriredScoreboards = NULL;

		m_NumScoreboardsToQuery = 0;
		m_NumUsersToQuery = 0;
		m_Initialised = false;
		m_Reading = false;
	}
}// end namespace	
