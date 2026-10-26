INLINE const bool C_Texture3D::IsCreated() const
{
	return m_glID != 0 ? true : false;
}

INLINE const U32 C_Texture3D::GetSize()
{
	return 0;
}

INLINE U8* C_Texture3D::GetAddress()
{
	return 0;
}

INLINE const Util::C_String C_Texture3D::GetFileName()
{
	return "";
}

INLINE const U64 C_Texture3D::GetFileTime()
{
	return 0;
}

INLINE const S32 C_Texture3D::GetSystemID()
{
	return -1;
}

INLINE void C_Texture3D::SetSize(const U32 size)
{
}

INLINE void C_Texture3D::SetAddress(U8* pAddress)
{
}

INLINE void C_Texture3D::SetFileName(const char* fileName)
{
}

INLINE void C_Texture3D::SetFileTime(const U64 fileTime)
{
}

INLINE void C_Texture3D::SetSystemID(const S32 systemID)
{
}
