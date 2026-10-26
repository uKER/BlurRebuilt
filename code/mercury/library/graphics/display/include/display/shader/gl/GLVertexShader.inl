INLINE const bool C_VertexShader::IsCreated() const
{
	return m_cgProgram ? true : false;
}

INLINE const U32 C_VertexShader::GetSize()
{
	return 0;
}

INLINE U8* C_VertexShader::GetAddress()
{
	return 0;
}

INLINE const Util::C_String C_VertexShader::GetFileName()
{
	return m_FileName;
}

INLINE const U64 C_VertexShader::GetFileTime()
{
	return m_FileTime;
}

INLINE const S32 C_VertexShader::GetSystemID()
{
	return -1;
}

INLINE void C_VertexShader::SetSize(const U32 size)
{
}

INLINE void C_VertexShader::SetAddress(U8* pAddress)
{
}

INLINE void C_VertexShader::SetFileName(const char* fileName)
{
	m_FileName = fileName;
}

INLINE void C_VertexShader::SetFileTime(const U64 fileTime)
{
	m_FileTime = fileTime;
}

INLINE void C_VertexShader::SetSystemID(const S32 systemID)
{
}
