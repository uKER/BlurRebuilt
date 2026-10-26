INLINE const bool C_IndexBuffer::IsCreated() const
{
	return m_Name != 0 ? true : false;
}

INLINE const U32 C_IndexBuffer::GetSize()
{
	return 0;
}

INLINE U8* C_IndexBuffer::GetAddress()
{
	return 0;
}

INLINE const U32 C_IndexBuffer::GetFlags()
{
	return 0;
}

INLINE const E_Format C_IndexBuffer::GetFormat()
{
	return E_Format_Unknown;
}

INLINE const S32 C_IndexBuffer::GetSystemID()
{
	return -1;
}

INLINE void C_IndexBuffer::SetSize(const U32 size)
{
}

INLINE void C_IndexBuffer::SetAddress(U8* pAddress)
{
}

INLINE void C_IndexBuffer::SetFlags(const U32 flags)
{
}

INLINE void C_IndexBuffer::SetFormat(const E_Format format)
{
}

INLINE void C_IndexBuffer::SetSystemID(const S32 systemID)
{
}
