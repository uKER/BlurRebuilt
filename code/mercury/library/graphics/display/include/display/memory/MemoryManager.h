#ifndef DISPLAY_MEMORY_MEMORY_MANAGER_H
#define DISPLAY_MEMORY_MEMORY_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"
#include "btl/containers/vector.h"

#if defined(DEVICE_PS3GCM)
#include "Display/Memory/GCM/GCMMemoryManager.h"
#elif defined(DEVICE_PS3GL)
#include "Display/Memory/GL/GLMemoryManager.h"
#else
//#pragma message("Unsupported device define when compiling MemoryManager.h")
#endif

namespace Display
{

typedef Ut::UtMap<U8*, U32> C_MemoryMap;

//! C_MemoryBuffer

struct MemoryBlock
{
	U32 size;
	const U8* address;
	bool quick;
};

#if defined(DEVICE_PS3GCM)
class C_MemoryBuffer : public GCM::C_GCMMemoryBuffer
#elif defined(DEVICE_PS3GL)
class C_MemoryBuffer : public GL::C_GLMemoryBuffer
#else
class C_MemoryBuffer
#endif
{
public:

	C_MemoryBuffer();
	~C_MemoryBuffer();

	const S32 GetAllocs() const;
	const U32 GetUsed() const;
	const U32 GetFree() const;
	
	bool Create(const U32 size, const U32 align, const U32 flags);
	void Destroy();

	bool AllocateFromStart(const U32 align, const U32 size, U8** ppAddress);
	bool AllocateFromEnd(const U32 align, const U32 size, U8** ppAddress);
	void Free(const U32 size, U8* pAddress);

	void PrintDebug(Debug::Logger& log) const;

	bool AlreadyInFreeList(const U32 size, U8* pAddress);

	void GetFreeList(Btl::Vector<MemoryBlock>& blocks) const;

private:

	C_MemoryMap::Iterator InsertIntoFreeList(const U32 size, U8* pAddress);
	void InsertIntoFreeList(C_MemoryMap::Iterator it, const U32 size, U8* pAddress);
	
	void MergeWithPrevBlock(C_MemoryMap::Iterator it);
	void MergeWithNextBlock(C_MemoryMap::Iterator it);

	bool AllocateFromStartOfBlock(C_MemoryMap::Iterator &it, const U32 align, const U32 size, U8** ppAddress);
	bool AllocateFromEndOfBlock(C_MemoryMap::ReverseIterator &it, const U32 align, const U32 size, U8** ppAddress);

	C_MemoryMap		m_FreeList;

private:

	S32				m_Allocs;
	U32				m_Used;
	U32				m_Free;
	MT::Mutex		m_Mutex;
};

}; // namespace Display

#include "Display/Memory/MemoryManager.inl"

#endif // DISPLAY_MEMORY_MEMORY_MANAGER_H
