#include "stdafx.h"
#include <process_texture/ProcessedMipmapChain.h>

#include <export_resource/ExportTextureOptions.h>
#include <export_resource/IExportTexture.h>
#include <export_resource/OutputResourceHeader.h>

#include "ExportArrayOfTextures2DGeneric.h"

#include "../../BundlerOutputOptions.h"
#include "../OutputTextureData.h"
#include "../../Utility.h"

using namespace ResourceBundler;

U32 GetMipSizeWherePitchIsPaddedToWidth0(C_ProcessedMipmapChain* pMipmapChain, const U32 level)
{
	U32 widthBlocks = pMipmapChain->GetWidth(0) / Utility::GetBlockWidth(pMipmapChain->GetFormat());
	U32 heightBlocks = pMipmapChain->GetHeight(level) / Utility::GetBlockWidth(pMipmapChain->GetFormat());
	U32 depthBlocks = pMipmapChain->GetDepth(level);
	
	U32 mipSize = widthBlocks * heightBlocks * depthBlocks* Utility::GetBytesPerBlock(pMipmapChain->GetFormat());
	return mipSize;
}

U32 GetMipChainSizeWherePitchIsPaddedToWidth0(C_ProcessedMipmapChain* pMipmapChain)
{
	U32 mipSize = 0;
	for (U32 level = 1; level < pMipmapChain->GetNumLevels(); level++)
		mipSize += GetMipSizeWherePitchIsPaddedToWidth0(pMipmapChain, level);
	return mipSize;
}

bool C_ExportArrayOfTextures2DGeneric::_GetOutputHeader(C_OutputResourceHeader& header)
{
	return true;
}

bool C_ExportArrayOfTextures2DGeneric::_Export(
	C_OutputTextureData& output, 
	const C_BundlerOutputOptions& bundlerOptions,
	const C_ExportTextureOptions& exportOptions)
{
	// PS3 linear textures need the pitch of each mip level to be the pitch of the top mip
	bool padPitchToWidth0;
	C_ProcessedMipmapChain* pMipmapChain0 = m_mipmapChains.Front();
	if (bundlerOptions.platform == Display::ResourceBundler::E_Platform_PS3_LibGCM && (
		pMipmapChain0->GetFormat() == Display::ResourceBundler::E_TextureFormat_L8 ||
		pMipmapChain0->GetFormat() == Display::ResourceBundler::E_TextureFormat_A8R8G8B8 ||
		pMipmapChain0->GetFormat() == Display::ResourceBundler::E_TextureFormat_A32R32G32B32))
		padPitchToWidth0 = true;
	else
		padPitchToWidth0 = false;

	U32 mipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 numImages = m_mipmapChains.Size();

	m_textureFormat = exportOptions.outputFormat;

	if (!output.AllocateStorage(numImages, mipLevels))
		return false;

	U8* pOutput;

	//Initialise output storage for our mip levels.
	{
		U32 baseSize = m_mipmapChains.Front()->GetBaseSize() * numImages;

		U32 mipSize;
		if(padPitchToWidth0)
			mipSize = GetMipChainSizeWherePitchIsPaddedToWidth0(m_mipmapChains.Front());
		else
			mipSize = m_mipmapChains.Front()->GetMipSize() * numImages;

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

			U32 mipSize;
			if (padPitchToWidth0)
				mipSize = GetMipSizeWherePitchIsPaddedToWidth0(pMipChain, iLevel);
			else
				mipSize = pMipChain->GetLevelSize(iLevel);
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
			if (padPitchToWidth0)
			{
				U32 width0 = pMipChain->GetWidth(0);
				U32 pitch0 = (width0 / Utility::GetBlockWidth(pMipChain->GetFormat())) * Utility::GetBytesPerBlock(pMipChain->GetFormat());

				U32 width = pMipChain->GetWidth(iLevel);
				U32 height = pMipChain->GetHeight(iLevel);
				U32 depth = pMipChain->GetDepth(iLevel);

				U8* src = pSource;
				U8* dst = pOutput+mipOffset;
				
				for (U32 d = 0; d < depth; d++)
				{
					for (U32 h = 0; h < height; h++)
					{
						U32 pitch = (width / Utility::GetBlockWidth(pMipChain->GetFormat())) * Utility::GetBytesPerBlock(pMipChain->GetFormat());

						memcpy(dst, src, pitch);

						src += pitch;
						dst += pitch0;
					}
				}
			}
			else
			{
				memcpy(
					pOutput+mipOffset,
					pSource,
					mipSize);
			}

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

