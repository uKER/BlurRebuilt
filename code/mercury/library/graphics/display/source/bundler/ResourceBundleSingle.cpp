#include "stdafx.h"
#include <system/serialise.h>

#include <Display/Bundler/ResourceBundlerDefines.h>
#include <Display/Texture/Texture.h>
#include <Display/Texture/Texture2D.h>
#include <Display/Texture/Texture3D.h>
#include <Display/Texture/TextureCube.h>

#include <display/bundler/BundleHeader.h>
#include <display/bundler/ResourceHeader.h>
#include <display/bundler/ResourceTexture.h>
#include <display/bundler/ResourceTexture2D.h>
#include <display/bundler/ResourceTextureCubeMap.h>
#include <display/bundler/ResourceTextureArray2D.h>
#include <display/bundler/ResourceTexture3D.h>
#include <display/bundler/ResourceBundleReader.h>
#include <Display/Bundler/I_ResourceFactory.h>
#include <display/bundler/I_ResourceBundle.h>
#include <display/bundler/ResourceBundleSingle.h>
#include <display/device/Device.h>

using namespace Display::ResourceBundler;

C_ResourceBundleSingle::C_ResourceBundleSingle() : m_FileName("")
{
	m_Flags			= 0;
	m_SystemID		= -1;
	m_SysMemorySize	= 0;
	m_pSysMemory	= NULL;
	m_VidMemorySize	= 0;
	m_pVidMemory	= NULL;
}

C_ResourceBundleSingle::~C_ResourceBundleSingle()
{
	for (S32 i=0; i < m_Texture2D.Size(); i++)
		Device()->Destroy(&m_Texture2D[i]);
	m_Texture2D.Clear();
	
	SAFE_DELETE_ARY(m_pSysMemory);
	C_ResourceBundleSingle::FreeVideoMemory();
}

Display::E_Format C_ResourceBundleSingle::GetFormat(const U32 format)
{
	switch(format)
	{
	case ResourceBundler::E_TextureFormat_DXT1:
		return Display::E_Format_DXT1;
	case ResourceBundler::E_TextureFormat_DXT3:
		return Display::E_Format_DXT3;
	case ResourceBundler::E_TextureFormat_DXT5:
		return Display::E_Format_DXT5;
	case ResourceBundler::E_TextureFormat_A8R8G8B8:
		return Display::E_Format_A8R8G8B8;
	case ResourceBundler::E_TextureFormat_L8:
		return Display::E_Format_L8;
	default:
		LOGFATAL(g_Log, "Unsupported format %i\n", format);
		assert(0 && "Unsupported format");
		return E_Format_Unknown;
	}
}

bool C_ResourceBundleSingle::Create(const Sys::C_FilePath& fileName)
{
	m_FileName = fileName;

	if (!C_ResourceBundleReader::Read(
		m_SysMemorySize,
		m_pSysMemory,
		m_VidMemorySize,
		m_pVidMemory,
		*this,
		fileName))
		return false;
	
	#if defined PLATFORM_PC
	FreeVideoMemory();
	#endif

	return true;
}

bool C_ResourceBundleSingle::Create(Sys::I_File& file)
{
	m_FileName = file.Name().Chars();

	if (!C_ResourceBundleReader::Read(
		m_SysMemorySize,
		m_pSysMemory,
		m_VidMemorySize,
		m_pVidMemory,
		*this,
		file))
		return false;

	#if defined PLATFORM_PC
	FreeVideoMemory();
	#endif

	return true;
}

void C_ResourceBundleSingle::Destroy()
{
	for (S32 i=0; i<m_Texture2D.Size(); i++)
		Device()->Destroy(&m_Texture2D[i]);
	m_Texture2D.Clear();

	SAFE_DELETE_ARY(m_pSysMemory);
	FreeVideoMemory();
}

const S32 C_ResourceBundleSingle::GetTexture2DCount()
{
	return m_Texture2D.Size();
}

Display::C_RefTexture* C_ResourceBundleSingle::GetTexture2D(const S32 textureIndex)
{
	if (textureIndex >= 0 && textureIndex < m_Texture2D.Size())
	{
		return &m_Texture2D[textureIndex];
	}
	else
	{
		LOGWARN(g_Log, "Failed to get texture '%i' from bundle '%s'\n", textureIndex, m_FileName.Chars());
		return 0;
	}
}

Display::C_RefTexture* C_ResourceBundleSingle::GetTexture2D(const Util::C_String& textureName)
{
	Util::C_String lowerTextureName(textureName);
	lowerTextureName.ConvertToLower();
	for (S32 i=0; i<m_Texture2D.Size(); i++)
	{
		if (lowerTextureName == m_Texture2D[i].GetFileName())
		{
			return &m_Texture2D[i];
		}
	}
	LOGWARN(g_Log, "Failed to get texture '%s' from bundle '%s'\n", textureName.Chars(), m_FileName.Chars());
	return 0;
}

const U32 C_ResourceBundleSingle::ResourceCount() const
{
	return m_Texture2D.Size();
}

const char* C_ResourceBundleSingle::GetResourceName(const S32 index)
{
	if (index >= 0 && index < m_Texture2D.Size())
	{
		return m_Texture2D[index].GetFileName().Chars();
	}
	else
	{
		LOGWARN(g_Log, "Failed to get texture '%i' from bundle '%s'\n", index, m_FileName.Chars());
		return 0;
	}
}

void C_ResourceBundleSingle::OnCreateHeader(
	const C_BundleHeader& bundleHeader,
	const U8* pResourceHeaders,
	U32 numTexture2D,
	U32 numTextureCubeMap,
	U32 numTextureArray2D,
	U32 numTexture3D)
{
	m_Texture2D.Reserve(numTexture2D);
}

bool C_ResourceBundleSingle::RequiresResourceCount()
{
	return true;
}

void C_ResourceBundleSingle::OnCreateTexture2D(
	const C_BundleHeader& bundleHeader,
	const C_ResourceTexture2D& resourceHeader)
{
	m_Texture2D.PushBack();
	C_RefTexture* pTexture = &m_Texture2D.Back();

	const E_Format format = GetFormat(resourceHeader.format);
	if (format != E_Format_Unknown)
	{
		U8* pData = m_pVidMemory + resourceHeader.offsetData;

		U32 flags = E_Usage_Bundled | E_Usage_Indirect;

		#ifdef PLATFORM_PS3
		flags |= E_Format_Raw; // Textures are 'as is' in memory...
		switch(format)
		{
		case E_Format_DXT1:
		case E_Format_DXT3:
		case E_Format_DXT5:
			break;
		default:
			flags |= E_Format_Linear; // Linear textures pad out mip widths to top mip width...
			break;
		}
		#endif

		const U32 size = Display::GetTexture2DSize(resourceHeader.width, resourceHeader.height, resourceHeader.mipLevels, flags, format);

		if (Device()->CreateTexture2D(resourceHeader.width, resourceHeader.height, resourceHeader.mipLevels, flags, format, pTexture, pData, size))
		{
			const char* pName = 
				(const char*)(m_pSysMemory + bundleHeader.stringTableOffset + resourceHeader.offsetString);
			Util::C_String lowerCaseName(pName);
			lowerCaseName.ConvertToLower();
			pTexture->SetFileName(lowerCaseName);
			
			BIZ_ASSERT_SPEED(resourceHeader.uncompressedDataSize <= pTexture->GetSize()); // Uncompressed size looks dodgy on ps3 so make sure the bundler never thinks the texture is bigger than what display does...

			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetTextureName(pTexture, lowerCaseName);
			#endif

			#if defined (PLATFORM_PC)
			C_TextureParams uploadParams;
			uploadParams.m_Width = resourceHeader.width;
			uploadParams.m_Height = resourceHeader.height;
			uploadParams.m_Levels = resourceHeader.mipLevels;
			uploadParams.m_MipTail = resourceHeader.mipLevels-1;
			uploadParams.m_Format = GetFormatFromTextureFormat(pTexture->GetFormat());
			uploadParams.m_MipSize	= size;
			uploadParams.m_pMipData = pData;

			UploadTexture2D(pTexture, &uploadParams);
			#endif
		}
	}
	else
		LOGFATAL(g_Log, "Unsupported texture 2d format from bundle '%s'\n", m_FileName.Chars());
}

void C_ResourceBundleSingle::OnCreateTextureCubeMap(
	const C_BundleHeader& bundleHeader,
	const C_ResourceTextureCubeMap& resoruceHeader)
{
	LOGWARN(g_Log, "Cube-maps not supported by Display::C_ResourceBundleSingle\n.");
}

void C_ResourceBundleSingle::OnCreateTextureArray2D(
	const C_BundleHeader& bundleHeader,
	const C_ResourceTextureArray2D& resourceHeader)
{
	LOGWARN(g_Log, "Array textures not supported by Display::C_ResourceBundleSingle\n.");
}

void C_ResourceBundleSingle::OnCreateTexture3D(
	const C_BundleHeader& bundleHeader,
	const C_ResourceTexture3D& resourceHeader)
{
	LOGWARN(g_Log, "Volume textures not supported by Display::C_ResourceBundleSingle\n.");
}

U8* C_ResourceBundleSingle::AllocateVideoMemory(size_t bytes)
{
	if(DeviceExists())
	{
		m_SystemID = Device()->MemoryGetSystemID();

		#ifdef PLATFORM_PC
			return AllocateSystemMemory(bytes);
		#else
			U8* pPtr = NULL;
			if(Device()->AllocResourceMemory(Display::E_ResourceType_Texture2D, m_Flags, bytes, &pPtr, m_SystemID))
			{
				#if TRACK_DISPLAY2_PROTECTED_MEMORY
				Display2::Device()->InsertProtectedMemory(bytes, pPtr, m_FileName.Chars());
				#endif

				return pPtr;
			}
			else
			{
				BIZ_ASSERT_FAIL_SPEED("Failed to allocate resource bundle video memory of size %i", bytes);
				return NULL;
			}
		#endif
	}
	else
	{
		BIZ_ASSERT_FAIL_SPEED("Display device does not exist when allocating resource bundle video memory of size %i", bytes);
		return NULL;
	}
}

void C_ResourceBundleSingle::FreeVideoMemory()
{
	if(m_pVidMemory)
	{
		if(DeviceExists())
		{
			#ifdef PLATFORM_PC
				FreeSystemMemory(m_pVidMemory);
			#else
				#if TRACK_DISPLAY2_PROTECTED_MEMORY
				Display2::Device()->RemoveProtectedMemory(m_VidMemorySize, m_pVidMemory);
				#endif

				Device()->FreeResourceMemory(Display::E_ResourceType_Texture2D, m_Flags, m_VidMemorySize, m_pVidMemory, m_SystemID);
			#endif
		}
		m_pVidMemory = NULL;
		m_VidMemorySize = 0;
	}
}

U8* C_ResourceBundleSingle::AllocateSystemMemory(size_t bytes)
{
	return new U8[bytes];
}

void C_ResourceBundleSingle::FreeSystemMemory(U8* p)
{
	SAFE_DELETE_ARY(p);
}

U32 C_ResourceBundleSingle::CalculateVideoMemorySize()
{
	U32 size = 0;

	const S32 textureCount = m_Texture2D.Size();
	for(S32 textureIndex=0; textureIndex<textureCount; textureIndex++)
	{
		const C_RefTexture& texture = m_Texture2D[textureIndex];

		size += Display::GetTexture2DSize(texture.GetWidth(), texture.GetHeight(), texture.GetLevels(), GetFormatFromTextureFormat(texture.GetFormat()));
	}

	return size;
}
