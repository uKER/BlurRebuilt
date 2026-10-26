INLINE const bool C_TextureCube::IsCreated() const
{
	return m_glID != 0 ? true : false;
}

INLINE const U32 C_TextureCube::GetSize()
{
	return 0;
}

INLINE U8* C_TextureCube::GetAddress()
{
	return 0;
}

INLINE const Util::C_String C_TextureCube::GetFileName()
{
	return "";
}

INLINE const U64 C_TextureCube::GetFileTime()
{
	return 0;
}

INLINE const S32 C_TextureCube::GetSystemID()
{
	return -1;
}

INLINE void C_TextureCube::SetSize(const U32 size)
{
}

INLINE void C_TextureCube::SetAddress(U8* pAddress)
{
}

INLINE void C_TextureCube::SetFileName(const char* fileName)
{
}

INLINE void C_TextureCube::SetFileTime(const U64 fileTime)
{
}

INLINE void C_TextureCube::SetSystemID(const S32 systemID)
{
}
