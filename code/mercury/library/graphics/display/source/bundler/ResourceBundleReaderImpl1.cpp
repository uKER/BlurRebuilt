#include "stdafx.h"
#include <system/serialise.h>

#include <display/bundler/ResourceBundlerDefines.h>
#include <display/bundler/BundleHeader.h>
#include <display/bundler/ResourceHeader.h>
#include <display/bundler/ResourceTexture.h>
#include <display/bundler/ResourceTexture2D.h>
#include <display/bundler/ResourceTextureCubeMap.h>
#include <display/bundler/ResourceTextureArray2D.h>
#include <display/bundler/ResourceTexture3D.h>
#include <Display/Device/Device.h>
#include <Display/Texture/Texture.h>
#include <Display/Texture/Texture2D.h>
#include <Display/Texture/Texture3D.h>
#include <Display/Texture/TextureCube.h>
#include <Display/Bundler/I_ResourceFactory.h>
#include <Display/Bundler/I_ResourceBundle.h>
#include <Display/Bundler/I_ResourceBundleReaderImpl.h>
#include <Display/Bundler/ResourceBundleReaderImpl1.h>

#include <compression.h>

using namespace Display::ResourceBundler;


enum E_BizarreResourcePlatform
{
	kBRP_Unknown	= 0,
	kBRP_Xbox360	= 1,
	kBRP_PC,
	kBRP_PS3_OpenGL,
	kBRP_PS3_LibGCM,
	kBRP_Wii
};

enum E_BizarreResourceFormats
{
	kBRF_Unknown	= 0,
	kBRF_DXT1,
	kBRF_DXT2,
	kBRF_DXT3,
	kBRF_DXT4,
	kBRF_DXT5,
	kBRF_A8R8G8B8,
	kBRF_L8,
	kBRF_A8,
	kBRF_565,
	kBRF_A4R4G4B4,
	kBRF_DXN,
};

enum E_BIZARRE_RESOURCE_TYPE
{
	kBizResType_Unknown			= -1,

	kBizResType_Texture2D		= 1,

	kBizResType_Texture2DArray	= 10,

	kBizResType_Texture3D		= 20,

	kBizResType_TextureCubemap	= 30,
};

// This magic number identifies the file as a Bizarre resource file
#define BIZARRE_BUNDLER_MAGIC_VALUE		'BZR1'		// 'BZR1'

struct BIZ_BUNDLE_HEADER
{
	U32		magicID;			// Unique value so we can verify the file is Ok
	U8		version;			// Which revision of this format is this?
	U8		platformCode;		// 360, PS3, Wii?
	U16		resourceInfoSize;	// Size of resource info = Offset to platform specific resource data
	U16		stringTableOffset;
	U16		pad;
	U32		dataSize;			// Size of platform specific data
};

#pragma pack(push, 2)

struct BIZ_RESOURCE_HEADER
{
	U16		m_offsetNext;		// Offset to next resource, or Zero for last one
	U16		m_offsetString;		// Offset to string entry from start of this header
	U32		m_offsetResource;	// Offset to platform specific resource data from start of platform specific data
	U16		m_resourceType;
};

struct BIZ_RESOURCE_TEXTURE : BIZ_RESOURCE_HEADER
{
	U16		m_format;
	U16		m_mipLevels;
};

struct BIZ_RESOURCE_TEXTURE_2D : BIZ_RESOURCE_TEXTURE
{
	U16		m_width;
	U16		m_height;
	U16		m_pad;
};

struct BIZ_RESOURCE_TEXTURE_CUBE : BIZ_RESOURCE_TEXTURE
{
	U16		m_size;
};

struct BIZ_RESOURCE_TEXTURE_VOLUME : BIZ_RESOURCE_TEXTURE
{
	U16		m_width;
	U16		m_height;
	U16		m_depth;
};

#pragma pack(pop)

bool C_ResourceBundleReaderImpl1::Read(
	U32 version,
	U32& sysMemorySize,
	U8*& pSysMemory,
	U32& vidMemorySize,
	U8*& pVidMemory,
	I_ResourceBundle& bundle,
	Sys::I_File& file)
{
	file.SetPosition(0);

	BIZ_BUNDLE_HEADER header;
	file.Read(&header, sizeof(header));

	C_BundleHeader newHeader;
	newHeader.platformCode = header.platformCode;
	newHeader.resourceInfoSize = header.resourceInfoSize;
	newHeader.stringTableOffset = header.stringTableOffset;
	newHeader.flags = 0;

	#if defined(PLATFORM_PC)
	assert(header.platformCode == kBRP_PC);
	#elif defined(PLATFORM_PS3)
	assert(header.platformCode == kBRP_PS3_OpenGL || header.platformCode == kBRP_PS3_LibGCM);
	#else
	BIZ_ASSERT_FAIL("Unsupported platform code");
	#endif

	sysMemorySize = header.resourceInfoSize;
	pSysMemory = (U8*)bundle.AllocateSystemMemory(sysMemorySize);
	file.Read(pSysMemory, sysMemorySize);

	vidMemorySize = header.dataSize & ~0x80000000;

	bool compressed = ((header.dataSize & 0x80000000) != 0);
	if (compressed)
	{
		U8* pSysMemory = bundle.AllocateSystemMemory(vidMemorySize);

		file.Read(pSysMemory, vidMemorySize);

		U32 origSize		= ((U32*)pSysMemory)[0];
		U32 compressedSize	= ((U32*)pSysMemory)[1];
		newHeader.compressedDataSize = compressedSize;
		newHeader.uncompressedDataSize = origSize;
		newHeader.flags |= E_HeaderFlag_Compressed;

		vidMemorySize = origSize;
		pVidMemory = (U8*)bundle.AllocateVideoMemory(vidMemorySize);

		CCompression compress;
		bool compressOk = compress.Decompress(pSysMemory+8, compressedSize, pVidMemory, vidMemorySize);
		assert(compressOk || "Decompression Failed");

		bundle.FreeSystemMemory(pSysMemory);
	}
	else
	{
		newHeader.compressedDataSize = 0;
		newHeader.uncompressedDataSize = vidMemorySize;
		pVidMemory = (U8*)bundle.AllocateVideoMemory(vidMemorySize);
		file.Read(pVidMemory, vidMemorySize);
	}

	const BIZ_RESOURCE_HEADER* pResHeader = (BIZ_RESOURCE_HEADER*)pSysMemory;

	//Count the number of resource types.
	{
		U32 numTexture2D = 0;
		U32 numTexture3D = 0;
		U32 numTextureArray2D = 0;
		U32 numTextureCubeMap = 0;
		while(true)
		{
			switch (pResHeader->m_resourceType)
			{
			case kBizResType_Texture2D:
				numTexture2D++;
				break;
			case kBizResType_Texture3D:
				numTexture3D++;
				break;
			case kBizResType_Texture2DArray:
				numTextureArray2D++;
				break;
			case kBizResType_TextureCubemap:
				numTextureCubeMap++;
				break;
			default:
				BIZ_ASSERT_FAIL("Unsupported resource header type");
				break;
			}

			if (!pResHeader->m_offsetNext)
				break;

			pResHeader = (BIZ_RESOURCE_HEADER*)((U8*)pResHeader + pResHeader->m_offsetNext);	// [Blur2PC][x64]
		}
		//Set our count so that our bundle can allocate any memory it needs to.
		bundle.OnCreateHeader(
			newHeader,
			pSysMemory,
			numTexture2D,
			numTextureCubeMap,
			numTextureArray2D,
			numTexture3D);
	}

	pResHeader = (BIZ_RESOURCE_HEADER*)pSysMemory;
	while(true)
	{
		switch (pResHeader->m_resourceType)
		{
		case kBizResType_Texture2D:
			{
				BIZ_RESOURCE_TEXTURE_2D* pOldHeader = (BIZ_RESOURCE_TEXTURE_2D*)pResHeader;
				C_ResourceTexture2D newResourceHeader;

				newResourceHeader.offsetNext = pOldHeader->m_offsetNext;
				newResourceHeader.offsetString = pOldHeader->m_offsetString;
				newResourceHeader.offsetData = pOldHeader->m_offsetResource;
				newResourceHeader.type = pOldHeader->m_resourceType;

				newResourceHeader.width = pOldHeader->m_width;
				newResourceHeader.height = pOldHeader->m_height;
				newResourceHeader.mipLevels = pOldHeader->m_mipLevels;
				newResourceHeader.format = pOldHeader->m_format;

				((I_ResourceFactory&)bundle).OnCreateTexture2D(
					newHeader,
					(C_ResourceTexture2D&)newResourceHeader);
			}
			break;

		case kBizResType_Texture3D:
			{
				BIZ_RESOURCE_TEXTURE_VOLUME* pOldHeader = (BIZ_RESOURCE_TEXTURE_VOLUME*)pResHeader;
				C_ResourceTexture3D newResourceHeader;

				newResourceHeader.offsetNext = pOldHeader->m_offsetNext;
				newResourceHeader.offsetString = pOldHeader->m_offsetString;
				newResourceHeader.offsetData = pOldHeader->m_offsetResource;
				newResourceHeader.type = pOldHeader->m_resourceType;

				newResourceHeader.width = pOldHeader->m_width;
				newResourceHeader.height = pOldHeader->m_height;
				newResourceHeader.depth = pOldHeader->m_depth;
				newResourceHeader.mipLevels = pOldHeader->m_mipLevels;
				newResourceHeader.format = pOldHeader->m_format;

				((I_ResourceFactory&)bundle).OnCreateTexture3D(
					newHeader,
					(C_ResourceTexture3D&)newResourceHeader);
			}
			break;

		case kBizResType_Texture2DArray:
			BIZ_ASSERT_FAIL("Array textures never supported by old bundler.");
			break;

		case kBizResType_TextureCubemap:
			{
				BIZ_RESOURCE_TEXTURE_CUBE* pOldHeader = (BIZ_RESOURCE_TEXTURE_CUBE*)pResHeader;
				C_ResourceTextureCubeMap newResourceHeader;

				newResourceHeader.offsetNext =pOldHeader->m_offsetNext;
				newResourceHeader.offsetString = pOldHeader->m_offsetString;
				newResourceHeader.offsetData = pOldHeader->m_offsetResource;
				newResourceHeader.type = pOldHeader->m_resourceType;

				newResourceHeader.size = pOldHeader->m_size;
				newResourceHeader.mipLevels = pOldHeader->m_mipLevels;
				newResourceHeader.format = pOldHeader->m_format;

				((I_ResourceFactory&)bundle).OnCreateTextureCubeMap(
					newHeader,
					(C_ResourceTextureCubeMap&)newResourceHeader);
			}
			break;
		default:
			BIZ_ASSERT_FAIL("Unsupported resource header type");
			break;
		}
		if (!pResHeader->m_offsetNext)
			break;
		pResHeader = (BIZ_RESOURCE_HEADER*)((U8*)pResHeader + pResHeader->m_offsetNext);	// [Blur2PC][x64]
	}

	return true;
}

