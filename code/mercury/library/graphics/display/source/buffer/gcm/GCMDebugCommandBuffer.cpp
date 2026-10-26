#include "stdafx.h"
#include "Display/Buffer/GCM/GCMDebugCommandBuffer.h"
#include <Core/Threads/Atomic.h>

namespace Display
{

#if DEBUG_COMMAND_BUFFER

C_DebugCommandBuffer::C_DebugCommandBuffer()
{
	m_CurFrame = 0;

	for(U32 i=0; i<m_MaxFrames; i++)
	{
		m_Allocs[i]		= 0;
		m_Reallocs[i]	= 0;
		m_Usage[i]		= 0;
		m_Wastage[i]	= 0;
	}
}

void C_DebugCommandBuffer::Alloc(const U32 size)
{
	S32 curFrame = MT::AtomicGet(&m_CurFrame);
	BIZ_ASSERT(curFrame < m_MaxFrames);
	MT::AtomicIncrement(&m_Allocs[curFrame]);
	MT::AtomicAdd(&m_Usage[curFrame], size);
}

void C_DebugCommandBuffer::Realloc(const U32 size)
{
	S32 curFrame = MT::AtomicGet(&m_CurFrame);
	BIZ_ASSERT(curFrame < m_MaxFrames);
	MT::AtomicIncrement(&m_Reallocs[curFrame]);
	MT::AtomicAdd(&m_Usage[curFrame], size);
}

void C_DebugCommandBuffer::Wastage(const U32 size)
{
	S32 curFrame = MT::AtomicGet(&m_CurFrame);
	BIZ_ASSERT(curFrame < m_MaxFrames);
	MT::AtomicAdd(&m_Wastage[curFrame], size);
}

void C_DebugCommandBuffer::Update()
{
	m_CurFrame = m_CurFrame + 1 >= m_MaxFrames ? 0 : m_CurFrame + 1;
	
	m_Allocs[m_CurFrame]	= 0;
	m_Reallocs[m_CurFrame]	= 0;
	m_Usage[m_CurFrame]		= 0;
	m_Wastage[m_CurFrame]	= 0;
}

void C_DebugCommandBuffer::Dump(const char* name)
{
	char temp[512];

	strcpy_s(temp, sizeof(temp), name);

	for(S32 i=0; i<m_MaxFrames; i++)
	{
		sprintf_s(temp, sizeof(temp), "%s : %s (%i %i %i %i)",
			temp,
			i == m_CurFrame ? "c" : " ",
			m_Allocs[i],
			m_Reallocs[i],
			m_Usage[i],
			m_Wastage[i]);
	}

	LOGINFO(g_Log, "%s\n", temp);
}

C_DebugCommandBuffer g_MainDebugCommandBuffer;
C_DebugCommandBuffer g_UserDebugCommandBuffer;

#endif

}; // namespace Display
