#include <StdAfx.h>
#include <Network/ScoreboardAttachmentWriter.h>
#include <Network/PerUserServerStorageWriter.h>
#include <Systemprofile/SystemProfile.h>


namespace Scoreboard
{
	// constructor
	C_AttachmentWriter::C_AttachmentWriter() :	m_Writing(false), m_pWriter(NULL)
	{
	}


	// destructor
	C_AttachmentWriter::~C_AttachmentWriter()
	{
		StopAndCleanup();
	}

	// stop any scoreboard write and reset internal data to default.
	bool C_AttachmentWriter::StopAndCleanup(void)
	{
		if (m_pWriter)
		{
			m_pWriter->StopAndCleanup();
			delete m_pWriter;
			m_pWriter = NULL;
		}
		m_Writing = false;
		return true;
	}

	bool C_AttachmentWriter::Start(const SysProfile::FullProfile& localPlayer, const U32 scoreboardId, const S64 scoreRating, U8* pAttachmentData, const U32 attachmentSize)
	{
		assert(m_Writing);
		StopAndCleanup();

		if (NULL == pAttachmentData || 0 == attachmentSize || 0 == scoreboardId)
			return false;

		char filename[64];
		sprintf_s(filename, sizeof(filename), "sb.%d.att", scoreboardId);

		m_pWriter = new NetPlay::C_PerUserServerStorageWriter;
		if (NULL == m_pWriter)
			return false;

		if (m_pWriter->Start(localPlayer, filename, pAttachmentData, attachmentSize))
			m_Writing = true;
		else
			StopAndCleanup();

		return m_Writing;
	}

	C_AttachmentWriter::eScoreboardAttachmentWriterState C_AttachmentWriter::Continue(void)
	{
		if (!m_Writing)
			return eWriteNotActive;
		assert(m_pWriter);

		eScoreboardAttachmentWriterState state = eWriteFailed;

		switch (m_pWriter->Continue())
		{
		case NetPlay::C_PerUserServerStorageWriter::eWriteInProgress:
			return eWriteInProgress;
			break;

		case NetPlay::C_PerUserServerStorageWriter::eWriteNotActive:
		case NetPlay::C_PerUserServerStorageWriter::eWriteFailed:
			state = eWriteFailed;
			break;

		case NetPlay::C_PerUserServerStorageWriter::eWriteSuccessful:
			state = eWriteSuccessful;
			break;

		default:
			assert(false);
			break;
		}

		StopAndCleanup();
		return state;
	}

}// end namespace