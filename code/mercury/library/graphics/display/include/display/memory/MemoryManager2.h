#ifndef DISPLAY_MEMORY_MEMORY_MANAGER_2_H
#define DISPLAY_MEMORY_MEMORY_MANAGER_2_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/memory/memorytracker.h>
#include "system/memory/memorybuffer.h"

#if !PLATFORM_PC
#define USE_DISPLAY2_MEMORY_MANAGER 1
#endif

#if USE_DISPLAY2_MEMORY_MANAGER
#include <Display2/Memory/Memory.h>
#endif

#if defined(DEVICE_PS3GCM)
#include "Display/Memory/GCM/GCMMemoryManager.h"
#elif defined(DEVICE_PS3GL)
#include "Display/Memory/GL/GLMemoryManager.h"
#else
//#pragma message("Unsupported device define when compiling MemoryManager2.h")
#endif

namespace Display
{

#if defined(DEVICE_PS3GCM)
class MemoryBuffer : public GCM::C_GCMMemoryBuffer
#elif defined(DEVICE_PS3GL)
class MemoryBuffer : public GL::C_GLMemoryBuffer
#else
class MemoryBuffer
#endif
{
public:
	bool Create(U32 size, U32 align, U32 flags);
	void Destroy();
};

////////////////////////////////////////////////////////////////////////////////

class C_MemoryBuffer
{
public:

	C_MemoryBuffer();
	~C_MemoryBuffer();

	const S32 GetAllocs() const;
	const U32 GetUsed() const;
	const U32 GetFree() const;
	
	bool Create(const U32 size, const U32 align, const U32 flags, const char* bufferName);
	void Destroy();

#if defined(DEVICE_PS3GCM)
	void SetType(const GCM::E_GCMMemoryBufferType type);
#endif

	bool AllocateFromStart(const U32 align, const U32 size, U8** ppAddress);
	bool AllocateFromEnd(const U32 align, const U32 size, U8** ppAddress);
	void Free(const U32 size, U8* pAddress);

	const U8* GetAddress() const;
	U32 GetSize() const;
	void PrintDebug(Debug::Logger& log) const;	
	void GetFreeList(Btl::Vector<Sys::MemoryBlock>& blocks) const;

#if USE_DISPLAY2_MEMORY_MANAGER
	Display2::C_MemoryManager* GetDisplay2();
	const Display2::C_MemoryManager* GetDisplay2() const;
	void SetDisplay2(Display2::C_MemoryManager* pDisplay2);
#endif

private:

#if USE_DISPLAY2_MEMORY_MANAGER
	#if PLATFORM_PS3
	GCM::E_GCMMemoryBufferType	m_Type;
	#endif
	Display2::C_MemoryManager*	m_pDisplay2;
#else
	Memory::Tracker				m_memoryTracker;
	MemoryBuffer				m_buffer;
	Sys::MemoryBuffer			m_allocator;

	S32							m_Allocs;
	MT::Lock					m_Lock;
#endif
};

}

#endif
