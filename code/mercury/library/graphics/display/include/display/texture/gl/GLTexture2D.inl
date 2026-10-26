INLINE const U32 C_Texture2D::GetWidth()
{
	return 0;
}

INLINE const U32 C_Texture2D::GetHeight()
{
	return 0;
}

INLINE const U32 C_Texture2D::GetLevels()
{
	return 0;
}

INLINE const E_Format C_Texture2D::GetFormat()
{
	return E_Format_Unknown;
}

INLINE const bool C_Texture2D::IsCreated() const
{
	return m_glID != 0 ? true : false;
}

INLINE const U32 C_Texture2D::GetSize()
{
	return 0;
}

INLINE U8* C_Texture2D::GetAddress()
{
	return 0;
}

INLINE const U32 C_Texture2D::GetFlags()
{
	return m_Flags;
}

INLINE const Util::C_String C_Texture2D::GetFileName()
{
	return "";
}

INLINE const U64 C_Texture2D::GetFileTime()
{
	return 0;
}

INLINE const S32 C_Texture2D::GetSystemID()
{
	return -1;
}

INLINE void C_Texture2D::SetSize(const U32 size)
{
}

INLINE void C_Texture2D::SetAddress(U8* pAddress)
{
}

INLINE void C_Texture2D::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

INLINE void C_Texture2D::SetFileName(const char* fileName)
{
}

INLINE void C_Texture2D::SetFileTime(const U64 fileTime)
{
}

INLINE void C_Texture2D::SetSystemID(const S32 systemID)
{
}
