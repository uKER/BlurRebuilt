#ifndef __MERCURY_SYSTEM_ATOMIC_H__
#define __MERCURY_SYSTEM_ATOMIC_H__

#ifdef PLATFORM_PC
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>
typedef __int64 LONG64;
#endif
#ifdef PLATFORM_PC
#include <intrin.h>
#undef assert
#include <core\utility\CoreDebug.h>
#endif

#ifdef PLATFORM_PS3
#include <cell/atomic.h>
#if CELL_SDK_VERSION < 0x270000
#include "ppu_intrinsics_gcc.h"
#else
#include "ppu_intrinsics.h"
#endif
#endif
#ifdef PLATFORM_SPU
#include <cell/atomic.h>
#endif


#ifdef PLATFORM_PC
#undef PLATFORM_PC
#define PLATFORM_PC 1
#endif

/// atomic operations on U32 and U64 datatypes

namespace MT
{

/** \defgroup Atomic operations.
	Atomic operations are those which read, modify, and write a variable without letting any other thread interfere during the operation.
	@{
*/

/** Inserts a memory read/write barrier.

	Calling this function prevents any future memory loads from executing until after any prior memory stores have completed.

	Since memory architectures support reordering memory operations to improve performance, it is impossible to guarantee that
	a memory operation has occured based on reading the result of another operation, unless a barrier is used.

	\warning The XBox 360 CPU can even reorder memory operations *across* interlocked functions!

	There are two ways to use memory barriers, called 'acquire' semantics and 'release' semantics.  When coordinating a memory
	operation between two threads, *both* semantics must be used by their respective threads.

	See "Interlock Functions" in the Xenon SDK documentation for more information.
*/
inline void MemoryFence()
{
#if PLATFORM_PC
	// On PC, all interlocked methods include an implicit MemoryFence.
	long a=0;
	InterlockedExchange(&a, 0);
#endif
#if PLATFORM_PS3 && !TL_COMPILER_SNC
	__lwsync();
#endif
#if PLATFORM_REVOLUTION
    PPCSync();
#endif
}

/** Atomically assigns a new value to a 32bit variable, if it previously was equal to a given comparison value.

	The following code snippet describes the operation of this function.

	\code
	if (*var == compare)
	{
		*var = newvalue;
		return true;
	}
	else
		return false;
	\endcode

	\return true if newvalue was assigned to variable.
*/
inline bool AtomicCompareAndSwap(register volatile U32* var, register U32 newvalue, register U32 compare)
{
#if PLATFORM_PC
	return InterlockedCompareExchange((long*)var, newvalue, compare) == (long)compare;
#endif
#if PLATFORM_PS3
	return cellAtomicCompareAndSwap32((uint32_t*)var, compare, newvalue) == compare;
#endif
#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	return cellAtomicCompareAndSwap32(temp, (uint64_t)var, compare, newvalue) == compare;
#endif
#if PLATFORM_REVOLUTION
#if TL_COMPILER_MW
    register U32 oldval;
    asm {
        @0:
        lwarx oldval, r0, var;
        cmpw compare, oldval;
        bne @1;
        stwcx. newvalue, r0, var;
        bne @0;
        @1:
    }
    return oldval == compare;
#elif TL_COMPILER_GCC
    U32 oldval;
    asm volatile (
        "0: lwarx %0, 0, %1\n"
        "   cmpw %2, %0\n"
        "   bne 1f\n"
        "   stwcx. %3, 0, %1\n"
        "   bne 0b\n"
        "1:\n"
        : "=&r" (oldval) 
        : "r" (var), "r" (compare), "r" (newvalue), "0" (oldval)
        : "memory");
    return oldval == compare;
#endif
#endif
}

/** Atomically ADDs a 32bit variable with a value.
  \param var      Pointer to value to decrement.
  \param value    Value to add to *var.
  \return The original value of *var.
*/
inline U32 AtomicAdd(register volatile U32* var, register U32 value)
{

#if PLATFORM_PS3
	return cellAtomicAdd32((uint32_t*)var, value);
#endif // PLATFORM_PS3

#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	return cellAtomicAdd32(temp, (uint64_t)var, value);
#endif // PLATFORM_SPU

#if PLATFORM_REVOLUTION
 #if TL_COMPILER_MW
    asm {
        @0:
        lwarx r0, r0, var;
        add r0, r0, value;
        stwcx. r0, r0, var;
        bne @0;
        mr r3, r0;
        blr;
    }
 #elif TL_COMPILER_GCC
    U32 ret;
    asm volatile(
        "0: lwarx %0, 0, %1\n"
        "   add %0, %0, %2\n"
        "   stwcx. %0, 0, %1\n"
        "   bne 0b\n"
        : "=&r" (ret)
        : "r" (var), "r" (value)
        );
    return ret;
 #endif // TL_COMPILER_...
#endif // PLATFORM_REVOLUTION

#if PLATFORM_PC
	return InterlockedExchangeAdd((LONG*)var, value);
#endif // PLATFORM_PC
}

#if !PLATFORM_PC
/** Atomically ADDs a 32bit variable with a value.
  \param var      Pointer to value to decrement.
  \param value    Value to add to *var.
  \return The original value of *var.
*/
inline U64 AtomicAdd(register volatile U64* var, register U64 value)
{

#if PLATFORM_PS3
	return cellAtomicAdd64((uint64_t*)var, value);
#endif // PLATFORM_PS3

#if PLATFORM_SPU
	uint64_t temp[16] __attribute__((aligned(128)));
	return cellAtomicAdd64(temp, (uint64_t)var, value);
#endif // PLATFORM_SPU

#if PLATFORM_REVOLUTION
 #if TL_COMPILER_MW
    asm {
        @0:
        lwarx r0, r0, var;
        add r0, r0, value;
        stwcx. r0, r0, var;
        bne @0;
        mr r3, r0;
        blr;
    }
 #elif TL_COMPILER_GCC
    U32 ret;
    asm volatile(
        "0: lwarx %0, 0, %1\n"
        "   add %0, %0, %2\n"
        "   stwcx. %0, 0, %1\n"
        "   bne 0b\n"
        : "=&r" (ret)
        : "r" (var), "r" (value)
        );
    return ret;
 #endif // TL_COMPILER_...
#endif // PLATFORM_REVOLUTION

#if PLATFORM_PC
	return InterlockedExchangeAdd64((volatile LONG64*)var, value);
#endif // PLATFORM_PC
}
#endif


/** Atomically increments a 32bit variable.
  \param var Pointer to value to increment.
  \return The original value of *variable.
*/
inline int AtomicIncrement(volatile int* var)
{
#if PLATFORM_PC
	return InterlockedIncrement((long*)var) - 1;
#endif
#if PLATFORM_PS3
	return cellAtomicIncr32((uint32_t*)var);
#endif
#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	return cellAtomicIncr32(temp, (uint64_t)var);
#endif
#if PLATFORM_REVOLUTION
    return AtomicAdd((volatile U32*)var, 1);
#endif
}

/** Atomically decrements a 32bit variable.
  \param var Pointer to value to decrement.
  \return The original value of *variable.
*/
inline int AtomicDecrement(volatile int* var)
{
#if PLATFORM_PC
	return InterlockedDecrement((long*)var) + 1;
#endif
#if PLATFORM_PS3
	return cellAtomicDecr32((uint32_t*)var);
#endif
#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	return cellAtomicDecr32(temp, (uint64_t)var);
#endif
#if PLATFORM_REVOLUTION
    return AtomicAdd((volatile U32*)var, (U32)-1);
#endif
}

/** Atomically ANDs a 32bit variable with a value.
  \param var      Pointer to value to decrement.
  \param value    Value to and with *var.
  \return The original value of *var.
*/
inline U32 AtomicAnd(register volatile U32* var, register U32 value)
{
#if PLATFORM_PC
	return _InterlockedAnd((long*)var, value);
#endif
#if PLATFORM_PS3
	return cellAtomicAnd32((uint32_t*)var, value);
#endif
#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	return cellAtomicAnd32(temp, (uint64_t)var, value);
#endif
#if PLATFORM_REVOLUTION
#if TL_COMPILER_MW
    asm {
        @0:
        lwarx r0, r0, var;
        and r0, r0, value;
        stwcx. r0, r0, var;
        bne @0;
        mr r3, r0;
        blr;
    }
#elif TL_COMPILER_GCC
    U32 ret;
    asm volatile (
        "0: lwarx %0, 0, %1\n"
        "   and %0, %0, %2\n"
        "   stwcx. %0, 0, %1\n"
        "   bne 0b"
        : "=&r" (ret)
        : "r" (var), "r" (value)
        );
    return ret;
#endif
#endif
}

/** Atomically ORs a 32bit variable with a value.
  \param var      Pointer to value to decrement.
  \param value    Value to and with *var.
  \return The original value of *var.
*/
inline U32 AtomicOr(register volatile U32* var, register U32 value)
{
#if PLATFORM_PC
	return _InterlockedOr((long*)var, value);
#endif
#if PLATFORM_PS3
	return cellAtomicOr32((uint32_t*)var, value);
#endif
#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	return cellAtomicOr32(temp, (uint64_t)var, value);
#endif
#if PLATFORM_REVOLUTION
#if TL_COMPILER_MW
    asm {
        @0:
        lwarx r0, r0, var;
        or r0, r0, value;
        stwcx. r0, r0, var;
        bne @0;
        mr r3, r0;
    }
#elif TL_COMPILER_GCC
    U32 ret;
    asm volatile (
        "0: lwarx %0, 0, %1\n"
        "   or %0, %0, %2\n"
        "   stwcx. %0, 0, %1\n"
        "   bne 0b"
        : "=&r" (ret)
        : "r" (var), "r" (value)
        );
    return ret;
#endif
#endif
}

/** Atomically returns a 32bit value from memory.
	\param var   Pointer to variable.
	\return The value of *var.
*/
inline U32 AtomicGet(volatile U32* var)
{
#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	return cellAtomicOr32(temp, (uint64_t)var, 0);
#else
	return *var;
#endif
}

/** Atomically returns a 32bit signed value from memory.
	\param var   Pointer to variable.
	\return The value of *var.
*/
inline S32 AtomicGet(volatile S32* var)
{
#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	return cellAtomicOr32(temp, (uint64_t)var, 0);
#else
	return *var;
#endif
}

/** Atomically stores a 32bit value to memory.
	\param var     Pointer to variable.
	\param value   Value to store in variable.
*/
inline void AtomicSet(volatile U32* var, U32 value)
{
#if PLATFORM_SPU
	uint32_t temp[32] __attribute__((aligned(128)));
	cellAtomicStore32(temp, (uint64_t)var, value);
#else
	*var = value;
#endif
}

// Revolution cannot implement these. Threading will be broken until this gets investigated.
#if PLATFORM_PC
#if WORD_SIZE != 64
inline U64 _InterlockedCompareExchange64(volatile LONG64* dest, U64 newvalue, U64 compare)
{
	__asm 
	{
		lea esi,compare
		lea edi,newvalue
		mov eax,[esi]
		mov edx,4[esi]
		mov ebx,[edi]
		mov ecx,4[edi]
		mov esi,dest
		lock cmpxchg8b [esi]
	}
}
#ifdef InterlockedCompareExchange64
#undef InterlockedCompareExchange64
#endif
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#endif
#endif

/// Atomically assigns a new value to a 64bit variable, if it previously was equal to a given comparison value.
inline bool AtomicCompareAndSwap(volatile U64* var, U64 newvalue, U64 compare)
{
#if PLATFORM_PC
	return InterlockedCompareExchange64((LONG64*)var, newvalue, compare) == (LONG64)compare;
#endif
#if PLATFORM_PS3
	return cellAtomicCompareAndSwap64((uint64_t*)var, compare, newvalue) == compare;
#endif
#if PLATFORM_SPU
	uint64_t temp[16] __attribute__((aligned(128)));
	return cellAtomicCompareAndSwap64(temp, (uint64_t)var, compare, newvalue) == compare;
#endif
#if PLATFORM_REVOLUTION
    assert(0); // 64 bit atomic operations unsupported on Revolution
    return false;
#endif
}

#ifndef PLATFORM_PC

/// Atomically increments a 64bit variable.
inline S64 AtomicIncrement(volatile S64* var)
{
#if PLATFORM_PC
	return InterlockedIncrement64((LONG64*)var) - 1;
#endif
#if PLATFORM_PS3
	return cellAtomicIncr64((uint64_t*)var);
#endif
#if PLATFORM_SPU
	uint64_t temp[16] __attribute__((aligned(128)));
	return cellAtomicIncr64(temp, (uint64_t)var);
#endif
#if PLATFORM_REVOLUTION
    assert(0); / 64 bit atomic operations unsupported on Revolution
    return false;
#endif
}

/// Atomically decrements a 64bit variable.
inline S64 AtomicDecrement(volatile S64* var)
{
#if PLATFORM_PC
	return InterlockedDecrement64((LONG64*)var) + 1;
#endif
#if PLATFORM_PS3
	return cellAtomicDecr64((uint64_t*)var);
#endif
#if PLATFORM_SPU
	uint64_t temp[16] __attribute__((aligned(128)));
	return cellAtomicDecr64(temp, (uint64_t)var);
#endif
#if PLATFORM_REVOLUTION
    assert(0); //64 bit atomic operations unsupported on Revolution
    return false;
#endif
}

#ifndef PLATFORM_PC
/// Atomically ANDs a 64bit variable with a value.
inline U64 AtomicAnd(volatile U64* var, U64 value)
{
#if PLATFORM_PS3
	return cellAtomicAnd64((uint64_t*)var, value) & value;
#endif
#if PLATFORM_SPU
	uint64_t temp[16] __attribute__((aligned(128)));
	return cellAtomicAnd64(temp, (uint64_t)var, value) & value;
#endif
#if PLATFORM_REVOLUTION
    assert(0); // 64 bit atomic operations unsupported on Revolution
    return false;
#endif
}

/// Atomically ORs a 64bit variable with a value.
inline U64 AtomicOr(volatile U64* var, U64 value)
{
#if PLATFORM_PS3
	return cellAtomicOr64((uint64_t*)var, value) | value;
#endif
#if PLATFORM_SPU
	uint64_t temp[16] __attribute__((aligned(128)));
	return cellAtomicOr64(temp, (uint64_t)var, value) | value;
#endif
#if PLATFORM_REVOLUTION
    assert(0); // 64 bit atomic operations unsupported on Revolution
    return false;
#endif
}
#endif

#endif // !PLATFORM_PC

#if PLATFORM_PC && defined(_WIN64)
// [Blur2PC][x64] Pointer-wide atomic add (e.g. the contact stream write pointer).
inline U64 AtomicAdd(volatile U64* var, U64 value)
{
	return InterlockedExchangeAdd64((volatile LONG64*)var, value);
}
#endif

/** Atomically returns a 64bit value from memory.
	\param var      Pointer to variable.
	\return The value of *var.
*/
inline U64 AtomicGet(volatile U64* var)
{
#if PLATFORM_SPU
	uint64_t temp[16] __attribute__((aligned(128)));
	return cellAtomicOr64(temp, (uint64_t)var, 0);
#else
	return *var;
#endif
#if PLATFORM_REVOLUTION
    assert(0); // 64 bit atomic operations unsupported on Revolution
    return false;
#endif
}

/** Atomically stores a 64bit value to memory.
	\param var      Pointer to variable.
	\param value    Value to store in variable.
*/
inline void AtomicSet(volatile U64* var, U64 value)
{
#if PLATFORM_SPU
	uint64_t temp[16] __attribute__((aligned(128)));
	cellAtomicStore64(temp, (uint64_t)var, value);
#else
	*var = value;
#endif
#if PLATFORM_REVOLUTION
    assert(0); // 64 bit atomic operations unsupported on Revolution
#endif
}

}; // namespace MT

#endif
