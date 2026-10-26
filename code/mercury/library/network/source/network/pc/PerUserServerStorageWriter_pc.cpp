#include <StdAfx.h>
#include <Network\Core.h>
#include <Network\PerUserServerStorageWriter.h>
#include <bdNet\bdNet.h>
#include <bdLobby\bdLobbyService.h>
#include <bdLobby\bdRemoteTaskManager\bdRemoteTask.h>
#include <bdLobby\bdStorage\bdStorage.h>
#include <SystemProfile\SystemProfile.h>


namespace NetPlay
{
	// constructor
	C_PerUserServerStorageWriter::C_PerUserServerStorageWriter() : m_Writing(false), m_pUploadBuffer(NULL), m_UploadFileSize(0)
	{
	}

	// destructor.
	C_PerUserServerStorageWriter::~C_PerUserServerStorageWriter()
	{
		StopAndCleanup();
	}

	bool C_PerUserServerStorageWriter::Start(const SysProfile::FullProfile& localPlayer, const char* fileName, U8* pUploadBuffer, const U32 uploadFileSize)
	{
		assert(!m_Writing); 

		StopAndCleanup();

		if (NULL == pUploadBuffer || 0 == uploadFileSize)
			return false;

		// check file name for valid size
		const U32 fileNameLen = strlen(fileName);
		if (0 == fileNameLen || (fileNameLen + 1) > MAX_PER_USER_FILE_NAME)
		{
			LOGWARN(g_Log, "C_PerUserServerStorageWriter::Start() - Invalid file name supplied.");
			return false;
		}

		if (SysProfile::eProfileStateOnline != localPlayer.m_State || localPlayer.m_IsGuest)
			return false; // only full online players can upload files to the storage service.

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(g_Log, "C_PerUserServerStorageWriter::Start() - Not connected to demonware lobby service, unable to download file.");
			return false;
		}

		bdStorage* storageService  = lobbyService->getStorage();
		if (NULL == storageService)
			return false;

		// copy data for later.
		m_pUploadBuffer = pUploadBuffer;
		m_UploadFileSize = uploadFileSize;
		strcpy(m_FileName, fileName);

		const bdOnlineUserID ownerID = localPlayer.m_Uid;

		m_AsyncTask = storageService->uploadFile(m_FileName, m_pUploadBuffer, m_UploadFileSize, bdFileInfo::BD_VISIBLE_PUBLIC);
		if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
		{
			m_Writing = true;
			return true;
		}

		StopAndCleanup();
		return false;
	}

	C_PerUserServerStorageWriter::ePerUserStorageWriterState C_PerUserServerStorageWriter::Continue(void)
	{
		if (!m_Writing)
			return eWriteNotActive;

		if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
			return eWriteInProgress;

		ePerUserStorageWriterState ret = eWriteFailed;
		if (BD_NO_ERROR == m_AsyncTask->getErrorCode())
			ret = eWriteSuccessful;
		if (BD_FILESIZE_LIMIT_EXCEEDED == m_AsyncTask->getErrorCode())
		{
			LOGWARN(g_Log, "C_PerUserServerStorageWriter - Unable to upload file as it exceeds the file size limit on the server.");
		}
		
		StopAndCleanup();
		return ret;
	}

	void C_PerUserServerStorageWriter::StopAndCleanup(void)
	{
		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);
		m_Writing = false;
		m_FileName[0] = '\0';
		m_pUploadBuffer = NULL;
		m_UploadFileSize = 0;
	}
}
