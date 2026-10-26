#include "stdafx.h"

#include <process_texture/ProcessedMipmapChain.h>

#include "../IExportTexture.h"
#include "ExportTexture360.h"
#include "ExportArrayOfTextures2D360.h"
#include "ExportTextureCubeMap360.h"
#include <export_resource/OutputResourceHeader.h>

using namespace ResourceBundler;

C_ExportTextureCubeMap360::C_ExportTextureCubeMap360()
{
}

bool C_ExportTextureCubeMap360::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;

	header.textureCubeMap.size	= m_mipmapChains.Front()->GetWidth();

	return true;
}

bool C_ExportTextureCubeMap360::_GetD3DHeader(
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
	U32		flags	= 0;
	U32		baseOffset = 0;
	U32		mipOffset = XGHEADER_CONTIGUOUS_MIP_OFFSET;

	tiledSize = XGSetCubeTextureHeaderEx(
		mip0Width,
		mipLevels, 
		usage,
		(D3DFORMAT)_GetXboxFormat(m_textureFormat), 
		expBias, 
		flags,
		baseOffset,
		mipOffset,
		(D3DCubeTexture*)&header,
		pBaseSize, 
		pMipSize);

	return true;
}

