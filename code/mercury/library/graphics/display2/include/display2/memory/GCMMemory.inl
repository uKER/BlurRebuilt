#ifndef DISPLAY2_GCM_MEMORY_INL
#define DISPLAY2_GCM_MEMORY_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

// C_MemoryBuffer

INLINE U32 C_MemoryBuffer::GetSize() const
{
	return m_Size;
}

INLINE U32 C_MemoryBuffer::GetAlign() const
{
	return m_Align;
}

INLINE U32 C_MemoryBuffer::GetFlags() const
{
	return m_Flags;
}

INLINE U32 C_MemoryBuffer::GetOffset() const
{
	return m_Offset;
}

INLINE U8* C_MemoryBuffer::GetAddress() const
{
	return m_pAddress;
}

INLINE E_MemoryBuffer C_MemoryBuffer::GetType() const
{
	return m_Type;
}

} // namespace Display2

#endif // DISPLAY2_GCM_MEMORY_INL
