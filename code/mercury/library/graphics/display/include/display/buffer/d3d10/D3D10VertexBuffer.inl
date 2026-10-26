INLINE const bool C_VertexBuffer::IsCreated() const
{
	return m_pd3dBuffer ? true : false;
}

INLINE const U32 C_VertexBuffer::GetSize()
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

INLINE const U32 C_VertexBuffer::GetOffset()
{
	return 0;
}

INLINE const U32 C_VertexBuffer::GetFlags()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_Flags;
#else
	return 0;
#endif
}

INLINE const S32 C_VertexBuffer::GetSystemID()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_SystemID;
#else
	return -1;
#endif
}

INLINE void C_VertexBuffer::SetSize(const U32 size)
{
}

INLINE void C_VertexBuffer::SetOffset(const U32 offset)
{
}

INLINE void C_VertexBuffer::SetFlags(const U32 flags)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_Flags = flags;
#endif
}

INLINE void C_VertexBuffer::SetSystemID(const S32 systemID)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_SystemID = systemID;
#endif
}
