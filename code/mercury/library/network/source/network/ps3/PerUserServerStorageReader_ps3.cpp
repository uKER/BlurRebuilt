#include <StdAfx.h>
#include <Network\Core.h>
#include <Network\PerUserServerStorageReader.h>
#include <bdNet\bdNet.h>
#include <bdLobby\bdLobbyService.h>
#include <bdLobby\bdRemoteTaskManager\bdRemoteTask.h>
#include <bdLobby\bdStorage\bdStorage.h>
#include <SystemProfile\SystemProfile.h>
#include "bdLobby/bdLobbyCommon/bdAuthCommon.h"


namespace NetPlay
{
	// constructor
	C_PerUserServerStorageReader::C_PerUserServerStorageReader() : m_Reading(false), m_pDownloadBuffer(NULL), m_DownloadBufferSize(0), m_Searching(false)
	{
	}

	// destructor.
	C_PerUserServerStorageReader::~C_PerUserServerStorageReader()
	{
		StopAndCleanup();
	}

	bool C_PerUserServerStorageReader::Start(const SysProfile::FullProfile& localPlayer, const SysProfile::UserIdentifier& fileOwner, const char* fileName, U8* pDownloadBuffer, const U32 downloadBufferSize)
	{
		assert(!m_Reading); 

		StopAndCleanup();

		if (NULL == pDownloadBuffer || 0 == downloadBufferSize)
			return false;

		// check file name for valid size
		const U32 fileNameLen = strlen(fileName);
		if (0 == fileNameLen || (fileNameLen + 1) > MAX_PER_USER_FILE_NAME)
		{
			LOGWARN(g_Log, "C_PerUserServerStorageReader::Start() - Invalid file name supplied.");
			return false;
		}

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(g_Log, "C_PerUserServerStorageReader::Start() - Not connected to demonware lobby service, unable to download file.");
			return false;
		}

		bdStorage* storageService  = lobbyService->getStorage();
		if (NULL == storageService)
			return false;

		// copy data for later.
		m_pDownloadBuffer = pDownloadBuffer;
		m_DownloadBufferSize = downloadBufferSize;
		strcpy(m_FileName, fileName);

		bdOnlineUserID ownerId = bdAuthUtility::getUserID(fileOwner.m_Uid.handle.data);

		m_AsyncTask = storageService->listFilesByOwner(ownerId, 0, &m_FileInfo, 1, 0, m_FileName);
		if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
		{
			m_Searching = true;
			m_Reading = true;
			return true;
		}

		StopAndCleanup();
		return false;
	}

	C_PerUserServerStorageReader::ePerUserStorageReaderState C_PerUserServerStorageReader::Continue(U32& downloadedSize)
	{
		downloadedSize = 0;

		if (!m_Reading)
			return eReadNotActive;

		switch (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return eReadInProgress;
			break;
		}

		ePerUserStorageReaderState ret = eReadFailed;
		if (m_Searching)
		{
			if (BD_NO_ERROR == m_AsyncTask->getErrorCode())
			{
				if (m_AsyncTask->getNumResults() == 1)
				{
					if (m_DownloadBufferSize >= m_FileInfo.m_fileSize)
					{
						NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

						// attempt to download file.
						m_FileData.m_fileData = m_pDownloadBuffer;
						m_FileData.m_fileSize = m_FileInfo.m_fileSize;

						bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
						if (lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
						{
							bdStorage* storageService  = lobbyService->getStorage();
							if (storageService)
							{
								m_AsyncTask = storageService->getFileByID(m_FileInfo.m_fileID, &m_FileData);
								if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
								{
									m_Searching = false;
									return eReadInProgress;
								}
							}
						}
					}
					else
						ret = eReadDownloadBufferToSmall;
				}
				else
					ret = eReadFileDoesNotExist;
			}
		}
		else
		{	
			if (BD_NO_ERROR == m_AsyncTask->getErrorCode())
			{
				downloadedSize=m_FileData.m_fileSize;
				ret = eReadSuccessful;
			}
			else if (BD_NO_FILE == m_AsyncTask->getErrorCode())
				ret = eReadFileDoesNotExist;
		}

		StopAndCleanup();
		return ret;
	}

	void C_PerUserServerStorageReader::StopAndCleanup(void)
	{
		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		m_Reading = false;
		m_FileName[0] = '\0';
		m_pDownloadBuffer = NULL;
		m_DownloadBufferSize = 0;
		m_Searching = false;
	}
}
