#ifndef DISPLAY_MEMORY_MEMORY_TRACKER_INL
#define DISPLAY_MEMORY_MEMORY_TRACKER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE C_MemoryCounter::C_MemoryCounter(const S32 id) : m_ID(id),
	m_pParent((C_MemoryCounter*)m_TLSpParent.GetData())
{
	m_TLSpParent.SetData(this);
}

INLINE C_MemoryCounter::~C_MemoryCounter()
{
	m_TLSpParent.SetData(m_pParent);
}

INLINE const S32 C_MemoryTracker::MemoryGetSystemID() const
{
	C_MemoryCounter* pCounter = (C_MemoryCounter*)C_MemoryCounter::m_TLSpParent.GetData();
	return pCounter ? pCounter->m_ID : 1;
}

INLINE const S32 C_MemoryTracker::MemoryGetSystemCount() const
{
	return m_System.Size();
}

INLINE C_MemorySystem* C_MemoryTracker::MemoryGetSystem(const S32 id)
{
	assert(m_System.Size() > 1);
	return id >= 0 && id < m_System.Size() ? &m_System[id] : &m_System[1];
}

}; // namespace Display

#endif // DISPLAY_MEMORY_MEMORY_TRACKER_INL
