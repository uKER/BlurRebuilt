INLINE const bool C_PixelShader::IsCreated() const
{
	return m_cgProgram ? true : false;
}

INLINE const U32 C_PixelShader::GetSize()
{
	return 0;
}

INLINE U8* C_PixelShader::GetAddress()
{
	return 0;
}

INLINE const Util::C_String C_PixelShader::GetFileName()
{
	return m_FileName;
}

INLINE const U64 C_PixelShader::GetFileTime()
{
	return m_FileTime;
}

INLINE const S32 C_PixelShader::GetSystemID()
{
	return -1;
}

INLINE void C_PixelShader::SetSize(const U32 size)
{
}

INLINE void C_PixelShader::SetAddress(U8* pAddress)
{
}

INLINE void C_PixelShader::SetFileName(const char* fileName)
{
	m_FileName = fileName;
}

INLINE void C_PixelShader::SetFileTime(const U64 fileTime)
{
	m_FileTime = fileTime;
}

INLINE void C_PixelShader::SetSystemID(const S32 systemID)
{
}
