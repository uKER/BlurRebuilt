#include <StdAfx.h>
#include <SystemProfile/SystemProfile.h>
#include <Network/ScoreboardAttachmentReader.h>
#include <Network/PerUserServerStorageReader.h>


namespace Scoreboard
{
	// constructor
	C_AttachmentReader::C_AttachmentReader() :	m_Reading(false), m_pReader(NULL)
	{
	}


	// destructor
	C_AttachmentReader::~C_AttachmentReader()
	{
		StopAndCleanup();
	}

	bool C_AttachmentReader::StopAndCleanup(void)
	{
		if (m_pReader)
		{
			m_pReader->StopAndCleanup();
			delete m_pReader;
			m_pReader = NULL;
		}
		m_Reading = false;
		return true;
	}

	bool C_AttachmentReader::Start(const SysProfile::FullProfile& localPlayer, const SysProfile::BaseProfile& attachmentOwner, const U32 scoreboardId, U8* pStorage, const U32 storageSize)
	{
		assert(m_Reading);
		StopAndCleanup();

		if (NULL == pStorage || 0 == storageSize || 0 == scoreboardId)
			return false;

		char filename[64];
		sprintf_s(filename, sizeof(filename), "sb.%d.att", scoreboardId);

		m_pReader = new NetPlay::C_PerUserServerStorageReader;
		if (NULL == m_pReader)
			return false;

		if (m_pReader->Start(localPlayer, attachmentOwner, filename, pStorage, storageSize))
			m_Reading = true;
		else
			StopAndCleanup();

		return m_Reading;
	}

	C_AttachmentReader::eScoreboardAttachmentReaderState C_AttachmentReader::Continue(U32& numBytesDownloaded)
	{
		if (!m_Reading)
			return eReadNotActive;
		assert(m_pReader);

		eScoreboardAttachmentReaderState state = eReadFailed;

		switch (m_pReader->Continue(numBytesDownloaded))
		{
		case NetPlay::C_PerUserServerStorageReader::eReadInProgress:
			return eReadInProgress;
			break;

		case NetPlay::C_PerUserServerStorageReader::eReadNotActive:
		case NetPlay::C_PerUserServerStorageReader::eReadFailed:
			state = eReadFailed;
			break;

		case NetPlay::C_PerUserServerStorageReader::eReadFileDoesNotExist:
			state = eReadFileDoesNotExist;
			break;

		case NetPlay::C_PerUserServerStorageReader::eReadDownloadBufferToSmall:
			state = eReadDownloadBufferToSmall;
			break;

		case NetPlay::C_PerUserServerStorageReader::eReadSuccessful:
			state = eReadSuccessful;
			break;

		default:
			assert(false);
			break;
		}

		StopAndCleanup();
		return state;
	}

}// end namespace

