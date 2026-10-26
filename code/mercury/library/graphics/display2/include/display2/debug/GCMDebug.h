#ifndef DISPLAY2_GCM_DEBUG_H
#define DISPLAY2_GCM_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <Core/Threads/Mutex.h>

#if !CFG_GOLD
#define TRACK_DISPLAY2_COMMAND_BUFFERS 1
#endif

namespace Display2
{
	
#define METHODOFFSET_START		0
#define METHODOFFSET_END		13
#define METHODOFFSET_COUNT		(METHODOFFSET_END-METHODOFFSET_START+1)

#define METHODCOUNT_START		18
#define METHODCOUNT_END			28
#define METHODCOUNT_COUNT		(METHODCOUNT_END-METHODCOUNT_START+1)

#define METHODEXTRACT(_command, _start, _end) ((_command) >> (_start)) & ((1 << (_end-_start+1)) - 1);

void ToBinary(U32 method, const bool spacing = true, const Util::C_String name = "");

struct C_CommandBufferInstance
{
	C_CommandBufferInstance();

	char						m_Name[64];
	U8*							m_pHeadAddress;
	U8*							m_pTailAddress;
};

struct C_CommandBufferTracker
{
	C_CommandBufferTracker();
	C_CommandBufferInstance* AllocInstance(char* name);
	C_CommandBufferInstance* FindInstance(const U32 offset);
	void Dump();

	const static U32			m_MaxInstances = 64;
	U32							m_CurInstance;
	C_CommandBufferInstance		m_Instances[m_MaxInstances];
	MT::Mutex					m_Mutex;
};

struct C_CommandBufferWalkerInput
{
	C_CommandBufferWalkerInput();

	bool						m_OutputToLog;
	bool						m_OutputToFile;
	U32							m_MaxSizeInBytes;
};

struct C_CommandBufferWalkerOutput
{
	C_CommandBufferWalkerOutput();
	~C_CommandBufferWalkerOutput();

	U32							m_SizeInBytes;
	U32*						m_pLastJumpAddress;
	U32*						m_pLastCallAddress;
	U32*						m_pLastReturnAddress;
	U32*						m_pLastExecutedAddress;
	U32*						m_pLastExecutedBeforeJumpAddress;
	Btl::Vector<U32*>			m_JumpCallstack;
};

struct C_CommandBufferWalkerLogger
{
	C_CommandBufferWalkerLogger();
	~C_CommandBufferWalkerLogger();

	void Init(const U32 outputBufferMaxSize);
	void Flush();
	void Output(const char* str);

	bool						m_OutputToLog;
	bool						m_OutputToFile;
	U32							m_OutputBufferSize;
	U32							m_OutputBufferMaxSize;
	char*						m_pOutputBuffer;
};

struct C_CommandBufferWalker
{
	C_CommandBufferWalker();

	void Walk();

	U8*							m_pCurrHeadAddress;
	U8*							m_pCurrTailAddress;
	U8*							m_pPrevHeadAddress;
	U8*							m_pPrevTailAddress;
};

bool WalkCommandBuffer(U32* pHeadAddress, U32* pTailAddress, const C_CommandBufferWalkerInput& input, C_CommandBufferWalkerOutput& output);

bool WalkCommandBuffer(const U32 headOffset, const U32 tailOffset, const bool outputToLog = false, const bool outputToFile = false, const U32 maxSizeInBytes = 1024*512);

bool WalkCommandBufferOffsets(const U32 headOffset, const U32 tailOffset, const bool outputToLog = false, const bool outputToFile = false, const U32 maxSizeInBytes = 1024*512);

bool WalkCommandBufferAddress(const U8* pHeadAddress, const U8* pTailAddress, const bool outputToLog = false, const bool outputToFile = false, const U32 maxSizeInBytes = 1024*512);

#if TRACK_DISPLAY2_COMMAND_BUFFERS
extern C_CommandBufferTracker g_CommandBufferTracker;
extern C_CommandBufferWalker g_CommandBufferWalker;
#endif

extern C_CommandBufferWalkerLogger g_CommandBufferWalkerLogger;

} // namespace Display2

#endif // DISPLAY2_GCM_DEBUG_H
