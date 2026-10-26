#ifndef DISPLAY_GCM_DEBUG_COMMAND_BUFFER_H
#define DISPLAY_GCM_DEBUG_COMMAND_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

#if !CFG_GOLD && !PLATFORM_SPU
#define DEBUG_COMMAND_BUFFER 1
#endif

#if DEBUG_COMMAND_BUFFER
	
struct C_DebugCommandBuffer
{
	C_DebugCommandBuffer();

	void Alloc(const U32 size);
	void Realloc(const U32 size);
	void Wastage(const U32 size);
	void Update();
	void Dump(const char* name);

	S32 					m_CurFrame;
	static const U32		m_MaxFrames = 5;
	S32 					m_Allocs[m_MaxFrames];
	S32 					m_Reallocs[m_MaxFrames];
	U32 					m_Usage[m_MaxFrames];
	U32 					m_Wastage[m_MaxFrames];
};

extern C_DebugCommandBuffer g_MainDebugCommandBuffer;
extern C_DebugCommandBuffer g_UserDebugCommandBuffer;

#endif

}; // namespace Display

#endif // DISPLAY_GCM_DEBUG_COMMAND_BUFFER_H
