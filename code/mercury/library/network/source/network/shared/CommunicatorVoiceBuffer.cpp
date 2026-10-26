#include <StdAfx.h>
#include <Core/Core.h>
#include <Network/network.h>
#include <Network/CommunicatorVoiceBuffer.h>

namespace Communicator
{
	// constructor
	C_VoiceBuffer::C_VoiceBuffer() : m_Initialised(false), m_AllocatedSize(0), m_CurrentSize(0), m_pData(NULL)
	{
	}

	// destructor
	C_VoiceBuffer::~C_VoiceBuffer()
	{
		Shutdown();
	}

	bool C_VoiceBuffer::Initialise(const U32 bufferSize)
	{
		if (m_Initialised)
			return true;

		if (0 == bufferSize)
			return false;
  
		m_CurrentSize = 0;
		m_AllocatedSize = bufferSize;
		m_pData = new U8[m_AllocatedSize];
		m_Initialised = (NULL == m_pData) ? false : true;

		return m_Initialised;
	}

	void C_VoiceBuffer::Shutdown(void)
	{
		if (m_pData)
			delete [] m_pData;
		m_pData = NULL;
		m_CurrentSize = 0;
		m_AllocatedSize = 0;
		m_Initialised = false;
	}

	void C_VoiceBuffer::Flush(void)
	{
		if (!m_Initialised)
			return;
		m_CurrentSize = 0;
	}

	bool C_VoiceBuffer::Append(const U8 *pData, const U32 dataSize)
	{
		if (! m_Initialised || NULL == pData || 0 == dataSize)
			return false;

		if (pData == m_pData)
			return false; // can't append the same data

		const U32 remaining = Capacity() - Size();
		if (dataSize > remaining)
		{
			LOGWARN(NetPlay::g_Log, "C_VoiceBuffer::Append() - not enough room remaining to append new encoded voice data.");
			return false;
		}

		memcpy(&m_pData[m_CurrentSize], pData, dataSize);
		m_CurrentSize += dataSize;
		return true;
	}

	bool C_VoiceBuffer::Append(const C_VoiceBuffer& buffer)
	{
		if (!m_Initialised || !buffer.IsInitialised() || buffer.Size() == 0)
			return false;

		if (buffer.m_pData == m_pData)
			return false; // can't append same object.

		const U32 remaining = Capacity() - Size();
		if (buffer.Size() > remaining)
		{
			LOGWARN(NetPlay::g_Log, "C_VoiceBuffer::Append() - not enough room remaining to append new encoded voice data.");
			return false;
		}

		memcpy(&m_pData[m_CurrentSize], buffer.m_pData, buffer.Size());
		m_CurrentSize += buffer.Size();
		return true;
	}
}
