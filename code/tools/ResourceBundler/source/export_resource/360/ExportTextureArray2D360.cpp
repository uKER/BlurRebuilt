#include "stdafx.h"

#include <process_texture/ProcessedMipmapChain.h>

#include "../IExportTexture.h"
#include "ExportTexture360.h"
#include "ExportArrayOfTextures2D360.h"
#include "ExportTextureArray2D360.h"
#include <export_resource/OutputResourceHeader.h>

using namespace ResourceBundler;


C_ExportTextureArray2D360::C_ExportTextureArray2D360()
{
}

bool C_ExportTextureArray2D360::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;

	header.arrayTexture2D.numImages	= m_mipmapChains.Size();
	header.arrayTexture2D.width		= m_mipmapChains.Front()->GetWidth();
	header.arrayTexture2D.height	= m_mipmapChains.Front()->GetHeight();

	return true;
}

bool C_ExportTextureArray2D360::_GetD3DHeader(
	D3DBaseTexture& header, 
	U32& tiledSize,
	U32 *pBaseSize,
	U32 *pMipSize)
{
	U32 mip0Width = m_mipmapChains.Front()->GetWidth();
	U32 mip0Height = m_mipmapChains.Front()->GetHeight();
	U32 mipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 numImages = m_mipmapChains.Size();

	U32		usage	= 0;
	S32		expBias	= 0;
	U32		pitch	= 0;
	U32		flags	= 0;
	U32		baseOffset = 0;
	U32		mipOffset = XGHEADER_CONTIGUOUS_MIP_OFFSET;

	XGSetArrayTextureHeaderEx(
		mip0Width,
		mip0Height,
		numImages,
		mipLevels,
		usage,
		(D3DFORMAT)_GetXboxFormat(m_textureFormat),
		expBias, 
		flags,
		baseOffset, 
		mipOffset, 
		pitch,
		(D3DArrayTexture*)&header,
		pBaseSize, 
		pMipSize);

	return true;
}


