INLINE const bool C_VertexShader::IsCreated() const
{
	return m_pd3dVertexShader ? true : false;
}

INLINE const U32 C_VertexShader::GetSize()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_Size;
#else
	return 0;
#endif
}

INLINE const U32 C_VertexShader::GetOffset()
{
	return 0;
}

INLINE const Util::C_String C_VertexShader::GetFileName()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_FileName;
#else
	return "";
#endif
}

INLINE const U64 C_VertexShader::GetFileTime()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_FileTime;
#else
	return 0;
#endif
}

INLINE const S32 C_VertexShader::GetSystemID()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_SystemID;
#else
	return -1;
#endif
}

INLINE void C_VertexShader::SetSize(const U32 size)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_Size = size;
#endif
}

INLINE void C_VertexShader::SetOffset(const U32 offset)
{
}

INLINE void C_VertexShader::SetFileName(const char* fileName)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_FileName = fileName;
#endif
}

INLINE void C_VertexShader::SetFileTime(const U64 fileTime)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_FileTime = fileTime;
#endif
}

INLINE void C_VertexShader::SetSystemID(const S32 systemID)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_SystemID = systemID;
#endif
}
