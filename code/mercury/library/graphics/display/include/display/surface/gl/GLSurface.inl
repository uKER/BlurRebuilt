INLINE const bool C_Surface::IsCreated() const
{
	return m_glID != 0 ? true : false;
}

INLINE const U32 C_Surface::GetWidth() const
{
	if (!IsCreated())
		return 0;

	return 0; // TODO: Return width.
}

INLINE const U32 C_Surface::GetHeight() const
{
	if (!IsCreated())
		return 0;

	return 0; // Return height.
}

INLINE const E_Format C_Surface::GetFormat() const
{
	return E_Format_Unknown; // TODO: Return format.
}

INLINE const U32 C_Surface::GetSize()
{
#if defined(PLATFORM_PS3) || defined(DEVICE_DEBUG)
	return GetHeader()->m_Size;
#else
	return 0;
#endif
}

INLINE const U32 C_Surface::GetFlags()
{
#if defined(PLATFORM_PS3) || defined(DEVICE_DEBUG)
	return GetHeader()->m_Flags;
#else
	return 0;
#endif
}

INLINE U8* C_Surface::GetAddress()
{
#if defined(PLATFORM_PS3)
	return GetHeader()->m_pAddress;
#else
	return 0;
#endif
}

INLINE const S32 C_Surface::GetSystemID()
{
#if defined(DEVICE_DEBUG)
	return GetHeader()->m_SystemID;
#else
	return -1;
#endif
}

INLINE void C_Surface::SetSize(const U32 size)
{
#if defined(PLATFORM_PS3) || defined(DEVICE_DEBUG)
	GetHeader()->m_Size = size;
#endif
}

INLINE void C_Surface::SetFlags(const U32 flags)
{
#if defined(PLATFORM_PS3) || defined(DEVICE_DEBUG)
	GetHeader()->m_Flags = flags;
#endif
}

INLINE void C_Surface::SetAddress(U8* pAddress)
{
#if defined(PLATFORM_PS3)
	GetHeader()->m_pAddress = pAddress;
#endif
}

INLINE void C_Surface::SetSystemID(const S32 systemID)
{
#if defined(DEVICE_DEBUG)
	GetHeader()->m_SystemID = systemID;
#endif
}
