INLINE const bool C_PixelShader::IsCreated() const
{
	return m_pd3dPixelShader ? true : false;
}

INLINE const U32 C_PixelShader::GetSize()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_Size;
#else
	return 0;
#endif
}

INLINE const U32 C_PixelShader::GetOffset()
{
	return 0;
}

INLINE const Util::C_String C_PixelShader::GetFileName()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_FileName;
#else
	return "";
#endif
}

INLINE const U64 C_PixelShader::GetFileTime()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_FileTime;
#else
	return 0;
#endif
}

INLINE const S32 C_PixelShader::GetSystemID()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_SystemID;
#else
	return -1;
#endif
}

INLINE void C_PixelShader::SetSize(const U32 size)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_Size = size;
#endif
}

INLINE void C_PixelShader::SetOffset(const U32 offset)
{
}

INLINE void C_PixelShader::SetFileName(const char* fileName)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_FileName = fileName;
#endif
}

INLINE void C_PixelShader::SetFileTime(const U64 fileTime)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_FileTime = fileTime;
#endif
}

INLINE void C_PixelShader::SetSystemID(const S32 systemID)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_SystemID = systemID;
#endif
}
