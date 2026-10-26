#ifndef __GNUC__
#pragma once
#endif
#ifndef _POINTERSLOT32_H_
#define _POINTERSLOT32_H_

// [Blur2PC][x64] 32-bit slots that carry pointers.
//
// Some engine data formats reserve a 32-bit word for a pointer that is filled in
// at runtime: the FX sampler parameter (nglFXParam_Sampler::Texture, two words in
// every effect's converter-built ParameterData) and the material texture-pointer
// element (MATERIAL_ELEMENT_TYPE_TEXTURE_PTR). Widening those words would move
// every converter-baked parameter offset after them, so on x64 the word holds a
// slot number and this registry maps it back to the pointer.
//
// On x86 the slot is the pointer itself, so that build is unchanged.
//
// A slot is assigned per distinct address and never reused, so a slot means
// exactly what the 32-bit pointer would have meant: a stale slot refers to the
// same address a stale pointer would have. Slot 0 is NULL.

#include <core/Core.h>
#include <core/BizAssert.h>

#if defined(_WIN64)
#include <mutex>
#include <unordered_map>
#include <atomic>
#endif

namespace Core
{

#if defined(_WIN64)

namespace PointerSlot32Impl
{
	enum
	{
		PageBits = 12,
		PageSize = 1 << PageBits,
		MaxPages = 4096,	// 16M slots
	};

	struct Registry
	{
		std::mutex                          lock;
		std::unordered_map<const void*, U32> slots;
		std::atomic<void**>                 pages[MaxPages];
		U32                                 count;

		Registry() : count(1)	// slot 0 is NULL
		{
			for (int i = 0; i < MaxPages; ++i)
				pages[i].store(NULL, std::memory_order_relaxed);
		}
	};

	inline Registry& Get()
	{
		static Registry s_registry;	// one instance across translation units (inline function)
		return s_registry;
	}
}

/// Slot number for a pointer; the same address always yields the same slot.
inline U32 PointerToSlot32(const void* p)
{
	if (p == NULL)
		return 0;
	PointerSlot32Impl::Registry& r = PointerSlot32Impl::Get();
	std::lock_guard<std::mutex> guard(r.lock);
	std::unordered_map<const void*, U32>::const_iterator it = r.slots.find(p);
	if (it != r.slots.end())
		return it->second;
	const U32 slot = r.count;
	const U32 page = slot >> PointerSlot32Impl::PageBits;
	BIZ_ASSERT(page < PointerSlot32Impl::MaxPages);
	void** entries = r.pages[page].load(std::memory_order_relaxed);
	if (entries == NULL)
	{
		entries = new void*[PointerSlot32Impl::PageSize];
		r.pages[page].store(entries, std::memory_order_release);
	}
	entries[slot & (PointerSlot32Impl::PageSize - 1)] = const_cast<void*>(p);
	r.slots[p] = slot;
	r.count = slot + 1;
	return slot;
}

/// Pointer for a slot number from PointerToSlot32. Lock-free: a slot is only
/// handed out after its page and entry are written.
inline void* Slot32ToPointer(U32 slot)
{
	if (slot == 0)
		return NULL;
	void** entries = PointerSlot32Impl::Get().pages[slot >> PointerSlot32Impl::PageBits].load(std::memory_order_acquire);
	BIZ_ASSERT(entries != NULL);
	return entries[slot & (PointerSlot32Impl::PageSize - 1)];
}

#else

inline U32 PointerToSlot32(const void* p)
{
	return (U32)p;
}

inline void* Slot32ToPointer(U32 slot)
{
	return (void*)slot;
}

#endif

} // namespace Core

#endif // _POINTERSLOT32_H_
