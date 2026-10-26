/** \file jobqueue_mem.h
	JobQueue SPU memory management abstraction layer.

	This module implements a memory abstraction between Cell SPU memory architectures and standard memory architectures, for use by JobQueue modules.  
	It is intended that modules can share the same code for both architectures using the provided abstractions.

	The Cell SPU architecture has both system memory, which can only be accessed by the general purpose main processor, and SPU local memory, which can only 
	be accessed by the individual SPU processors.  DMA transfers can be used to copy data between the two memory types.

	Standard architectures have only system memory.  Note that JobQueue modules running on the Cell PPU fall into the standard architecture.

	The abstraction is primarily based around the Ptr template class, which on SPU architectures stores both a system memory and SPU local memory pointer.
	Various operations are provided to copy data between the two pointers of a Ptr object, and to allocate the SPU local memory for the Ptr.
	On standard architectures, Ptr contains only a single pointer so these operations do nothing.

	This module is also intended to include various memory management options for partitioning SPU local memory.  Currently all that is available is the
	linear memory allocator, which starts at the end of kernel memory and allocates forward.  No free operation is provided.  This is the base memory allocator
	used by the kernel, so all other memory allocators should use it to allocate their heaps.
*/
#ifndef JOBQUEUE_MEM_H
#define JOBQUEUE_MEM_H

#include <string.h>

#include "system/scheduler/core/tl_thread.h"

#if TL_PLAT_PS3_SPU
#include <spu_mfcio.h>
#include <cell/dma.h>
#include "system/scheduler/jobqueue_ps3.h"
#endif

//! Useful macro to isolate individual SPU-specific statements.
#if TL_PLAT_PS3_SPU
#define JQ_SPU( x ) x
#else
#define JQ_SPU( x ) 
#endif

//to eliminate meaningless warning messages
#if !(TL_PLAT_PS3 || TL_PLAT_PS3_SPU || TL_PLAT_GC || TL_PLAT_REV)
#pragma warning(disable:4100)
#endif

//! Ptr is a dual-memory pointer which has values for both EA (main memory) and LS (local storage).
/** 
	Depending on which processor the code is compiled for it will evaluate to the "local" option, or you can also access the values directly.

	This is a useful abstraction technique, since it allows you to use Ptr objects to hide the difference between architectures that
	have two memory types and those that only have one.  Use Ptr objects for all pointers, and invoke the appropriate Fetch / Store
	operations on each.  which allows you to ignore SPU memory altogether and just call FetchAlloc on each data object 
	referenced by the module.

	\remark Note that sizeof(Ptr) changes between the PS3 and Xenon/PC/Generic jobqueue implementations.  Do not store them in
	cross platform file formats.
*/
namespace Spu
{

#if TL_PLAT_PS3 || TL_PLAT_PS3_SPU

template <typename T>
class Ptr
{
public:
	T* EA;
	T* LS;
	Ptr<T>() {}
#if TL_PLAT_PS3_SPU
	T*& RawPtr() { return LS; }
	const T* RawPtr() const { return LS; }
#else
	T*& RawPtr() { return EA; }
	const T* RawPtr() const { return EA; }
#endif
	Ptr<T>(T* v) { RawPtr() = v; }
	const Ptr<T>& operator=(T* v) { RawPtr() = v; return *this; }
	operator T*() { return RawPtr(); }
	operator const T*() const { return RawPtr(); }
	const T* operator -> () const	{ return RawPtr(); }
	T* operator -> () { return RawPtr(); }
};

#else

template <typename T>
class Ptr
{
public:
	T* Value;
	Ptr<T>() {}
	T*& RawPtr() { return Value; }
	const T* RawPtr() const { return Value; }
	Ptr<T>(T* v) { RawPtr() = v; }
	const Ptr<T>& operator=(T* v) { RawPtr() = v; return *this; }
	operator T*() { return RawPtr(); }
	operator const T*() const { return RawPtr(); }
	const T* operator -> () const	{ return RawPtr(); }
	T* operator -> () { return RawPtr(); }
};

#endif

//! Copies data from system memory to SPU local memory.
/** You can think of Fetch as a kind of one-way memcpy from system memory to SPU local memory.  To copy in the other direction, use Store.

	This function is blocking, meaning that the data will be present at dest when the function returns.

	When compiling JobQueue modules for non-SPU systems, this function simply calls memcpy(dest, src, size), so it is possible to write code
	once for both SPU and non-SPU architectures.

	\param dest Destination address in SPU local memory.  Must be 16 byte aligned.
	\param src	Source address in system memory.  Must be 16 byte aligned.
	\param size Size in bytes to copy.  Must be 16 byte aligned.
*/
void* Fetch(void* dest, const void* src, unsigned size);
void* FetchAligned(void* dest, const void* src, unsigned size);

//! Copies data from SPU local memory to system memory.
/** You can think of Store as a kind of one-way memcpy from SPU local memory to system memory.  To copy in the other direction, use Fetch.

	This function is blocking, meaning that the data will be present when the function returns.

	When compiling JobQueue modules for non-SPU systems, this function simply calls memcpy(dest, src, size), so it is possible to write code
	once for both SPU and non-SPU architectures.

	\param dest Destination address in system memory.  Must be 16 byte aligned.
	\param src	Source address in SPU local memory.  Must be 16 byte aligned.
	\param size Size in bytes to copy.  Must be 16 byte aligned.
*/
void Store(void* dest, const void* src, unsigned size);

//! Asynchronously copies data from system memory to SPU local memory.
/** This function is an asynchronous analog to \ref Fetch.  

	The copy can be marked with an arbitrary tag from 0-31, which is used to wait for it to finish.  Multiple fetch calls can use the same tag.  

	This function is non-blocking, meaning that the function will return immediately and the data will arrive sometime later.  You can wait
	for the transfer to finish by calling \ref Wait with the same tag you passed to FetchAsync.

	When compiling JobQueue modules for non-SPU systems, this function simply calls memcpy(dest, src, size), so it is possible to write code
	once for both SPU and non-SPU architectures.

	\param dest Destination address in SPU local memory.  Must be 16 byte aligned.
	\param src	Source address in system memory.  Must be 16 byte aligned.
	\param size Size in bytes to copy.  Must be 16 byte aligned.
	\param tag  Arbitrary tag value from 0-31.
*/
void* FetchAsync(void* dest, const void* src, unsigned size, unsigned tag);

//! Asynchronously copies data from SPU local memory to system memory.
/** This function is an asynchronous analog to \ref Store.  

	The copy can be marked with an arbitrary tag from 0-31, which is used to wait for it to finish.  Multiple store calls can use the same tag.  

	This function is non-blocking, meaning that the function will return immediately and the data will arrive sometime later.  You can wait
	for the transfer to finish by calling \ref Wait with the same tag you passed to StoreAsync.  You must not modify the SPU local
	memory used by the transfer until after you have waited for the transfer to finish.

	When compiling JobQueue modules for non-SPU systems, this function simply calls memcpy(dest, src, size), so it is possible to write code
	once for both SPU and non-SPU architectures.

	\param dest Destination address in system memory.  Must be 16 byte aligned.
	\param src	Source address in SPU local memory.  Must be 16 byte aligned.
	\param size Size in bytes to copy.  Must be 16 byte aligned.
	\param tag  Arbitrary tag value from 0-31.
*/
void StoreAsync(void* dest, const void* src, unsigned size, unsigned tag);

//! Waits for all transfers using the given tag to complete.
/** This function stalls until all transfers using the given tag have completed.  If no transfers are active, it returns immediately.

	When compiling JobQueue modules for non-SPU systems this function does nothing. 

	\param tag  Arbitrary tag value from 0-31.
*/
void Wait(unsigned tag);

//! Waits for all transfers using any of the given set of tags to complete.
/** This function is like \ref Wait, but can wait on multiple tags simultaneously.  Combine the tags by ORing 1<<tag into the mask, for each tag.

	This function stalls until all transfers using the given tags have completed.  If no transfers are active, it returns immediately.

	When compiling JobQueue modules for non-SPU systems this function does nothing. 

	\param tag  Mask value where each bit represents an arbitrary tag from 0-31.
*/
void WaitMultiple(unsigned mask);

//! Copies data from the system memory address of a Ptr object to its SPU local memory address.
/**
	This function performs a \ref Fetch on a Ptr<T> object, which contains both system memory and SPU local memory pointers.

	See \ref Fetch for details about the function's operation.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr  Ptr object containing system memory and SPU local pointers.
	\param size Size in bytes to copy.  Must be 16 byte aligned.
*/
template <typename T>
inline void Fetch(Ptr<T> ptr, unsigned size)
{
#if TL_PLAT_PS3_SPU
	Fetch(ptr.LS, ptr.EA, size);
#endif
}

//! Copies data from the SPU local memory address of a Ptr object to its system memory address.
/**
	This function performs a \ref Store on a Ptr<T> object, which contains both system memory and SPU local memory pointers.

	See \ref Store for details about the function's operation.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr  Ptr object containing system memory and SPU local pointers.
	\param size Size in bytes to copy.  Must be 16 byte aligned.
*/
template <typename T>
inline void Store(Ptr<T>& ptr, unsigned size)
{
#if TL_PLAT_PS3_SPU
	Store(ptr.EA, ptr.LS, size);
#endif
}

//! Asynchronously copies data from the system memory address of a Ptr object to its SPU local memory address.
/**
	This function performs a \ref FetchAsync on a Ptr<T> object, which contains both system memory and SPU local memory pointers.

	See \ref FetchAsync for details about the function's operation.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr  Ptr object containing system memory and SPU local pointers.
	\param size Size in bytes to copy.  Must be 16 byte aligned.
	\param tag  Arbitrary tag value from 0-31.
*/
template <typename T>
inline void FetchAsync(Ptr<T> ptr, unsigned size)
{
#if TL_PLAT_PS3_SPU
	FetchAsync(ptr.LS, ptr.EA, size);
#endif
}

//! Asynchronously copies data from the SPU local memory address of a Ptr to its system memory address.
/**
	This function performs a \ref StoreAsync on a Ptr<T> object, which contains both system memory and SPU local memory pointers.

	See \ref StoreAsync for details about the function's operation.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr  Ptr object containing system memory and SPU local pointers.
	\param size Size in bytes to copy.  Must be 16 byte aligned.
	\param tag  Arbitrary tag value from 0-31.
*/
template <typename T>
inline void StoreAsync(const Ptr<T>& ptr, unsigned size, unsigned tag)
{
#if TL_PLAT_PS3_SPU
	StoreAsync(ptr.EA, ptr.LS, size, tag);
#endif
}

//! Sets a new base value for the linear memory allocator.  This function can be used to free previously allocated memory by resetting the allocator's base address.
/**
	When compiling JobQueue modules for non-SPU systems the linear memory allocator is not enabled, so this function does nothing. 

	\param ptr  Address of the new linear memory allocator base.  Must be 16 byte aligned.
*/
void SetMemBase(const void* ptr);

//! Sets a new value for the size of the SPU stack.  This limits how much memory the linear allocator will return before erroring.
/**
	When compiling JobQueue modules for non-SPU systems the linear memory allocator is not enabled, so this function does nothing. 

	\param size New size of the SPU stack.  Must be 16 byte aligned.
*/
void SetStackSize(int size);

//! Returns the amount of SPU memory available.
/**
	When compiling JobQueue modules for non-SPU systems the linear memory allocator is not enabled, so this function returns 0.

	SPU memory availability is limited by the size of the kernel (16k), the size of the module's executable code, and the size
	of the stack.

	\return The number of bytes of SPU memory available.
*/
int GetMemAvailable();

//! Allocates memory from the linear memory allocator.
/**
	When compiling JobQueue modules for non-SPU systems the linear memory allocator is not enabled, so this function returns NULL.

	\param size  Size of the block to allocate.  Must be 16 byte aligned.

	\return A pointer to the allocated memory.
*/
void* Alloc(unsigned size);

//! Returns the current allocation address of the linear memory allocator.
void* GetMemBase();

//! Utility class which resets the linear memory allocator to its original position when the variable leaves scope.
/**
	Intended to be allocated on the stack, this class provides a convenient way to save and restore the state of
	the linear memory allocator around the current scope.

	It also provides a method to manually reset the allocator, so you can do it multiple times within a scope.
*/
class MemBaseMarker
{
public:
	void* MemBaseRestore;
	MemBaseMarker() { MemBaseRestore = GetMemBase(); }
	~MemBaseMarker() { SetMemBase(MemBaseRestore); }
	void Restore() { SetMemBase(MemBaseRestore); }
};

//! Copies data from system memory to SPU local memory, which is allocated from the linear memory allocator.
/**
	When compiling JobQueue modules for non-SPU systems the linear memory allocator is not enabled, so this function returns the source pointer.
	This is a useful abstraction technique, which allows you to ignore SPU memory altogether and just call FetchAlloc on each data object 
	referenced by the module.

	\param ptr   System memory address to copy form.  Must be 16 byte aligned.
	\param size  Size of the block to allocate.  Must be 16 byte aligned.

	\return The location of the data in SPU local memory.
*/
inline void* FetchAlloc(const void* ptr, unsigned size)
{
#if TL_PLAT_PS3_SPU
	void* dest = Alloc(size);
	Fetch(dest, ptr, size);
	return dest;
#else
	return (void*)ptr;
#endif
}

//! Copies data from system memory to SPU local memory, which is allocated from the linear memory allocator, without requiring the source data to be 16 byte aligned.
/**
	When compiling JobQueue modules for non-SPU systems the linear memory allocator is not enabled, so this function returns the source pointer.
	This is a useful abstraction technique, which allows you to ignore SPU memory altogether and just call FetchAlloc on each data object 
	referenced by the module.

	\param ptr   System memory address to copy form.  
	\param size  Size of the block to allocate.  

	\return The location of the data in SPU local memory.
*/
inline void* FetchAllocAlign(const void* ptr, unsigned size)
{
#if TL_PLAT_PS3_SPU

	U32	addressRoundDown = U32(ptr) & ~0xF;
	U32	alignOffset	= U32(ptr) & 0xF;
	U32	sizeRoundUp	= size + alignOffset;
	sizeRoundUp = (sizeRoundUp + 0xF) & ~0xF;

	return ((U8*) Spu::FetchAlloc((void*)addressRoundDown, sizeRoundUp)) + alignOffset;
#else
	return (void*)ptr;
#endif
}

//! Asynchronously copies data from system memory to SPU local memory, which is allocated from the linear memory allocator.
/**
	When compiling JobQueue modules for non-SPU systems the linear memory allocator is not enabled, so this function returns the source pointer.
	This is a useful abstraction technique, which allows you to ignore SPU memory altogether and just call FetchAlloc on each data object 
	referenced by the module.

	\param ptr   System memory address to copy form.  Must be 16 byte aligned.
	\param size  Size of the block to allocate.  Must be 16 byte aligned.
	\param tag   Arbitrary tag value from 0-31.

	\return The location of the data in SPU local memory.
*/
inline void* FetchAllocAsync(const void* ptr, unsigned size, unsigned tag)
{
#if TL_PLAT_PS3_SPU
	void* dest = Alloc(size);
	FetchAsync(dest, ptr, size, tag);
	return dest;
#else
	return (void*)ptr;
#endif
}

//! Copies data from system memory to SPU local memory, which is allocated from the linear memory allocator, and stored in a Ptr object.
/**
	This function copies data from the system memory pointer contained in the Ptr object.  The SPU local memory pointer in the Ptr object
	is filled in with the allocated address.

	See \ref FetchAlloc and \ref Fetch for more information.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr   Ptr object containing system memory address to copy from, whose local storage address will be filled in.  Must be 16 byte aligned.
	\param size  Size of the block to allocate.  Must be 16 byte aligned.
*/
template <typename T>
inline void FetchAlloc(Ptr<T>& ptr, size_t size)
{
#if TL_PLAT_PS3_SPU
	ptr.LS = (T*)FetchAlloc(ptr.EA, size);
#endif
}


//! Asynchronously copies data from system memory to SPU local memory, which is allocated from the linear memory allocator, and stored in a Ptr object.
/**
	This function copies data from the system memory pointer contained in the Ptr object.  The SPU local memory pointer in the Ptr object
	is filled in with the allocated address.

	See \ref FetchAllocAsync and \ref FetchAsync for more information.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr   Ptr object containing system memory address to copy from, whose local storage address will be filled in.  Must be 16 byte aligned.
	\param size  Size of the block to allocate.  Must be 16 byte aligned.
	\param tag   Arbitrary tag value from 0-31.
*/
template <typename T>
inline void FetchAllocAsync(Ptr<T>& ptr, unsigned size, unsigned tag)
{
#if TL_PLAT_PS3_SPU
	ptr.LS = (T*)FetchAllocAsync(ptr.EA, size, tag);
#endif
}

template <typename T>
inline T* FetchAllocAsync(T* ptr, unsigned tag)
{
#if TL_PLAT_PS3_SPU
	return (T*)FetchAllocAsync(ptr, sizeof(T), tag);
#else
	return ptr;
#endif
}

// Copies data from system memory to SPU local memory, which is allocated from the linear memory allocator.  The transfer size is automatically determined.
/**
	This function replaces the value of the input pointer with the newly allocated address.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr   Pointer value containing system memory address to copy from, whose address will be filled in.  Must be 16 byte aligned.
	\param size  Size of the block to allocate.  Must be 16 byte aligned.
*/
template <typename T>
void FetchAlloc(T*& p)
{
#if TL_PLAT_PS3_SPU
	p = (T*)FetchAlloc(p, tl_align(sizeof(T),16));
#endif
}

// Copies an array from system memory to SPU local memory, which is allocated from the linear memory allocator.  The transfer size is automatically determined.
/**
	This function replaces the value of the input pointer with the newly allocated address.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr   Pointer value containing system memory address to copy from, whose address will be filled in.  Must be 16 byte aligned.
	\param count Number of array elements to copy.
*/
template <typename T>
void FetchAllocArray(T*& p, int count=1)
{
  p = (T*)FetchAlloc(p, tl_align(sizeof(T)*count,16));
}

// Copies unaligned data from system memory to SPU local memory, which is allocated from the linear memory allocator.
/**
	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr   Pointer value containing system memory address to copy from, whose address will be filled in.  Must be 16 byte aligned.
	\param size  Size in bytes to allocate and copy.

  \return The newly allocated address containing the fetched data.
*/
inline void* FetchAllocUnaligned(const void* src, U32 size)
{
#if TL_PLAT_PS3_SPU
	U32 align = (U32)src&0xf;
	void* addr = FetchAlloc((const void*)((U32)src & ~0xf), tl_align(size+align, 16));
	addr = (void*)((U32)addr + align);
	return addr;
#else
	return (void*)src;
#endif
}

// Asynchronously copies unaligned data from system memory to SPU local memory, which is allocated from the linear memory allocator.
/**
	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr   Pointer value containing system memory address to copy from, whose address will be filled in.  Must be 16 byte aligned.
	\param size  Size in bytes to allocate and copy.

  \return The newly allocated address containing the fetched data.
*/
inline void* FetchAllocUnalignedAsync(const void* src, U32 size, unsigned tag)
{
#if TL_PLAT_PS3_SPU
	U32 align = (U32)src&0xf;
	void* addr = FetchAllocAsync((const void*)((U32)src & ~0xf), tl_align(size+align, 16), tag);
	addr = (void*)((U32)addr + align);
	return addr;
#else
	return (void*)src;
#endif
}


//! Copies unaligned data from system memory to SPU local memory, which is allocated from the linear memory allocator, and stored in a Ptr object.
/**
	This function copies data from the system memory pointer contained in the Ptr object.  The SPU local memory pointer in the Ptr object
	is filled in with the allocated address.

	See \ref FetchAlloc and \ref Fetch for more information.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr   Ptr object containing system memory address to copy from, whose local storage address will be filled in.  Must be 16 byte aligned.
	\param size  Size of the block to allocate. 
*/
template <typename T>
inline void FetchAllocUnaligned(Ptr<T>& ptr, size_t size)
{
#if TL_PLAT_PS3_SPU
	ptr.LS = (T*)FetchAllocUnaligned(ptr.EA, size);
#endif
}

//! Copies unaligned data from system memory to SPU local memory, which is allocated from the linear memory allocator, and stored in a Ptr object.
/**
	This function copies data from the system memory pointer contained in the Ptr object.  The SPU local memory pointer in the Ptr object
	is filled in with the allocated address.

	See \ref FetchAllocAsync and \ref FetchAsync for more information.

	When compiling JobQueue modules for non-SPU systems, there is only one memory type so this function does nothing.  See \ref Ptr
	for information on how to use this feature to write cross platform JobQueue modules.

	\param ptr   Ptr object containing system memory address to copy from, whose local storage address will be filled in.  Must be 16 byte aligned.
	\param size  Size of the block to allocate. 
*/
template <typename T>
inline void FetchAllocUnalignedAsync(Ptr<T>& ptr, size_t size, unsigned tag)
{
#if TL_PLAT_PS3_SPU
	ptr.LS = (T*)FetchAllocUnalignedAsync(ptr.EA, size, tag);
#endif
}



/** Transfers and returns an unsigned 8bit value from main memory.
*/
inline U8 Get(const U8* ptr)
{
	assert(ptr);
#if TL_PLAT_PS3_SPU
	return cellDmaGetUint8((uint64_t)ptr, 0, 0, 0);
#else
	return *ptr;
#endif
}

/** Transfers and returns an unsigned 16bit value from main memory.
*/
inline U16 Get(const U16* ptr)
{
	assert(ptr);
#if TL_PLAT_PS3_SPU
	return cellDmaGetUint16((uint64_t)ptr, 0, 0, 0);
#else
	return *ptr;
#endif
}

/** Transfers and returns an unsigned 32bit value from main memory.
*/
inline U32 Get(const U32* ptr)
{
	assert(ptr);
#if TL_PLAT_PS3_SPU
	return cellDmaGetUint32((uint64_t)ptr, 0, 0, 0);
#else
	return *ptr;
#endif
}

/** Transfers and returns an unsigned 64bit value from main memory.
*/
inline U64 Get(const U64* ptr)
{
	assert(ptr);
#if TL_PLAT_PS3_SPU
	return cellDmaGetUint64((uint64_t)ptr, 0, 0, 0);
#else
	return *ptr;
#endif
}

//! Transfers and returns a signed 8bit value from main memory.
inline S8 Get(const S8* ptr) { return (S8)Get((U8*)ptr); }

//! Transfers and returns a signed 16bit value from main memory.
inline S16 Get(const S16* ptr) { return (S16)Get((U16*)ptr); }

//! Transfers and returns a signed 32bit value from main memory.
inline S32 Get(const S32* ptr) { return (int)Get((U32*)ptr); }

//! Transfers and returns a signed 64bit value from main memory.
inline S64 Get(const S64* ptr) { return (S64)Get((U64*)ptr); }

//! Transfers and returns a signed floating point value from main memory.
inline float Get(const float* ptr) { return tlIntAsFloat(Get((U32*)ptr)); }

//! Transfers an unsigned 8bit value to main memory.
inline void Set(U8* ptr, U8 val)
{
	assert(ptr);
#if TL_PLAT_PS3_SPU
	cellDmaPutUint8(val, (uint64_t)ptr, 0, 0, 0);
#else
	*ptr = val;
#endif
}

//! Transfers an unsigned 16bit value to main memory.
inline void Set(U16* ptr, U16 val)
{
	assert(ptr);
#if TL_PLAT_PS3_SPU
	cellDmaPutUint16(val, (uint64_t)ptr, 0, 0, 0);
#else
	*ptr = val;
#endif
}

//! Transfers an unsigned 32bit value to main memory.
inline void Set(U32* ptr, U32 val)
{
	assert(ptr);
#if TL_PLAT_PS3_SPU
	cellDmaPutUint32(val, (uint64_t)ptr, 0, 0, 0);
#else
	*ptr = val;
#endif
}

//! Transfers an unsigned 64bit value to main memory.
inline void Set(U64* ptr, U64 val)
{
	assert(ptr);
#if TL_PLAT_PS3_SPU
	cellDmaPutUint64(val, (uint64_t)ptr, 0, 0, 0);
#else
	*ptr = val;
#endif
}

//! Transfers and returns a signed 8bit value from main memory.
inline void Get(S8* ptr, S8 val) { Set((U8*)ptr, (U8)val); }

//! Transfers and returns a signed 16bit value from main memory.
inline void Get(S16* ptr, S16 val) { Set((U16*)ptr, (U16)val); }

//! Transfers and returns a signed 32bit value from main memory.
inline void Get(S32* ptr, S32 val) { Set((U32*)ptr, (U32)val); }

//! Transfers and returns a signed 64bit value from main memory.
inline void Get(S64* ptr, S64 val) { Set((U64*)ptr, (U64)val); }

//! Transfers and returns a signed floating point value from main memory.
inline void Get(float* ptr, float val) { Set((U32*)ptr, tlFloatAsInt(val)); }

//Moving some functions into here if not SPU, because they're empty so letting compiler optimise them away
#if !TL_PLAT_PS3_SPU
inline void jqCheckStack()
{
}

inline void Wait(unsigned tag) 
{
}

inline void WaitMultiple(unsigned mask) 
{
}

inline void SetMemBase(const void* ptr) 
{
}

inline void SetStackSize(int size) 
{
}

inline int GetMemAvailable()
{
	return 0;
}

inline void* Alloc(unsigned size) 
{
	return NULL;
}

inline void* GetMemBase()
{
	return NULL;
}
#endif //TL_PLAT_PS3_SPU

// similar to Spu::FetchAlloc, but allocates memory from the stack instead of from the linear allocator
#if PLATFORM_SPU
template <typename T>
class __attribute__(( aligned( 16 ) )) FetchLocal
{
public:
	FetchLocal(const T *src)
	{
		U32 align = (U32)src&0xf;
		void* addr = Spu::Fetch(m_data, (const void*)((U32)src - align), tl_align(sizeof(T)+align, 16));
		m_p = (T*)((U32)addr + align);
	}

	const T* operator -> () const { return (const T*)m_p; }
	T* operator -> () { return m_p; }
	const T* Ptr() { return (const T*)m_p; }
	const T& operator *() const {return *m_p;}

private:
	U8 m_data[sizeof(T)+32];
	T *m_p;
};
#else
template <typename T>
class FetchLocal
{
public:
	FetchLocal(const T *p)
	{
		m_p = p;
	}

	const T* operator -> () const { return m_p; }
	T* operator -> () { return (T*)m_p; }
	const T* Ptr() const { return m_p; }

private:
	const T *m_p;
};
#endif


}; // namespace Spu

#endif // JOBQUEUE_MEM_H
