#include "stdafx.h"

#include "../../BundlerOutputOptions.h"
#include "../../Bundler.h"
#include "../../Utility.h"

#include <process_texture/ProcessedMipmapChain.h>

#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>

#include "../IExportTexture.h"
#include "ExportTexture360.h"
#include "ExportArrayOfTextures2D360.h"
#include "../OutputTextureData.h"

using namespace ResourceBundler;

C_ExportArrayOfTextures2D360::C_ExportArrayOfTextures2D360()
{
}

bool C_ExportArrayOfTextures2D360::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;
	return true;
}

bool C_ExportArrayOfTextures2D360::_Export(
	C_OutputTextureData& output, 
	const C_BundlerOutputOptions& bundlerOptions,
	const C_ExportTextureOptions& exportOptions)
{
	U32 mip0Width = m_mipmapChains.Front()->GetWidth();
	U32 mip0Height = m_mipmapChains.Front()->GetHeight();
	U32 mipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 numImages = m_mipmapChains.Size();

	m_textureFormat = exportOptions.outputFormat;

	U32	tiledSize;
	U32	baseSize;
	U32	mipSize;
	D3DBaseTexture	d3dHeader;
	if (!_GetD3DHeader(
		d3dHeader,
		tiledSize,
		&baseSize,
		&mipSize))
		return false;

	// Get some texture properties
	XGTEXTURE_DESC baseDesc;
	XGGetTextureDesc(&d3dHeader, 0, &baseDesc);

	D3DFORMAT dstFormat = (D3DFORMAT)(baseDesc.Format);

	if (!output.AllocateStorage(numImages, mipLevels))
		return false;

	U8* pOutput;
	//Initialise output storage for our mip levels.
	{
		pOutput = output.IncreaseStorage(baseSize + mipSize);
		if (!pOutput)
			return false;
		// clear any unused areas of the texture
		memset(pOutput , 255, baseSize + mipSize);
	}
	//Allocate working storage
	U8* pWorkingTexture = new U8[baseSize + mipSize];

	assert(m_mipmapChains.Size() >= 1);

	D3DFORMAT srcFormat = (D3DFORMAT)_GetXboxFormat(m_mipmapChains.Front()->GetFormat());
	U32 endianSwapItemSize;
	switch (srcFormat)
	{
		case D3DFMT_A8R8G8B8:
			endianSwapItemSize = 4;
			break;
		case D3DFMT_L8:
			//No swap
			endianSwapItemSize = 1;
			break;
		case D3DFMT_A32B32G32R32:
			endianSwapItemSize = 16;
			break;

		case D3DFMT_DXT1:
		case D3DFMT_DXT2:
		case D3DFMT_DXT5:
		case D3DFMT_DXN:
			//Two bytes to swap (8 in 16)
			endianSwapItemSize = 2;
			break;

		default:
			printf("Unsupported texture format.\n");
			return false;
	}

	switch (srcFormat)
	{
		case D3DFMT_A8R8G8B8:
		//TODO: other formats here:
		{
			for (U32 iLevel = 0; iLevel < mipLevels; iLevel++)
			{
				U32 imageIndex = 0;
				for (
					Ut::UtList<C_ProcessedMipmapChain*>::Iterator iter = m_mipmapChains.Begin(); 
					iter != m_mipmapChains.End(); ++iter)
				{
					C_ProcessedMipmapChain* pMipChain = *iter;
					U32 mipSize = pMipChain->GetLevelSize(iLevel);
					U8* pSource = pMipChain->GetLevelPtr(iLevel);
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
			}
		}
		break;

		default:
			break;
	}


	//Ensure that our source format is set to be linear (non-tiled).
	srcFormat = (D3DFORMAT)(srcFormat & ~D3DFORMAT_TILED_MASK);

	for (U32 iLevel = 0; iLevel < mipLevels; iLevel++)
	{
		HRESULT hr;
		XGTEXTURE_DESC mipDesc;
		XGGetTextureDesc(&d3dHeader, iLevel, &mipDesc);

		U32 imageIndex = 0;
		for (
			Ut::UtList<C_ProcessedMipmapChain*>::Iterator iter = m_mipmapChains.Begin(); 
			iter != m_mipmapChains.End();
			++imageIndex, ++iter)
		{
			C_ProcessedMipmapChain* pMipChain = *iter;

			const void* pSrcData = pMipChain->GetLevelPtr(iLevel);
			U32 srcPitch = Utility::GetPitch(mipDesc.Width, pMipChain->GetFormat());
			U32 srcMipSize = mipDesc.WidthInBlocks*mipDesc.HeightInBlocks*mipDesc.BytesPerBlock;
			//Assert that our source format is not tiled.
			assert(!(srcFormat & D3DFORMAT_TILED_MASK));

			//Copy to our working surface. The surface remains linear at this point.
			hr = XGCopySurface(
				pWorkingTexture, 
				mipDesc.RowPitch, 
				mipDesc.Width, 
				mipDesc.Height, 
				(D3DFORMAT)MAKELINFMT(dstFormat),	// Copy destination format (linear, non packed)
				NULL,
				pSrcData, 
				srcPitch, 
				srcFormat,	// Copy source format (linear, non packed).
				NULL,
				0,
				0.5);
			if (hr != S_OK)
				return false;
			// Calculate the mip level offset
			U32 mipLevelOffset = XGGetMipLevelOffset(&d3dHeader, imageIndex, iLevel);
			if ((iLevel > 0) && (baseDesc.Width >= 32 || baseDesc.Height >= 32))
				mipLevelOffset += baseSize;

			if (bundlerOptions.tileTextures)
			{
				XGTileTextureLevel(
					baseDesc.Width,
					baseDesc.Height,
					iLevel,
					XGGetGpuFormat(dstFormat),
					0,
					pOutput + mipLevelOffset,
					NULL, 
					pWorkingTexture, 
					mipDesc.RowPitch,
					NULL);
			}
			//We're not tiled.
			else
			{
				printf("Only tiled exports supported for 360.\n");
				return false;
			}
			//Let's set the mip header.
			output.SetMipInfo(imageIndex, iLevel, pOutput+mipLevelOffset, srcMipSize);
		}
	}

	//Free working storage.
	delete[] pWorkingTexture;
	return true;
}
