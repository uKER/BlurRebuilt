#ifndef AFTK_MEMORY_MANAGER_H
#define AFTK_MEMORY_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_MemoryManager
	{
	public:
		static bool PoolReserve(U32 elementSize, U32 capacityInBytes);

		static void* PoolAllocate(U32 elementSize);

		static void PoolFree(U32 elementSize, void* pItem);

		///
		///Returns the number of pools.
		static U32 GetNumPools();

		static Ut::SSSMemoryPool* GetPoolPtr(U32 index);

		///
		///Returns the current memory usage in bytes.
		static U32 GetMemoryUsage();

		static U32 CollectGarbage();

	private:
		static bool Create();

		static void Destroy();

		static U32 m_referenceCount;
		static Ut::SSSMultiMemoryPool*			m_pMultiMemoryPool;

		friend class C_Global;
	};
}
#endif
