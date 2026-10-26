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
#include "ExportTexture3D360.h"
#include "../OutputTextureData.h"

using namespace ResourceBundler;

bool C_ExportTexture3D360::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;

	header.arrayTexture2D.numImages	= m_mipmapChains.Front()->GetDepth();
	header.arrayTexture2D.width		= m_mipmapChains.Front()->GetWidth();
	header.arrayTexture2D.height	= m_mipmapChains.Front()->GetHeight();

	return true;
}

bool C_ExportTexture3D360::_GetD3DHeader(
	D3DBaseTexture& header, 
	U32& tiledSize,
	U32* pBaseSize, 
	U32* pMipSize)
{
	U32 mip0Width = m_mipmapChains.Front()->GetWidth();
	U32 mip0Height = m_mipmapChains.Front()->GetHeight();
	U32 mip0Depth = m_mipmapChains.Front()->GetDepth();
	U32 mipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 numImages = m_mipmapChains.Size();

	U32		usage	= 0;
	S32		expBias	= 0;
	DWORD	flags	= 0;
	U32		baseOffset = 0;
	U32		mipOffset = XGHEADER_CONTIGUOUS_MIP_OFFSET;

	DWORD volSize = XGSetVolumeTextureHeaderEx(
		mip0Width, 
		mip0Height, 
		mip0Depth,
		mipLevels,
		usage, 
		(D3DFORMAT)_GetXboxFormat(m_textureFormat), 
		expBias,
		flags,
		baseOffset, 
		mipOffset,
		(D3DVolumeTexture*)&header, 
		pBaseSize, 
		pMipSize);

	return true;
}


bool C_ExportTexture3D360::_Export(
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
	
	D3DFORMAT srcFormat;
	//Set appropriate source format. If uncompressed it will be E_TextureFormat_A32R32G32B32.
	//If compressed then it should be equal to the output format.
	if (m_mipmapChains.Front()->GetFormat() == Display::ResourceBundler::E_TextureFormat_A32R32G32B32)
	{
		assert(!m_mipmapChains.Front()->IsBigEndian());
		srcFormat = (D3DFORMAT)MAKED3DFMT2(
						GPUTEXTUREFORMAT_32_32_32_32_FLOAT,
						GPUENDIAN_NONE, 
						FALSE,
						GPUSIGN_SIGNED, GPUSIGN_SIGNED, GPUSIGN_SIGNED, GPUSIGN_SIGNED,
						GPUNUMFORMAT_INTEGER,
						GPUSWIZZLE_X, GPUSWIZZLE_Y, GPUSWIZZLE_Z, GPUSWIZZLE_W);
	}
	else
	{
		assert(m_mipmapChains.Front()->IsBigEndian());

		if (m_mipmapChains.Front()->GetFormat() != m_textureFormat)
		{
			assert(!"Texture format mismatch");
			return false;
		}
		//Set d3d format using m_textureFormat, which we've asserted == mipChain.GetFormat()
		srcFormat = (D3DFORMAT)_GetXboxFormat(m_textureFormat);
	}
	//Ensure that our source format is set to be linear (non-tiled).
	srcFormat = (D3DFORMAT)(srcFormat & ~D3DFORMAT_TILED_MASK);

	for (U32 level = 0; level < mipLevels; level++)
	{
		HRESULT hr;
		XGTEXTURE_DESC mipDesc;
		XGGetTextureDesc(&d3dHeader, level, &mipDesc);

		U32 imageIndex = 0;
		for (
			Ut::UtList<C_ProcessedMipmapChain*>::Iterator iter = m_mipmapChains.Begin(); 
			iter != m_mipmapChains.End();
			++imageIndex, ++iter)
		{
			C_ProcessedMipmapChain* pMipChain = *iter;

			const void* pSrcData = pMipChain->GetLevelPtr(level);
			U32 srcPitch = Utility::GetPitch(mipDesc.Width, pMipChain->GetFormat());
			U32 srcSlicePitch = Utility::GetSlicePitch(mipDesc.Width, mipDesc.Height, pMipChain->GetFormat());
			U32 srcMipSize = mipDesc.WidthInBlocks*mipDesc.HeightInBlocks*mipDesc.DepthInBlocks*mipDesc.BytesPerBlock;
			//Assert that our source format is not tiled.
			assert(!(srcFormat & D3DFORMAT_TILED_MASK));

			//Copy to our working surface. The surface remains linear and non packed at this point.
			hr = XGCopyVolume(
				pWorkingTexture, 
				mipDesc.RowPitch, 
				mipDesc.SlicePitch,
				mipDesc.Width, 
				mipDesc.Height, 
				mipDesc.Depth,
				(D3DFORMAT)MAKELINFMT(dstFormat),	// Copy destination format (linear, non packed)
				NULL,
				pSrcData, 
				srcPitch, 
				srcSlicePitch,
				srcFormat,	// Copy source format (linear, non packed).
				NULL,
				0,
				0.5);
			if (hr != S_OK)
				return false;
			// Calculate the mip level offset
			U32 mipLevelOffset = XGGetMipLevelOffset(&d3dHeader, imageIndex, level);
			if ((level > 0) && (baseDesc.Width >= 32 || baseDesc.Height >= 32 || baseDesc.Depth >= 32))
				mipLevelOffset += baseSize;
			{
				//Copy out our data.
				memcpy(pOutput+mipLevelOffset, pWorkingTexture,	srcMipSize);
			}

			//Let's set the mip header.
			output.SetMipInfo(imageIndex, level, pOutput+mipLevelOffset, srcMipSize);
		}
	}

	// Tile the volume texture
	if (bundlerOptions.tileTextures)
	{
		U8* tilingTemp = new U8[baseSize + mipSize];
		memcpy(tilingTemp, pOutput, baseSize + mipSize);

		for (U32 level=0; level < mipLevels; level++)
		{
			XGTEXTURE_DESC mipDesc;
			XGGetTextureDesc(&d3dHeader, level, &mipDesc);

			U32 mipLevelOffset = XGGetMipLevelOffset(&d3dHeader, 0, level);
			if ((level > 0) && (baseDesc.WidthInBlocks >= 32 || baseDesc.HeightInBlocks >= 32 || baseDesc.DepthInBlocks >= 32))
				mipLevelOffset += baseSize;
			D3DFORMAT d3dFmt2 = D3DFORMAT(dstFormat | D3DFORMAT_TILED_MASK);

			XGTileVolumeTextureLevel(
				baseDesc.Width, 
				baseDesc.Height, baseDesc.Depth, level,
				XGGetGpuFormat(d3dFmt2), 
				0, // tiling flags
				pOutput + mipLevelOffset, 
				NULL, 
				tilingTemp + mipLevelOffset, 
				mipDesc.RowPitch,
				mipDesc.SlicePitch,
				NULL);
		}

		delete [] tilingTemp;
	}
	else
	{
		printf("Only tiled exports supported for 360.\n");
		return false;
	}

	//Free working storage.
	delete[] pWorkingTexture;
	return true;
}