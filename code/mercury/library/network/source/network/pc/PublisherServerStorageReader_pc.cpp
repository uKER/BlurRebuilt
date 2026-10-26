#include <StdAfx.h>
#include <Network\Core.h>
#include <Network\PublisherServerStorageReader.h>
#include <bdNet\bdNet.h>
#include <bdLobby\bdLobbyService.h>
#include <bdLobby\bdRemoteTaskManager\bdRemoteTask.h>
#include <bdLobby\bdStorage\bdStorage.h>
#include <SystemProfile\SystemProfile.h>

namespace NetPlay
{
	// constructor
	C_PublisherServerStorageReader::C_PublisherServerStorageReader() : m_Reading(false), m_pDownloadBuffer(NULL), m_DownloadBufferSize(0)
	{
	}

	// destructor.
	C_PublisherServerStorageReader::~C_PublisherServerStorageReader()
	{
		StopAndCleanup();
	}

	bool C_PublisherServerStorageReader::Start(const SysProfile::FullProfile& localPlayer, const char* fileName, U8* pDownloadBuffer, const U32 downloadBufferSize)
	{
		assert(!m_Reading); 

		StopAndCleanup();

		if (NULL == pDownloadBuffer || 0 == downloadBufferSize)
			return false;

		// check file name for valid size
		const U32 fileNameLen = strlen(fileName);
		if (0 == fileNameLen || (fileNameLen + 1) > MAX_PER_USER_FILE_NAME)
		{
			LOGWARN(g_Log, "C_PublisherServerStorageReader::Start() - Invalid file name supplied.");
			return false;
		}

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(g_Log, "C_PublisherServerStorageReader::Start() - Not connected to demonware lobby service, unable to download file.");
			return false;
		}

		bdStorage* storageService = lobbyService->getStorage();
		if (NULL == storageService)
			return false;

		// copy data for later.
		m_pDownloadBuffer = pDownloadBuffer;
		m_DownloadBufferSize = downloadBufferSize;

		m_AsyncTask = storageService->getFile(fileName, &m_FileData);
		if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
		{
			m_Reading = true;
			return true;
		}

		StopAndCleanup();
		return false;
	}

	C_PublisherServerStorageReader::ePerUserStorageReaderState C_PublisherServerStorageReader::Continue(U32& downloadedSize)
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
		if (BD_NO_ERROR == m_AsyncTask->getErrorCode())
			ret = eReadSuccessful;
		else if (BD_NO_FILE == m_AsyncTask->getErrorCode())
			ret = eReadFileDoesNotExist;

		StopAndCleanup();
		return ret;
	}

	void C_PublisherServerStorageReader::StopAndCleanup(void)
	{
		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		m_Reading = false;
		m_pDownloadBuffer = NULL;
		m_DownloadBufferSize = 0;
	}
}
