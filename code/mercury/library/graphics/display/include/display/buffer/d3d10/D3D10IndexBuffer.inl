INLINE const bool C_IndexBuffer::IsCreated() const
{
	return m_pd3dBuffer ? true : false;
}

INLINE const U32 C_IndexBuffer::GetSize()
{
	if (m_pd3dBuffer)
	{
		D3D10_BUFFER_DESC desc;
		m_pd3dBuffer->GetDesc(&desc);
		return desc.ByteWidth;
	}
	else
	{
		return 0;
	}
}

INLINE const U32 C_IndexBuffer::GetOffset()
{
}

INLINE const U32 C_IndexBuffer::GetFlags()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_Flags;
#else
	return 0;
#endif
}

INLINE const S32 C_IndexBuffer::GetSystemID()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_SystemID;
#else
	return -1;
#endif
}

INLINE const E_Format C_IndexBuffer::GetFormat()
{
	return m_Format;
}

INLINE void C_IndexBuffer::SetSize(const U32 size)
{
}

INLINE void C_IndexBuffer::SetOffset(const U32 offset)
{
}

INLINE void C_IndexBuffer::SetFlags(const U32 flags)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_Flags = flags;
#endif
}

INLINE void C_IndexBuffer::SetSystemID(const S32 systemID)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_SystemID = systemID;
#endif
}

INLINE void C_IndexBuffer::SetFormat(const E_Format format)
{
	m_Format = format;
}
