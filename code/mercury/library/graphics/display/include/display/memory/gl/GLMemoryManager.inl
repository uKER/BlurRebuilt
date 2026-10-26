namespace NewDisplay
{
namespace GL
{

INLINE const bool C_GLMemoryBuffer::IsCreated() const
{
	return false;
}

INLINE const U32 C_GLMemoryBuffer::GetSize() const
{
	return m_Size;
}

INLINE const U32 C_GLMemoryBuffer::GetAlign() const
{
	return m_Align;
}

INLINE const U32 C_GLMemoryBuffer::GetFlags() const
{
	return m_Flags;
}

INLINE U8* C_GLMemoryBuffer::GetAddress() const
{
	return NULL;
}

}; // namespace GL
}; // namespace NewDisplay
