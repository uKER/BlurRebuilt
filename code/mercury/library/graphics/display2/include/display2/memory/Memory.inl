#ifndef DISPLAY2_MEMORY_INL
#define DISPLAY2_MEMORY_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

INLINE bool C_MemoryManager::IsCreated() const
{
	return m_Buffer.GetAddress() ? true : false;
}

INLINE U32 C_MemoryManager::GetSize() const
{
	return m_SysAllocator.GetTotalSize();
}

INLINE U32 C_MemoryManager::GetUsed() const
{
	return m_SysAllocator.GetTotalAllocated();
}

INLINE U32 C_MemoryManager::GetFree() const
{
	return m_SysAllocator.GetTotalFree();
}

INLINE const U8* C_MemoryManager::GetAddress() const
{
	return m_SysAllocator.BufferBegin();
}

INLINE bool C_MemoryManager::Inside(const U32 size, const U8* pAddress) const
{
	return m_Buffer.Inside(size, pAddress);
}

INLINE C_MemoryBuffer* C_MemoryManager::GetBuffer()
{
	return &m_Buffer;
}

INLINE const C_MemoryBuffer* C_MemoryManager::GetBuffer() const
{
	return &m_Buffer;
}

INLINE C_MemoryTracker* C_MemoryManager::GetTracker()
{
	return &m_Tracker;
}

INLINE const C_MemoryTracker* C_MemoryManager::GetTracker() const
{
	return &m_Tracker;
}

} // namespace Display2

#endif // DISPLAY2_MEMORY_INL
