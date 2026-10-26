#include "stdafx.h"
#include <process_texture/ProcessedMipmapChain.h>

#include <export_resource/ExportTextureOptions.h>
#include <export_resource/IExportTexture.h>
#include <export_resource/OutputResourceHeader.h>

#include "ExportArrayOfTextures2DGeneric.h"

#include "../../BundlerOutputOptions.h"
#include "../OutputTextureData.h"

using namespace ResourceBundler;


bool C_ExportArrayOfTextures2DGeneric::_GetOutputHeader(C_OutputResourceHeader& header)
{
	return true;
}

bool C_ExportArrayOfTextures2DGeneric::_Export(
	C_OutputTextureData& output, 
	const C_BundlerOutputOptions& bundlerOptions,
	const C_ExportTextureOptions& exportOptions)
{
	U32 mipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 numImages = m_mipmapChains.Size();

	m_textureFormat = exportOptions.outputFormat;

	if (!output.AllocateStorage(numImages, mipLevels))
		return false;

	U8* pOutput;

	//Initialise output storage for our mip levels.
	{
		U32 baseSize = m_mipmapChains.Front()->GetBaseSize() * numImages;
		U32 mipSize = m_mipmapChains.Front()->GetMipSize() * numImages;

		pOutput = output.IncreaseStorage(baseSize + mipSize);
		if (!pOutput)
			return false;
		// clear any unused areas of the texture
		memset(pOutput , 127, baseSize + mipSize);
	}

	U32 endianSwapItemSize;
	switch (m_mipmapChains.Front()->GetFormat())
	{
		case Display::ResourceBundler::E_TextureFormat_A8R8G8B8:
			endianSwapItemSize = 4;
			break;
		case Display::ResourceBundler::E_TextureFormat_L8:
			//No swap
			endianSwapItemSize = 1;
			break;
		case Display::ResourceBundler::E_TextureFormat_A32R32G32B32:
			endianSwapItemSize = 16;
			break;

		case Display::ResourceBundler::E_TextureFormat_DXT1:
		case Display::ResourceBundler::E_TextureFormat_DXT2:
		case Display::ResourceBundler::E_TextureFormat_DXT3:
		case Display::ResourceBundler::E_TextureFormat_DXT4:
		case Display::ResourceBundler::E_TextureFormat_DXT5:
		case Display::ResourceBundler::E_TextureFormat_DXN:
			//Two bytes to swap (8 in 16)
			endianSwapItemSize = 2;
			break;

		default:
			printf("Unsupported compression format for 360.\n");
			return false;
	}

	U32 nMipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 mipOffset = 0;
	for (U32 iLevel = 0; iLevel < nMipLevels; iLevel++)
	{
		U32 imageIndex = 0;
		for (
			Ut::UtList<C_ProcessedMipmapChain*>::Iterator iter = m_mipmapChains.Begin(); 
			iter != m_mipmapChains.End();
			++imageIndex,
			++iter)
		{
			C_ProcessedMipmapChain* pMipChain = *iter;
			//Set appropriate source format. If uncompressed it will be E_TextureFormat_A32R32G32B32.
			//If compressed then it should be equal to the output format.
			if (pMipChain->GetFormat() == Display::ResourceBundler::E_TextureFormat_A32R32G32B32)
			{
				printf("Uncompressed generic textures not supported yet.\n");
				return false;
			}
			else
			{
				if (pMipChain->GetFormat() != m_textureFormat)
				{
					assert(!"Texture format mismatch");
					return false;
				}
			}

			U32 mipSize = pMipChain->GetLevelSize(iLevel);
			U8* pSource = pMipChain->GetLevelPtr(iLevel);

			//Our data will be in little endian form, swap if required here.
			if (bundlerOptions.bigEndianTexture)
			{
				switch(endianSwapItemSize)
				{
				case 1:
					//No endian swapping required.
					break;

				case 2:
					// Short, WORD - 8 in 16 swap
					Ut::DoEndianSwap::EndianSwap(
						pSource, 
						"2", 
						mipSize / 2);
					break;

				case 4:
					Ut::DoEndianSwap::EndianSwap(
						pSource, 
						"4", 
						mipSize / 4);
					break;

				case 16:
					Ut::DoEndianSwap::EndianSwap(
						pSource, 
						"4444", 
						mipSize / 16);
					break;

				default:
					printf("Unknown endian swap item size.\n");
					return false;
				};
			}

			//Copy out our data.
			memcpy(
				pOutput+mipOffset,
				pSource,
				mipSize);

			//Let's set the mip header.
			output.SetMipInfo(
				imageIndex,
				iLevel, 
				pOutput+mipOffset,
				mipSize);

			//Increase offset.
			mipOffset += mipSize;
		}
	}

	return true;
}

