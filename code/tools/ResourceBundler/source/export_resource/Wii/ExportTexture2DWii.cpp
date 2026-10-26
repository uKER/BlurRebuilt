#include "stdafx.h"

#include <process_texture/ProcessedMipmapChain.h>

#include "../IExportTexture.h"
#include "ExportTextureWii.h"
#include "ExportArrayOfTextures2DWii.h"
#include "ExportTexture2DWii.h"
#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>

#include "../OutputTextureData.h"
#include "../../BundlerOutputOptions.h"
#include "../../Utility.h"

using namespace ResourceBundler;

bool C_ExportTexture2DWii::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;

	header.texture2D.width	= m_mipmapChains.Front()->GetWidth();
	header.texture2D.height	= m_mipmapChains.Front()->GetHeight();

	return true;
}
#if 0
bool C_ExportTexture2DWii::_GetD3DHeader(
	D3DBaseTexture& header,
	U32& tiledSize,
	U32* pBaseSize, 
	U32* pMipSize)
{
	U32 mip0Width = m_mipmapChains.Front()->GetWidth();
	U32 mip0Height = m_mipmapChains.Front()->GetHeight();
	U32 mipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 numImages = m_mipmapChains.Size();

	U32		usage	= 0;
	S32		expBias	= 0;
	U32		pitch	= 0;
	DWORD	flags	= 0;
	U32		baseOffset = 0;
	U32		mipOffset = XGHEADER_CONTIGUOUS_MIP_OFFSET;

	tiledSize = XGSetTextureHeaderEx(
		mip0Width, 
		mip0Height,
		mipLevels, 
		usage,
		(D3DFORMAT)_GetXboxFormat(m_textureFormat), 
		expBias,
		flags,
		baseOffset,
		mipOffset, 
		pitch,
		(D3DTexture*)&header, 
		pBaseSize, 
		pMipSize);

	return true;
}
#endif