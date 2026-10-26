#include "stdafx.h"

#include "ngl/system.h"
#include "texture_export/Palette.h"
#include "texture_export/Surface.h"
#include "texture_export/texture.h"
#include "texture_export/targets/pctex.h"
#include "texture_export/targets/ps3tex.h"
#include "texture_export/targets/xetex.h"
#include "texture_export/converters/generatemipmaps.h"
#include "texture_export/converters/atidxt.h"
#include "texture_export/TextureManager.h"
#include "texture_export/TextureProcessing.h"
#include "texture_export/converters/heightmaptonormalmap.h"
#include "texture_export/converters/calculatealphaflags.h"
#include "texture_export/converters/inr0.h"
#include "texture_export/converters/RescaleSurface.h"
#include "texture_export/converters/AdjustChannels.h"
#include "texture_export/converters/sRGB.h"
#include "texture_export/Utility.h"
#include <attk/attk.h>

#include <misc_shared/cache/cache.h>
#include <system/serialise.h>

using namespace ActTools;

namespace TextureExport
{

/// write contents of this class to a binary stream
void TextureInfo::Write(Sys::I_BinaryStream *pStream)
{
	TextureInfoBase::Write(pStream);

	Sys::BinarySerialiser<Ut::NoEndianSwap> ar(*pStream);

	ar << m_iFlags << m_sTexelFmtOverride << m_iDefaultColour << m_nMipMaps << (U32)m_MipFilter << (U32)m_EdgeFilterU << (U32)m_EdgeFilterV;
}

/// read contents of this class from a binary stream
void TextureInfo::Read(Sys::I_BinaryStream *pStream)
{
	TextureInfoBase::Read(pStream);

	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pStream);

	U32 m=ActTools::FT_LANCZOS, eU=ActTools::EDGE_TILE, eV=ActTools::EDGE_TILE;
	ar >> m_iFlags >> m_sTexelFmtOverride >> m_iDefaultColour >> m_nMipMaps >> m >> eU >> eV;
	m_MipFilter = (ActTools::FilterType)m;
	m_EdgeFilterU = (ActTools::EdgeFilter)eU;
	m_EdgeFilterV = (ActTools::EdgeFilter)eV;
}

/// convert to DXT or some other custom format. Chooses the converter based on the format and the alpha channel
void ConvertToCustomFormat(ActTools::Texture *pTexture, Util::C_String sFormat, bool bForceGreyScaleAlpha)
{
	// the default format is DXT1 or DXT5 depending on the alpha channel
	if (sFormat == "")
	{
		if (pTexture->GetAlphaPresence() == Texture::ALPHA_GREYSCALE ||
			(pTexture->GetAlphaPresence() == Texture::ALPHA_BLACKWHITE && bForceGreyScaleAlpha))
			sFormat = "DXT5";
		else
			sFormat = "DXT1";
	}

	ActTools::TexelFormat *p = NULL;
	// for DXT1 without alpha, use ATI DXT encoder
	if (sFormat == "DXT1")
	{
		if (pTexture->GetAlphaPresence() != Texture::ALPHA_BLACKWHITE &&
			pTexture->GetAlphaPresence() != Texture::ALPHA_GREYSCALE)
			p = AtiDXT::GetTexelFormat(sFormat);
	}
	// for DXT3, DXT5, DXN also use the ATI DXT encoder
	else if (sFormat == "DXT3" || sFormat == "DXT5" || sFormat == "DXN")
	{
		p = AtiDXT::GetTexelFormat(sFormat);
	}
	// by default use the DirectX SDK to convert textures to the custom format
	if (!p)
	{
		NglPcTex dummy;
		p = dummy.GetTexelFormat(sFormat);
	}

	// convert           
	pTexture->ConvertTo(p);
	delete p;
}


static unsigned long LowestPow2(const unsigned long nSize)
{
	if (nSize < 1) return 1;
	unsigned long x = 1 << 31;
	for (; !(x & nSize); x >>= 1);
	return x;
}

/// load input texture files using the ATTK
void LoadInputFiles(TextureInfoBase *pTexInfo, Ut::Vector<Texture*> &textures, U32 iDefaultColour = 0xffffffff)
{
	U32 iLargestWidth=1, iLargestHeight=1;

	for (int i=0; i<pTexInfo->m_InputFilenames.Size(); i++)
	{
		// Load the input file using the ATTK
		ActTools::Texture *pTexture;

		pTexture = LoadTexture(pTexInfo->m_InputFilenames[i].sName);
		if (!pTexture)
		{
			Attk::C_Image attkImage;

			// create empty texture with the default colour
			U32 *pTexels = new U32[4*4];
			for (int x=0; x<4*4; x++)
				pTexels[x] = iDefaultColour;
			attkImage.NewPixelData(4, 4, 1, Attk::E_PixelFormat_R8G8B8A8U, (U8*)pTexels );
			delete pTexels;

			// convert from Attk to export_texture format
			pTexture = TextureFromAttkImage(&attkImage);
		}

		if (i == 0)
			LOGDEBUG( g_Log, "Converting texture '%s'\n", pTexInfo->m_InputFilenames[i].sName.Chars());

		// check the texture's alpha channel
		Texutils::CalculateAlphaFlags(pTexture);

		textures.PushBack(pTexture);
	}
}


/// load input texture files using the ATTK
void ResizeTextures(Ut::Vector<Texture*> &textures, bool reduce)
{
	U32 iLargestWidth=1, iLargestHeight=1;

	for (int i=0; i<textures.Size(); i++)
	{
		ActTools::Texture *pTexture = textures[i];

		if (i < 2 || !reduce)
		{
			if (pTexture->GetWidth() > iLargestWidth)
				iLargestWidth = pTexture->GetWidth();
			if (pTexture->GetHeight() > iLargestHeight)
				iLargestHeight = pTexture->GetHeight();
		}
	}

	 // IPR processing hack
	if (reduce && textures.Size() > 2)
	{
		ActTools::Texture *pTexture = textures[2];

		if (iLargestWidth > 128 || iLargestHeight > 128)
		{
			iLargestWidth = Maths::Min(iLargestWidth, pTexture->GetWidth() / 2);
			iLargestHeight = Maths::Min(iLargestHeight, pTexture->GetHeight() / 2);
		}
	}

	// calculate largest texture size
	iLargestWidth = LowestPow2(iLargestWidth);
	iLargestHeight = LowestPow2(iLargestHeight);

	// maximum supported texture size
	if (iLargestWidth > 4096) iLargestWidth = 4096;
	if (iLargestHeight > 4096) iLargestHeight = 4096;

	// rescale all texture to the same size
	for (int i=0; i<textures.Size(); i++)
	{
		if (iLargestWidth != textures[i]->GetWidth() || iLargestHeight != textures[i]->GetHeight())
			Texutils::ResizeTexture(textures[i], iLargestWidth, iLargestHeight,
				DEFAULT_COLOR_FILTER, 1.f, 
				DEFAULT_ALPHA_FILTER, 1.f,
				CSTR_TILE, CSTR_TILE);
	}	
}

/// main texture processing function
bool GenerateTextureData(TextureInfoBase *_pTexInfo, DiskCache::CacheEntry *pEntry)
{
	// load input files
	TextureInfo *pTexInfo = (TextureInfo*)_pTexInfo;
	Ut::Vector<Texture*> textures;
	LoadInputFiles(pTexInfo, textures, pTexInfo->m_iDefaultColour);

	bool allowProcessing = true;
	
	//do not process mipmapped cubemaps
	{
		U32 nMips = textures[0]->GetNumMipMaps();
		if (textures[0]->GetDepth() == 6 && 
			textures[0]->GetWidth()==textures[0]->GetHeight() && 
			textures[0]->GetNumMipMaps())
			allowProcessing = false;
	}

	if (allowProcessing)
	{
		ResizeTextures(textures, pTexInfo->bReduce);

		// convert to inr0
		if (pTexInfo->bINR0)
			Texutils::Ipr0ToInr0(textures[0], textures[1]);

		// convert to normal map
		if (pTexInfo->convertToNormalMap)
			Texutils::HeightMapToNormalMap(textures[0]);

		// convert to normal map
		if (pTexInfo->invertNormalMapX)
			Texutils::InvertNormalMapX(textures[0]);

		// generate mipmaps
		if (pTexInfo->mipmapped && textures[0]->GetNumMipMaps() == 1)
		{
			//convert sRGB to linear RGB 
			//may need to use floating-point intermediate images as A8R8G8B8 may introduce minor quantisation errors
			if (pTexInfo->bSRGBTexture)
				Texutils::SRGBToLinear(textures[0]);

			Texutils::GenerateMipmaps(textures[0], pTexInfo->m_MipFilter, 1.f, pTexInfo->m_MipFilter, 1.f, pTexInfo->m_EdgeFilterU, pTexInfo->m_EdgeFilterV, pTexInfo->m_nMipMaps);
			if (pTexInfo->bEnhanceMipMapAlpha)
				Texutils::EnhanceMipContrast(textures[0], 0xff000000);

			//convert back to sRGB
			if (pTexInfo->bSRGBTexture)
				Texutils::LinearToSRGB(textures[0]);
		}

		// premultiply the alpha channel
		if (pTexInfo->preMultiplyAlpha)
			Texutils::PreMultiplyAlpha(textures[0]);

		// convert to DXT1 or some other format
		ConvertToCustomFormat(textures[0], pTexInfo->m_sTexelFmtOverride, pTexInfo->bForceGreyScaleAlpha);
	}

	// write platform-independent texture to cache
	Sys::I_BinaryStream *pStream = pEntry->SetData("TEX");
	Sys::BinarySerialiser<Ut::NoEndianSwap> ar(*pStream);

	textures[0]->SetSRGBFlag(pTexInfo->bSRGBTexture);

	ar << *textures[0];

	for (S32 i=0; i<textures.Size(); ++i)
		delete textures[i];

	delete pStream;

	return true;
}

}

/*
* automatically load images into ATTK

* make all images the same size, resize to lowest pow2

* if one of the images was not found, use an empty white image

* optionally convert heightmap to normal map

* optionally generate mipmaps

* detect how many alpha bits are needed and choose DXT1, DXT4 accordingly

* use ATI compression library, or the XDK one if the texture contains alpha

* optionally override texture format e.g. DXT1, DXT4, A8R8G8B8 etc.

* optionally premultiply the alpha

----------------------
separate texture header cache containing information:
- width, height, final format (e.g. DXT1, DXT5, etc.)
- number of mipmaps, alpha channel usage (texture size will be worked out from this information)

*/

// TODO: * detect whether any of the images are entirely a single colour

