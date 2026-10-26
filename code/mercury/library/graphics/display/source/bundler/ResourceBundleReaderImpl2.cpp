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
#include <Display/Bundler/ResourceBundleReaderImpl2.h>

#include <compression.h>

struct C_ResourceHeaderOld
{
	U16		offsetNext;		// Offset to next resource, or Zero for last one
	U16		offsetString;	// Offset to string entry from start of this header
	U32		offsetData;		// Offset to platform specific resource data from start of platform specific data
	U16		type;
	U16		flags;
};

struct C_ResourceTextureOld : C_ResourceHeaderOld
{
	U16		format;
	U16		mipLevels;


};

struct C_ResourceTexture2DOld : C_ResourceTextureOld
{
	U16		width;
	U16		height;
};


struct C_ResourceTexture3DOld : C_ResourceTextureOld
{
	U16		width;
	U16		height;
	U16		depth;
	U16		pad;
};

struct C_ResourceTextureArray2DOld : C_ResourceTextureOld
{
	U16		width;
	U16		height;
	U16		numImages;
	U16		pad;
};

struct C_ResourceTextureCubeMapOld : C_ResourceTextureOld
{
	U16		size;
	U16		pad;

};

using namespace Display::ResourceBundler;

bool C_ResourceBundleReaderImpl2::Read(
	U32 version,
	U32& sysMemorySize,
	U8*& pSysMemory,
	U32& vidMemorySize,
	U8*& pVidMemory,
	I_ResourceBundle& bundle,
	Sys::I_File& file)
{
	C_BundleHeader header;
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(file);
	header.Deserialise(ar);

	#if defined(PLATFORM_PC)
	assert(header.platformCode == E_Platform_PC);
	#elif defined(PLATFORM_PS3)
	assert(	header.platformCode == E_Platform_PS3_OpenGL || 
			header.platformCode == E_Platform_PS3_LibGCM);
	#else
	BIZ_ASSERT_FAIL("Unsupported platform code");
	#endif


	sysMemorySize = header.resourceInfoSize;
	pSysMemory = bundle.AllocateSystemMemory(sysMemorySize);
	file.Read(pSysMemory, sysMemorySize);

	U32 resourceDataSize = header.uncompressedDataSize;
	switch (header.flags & E_HeaderMask_Compression)
	{
		case ResourceBundler::E_HeaderFlag_Compressed:
		{
			U32 origSize		= header.uncompressedDataSize;
			U32 compressedSize	= header.compressedDataSize;

			U8* pSysMemory = bundle.AllocateSystemMemory(resourceDataSize);
			file.Read(pSysMemory, compressedSize);

			vidMemorySize = origSize;
			pVidMemory = bundle.AllocateVideoMemory(resourceDataSize);

			CCompression compress;
			bool compressOk = compress.Decompress(pSysMemory, compressedSize, pVidMemory, vidMemorySize);
			assert(compressOk || "Decompression Failed");

			bundle.FreeSystemMemory(pSysMemory);
			break;
		}
		case ResourceBundler::E_HeaderFlag_CompressedIndividually:
		case ResourceBundler::E_HeaderFlag_Uncompressed:
		default:
		{
			vidMemorySize = header.uncompressedDataSize;
			pVidMemory = bundle.AllocateVideoMemory(resourceDataSize);
			file.Read(pVidMemory, vidMemorySize);
			break;
		}
	};
	//Count the number of resource types.
	{
		U32 numTexture2D = 0;
		U32 numTextureCubeMap = 0;
		U32 numTextureArray2D = 0;
		U32 numTexture3D = 0;

		//Note that extra header content not present in older file versions
		//will result in members toward the end of our resource header being garbage.
		//This is okay however, as this extra data will not be used.
		C_ResourceHeaderOld* pResHeader = (C_ResourceHeaderOld*)pSysMemory;
		while(true)
		{
			switch (pResHeader->type)
			{
			case E_ResourceType_Texture2D:
				numTexture2D++;
				break;
			case E_ResourceType_TextureCubeMap:
				numTextureCubeMap++;
				break;
			case E_ResourceType_TextureArray2D:
				numTextureArray2D++;
				break;
			case E_ResourceType_Texture3D:
				numTexture3D++;
				break;

			default:
				BIZ_ASSERT_FAIL("Unsupported resource header type");
				break;
			}
			if (!pResHeader->offsetNext)
				break;
			pResHeader = (C_ResourceHeaderOld*)((U8*)pResHeader + pResHeader->offsetNext);	// [Blur2PC][x64]
		}

		//Set our count so that our bundle can allocate any memory it needs to.
		bundle.OnCreateHeader(
			header,
			pSysMemory,
			numTexture2D,
			numTextureCubeMap,
			numTextureArray2D,
			numTexture3D);
	}

	//Create our resources.
	{
		C_ResourceHeaderOld* pResHeader = (C_ResourceHeaderOld*)pSysMemory;
		while(true)
		{
			switch (pResHeader->type)
			{
			case E_ResourceType_Texture2D:
				{
				C_ResourceTexture2DOld* pOldHeader = (C_ResourceTexture2DOld*)pResHeader;
				C_ResourceTexture2D newResourceHeader;
				newResourceHeader.offsetNext = pOldHeader->offsetNext;
				newResourceHeader.offsetString = pOldHeader->offsetString;
				newResourceHeader.offsetData = pOldHeader->offsetData;
				newResourceHeader.type = pOldHeader->type;
				newResourceHeader.uncompressedDataSize = 0;
				newResourceHeader.compressedDataSize = 0;

				newResourceHeader.width = pOldHeader->width;
				newResourceHeader.height = pOldHeader->height;
				newResourceHeader.mipLevels = pOldHeader->mipLevels;
				newResourceHeader.format = pOldHeader->format;

				((I_ResourceFactory&)bundle).OnCreateTexture2D(
					header,
					(C_ResourceTexture2D&)newResourceHeader);
				}
				break;
			case E_ResourceType_TextureCubeMap:
				{
				C_ResourceTextureCubeMapOld* pOldHeader = (C_ResourceTextureCubeMapOld*)pResHeader;
				C_ResourceTextureCubeMap newResourceHeader;

				newResourceHeader.offsetNext = pOldHeader->offsetNext;
				newResourceHeader.offsetString = pOldHeader->offsetString;
				newResourceHeader.offsetData = pOldHeader->offsetData;
				newResourceHeader.type = pOldHeader->type;
				newResourceHeader.uncompressedDataSize = 0;
				newResourceHeader.compressedDataSize = 0;

				newResourceHeader.size = pOldHeader->size;
				newResourceHeader.mipLevels = pOldHeader->mipLevels;
				newResourceHeader.format = pOldHeader->format;

				((I_ResourceFactory&)bundle).OnCreateTextureCubeMap(
					header,
					(C_ResourceTextureCubeMap&)newResourceHeader);
				}
				break;
			case E_ResourceType_TextureArray2D:
				{
				C_ResourceTextureArray2DOld* pOldHeader = (C_ResourceTextureArray2DOld*)pResHeader;
				C_ResourceTextureArray2D newResourceHeader;

				newResourceHeader.offsetNext = pOldHeader->offsetNext;
				newResourceHeader.offsetString = pOldHeader->offsetString;
				newResourceHeader.offsetData = pOldHeader->offsetData;
				newResourceHeader.type = pOldHeader->type;
				newResourceHeader.uncompressedDataSize = 0;
				newResourceHeader.compressedDataSize = 0;

				newResourceHeader.width = pOldHeader->width;
				newResourceHeader.height = pOldHeader->height;
				newResourceHeader.numImages = pOldHeader->numImages;
				newResourceHeader.mipLevels = pOldHeader->mipLevels;
				newResourceHeader.format = pOldHeader->format;

				((I_ResourceFactory&)bundle).OnCreateTextureArray2D(
					header,
					(C_ResourceTextureArray2D&)newResourceHeader);
				}
				break;
			case E_ResourceType_Texture3D:
				{
				C_ResourceTexture3DOld* pOldHeader = (C_ResourceTexture3DOld*)pResHeader;
				C_ResourceTexture3D newResourceHeader;

				newResourceHeader.offsetNext = pOldHeader->offsetNext;
				newResourceHeader.offsetString = pOldHeader->offsetString;
				newResourceHeader.offsetData = pOldHeader->offsetData;
				newResourceHeader.type = pOldHeader->type;
				newResourceHeader.uncompressedDataSize = 0;
				newResourceHeader.compressedDataSize = 0;

				newResourceHeader.width = pOldHeader->width;
				newResourceHeader.height = pOldHeader->height;
				newResourceHeader.depth = pOldHeader->depth;
				newResourceHeader.mipLevels = pOldHeader->mipLevels;
				newResourceHeader.format = pOldHeader->format;
				((I_ResourceFactory&)bundle).OnCreateTexture3D(
					header,
					(C_ResourceTexture3D&)newResourceHeader);
				break;
				}
			default:
				break;
			}
			if (!pResHeader->offsetNext)
				break;
			pResHeader = (C_ResourceHeaderOld*)((U8*)pResHeader + pResHeader->offsetNext);	// [Blur2PC][x64]
		}
	}
	
	return true;
}
