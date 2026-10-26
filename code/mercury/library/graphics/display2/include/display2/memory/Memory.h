#ifndef DISPLAY2_MEMORY_H
#define DISPLAY2_MEMORY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Memory/MemoryTracker.h>
#include <System/Memory/MemoryBuffer.h>

#include <Btl/Containers/ChainedHashMap.h>
#include <Btl/Allocators/Pool.h>
#include <Btl/Containers/Set.h>

#include "Display2/Display2Defines.h"

#if DEVICE_GCM
#include "Display2/Memory/GCMMemory.h"
#elif DEVICE_D3D11_PC
#include "Display2/Memory/PCD3D11Memory.h" // [Blur1PCDX11] Native DX11 Display2 memory declarations.
#endif

#if !CFG_GOLD
#define USE_DISPLAY2_MEMORY_TRACKING 1
#endif

#if !CFG_GOLD
//#define DISPLAY2_MEMORY_GUARDS_ENABLE 1
#endif

namespace Display2
{

enum E_MemoryType
{
	E_MemoryType_Unknown = 0,
	E_MemoryType_Indirect,
	E_MemoryType_Device,
	E_MemoryType_CommandBuffer,
	E_MemoryType_VertexBuffer,
	E_MemoryType_IndexBuffer,
	E_MemoryType_VertexShader,
	E_MemoryType_PixelShader,
	E_MemoryType_Surface,
	E_MemoryType_Texture,
	E_MemoryType_Misc,
	E_MemoryType_Count,
};

struct C_MemoryInfo
{
	C_MemoryInfo();

	S32						m_Size;
	S32						m_Count;
};

struct C_MemoryTracker
{
	C_MemoryTracker();

	Util::C_String GetNameFromType(const U32 type) const;

	void TrackAlloc(const E_MemoryType type, const U32 size);
	void TrackFree(const E_MemoryType type, const U32 size);

	C_MemoryInfo			m_Info[E_MemoryType_Count];
};

class C_MemoryManager;

struct C_MemoryGuardInfo
{
	C_MemoryGuardInfo(U8* pAddress, U32 size, C_MemoryManager* pManager);

	void Init(U8* pAddress, U32 size);
	void Free(U8* pAddress, U32 size);
	bool Valid(const U8* pAddress, U32 size) const;

	void Dump() const;

	U8* 				m_pAddress;
	U32 				m_Size;
	S32					m_CallstackSize;
	static const U32	m_MaxCallstackSize = 100;
	void*				m_pCallstack[m_MaxCallstackSize];
	C_MemoryManager*	m_pManager;
};

typedef Btl::ChainedHashMap<U8*, C_MemoryGuardInfo*, Btl::HashKeyOpDefault<void*>, Btl::DynamicPool<> > C_MemoryGuardInfoMap;

class C_MemoryManager
{
public:

	C_MemoryManager();
	~C_MemoryManager();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetUsed() const;
	U32 GetFree() const;
	const U8* GetAddress() const;

	S32 GetBlockSize();
	const S32 GetBlockSize() const;

	bool Create(const U32 bufferSize, const U32 bufferAlign, const U32 bufferFlags, const E_MemoryBuffer bufferType, const char* bufferName, U8* pBufferAddress = NULL);
	void Destroy();

	bool Alloc(const U32 align, const U32 size, U8** ppAddress, const E_MemoryType type = E_MemoryType_Unknown, const U32 flags = 0);
	void Free(const U32 size, U8* pAddress, const E_MemoryType type = E_MemoryType_Unknown, const U32 flags = 0);

	bool Inside(const U32 size, const U8* pAddress) const;

	void Lock() const;
	void Unlock() const;

	C_MemoryBuffer* GetBuffer();
	const C_MemoryBuffer* GetBuffer() const;

	C_MemoryTracker* GetTracker();
	const C_MemoryTracker* GetTracker() const;

	void GetFreeList(Btl::Vector<Sys::MemoryBlock>& blocks) const;

	void PrintDebug(Debug::Logger& log) const;

	void Dump(const Util::C_String& name);

	bool CheckMemoryGuards() const;
	void GetPrevAndNextMemoryGuardInfo(const U8* pAddress, C_MemoryGuardInfo** ppPrev, C_MemoryGuardInfo** ppNext) const;
	
private:

	void InsertMemoryGuardInfo(U8* pAddress, U32 size);
	void RemoveMemoryGuardInfo(U8* pAddress, U32 size);
	
private:

	mutable MT::Lock			m_Lock;
	
	C_MemoryBuffer				m_Buffer;
	C_MemoryTracker				m_Tracker;

	Memory::Tracker				m_SysTracker;
	Sys::MemoryBuffer			m_SysAllocator;

	#if DISPLAY2_MEMORY_GUARDS_ENABLE
	//mutable MT::Lock			m_MemoryGuardLock;
	C_MemoryGuardInfoMap		m_MemoryGuardInfoList;
	#endif
};

} // namespace Display2

#include "Display2/Memory/Memory.inl"

#endif // DISPLAY2_MEMORY_H
