#ifndef DISPLAY_MEMORY_MEMORY_MANAGER_INL
#define DISPLAY_MEMORY_MEMORY_MANAGER_INL

namespace Display
{

INLINE const S32 C_MemoryBuffer::GetAllocs() const
{
	return m_Allocs;
}

INLINE const U32 C_MemoryBuffer::GetUsed() const
{
	return m_Used;
}

INLINE const U32 C_MemoryBuffer::GetFree() const
{
	return m_Free;
}

}; // namespace Display

#endif // DISPLAY_MEMORY_MEMORY_MANAGER_INL
