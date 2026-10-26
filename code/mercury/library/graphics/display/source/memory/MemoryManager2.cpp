#include "stdafx.h"
#include "Display/Memory/MemoryManager2.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{

bool MemoryBuffer::Create(U32 size, U32 align, U32 flags)
{
	m_Size = size;
	m_Align = align;
	m_Flags = flags;

	return CreateAddress();
}

void MemoryBuffer::Destroy()
{
	DestroyAddress();
}

//////////////////////////////////////////////////////////////////////////////

C_MemoryBuffer::C_MemoryBuffer()
{
#if USE_DISPLAY2_MEMORY_MANAGER
	#if PLATFORM_PS3
	m_Type			= GCM::E_GCMMemoryBufferType_Local;
	#endif
	m_pDisplay2		= NULL;
#else
	m_Allocs		= 0;
#endif
}

C_MemoryBuffer::~C_MemoryBuffer()
{
}

#if USE_DISPLAY2_MEMORY_MANAGER

Display2::C_MemoryManager* C_MemoryBuffer::GetDisplay2()
{
	BIZ_ASSERT(m_pDisplay2);
	return m_pDisplay2;
}

const Display2::C_MemoryManager* C_MemoryBuffer::GetDisplay2() const
{
	BIZ_ASSERT(m_pDisplay2);
	return m_pDisplay2;
}

void C_MemoryBuffer::SetDisplay2(Display2::C_MemoryManager* pDisplay2)
{
	BIZ_ASSERT(pDisplay2);
	m_pDisplay2 = pDisplay2;
}

#endif

const S32 C_MemoryBuffer::GetAllocs() const
{
#if USE_DISPLAY2_MEMORY_MANAGER
	return 0;
#else
	return m_Allocs;
#endif
}

const U32 C_MemoryBuffer::GetUsed() const
{
#if USE_DISPLAY2_MEMORY_MANAGER
	return GetDisplay2()->GetUsed();
#else
	return m_allocator.GetTotalAllocated();
#endif
}

const U32 C_MemoryBuffer::GetFree() const
{
#if USE_DISPLAY2_MEMORY_MANAGER
	return GetDisplay2()->GetFree();
#else
	return m_allocator.GetTotalFree();
#endif
}

bool C_MemoryBuffer::Create(const U32 size, const U32 align, const U32 flags, const char* bufferName)
{
#if USE_DISPLAY2_MEMORY_MANAGER
	Display2::E_MemoryBuffer bufferType = Display2::E_MemoryBuffer_Unknown;
#if PLATFORM_PS3
	switch(m_Type)
	{
	case GCM::E_GCMMemoryBufferType_Main:
		bufferType = Display2::E_MemoryBuffer_Main;
		break;
	case GCM::E_GCMMemoryBufferType_Local:
		bufferType = Display2::E_MemoryBuffer_Local;
		break;
	case GCM::E_GCMMemoryBufferType_CmdBuf:
		bufferType = Display2::E_MemoryBuffer_Indirect;
		break;
	default:
		LOGERROR(g_Log, "Unsupported memory buffer type %i\n", m_Type);
		return false;
	}
#endif
	return GetDisplay2()->Create(size, align, flags, bufferType, bufferName);
#else
	// reserve memory
	bool success = m_buffer.Create(size, align, flags);

	m_memoryTracker.Initialize(bufferName, Memory::Tracker::Instance(), size);
	
	if (!success)
		return false;

	return m_allocator.Initialise(m_buffer.GetAddress(), size, align, 4);
#endif
}

#if defined(DEVICE_PS3GCM)

void C_MemoryBuffer::SetType(const GCM::E_GCMMemoryBufferType type)
{
#if USE_DISPLAY2_MEMORY_MANAGER
	m_Type = type;
#else
	m_buffer.SetType(type);
#endif
}

#endif

void C_MemoryBuffer::Destroy()
{
#if USE_DISPLAY2_MEMORY_MANAGER
	GetDisplay2()->Destroy();
#else
	m_allocator.Shutdown();
	m_buffer.Destroy();

	m_Allocs = 0;
#endif
}

bool C_MemoryBuffer::AllocateFromStart(const U32 align, const U32 size, U8** ppAddress)
{
#if USE_DISPLAY2_MEMORY_MANAGER
	return GetDisplay2()->Alloc(align, size, ppAddress, Display2::E_MemoryType_Misc);
#else
	m_Lock.Enter();

	U8* p = m_allocator.Allocate(size, align);

	if(p)
		m_Allocs++;

	*ppAddress = p;

	m_Lock.Exit();

	// Main memory tracker
	if (p != NULL && m_memoryTracker.Initialised())
	{
		Memory::AllocationNode* allocNode = (Memory::AllocationNode*) malloc(m_memoryTracker.TrackingStructureSize() + 4);

		m_memoryTracker.RememberAllocNodeForVirPhys(allocNode, p);

		m_memoryTracker.TrackAllocation(p,					// allocation address
			size,											// original size
			size,											// actual size (the difference is used to calculate memory wastage)
			Memory::kMemType_Physical,						// physical memory flag
			allocNode,										// address of tracking struct
			0												// Callstack overhead
			);
	}

	return p != 0;
#endif
}

bool C_MemoryBuffer::AllocateFromEnd(const U32 align, const U32 size, U8** ppAddress)
{
	return AllocateFromStart(align, size, ppAddress);
}

void C_MemoryBuffer::Free(const U32 size, U8* pAddress)
{
#if USE_DISPLAY2_MEMORY_MANAGER
	GetDisplay2()->Free(size, pAddress, Display2::E_MemoryType_Misc);
#else
	m_Lock.Enter();

	m_allocator.Free(pAddress, size);

	m_Allocs--;

	if (pAddress != NULL && m_memoryTracker.Initialised())
	{
		Memory::AllocationNode* allocNode = m_memoryTracker.ForgetAllocNodeForVirPhys(pAddress);

		if (allocNode)
		{
			m_memoryTracker.TrackDeallocation(size, Memory::kMemType_Physical, allocNode);

			free(allocNode);
		}
	}

	m_Lock.Exit();
#endif
}

void C_MemoryBuffer::PrintDebug(Debug::Logger& log) const
{
#if USE_DISPLAY2_MEMORY_MANAGER
	GetDisplay2()->PrintDebug(log);
#else
	m_allocator.PrintDebug(log);
#endif
}

const U8* C_MemoryBuffer::GetAddress() const
{
#if USE_DISPLAY2_MEMORY_MANAGER
	return GetDisplay2()->GetAddress();
#else
	return m_allocator.BufferBegin();
#endif
}

U32 C_MemoryBuffer::GetSize() const
{
#if USE_DISPLAY2_MEMORY_MANAGER
	return GetDisplay2()->GetSize();
#else
	return m_allocator.GetTotalSize();
#endif
}

void C_MemoryBuffer::GetFreeList(Btl::Vector<Sys::MemoryBlock>& blocks) const
{
#if USE_DISPLAY2_MEMORY_MANAGER
	GetDisplay2()->GetFreeList(blocks);
#else
	blocks.Clear();
	m_allocator.GetFreeList(blocks);
#endif
}

}
