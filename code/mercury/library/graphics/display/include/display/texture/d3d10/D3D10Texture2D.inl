INLINE const bool C_Texture2D::IsCreated() const
{
	return m_pd3dTexture2D ? true : false;
}

INLINE const U32 C_Texture2D::GetWidth()
{
	if (m_pd3dTexture2D)
	{
		D3D10_TEXTURE2D_DESC desc;
		m_pd3dTexture2D->GetDesc(&desc);
		return desc.Width;
	}
	else
	{
		return 0;
	}
}

INLINE const U32 C_Texture2D::GetHeight()
{
	if (m_pd3dTexture2D)
	{
		D3D10_TEXTURE2D_DESC desc;
		m_pd3dTexture2D->GetDesc(&desc);
		return desc.Height;
	}
	else
	{
		return 0;
	}
}

INLINE const U32 C_Texture2D::GetLevels()
{
	if (m_pd3dTexture2D)
	{
		D3D10_TEXTURE2D_DESC desc;
		m_pd3dTexture2D->GetDesc(&desc);
		return desc.MipLevels;
	}
	else
	{
		return 0;
	}
}

INLINE const E_Format C_Texture2D::GetFormat()
{
#if 1
	return E_Format_Unknown;
#else
	if (m_pd3dTexture2D)
	{
		D3D10_TEXTURE2D_DESC desc;
		m_pd3dTexture2D->GetDesc(&desc);
		switch (desc.Format)
		{
		//case DXGI_FORMAT_BC1_UNORM:
		//case DXGI_FORMAT_BC1_UNORM_SRGB:
		//	return E_Format_DXT1;

		//case DXGI_FORMAT_BC2_UNORM:
		//case DXGI_FORMAT_BC2_UNORM_SRGB:
		//	return E_Format_DXT3;

		//case DXGI_FORMAT_BC3_UNORM:
		//case DXGI_FORMAT_BC3_UNORM_SRGB:
		//	return E_Format_DXT5;

		default:
			return E_Format_Unknown;
		}
	}
	else
	{
		return E_Format_Unknown;
	}
#endif
}

INLINE const Util::C_String C_Texture2D::GetFormatAsString()
{
	if (m_pd3dTexture2D)
	{
		D3D10_TEXTURE2D_DESC desc;
		m_pd3dTexture2D->GetDesc(&desc);
		switch (desc.Format)
		{
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return "DXT1";

		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return "DXT3";

		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return "DXT5";

		default:
			return "Unknown";
		}
	}
	else
	{
		return "Unknown";
	}
}

INLINE const U32 C_Texture2D::GetSize()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_Size;
#else
	return 0;
#endif
}

INLINE const U32 C_Texture2D::GetOffset()
{
}

INLINE const Util::C_String C_Texture2D::GetFileName()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_FileName;
#else
	return "";
#endif
}

INLINE const U64 C_Texture2D::GetFileTime()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_FileTime;
#else
	return 0;
#endif
}

INLINE const S32 C_Texture2D::GetSystemID()
{
#ifdef DEVICE_DEBUG
	return GetHeader()->m_SystemID;
#else
	return -1;
#endif
}

INLINE void C_Texture2D::SetSize(const U32 size)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_Size = size;
#endif
}

INLINE void C_Texture2D::SetOffset(const U32 offset)
{
}

INLINE void C_Texture2D::SetFileName(const char* fileName)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_FileName = fileName;
#endif
}

INLINE void C_Texture2D::SetFileTime(const U64 fileTime)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_FileTime = fileTime;
#endif
}

INLINE void C_Texture2D::SetSystemID(const S32 systemID)
{
#ifdef DEVICE_DEBUG
	GetHeader()->m_SystemID = systemID;
#endif
}
