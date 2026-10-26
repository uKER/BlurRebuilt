#ifndef __SYSTEM_VIRTUALMEMORY_H__
#define __SYSTEM_VIRTUALMEMORY_H__

namespace Memory
{

/// Allocate a block of virtual address space
void *AllocVirtual(U64 cSize);

/// Free a block of virtual address space
void FreeVirtual(void *pAddress);

/// Allocate and map a block of physical memory into virtual address space
void* AllocPhysical(void *pAddress, U64 cSize);

/// Unmap a block of physical memory and free it
/// (frees the physical memory that is mapped to a virtual address [iAddress])
void FreePhysical(void *pAddress);

/// Returns true if the given memory address is valid for read access
bool AddressReadAccess(void *pAddress);

};

#endif
