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
#include <Display/Bundler/ResourceBundleReaderImpl3.h>

#include <compression.h>

using namespace Display::ResourceBundler;

#define BUNDLE_BUFFER_SIZE 1024*512

bool C_ResourceBundleReaderImpl3::Read(
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

	switch (header.flags & E_HeaderMask_Compression)
	{
		case ResourceBundler::E_HeaderFlag_Compressed:
		{
			U32 origSize		= header.uncompressedDataSize;
			U32 compressedSize	= header.compressedDataSize;
			vidMemorySize = origSize;
			pVidMemory = bundle.AllocateVideoMemory(vidMemorySize);

			assert(pVidMemory);

			//Decompress into video memory.
			{
				CCompression compress;
				compress.Decompress_Init(
					pVidMemory,
					vidMemorySize);

				U32 compressedBytesRead = 0;
				U8* decompressionBuffer = bundle.AllocateSystemMemory(BUNDLE_BUFFER_SIZE);
				while(compressedBytesRead != compressedSize)
				{
					U32 compressedBytesRemaining = compressedSize - compressedBytesRead;

					U32 compressedBytesToRead;
					if (compressedBytesRemaining >= BUNDLE_BUFFER_SIZE) 
						compressedBytesToRead = BUNDLE_BUFFER_SIZE;
					else
						compressedBytesToRead = compressedBytesRemaining;

					file.Read(decompressionBuffer, compressedBytesToRead);

					bool compressOk = compress.Decompress(
						decompressionBuffer, 
						compressedBytesToRead);
					assert(compressOk && "Decompression Failed");

					//Update the number of decompressed bytes that have been read.
					compressedBytesRead += compressedBytesToRead;
				}
				bundle.FreeSystemMemory(decompressionBuffer);
			}
			break;
		}
		case ResourceBundler::E_HeaderFlag_CompressedIndividually:
		case ResourceBundler::E_HeaderFlag_Uncompressed:
		default:
		{
			vidMemorySize = header.uncompressedDataSize;
			pVidMemory = bundle.AllocateVideoMemory(vidMemorySize);
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
		C_ResourceHeader* pResHeader = (C_ResourceHeader*)pSysMemory;
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
			pResHeader = (C_ResourceHeader*)((U8*)pResHeader + pResHeader->offsetNext);	// [Blur2PC][x64]
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
		C_ResourceHeader* pResHeader = (C_ResourceHeader*)pSysMemory;
		while(true)
		{
			switch (pResHeader->type)
			{
			case E_ResourceType_Texture2D:
				((I_ResourceFactory&)bundle).OnCreateTexture2D(
					header,
					(C_ResourceTexture2D&)*pResHeader);
				break;
			case E_ResourceType_TextureCubeMap:
				((I_ResourceFactory&)bundle).OnCreateTextureCubeMap(
					header,
					(C_ResourceTextureCubeMap&)*pResHeader);
				break;
			case E_ResourceType_TextureArray2D:
				((I_ResourceFactory&)bundle).OnCreateTextureArray2D(
					header,
					(C_ResourceTextureArray2D&)*pResHeader);
				break;
			case E_ResourceType_Texture3D:
				((I_ResourceFactory&)bundle).OnCreateTexture3D(
					header,
					(C_ResourceTexture3D&)*pResHeader);
				break;
			default:
				break;
			}
			if (!pResHeader->offsetNext)
				break;
			pResHeader = (C_ResourceHeader*)((U8*)pResHeader + pResHeader->offsetNext);	// [Blur2PC][x64]
		}
	}

	#if defined(PLATFORM_PC) && defined(CFG_DEBUG) && defined(USER_LLOYDW)
	S32 oldVidMemSize = (S32)vidMemorySize;
	S32 newVidMemSize = (S32)bundle.CalculateVideoMemorySize();
	if(oldVidMemSize != newVidMemSize)
	{
		LOGFATAL(Display::g_Log, "Resource bundle video memory size %i does not match mine %i by %i\n",
			oldVidMemSize,
			newVidMemSize,
			newVidMemSize-oldVidMemSize);
	}
	#endif
	
	return true;
}
