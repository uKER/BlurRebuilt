INLINE const bool C_VertexBuffer::IsCreated() const
{
	return m_Name != 0 ? true : false;
}

INLINE const U32 C_VertexBuffer::GetSize()
{
	return 0;
}

INLINE U8* C_VertexBuffer::GetAddress()
{
	return 0;
}

INLINE const U32 C_VertexBuffer::GetFlags()
{
	return 0;
}

INLINE const S32 C_VertexBuffer::GetSystemID()
{
	return -1;
}

INLINE void C_VertexBuffer::SetSize(const U32 size)
{
}

INLINE void C_VertexBuffer::SetAddress(U8* pAddress)
{
}

INLINE void C_VertexBuffer::SetFlags(const U32 flags)
{
}

INLINE void C_VertexBuffer::SetSystemID(const S32 systemID)
{
}
