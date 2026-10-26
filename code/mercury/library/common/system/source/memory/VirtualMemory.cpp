#include "SystemPch.h"

#include <system/memory/virtualmemory.h>

#if defined( PLATFORM_PC )
#include <windows.h>
#elif defined( PLATFORM_PS3 )
#include <sys/memory.h>
#else
#include <sys/mman.h>
#endif

namespace Memory
{

#if defined( COMPILER_VC )
// Microsoft Windows/Xbox implementation

/// Allocate a block of virtual address space
void *AllocVirtual(U64 cSize)
{
	DWORD allocationFlags = MEM_RESERVE;
	return ::VirtualAlloc(0, (SIZE_T)cSize, allocationFlags, PAGE_READWRITE);
}

/// Free a block of virtual address space
void FreeVirtual(void *pAddress)
{
	::VirtualFree(pAddress, 0, MEM_RELEASE);
}

/// Map a block of physical memory into virtual address space
void* AllocPhysical(void *pAddress, U64 cSize)
{
    DWORD allocationFlags = MEM_COMMIT;

	return ::VirtualAlloc( pAddress, (SIZE_T)cSize, allocationFlags, PAGE_READWRITE ); // (PT) Use 64KB pages here?  Variable sized pages?
}

/// Free a block of physical memory
/// (frees the physical memory that is mapped to a virtual address [iAddress])
void FreePhysical(void *pAddress)
{
	::VirtualFree( pAddress, 1, MEM_DECOMMIT);
}

/// Returns true if the given memory address is valid for read access
bool AddressReadAccess(void *pAddress)
{
	// WARNING: IsBadReadPtr is an incredibly dangerous function to call, it can cause
	// very subtle bugs which can be very difficult to track down.  Do not use it!  If
	// you feel you need to use it come speak with Stephen Cakebread and I'll explain why
	// you really don't want to use it.
	return true;//!IsBadReadPtr(pAddress, 4);
}

#elif defined( PLATFORM_PS3 )
// PS3 implementation
#define VIRTUAL_PAGE_FLAGS 128
bool		m_bVirtualInitialized = false;
U32			m_VirtualPageUsage[ VIRTUAL_PAGE_FLAGS ];  
sys_addr_t	m_VirtualBaseAddress = 0;

/// Allocate a block of virtual address space
void *AllocVirtual(U64 cSize)
{
	sys_addr_t address = 0;

	if (!m_bVirtualInitialized)
	{
		int ret = sys_mmapper_allocate_address( 0x10000000, SYS_MEMORY_PAGE_SIZE_64K | SYS_MEMORY_ACCESS_RIGHT_ANY, 0x10000000, &m_VirtualBaseAddress);
		BIZ_ASSERT_SPEED(ret == CELL_OK);
		m_bVirtualInitialized = true;
		memset(&m_VirtualPageUsage, 0x00, sizeof(m_VirtualPageUsage));
	}
	
	// find unallocated page
	for (int p = 0 ; p < VIRTUAL_PAGE_FLAGS ; p++)
	{
		if ( m_VirtualPageUsage[p] != 0xffffffff )
		{
			// Free page somewhere here
			for (int i = 0 ; i < 32 ; i++)
			{
				if (! (m_VirtualPageUsage[p] & (1 << i) )) // Unused page
				{
					m_VirtualPageUsage[p] |= 1 << i; // Set this page to used
					return (void*)(m_VirtualBaseAddress + address);
				}
				address += 1 << 16; // add 65536 to address for next block
			}
			// Should never get here
			DebugBreak();
			return NULL;
		}
		else
		{	// add 65536 * 32 to address
			address += 1 << 21;
		}
	}
	// Out of Memory!!
	return (void*)NULL;
}

/// Free a block of virtual address space
void FreeVirtual(void *pAddress)
{
	// find allocated page
	if (m_bVirtualInitialized)
	{
		sys_addr_t address = (sys_addr_t)pAddress - m_VirtualBaseAddress;
		
		int p = address >> 21;
		int m = address >> 16 & 0x1f; 
		if (m_VirtualPageUsage[p] & (1 << m))
		{
			m_VirtualPageUsage[p] &= ~(1 << m);
			// if all are free'd then release the whole virtual allocation
			for (int p = 0 ; p < VIRTUAL_PAGE_FLAGS ; p++)
			{
				if ( m_VirtualPageUsage[p] != 0x0 )
				{
					return;
				}
			}
			sys_mmapper_free_address(m_VirtualBaseAddress);
			m_bVirtualInitialized = false;
		}
		else
		{
			printf("Attempting to free a non-allocated block\n");
			return;
		}
	}
}

/// Map a block of physical memory into virtual address space
void* AllocPhysical(void *pAddress, U64 cSize)
{
	sys_memory_t memory_id;
	if (__builtin_expect(CELL_OK != sys_mmapper_allocate_memory(cSize, SYS_MEMORY_GRANULARITY_64K, &memory_id),0))
	{	
		pAddress = NULL;
		BIZ_ASSERT_SPEED(0 && "Failed to allocate unmapped memory");
		return;
	}
	
	int ret = sys_mmapper_map_memory((sys_addr_t)pAddress, memory_id, SYS_MEMORY_PROT_READ_WRITE);
	BIZ_ASSERT_SPEED(ret == CELL_OK);

	return (ret == CELL_OK) ? pAddress : NULL;
}

/// Free a block of physical memory
void FreePhysical(void *pAddress)
{
	sys_memory_t memory_id;
	sys_mmapper_unmap_memory((sys_addr_t)pAddress, &memory_id);

	sys_mmapper_free_memory(memory_id);
}

/// Returns true if the given memory address is valid for read access
bool AddressReadAccess(void *pAddress)
{
	sys_page_attr_t attr;
	sys_memory_get_page_attribute((sys_addr_t)pAddress, &attr);
	return (attr.access_right & SYS_MEMORY_ACCESS_RIGHT_PPU_THR);
}

#else
// Linux implementation
// this implementation simply commits a chunk of 500MB physical memory.
// not much difference between this and a malloc, except that this is page-aligned

/// Allocate a block of virtual address space
void *AllocVirtual(U64 cSize)
{
	// allocate a block of anonymous virtual address space
	void *p = mmap(0, cSize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	
	printf("%x\n", p);

	for (int i=0; i<cSize; i++)
	{
		((char*)p)[i] = 0xcd;
	}

	return p;
}

/// Free a block of virtual address space
void FreeVirtual(void *pAddress)
{
	munmap(pAddress, 0);
}

/// Map a block of physical memory into virtual address space
void* AllocPhysical(void *pAddress, U64 cSize)
{
//	void *p = mmap(0, cSize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
//	printf("allocatephysical(%x, %x) = %x\n", (int)pAddress, (int)cSize, (int)p);
	return pAddress;
}

/// Free a block of physical memory
void FreePhysical(void *pAddress)
{
//	munmap(pAddress, 0);
}

/// Returns true if the given memory address is valid for read access
bool AddressReadAccess(void *pAddress)
{
	assert(0);
	return true;
}

#endif

};
