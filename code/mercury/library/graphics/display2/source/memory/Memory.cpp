#include "StdAfx.h"
#include "Display2/Memory/Memory.h"
#include "Display2/Device/Device.h"
#include <System/Memory/CallStack.h>

#define DISPLAY2_MEMORY_GUARDS_CODE 0x58

Debug::Bool g_MemoryGuardBlocksEnable				("Graphics.Display2.MemoryGuardBlocksEnable",				true);
Debug::Int	g_MemoryGuardBlocksCheckSize			("Graphics.Display2.MemoryGuardBlocksCheckSize",			8, Debug::Range(0, 128));

Debug::Bool g_MemoryGuardBlocksCheckOnAlloc			("Graphics.Display2.MemoryGuardBlocksCheckOnAlloc",			false);
Debug::Bool g_MemoryGuardBlocksCheckOnFree			("Graphics.Display2.MemoryGuardBlocksCheckOnFree",			false);

Debug::Bool g_MemoryGuardBlocksCheckInfoOnAlloc		("Graphics.Display2.MemoryGuardBlocksCheckInfoOnAlloc",		false);
Debug::Bool g_MemoryGuardBlocksCheckInfoOnFree		("Graphics.Display2.MemoryGuardBlocksCheckInfoOnFree",		false);

#if !CFG_GOLD
//#define CHECK_GRAPHICS_MEMORY_ALLOCS 1
#endif

#if CHECK_GRAPHICS_MEMORY_ALLOCS
bool g_PrintAllocs = false;
bool g_BreakOnAlloc = false;
bool g_BreakOnFree = false;
#endif

namespace Memory
{
	Util::C_String MemSizeToStringK(size_t dwSize, bool bPad=true);
}

namespace Display2
{

#if CHECK_GRAPHICS_MEMORY_ALLOCS
Debug::Logger& g_MemoryLog = Debug::GetLogger("mercury.display2.memoryallocs");
#endif

void DataCacheBlockFlush(U8* pAddress, U32 size)
{
#if PLATFORM_PS3
	for(U32 i=0; i<size; i+=128)
		__dcbf(pAddress+i);
#endif
}

C_MemoryInfo::C_MemoryInfo() :
	m_Size(0),
	m_Count(0)
{
}

C_MemoryTracker::C_MemoryTracker()
{
	memset(m_Info, 0, sizeof(m_Info));
}

Util::C_String C_MemoryTracker::GetNameFromType(const U32 type) const
{
	switch(type)
	{
	case E_MemoryType_Unknown:			return "Unknown";
	case E_MemoryType_Indirect:			return "Indirect";
	case E_MemoryType_Device:			return "Device";
	case E_MemoryType_CommandBuffer:	return "CommandBuffer";
	case E_MemoryType_VertexBuffer:		return "VertexBuffer";
	case E_MemoryType_IndexBuffer:		return "IndexBuffer";
	case E_MemoryType_VertexShader:		return "VertexShader";
	case E_MemoryType_PixelShader:		return "PixelShader";
	case E_MemoryType_Surface:			return "Surface";
	case E_MemoryType_Texture:			return "Texture";
	case E_MemoryType_Misc:				return "Misc";
	default: BIZ_ASSERT(0);				return "";
	}
}

void C_MemoryTracker::TrackAlloc(const E_MemoryType type, const U32 size)
{
	BIZ_ASSERT(type >= 0 && type < E_MemoryType_Count);
	BIZ_ASSERT(size <= INT_MAX);
	
	m_Info[type].m_Size += size;
	m_Info[type].m_Count += 1;

#if !CFG_GOLD
	if(type == E_MemoryType_Unknown)
	{
		LOGWARN(g_Log, "Allocating unknown graphics memory of size %i\n", size);
		//DebugBreak();
	}
#endif
}

void C_MemoryTracker::TrackFree(const E_MemoryType type, const U32 size)
{
	BIZ_ASSERT(type >= 0 && type < E_MemoryType_Count);
	BIZ_ASSERT(size <= INT_MAX);
	
	m_Info[type].m_Size -= size;
	m_Info[type].m_Count -= 1;

	// TODO: Put these back in when all the games are fixed up with tracking
	//BIZ_ASSERT(m_Info[type].m_Size >= 0);
	//BIZ_ASSERT(m_Info[type].m_Count >= 0);

#if !CFG_GOLD
	if(type == E_MemoryType_Unknown)
	{
		LOGWARN(g_Log, "Freeing unknown graphics memory of size %i\n", size);
		//DebugBreak();
	}
#endif
}

C_MemoryGuardInfo::C_MemoryGuardInfo(U8* pAddress, U32 size, C_MemoryManager* pManager) : m_pAddress(pAddress), m_Size(size), m_pManager(pManager)
{
	BIZ_ASSERT_SPEED(m_pAddress);
	BIZ_ASSERT_SPEED(m_pManager);
	BIZ_ASSERT_SPEED(m_Size > (U32)m_pManager->GetBlockSize());

	m_CallstackSize = 0;
	for(U32 i=0; i<m_MaxCallstackSize; i++)
		m_pCallstack[i] = NULL;
}

void C_MemoryGuardInfo::Init(U8* pAddress, U32 size)
{
	BIZ_ASSERT_SPEED(m_pAddress == pAddress);
	BIZ_ASSERT_SPEED(m_Size == size);
	BIZ_ASSERT_SPEED(m_Size > (U32)m_pManager->GetBlockSize());

	memset(m_pAddress, UCHAR_MAX, m_Size - m_pManager->GetBlockSize());

	U32 guardOffset = m_Size - m_pManager->GetBlockSize();

	memset(&m_pAddress[guardOffset], DISPLAY2_MEMORY_GUARDS_CODE, m_pManager->GetBlockSize());

	DataCacheBlockFlush(pAddress, size);

	m_CallstackSize = Memory::CaptureCallStack(m_pCallstack, m_MaxCallstackSize);

	for(U32 i=m_CallstackSize; i<m_MaxCallstackSize; i++)
		m_pCallstack[i] = NULL;
}

void C_MemoryGuardInfo::Free(U8* pAddress, U32 size)
{
	BIZ_ASSERT_SPEED(m_pAddress == pAddress);
	BIZ_ASSERT_SPEED(m_Size == size);
	BIZ_ASSERT_SPEED(m_Size > (U32)m_pManager->GetBlockSize());

	memset(m_pAddress, UCHAR_MAX, m_Size);

	DataCacheBlockFlush(pAddress, size);
}

bool C_MemoryGuardInfo::Valid(const U8* pAddress, U32 size) const
{
	BIZ_ASSERT_SPEED(m_pAddress == pAddress);
	BIZ_ASSERT_SPEED(m_Size == size);
	BIZ_ASSERT_SPEED(m_Size > (U32)m_pManager->GetBlockSize());

	U32 guardBlockSize = m_pManager->GetBlockSize() > g_MemoryGuardBlocksCheckSize ? g_MemoryGuardBlocksCheckSize : m_pManager->GetBlockSize(); // Only look at the first N bytes of the guard block

	for(U32 i=0; i<guardBlockSize; i++) 
	{
		U32 j = m_Size - m_pManager->GetBlockSize() + i;

		if(m_pAddress[j] != DISPLAY2_MEMORY_GUARDS_CODE)
		{
			void* pResource = NULL;
			void* pResourceInfo = NULL;
			E_ResourceType resourceType = E_ResourceType_Unknown;

			C_MemoryGuardInfo* pPrev = NULL;
			C_MemoryGuardInfo* pNext = NULL;
			m_pManager->GetPrevAndNextMemoryGuardInfo(pAddress, &pPrev, &pNext);

			void* pPrevResource = NULL;
			void* pPrevResourceInfo = NULL;
			E_ResourceType prevResourceType = E_ResourceType_Unknown;
			#if TRACK_DISPLAY2_RESOURCES
			if(pPrev)
				Device()->FindResourceFromAddress(pPrev->m_pAddress, &pPrevResource, &pPrevResourceInfo, prevResourceType);
			#endif

			void* pNextResource = NULL;
			void* pNextResourceInfo = NULL;
			E_ResourceType nextResourceType = E_ResourceType_Unknown;
			#if TRACK_DISPLAY2_RESOURCES
			if(pNext)
				Device()->FindResourceFromAddress(pNext->m_pAddress, &pNextResource, &pNextResourceInfo, nextResourceType);
			#endif

			#if TRACK_DISPLAY2_RESOURCES
			if(Device()->FindResourceFromAddress(pAddress, &pResource, &pResourceInfo, resourceType))
			#else
			if(0)
			#endif
			{
				switch(resourceType)
				{
				case E_ResourceType_VertexBuffer:
					{
						C_VertexBuffer* pVertexBuffer = (C_VertexBuffer*)pResource;
						C_ResourceInfo* pVertexBufferResourceInfo = (C_ResourceInfo*)pResourceInfo;
						U32 vertexBufferSize = pVertexBuffer->GetSize();
						U8* pVertexBufferAddress = (U8*)pVertexBuffer->GetAddress();
						Dump();
						BIZ_ASSERT_SPEED(0 && "Memory stomping on a vertex buffer!");
					}
					break;
				case E_ResourceType_IndexBuffer:
					{
						C_IndexBuffer* pIndexBuffer = (C_IndexBuffer*)pResource;
						C_ResourceInfo* pIndexBufferResourceInfo = (C_ResourceInfo*)pResourceInfo;
						U32 indexBufferSize = pIndexBuffer->GetSize();
						U8* pIndexBufferAddress = (U8*)pIndexBuffer->GetAddress();
						Dump();
						BIZ_ASSERT_SPEED(0 && "Memory stomping on an index buffer!");
					}
					break;
				case E_ResourceType_Texture2D:
					{
						C_Texture* pTexture = (C_Texture*)pResource;
						C_ResourceInfo* pTextureResourceInfo = (C_ResourceInfo*)pResourceInfo;
						U32 textureSize = pTexture->GetSize();
						U8* pTextureAddress = pTexture->GetAddress();
						Dump();
						BIZ_ASSERT_SPEED(0 && "Memory stomping on a texture!");
					}
					break;
				case E_ResourceType_Surface:
					{
						C_Surface* pSurface = (C_Surface*)pResource;
						C_ResourceInfo* pSurfaceResourceInfo = (C_ResourceInfo*)pResourceInfo;
						U32 surfaceSize = pSurface->GetSize();
						U8* pSurfaceAddress = pSurface->GetAddress();
						Dump();
						BIZ_ASSERT_SPEED(0 && "Memory stomping on a surface!");
					}
					break;
				default:
					Dump();
					BIZ_ASSERT_SPEED(0 && "Memory stomping on an untracked resource!");
					break;
				}
			}
			else
			{
				Dump();
				BIZ_ASSERT_SPEED(0 && "Memory stomping on an unknown resource!");
			}

			return false;
		}
	}

	return true;
}

void C_MemoryGuardInfo::Dump() const
{
	LOGINFO(g_Log, "Size %i Address 0x%.8x\n", m_Size, m_pAddress);
	
	LOGINFO(g_Log, "Callstack %i\n", m_CallstackSize);
	for(S32 i=0; i<m_CallstackSize; i++)
	{
		LOGINFO(g_Log, "	0x%p\n", (void*)m_pCallstack[i]);	// [Blur2PC][x64]
	}
}

C_MemoryManager::C_MemoryManager()
{
}

C_MemoryManager::~C_MemoryManager()
{
}

S32 C_MemoryManager::GetBlockSize()
{
	return m_SysAllocator.BlockSize();
}

const S32 C_MemoryManager::GetBlockSize() const
{
	return m_SysAllocator.BlockSize();
}

bool C_MemoryManager::Create(const U32 bufferSize, const U32 bufferAlign, const U32 bufferFlags, const E_MemoryBuffer bufferType, const char* bufferName, U8* pBufferAddress)
{
	if(!m_Buffer.Create(bufferSize, bufferAlign, bufferFlags, bufferType, pBufferAddress))
		return false;

	if(!m_SysAllocator.Initialise(m_Buffer.GetAddress(), bufferSize, bufferAlign, 4))
		return false;

	m_SysTracker.Initialize(bufferName, Memory::Tracker::Instance(), bufferSize);

	return true;
}

void C_MemoryManager::Destroy()
{
	m_SysAllocator.Shutdown();

	m_Buffer.Destroy();
}

bool C_MemoryManager::Alloc(const U32 align, const U32 size, U8** ppAddress, const E_MemoryType type, const U32 flags)
{
	m_Lock.Enter();

	#if DISPLAY2_MEMORY_GUARDS_ENABLE
	U32 newSize = m_SysAllocator.BlockSize();
	if(flags & E_Flags_AlignSizeToBlockSize)
		newSize += Btl::AlignUp(size, m_SysAllocator.BlockSize());
	else
		newSize += size;
	#else
	const U32 newSize = size;
	#endif
	
	*ppAddress = m_SysAllocator.Allocate(newSize, align);

#if CHECK_GRAPHICS_MEMORY_ALLOCS
	if(g_PrintAllocs)
	{
		LOGINFO(g_MemoryLog, "Alloc size %9i address 0x%.8x align %4i\n", size, *ppAddress, align);
	}

	if(g_BreakOnAlloc)
	{
		DebugBreak();
	}
#endif

	#if USE_DISPLAY2_MEMORY_TRACKING
	if(*ppAddress != NULL)
		m_Tracker.TrackAlloc(type, newSize);
	#endif

	#if DISPLAY2_MEMORY_GUARDS_ENABLE
	if(*ppAddress != NULL)
		InsertMemoryGuardInfo(*ppAddress, newSize);
	#endif
	
	m_Lock.Exit();

	if(*ppAddress == NULL)
		return false;

	if(m_SysTracker.Initialised())
	{
		Memory::AllocationNode* pAllocNode = (Memory::AllocationNode*)malloc(m_SysTracker.TrackingStructureSize() + 4);

		m_SysTracker.RememberAllocNodeForVirPhys(pAllocNode, *ppAddress);

		m_SysTracker.TrackAllocation(*ppAddress,			// allocation address
			newSize,											// original newSize
			newSize,											// actual newSize (the difference is used to calculate memory wastage)
			Memory::kMemType_Physical,						// physical memory flag
			pAllocNode,										// address of tracking struct
			0);												// Callstack overhead
	}

	return true;
}

void C_MemoryManager::Free(const U32 size, U8* pAddress, const E_MemoryType type, const U32 flags)
{
	if(pAddress == NULL)
		return;

	BIZ_ASSERT(size > 0);

	#if DISPLAY2_MEMORY_GUARDS_ENABLE
	U32 newSize = m_SysAllocator.BlockSize();
	if(flags & E_Flags_AlignSizeToBlockSize)
		newSize += Btl::AlignUp(size, m_SysAllocator.BlockSize());
	else
		newSize += size;
	#else
	const U32 newSize = size;
	#endif

	m_Lock.Enter();

#if CHECK_GRAPHICS_MEMORY_ALLOCS
	if(g_PrintAllocs)
	{
		LOGINFO(g_MemoryLog, "Free  size %9i address 0x%.8x\n", size, pAddress);
	}

	if(g_BreakOnFree)
	{
		DebugBreak();
	}
#endif

	m_SysAllocator.Free(pAddress, newSize);

	#if DISPLAY2_MEMORY_GUARDS_ENABLE
	RemoveMemoryGuardInfo(pAddress, newSize);
	#endif

	#if USE_DISPLAY2_MEMORY_TRACKING
	m_Tracker.TrackFree(type, newSize);
	#endif

	m_Lock.Exit();

	if(m_SysTracker.Initialised())
	{
		Memory::AllocationNode* pAllocNode = m_SysTracker.ForgetAllocNodeForVirPhys(pAddress);

		if(pAllocNode)
		{
			m_SysTracker.TrackDeallocation(newSize, Memory::kMemType_Physical, pAllocNode);

			free(pAllocNode);
		}
	}
}

void C_MemoryManager::Lock() const
{
	m_Lock.Enter();
}

void C_MemoryManager::Unlock() const
{
	m_Lock.Exit();
}

void C_MemoryManager::GetFreeList(Btl::Vector<Sys::MemoryBlock>& blocks) const
{
	m_Lock.Enter();

	blocks.Clear();
	m_SysAllocator.GetFreeList(blocks);

	m_Lock.Exit();
}

void C_MemoryManager::PrintDebug(Debug::Logger& log) const
{
	m_SysAllocator.PrintDebug(log);
}

void C_MemoryManager::Dump(const Util::C_String& name)
{
	m_Lock.Enter();

	U32 count, largest, totalFree, totalQuick;
	m_SysAllocator.GetStats(count, largest, totalFree, totalQuick);

	LOGINFO(g_Log, "%s size(%s) used(%s) free(%s) largest(%s)",
		name.Chars(),
		Memory::MemSizeToStringK(GetSize(), false).Chars(),
		Memory::MemSizeToStringK(GetUsed(), false).Chars(),
		Memory::MemSizeToStringK(GetFree(), false).Chars(),
		Memory::MemSizeToStringK(largest, false).Chars());

	for (U32 i=0; i < E_MemoryType_Count; i++)
	{
		const C_MemoryInfo& memoryInfo = m_Tracker.m_Info[i];
		
		if (memoryInfo.m_Count > 0)
		{
			LOGINFO(g_Log, "   %16s %5i     %s\n", 
				m_Tracker.GetNameFromType(i).Chars(),
				memoryInfo.m_Count,
				Memory::MemSizeToStringK(memoryInfo.m_Size, false).Chars());
		}
	}

	m_Lock.Exit();
}

void C_MemoryManager::InsertMemoryGuardInfo(U8* pAddress, U32 size)
{
#if DISPLAY2_MEMORY_GUARDS_ENABLE
	//m_MemoryGuardLock.Enter();

	if(g_MemoryGuardBlocksCheckOnAlloc)
		CheckMemoryGuards();

	BIZ_ASSERT_SPEED(pAddress != NULL);
	BIZ_ASSERT_SPEED(m_MemoryGuardInfoList.Find(pAddress) == m_MemoryGuardInfoList.End());
	BIZ_ASSERT_SPEED(size > m_SysAllocator.BlockSize());

	C_MemoryGuardInfo* pMemoryGuardInfo = new C_MemoryGuardInfo(pAddress, size, this);
	BIZ_ASSERT_SPEED(pMemoryGuardInfo != NULL);
	m_MemoryGuardInfoList[pAddress] = pMemoryGuardInfo;

	pMemoryGuardInfo->Init(pAddress, size);
	if(g_MemoryGuardBlocksCheckInfoOnAlloc)
	{
		BIZ_ASSERT_SPEED(pMemoryGuardInfo->Valid(pAddress, size));
	}

	//m_MemoryGuardLock.Exit();
#endif
}

void C_MemoryManager::RemoveMemoryGuardInfo(U8* pAddress, U32 size)
{
#if DISPLAY2_MEMORY_GUARDS_ENABLE
	//m_MemoryGuardLock.Enter();

	BIZ_ASSERT_SPEED(pAddress != NULL);
	BIZ_ASSERT_SPEED(m_MemoryGuardInfoList.Find(pAddress) != m_MemoryGuardInfoList.End());

	C_MemoryGuardInfo* pMemoryGuardInfo = m_MemoryGuardInfoList[pAddress];
	BIZ_ASSERT_SPEED(pMemoryGuardInfo != NULL);

	if(g_MemoryGuardBlocksCheckInfoOnFree)
	{
		BIZ_ASSERT_SPEED(pMemoryGuardInfo->Valid(pAddress, size));
	}
	pMemoryGuardInfo->Free(pAddress, size);

	SAFE_DELETE_PTR(pMemoryGuardInfo);
	m_MemoryGuardInfoList.Erase(pAddress);

	if(g_MemoryGuardBlocksCheckOnFree)
		CheckMemoryGuards();

	//m_MemoryGuardLock.Exit();
#endif
}

bool C_MemoryManager::CheckMemoryGuards() const
{
#if DISPLAY2_MEMORY_GUARDS_ENABLE
	if(!g_MemoryGuardBlocksEnable)
		return true;

	m_Lock.Enter();

	C_MemoryGuardInfoMap::ConstIterator it = m_MemoryGuardInfoList.Begin();
	while(it != m_MemoryGuardInfoList.End())
	{
		U8* pAddress  = (U8*)it->GetKey();
		const C_MemoryGuardInfo* pMemoryGuardInfo = it->GetValue();
		it++;

		BIZ_ASSERT_SPEED(pAddress);
		BIZ_ASSERT_SPEED(pMemoryGuardInfo);

		if(!pMemoryGuardInfo->Valid(pAddress, pMemoryGuardInfo->m_Size))
		{
			m_Lock.Exit();

			return false;
		}
	}

	m_Lock.Exit();
#endif

	return true;
}

void C_MemoryManager::GetPrevAndNextMemoryGuardInfo(const U8* pAddress, C_MemoryGuardInfo** ppPrev, C_MemoryGuardInfo** ppNext) const
{
#if DISPLAY2_MEMORY_GUARDS_ENABLE
	BIZ_ASSERT_SPEED(pAddress);

	C_MemoryGuardInfoMap::ConstIterator it = m_MemoryGuardInfoList.Begin();
	while(it != m_MemoryGuardInfoList.End())
	{
		U8* pMemoryGuardAddress = (U8*)it->GetKey();
		C_MemoryGuardInfo* pMemoryGuardInfo = it->GetValue();
		it++;

		if(pMemoryGuardAddress < pAddress)
		{
			if(!(*ppPrev) || pMemoryGuardAddress > (*ppPrev)->m_pAddress)
				*ppPrev = pMemoryGuardInfo;
		}
		else
		if(pMemoryGuardAddress > pAddress)
		{
			if(!(*ppNext) || pMemoryGuardAddress < (*ppNext)->m_pAddress)
				*ppNext = pMemoryGuardInfo;
		}
	}
#endif
}

} // namespace Display2
