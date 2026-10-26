#include <stdafx.h>
#include <Core/Core.h>
#include <Network/Core.h>
#include <Network/DemonwareTask.h>
#include <Network/DataMiningWriter.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdTitleUtilities/bdTitleUtilities.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

namespace NetPlay
{
	C_DataMiningWriter::C_DataMiningWriter() : m_Initialised(false), m_Writing(false), m_pStorage(NULL), m_NumBuffersAllocated(0)
	{
	}

	C_DataMiningWriter::~C_DataMiningWriter()
	{
		Shutdown();
	}

	bool C_DataMiningWriter::Initialise(const U32 numBuffers)
	{
		if (m_Initialised)
			return false;

		assert(numBuffers);
		if (0 == numBuffers)
			return false;

		m_NumBuffersAllocated = numBuffers;
		
		m_pStorage = new C_DataMiningBuffer[m_NumBuffersAllocated];
		
		if (m_pStorage)
		{
			// add to free list.
			for (U32 i=0; i<m_NumBuffersAllocated; i++)
				m_FreeList.PushBack(m_pStorage[i]);

			m_Initialised = true;
		}
		else
			Shutdown();

		return m_Initialised;
	}

	void C_DataMiningWriter::Shutdown(void)
	{
		// if task active cancel it now..
		DemonwareRemoteTask::Close(m_AsyncTask);

		// erase from free and in use list.
		while (m_InUseList.Size())
			m_InUseList.Erase(m_InUseList.Begin());

		while (m_FreeList.Size())
			m_FreeList.Erase(m_FreeList.Begin());

		// delete storage.
		delete [] m_pStorage;
		m_pStorage = NULL;

		m_Writing = false;
		m_NumBuffersAllocated = 0;
	
		m_Initialised = false;
	}

	bool C_DataMiningWriter::IsWriting(void) const
	{
		if (!m_Initialised)
			return false;
		return m_Writing;
	}

	bool C_DataMiningWriter::Add(const char* str)
	{
		if (!m_Initialised || NULL == str || strlen(str) == 0)
			return false;

		if (m_Writing)
		{
			LOGWARN(g_Log, "C_DataMiningWriter::Add() - trying to write into buffers as they are being sent to the server.");
			return false;
		}

		if (m_InUseList.Size() == 0)
		{
			DataMiningBufferList::Iterator it = m_FreeList.Begin();
			m_FreeList.Erase(it);
			m_InUseList.PushBack(*it);
		}
		
		// how many chars do we want to add..
		const U32 numToAdd = strlen(str) + 2; // we add \0 and \n
		if (numToAdd > (U32)m_InUseList.Back().m_Data.Capacity())
		{
			LOGWARN(g_Log, "C_DataMiningWriter::Add() - string is too big to fit into a single buffer.");
			return false;
		}

		bool added = false;
		while (!added)
		{
			C_DataMiningBuffer& buff = m_InUseList.Back();
			const U32 remaining = buff.m_Data.Capacity() - buff.m_Used;
			if (remaining >= numToAdd)
			{
				for (U32 i=0; i<strlen(str); i++)
					buff.m_Data[buff.m_Used++] = str[i];
				buff.m_Data[buff.m_Used++] = '\0';
				buff.m_Data[buff.m_Used++] = '\n';

				added = true;
			}
			else
			{
				// try and allocate another buffer.
				if (m_FreeList.Size() == 0)
				{
					LOGWARN(g_Log, "C_DataMiningWriter::Add() - ran out of available buffer space!.");
					return false;
				}
				else
				{
					DataMiningBufferList::Iterator it = m_FreeList.Begin();
					m_FreeList.Erase(it);
					m_InUseList.PushBack(*it);
				}
			}
		}

		return true;
	}

	bool C_DataMiningWriter::WriteToServer(void)
	{
		if (!m_Initialised)
			return false;

		if (m_Writing)
		{
			LOGWARN(g_Log, "C_DataMiningWriter::WriteToServer() - called whilst already writing..");
			return false; // already writing.
		}

		if (m_InUseList.Size() == 0)
			return false; // nothing to write..

		DataMiningBufferList::Iterator it = m_InUseList.Begin();

		if (0 == it->m_Used)
			return false; // no data in the buffer yet.

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
			return false;

		bdTitleUtilities* titleUtil  = lobbyService->getTitleUtilities();
		if (NULL == titleUtil)
			return false;
		
		// write buffer to server.
		m_AsyncTask = titleUtil->recordEvent(&(it->m_Data[0]), it->m_Used);
		
		// release this data set even if we failed to create the server task.
		m_InUseList.Erase(it);
		m_FreeList.PushBack(*it);
		
		if (DemonwareRemoteTask::IsBusy(m_AsyncTask))
		{
			m_Writing = true;
			return true;
		}

		DemonwareRemoteTask::Close(m_AsyncTask);
		return false;

	}

	void C_DataMiningWriter::Poll(void)
	{
		if (!m_Initialised || !m_Writing)
			return;

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
			// lost connection to demonware, kill all objects.
			Flush();
			return;
		}

		bool error = true;
		switch (DemonwareRemoteTask::CheckState(m_AsyncTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return;
			break;

		case NetPlay::DemonwareRemoteTask::eDone:
			error = false;
			break;

		default: // all other cases assume failure.
			break;
		}
		
		if (!error)
		{
			if (m_AsyncTask->getErrorCode() == BD_EVENT_DESC_TRUNCATED)
			{
				LOGWARN(g_Log, "C_DataMiningWriter::Poll() - uploaded data stream was truncated on the server.");
			}

			DemonwareRemoteTask::Close(m_AsyncTask);

			m_Writing = false;

			// if we have more pending Data then upload that now.
			if (m_InUseList.Size())
				WriteToServer();
		}
		else
			Flush();
	}

	void C_DataMiningWriter::Flush(void)
	{
		if (!m_Initialised)
			return;

		// cancel demonware task if it's active.
		DemonwareRemoteTask::Close(m_AsyncTask);

		while (m_InUseList.Size())
		{
			DataMiningBufferList::Iterator it = m_InUseList.Begin();
			m_InUseList.Erase(it);
			m_FreeList.PushBack(*it);
		}
		
		m_Writing = false;
	}
}
