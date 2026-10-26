#ifndef DISPLAY_MEMORY_GCM_GCM_MEMORY_MANAGER_INL
#define DISPLAY_MEMORY_GCM_GCM_MEMORY_MANAGER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace GCM
{

INLINE const bool C_GCMMemoryBuffer::IsCreated() const
{
	return m_pAddress ? true : false;
}

INLINE const U32 C_GCMMemoryBuffer::GetSize() const
{
	return m_Size;
}

INLINE const U32 C_GCMMemoryBuffer::GetAlign() const
{
	return m_Align;
}

INLINE const U32 C_GCMMemoryBuffer::GetFlags() const
{
	return m_Flags;
}

INLINE U8* C_GCMMemoryBuffer::GetAddress() const
{
	return m_pAddress;
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_MEMORY_GCM_GCM_MEMORY_MANAGER_INL
