#include <stdafx.h>

using namespace Aftk;

U32 C_MemoryManager::m_referenceCount = 0;
Ut::SSSMultiMemoryPool* C_MemoryManager::m_pMultiMemoryPool = 0;

bool C_MemoryManager::PoolReserve(U32 elementSize, U32 capacityInBytes)
{
	return m_pMultiMemoryPool->ReserveBytes(elementSize, capacityInBytes);
}

void* C_MemoryManager::PoolAllocate(U32 elementSize)
{
	return m_pMultiMemoryPool->Allocate(elementSize);
}

void C_MemoryManager::PoolFree(U32 elementSize, void* pItem)
{
	m_pMultiMemoryPool->Free(elementSize, pItem);
}

///
///Returns the current memory usage in bytes.
U32 C_MemoryManager::GetMemoryUsage()
{
	U32 bytes = 0;
	for (U32 i = 0; i < m_pMultiMemoryPool->NumPools(); i++)
	{
		if (m_pMultiMemoryPool->PoolPtr(i))
			bytes += m_pMultiMemoryPool->PoolPtr(i)->CapacityBytes();
	}
	return bytes;
}

U32 C_MemoryManager::GetNumPools()
{
	return m_pMultiMemoryPool->NumPools();
}

Ut::SSSMemoryPool* C_MemoryManager::GetPoolPtr(U32 index)
{
	return m_pMultiMemoryPool->PoolPtr(index);
}

bool C_MemoryManager::Create()
{
	if (m_referenceCount == 0)
	{
		m_pMultiMemoryPool = new Ut::SSSMultiMemoryPool();
		m_pMultiMemoryPool->Create();
	}
	m_referenceCount++;

	return true;
}

void C_MemoryManager::Destroy()
{
	if (m_referenceCount > 1)
	{
		m_referenceCount--;
		return;
	}

	if (m_referenceCount != 0)
	{
		m_pMultiMemoryPool->Destroy();
		delete m_pMultiMemoryPool;
		m_referenceCount = 0;
	}
}

U32 C_MemoryManager::CollectGarbage()
{
	if (m_pMultiMemoryPool)
		return m_pMultiMemoryPool->CollectGarbage();
	return 0;
}
