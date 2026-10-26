#include "stdafx.h"

#include <stdio.h>
#include <string.h>

#include "texture_export/Texture.h"
#include "texture_export/Surface.h"
#include "texture_export/Rescale.h"

#include "texture_export/converters/AdjustChannels.h"

//#define DEBUG_MIPPROCESSING

#ifdef DEBUG_MIPPROCESSING
#include <attk/attk.h>
#endif

using namespace ActTools;

namespace Texutils
{

static const U32 MASK_A=0xff000000;
static const U32 MASK_R=0x00ff0000;
static const U32 MASK_G=0x0000ff00;
static const U32 MASK_B=0x000000ff;

f32 Sigmoid(f32 t)
{
	f32 v = 1.01f/(1.f+expf(-(t*10.f - 5.f)));
	return v <= 1.0f ? v : 1.0f;
}

void GenerateEnhancementLUT(AdjustmentLUT& lut, u8 mipLevel)
{
	if (mipLevel == 0)
	{
		//linear curve, ie no adjustment
		for (U32 i=0; i<lengthof(lut.table); ++i)
			lut.table[i] = (u8)i;
	}
	else
	{
		u32 pivot = 256 >> mipLevel;
		if (mipLevel > 1)
			pivot += 256 >> (mipLevel+1);

		u32 width = pivot;
		u32 rangeMin = width > pivot ? 0 : pivot-width;
		u32 rangeMax = Maths::Min<u32>(lengthof(lut.table), pivot+width);


		for (u32 i=0; i<rangeMin; ++i)
			lut.table[i] = 0;

		for (u32 i=rangeMin; i<rangeMax; ++i)
			lut.table[i] = //(u8) Maths::Lerp(0.f, 255.f, ((f32)(i-pivot-width)/2.f*width));
							(u8) (255.f*Sigmoid((f32)(i-pivot+width)/(2.f*width)));

		for (u32 i=rangeMax; i<lengthof(lut.table); ++i)
			lut.table[i] = 255;
	}
}


ACTRESULT AdjustChannels( ActTools::Surface* aSurface,  const AdjustmentLUT& lut, u32 channelMask)
{
	if( _stricmp( aSurface->GetFormat()->GetId(), "A8R8G8B8" ) || !aSurface->GetTexels() )
		return ACT_ERROR; 

	U32 *pTexel = reinterpret_cast<U32*>(aSurface->GetTexels());
	for (u32 j=0; j<aSurface->GetHeight(); ++j)
	{
		for (u32 i=0; i<aSurface->GetWidth(); ++i)
		{
			const u32 a = (*pTexel & MASK_A) >> 24;
			const u32 r = (*pTexel & MASK_R) >> 16;
			const u32 g = (*pTexel & MASK_G) >> 8;
			const u32 b = (*pTexel & MASK_B);

			U32 v = (lut.table[a] << 24) | (lut.table[r] << 16) | (lut.table[g] << 8) | (lut.table[b]);
			*pTexel = (*pTexel & ~channelMask) | (channelMask & v);
			pTexel++;
		}
	}
	
	return ACT_OK;
}

ACTRESULT UnsharpMask(ActTools::Surface* pSrcSurface, u32 channelMask, F32 radius=1.0f, F32 amount=0.5f, F32 threshold=0.f)
{
	if( _stricmp( pSrcSurface->GetFormat()->GetId(), "A8R8G8B8" ) || !pSrcSurface->GetTexels() )
		return ACT_ERROR; 

	ActTools::Surface tmpSurface(*pSrcSurface);

	//gaussian blur
	ACTRESULT success = ACT_OK;
	success = RescaleSurface(&tmpSurface, pSrcSurface,
		ActTools::FT_GAUSSIAN, radius,
		ActTools::FT_GAUSSIAN, radius,
		ActTools::EDGE_CLAMP, ActTools::EDGE_CLAMP);

	if (success != ACT_OK)
		return success;

	//now sharpen
	U32 *pSrcTexel = reinterpret_cast<U32*>(pSrcSurface->GetTexels());
	const U32 *pTmpTexel = reinterpret_cast<U32*>(tmpSurface.GetTexels());

	u32 nTexels = pSrcSurface->GetWidth() * pSrcSurface->GetHeight();

	for (u32 i=0; i<nTexels; ++i)
	{		
		const s32 a0 = (*pSrcTexel & MASK_A) >> 24;
		const s32 r0 = (*pSrcTexel & MASK_R) >> 16;
		const s32 g0 = (*pSrcTexel & MASK_G) >> 8;
		const s32 b0 = (*pSrcTexel & MASK_B);

		const s32 a1 = (*pTmpTexel & MASK_A) >> 24;
		const s32 r1 = (*pTmpTexel & MASK_R) >> 16;
		const s32 g1 = (*pTmpTexel & MASK_G) >> 8;
		const s32 b1 = (*pTmpTexel & MASK_B);

		u32 a2 = (Maths::Abs<f32>(a0-a1) > threshold) ? (u32) Maths::Clamp<f32>(amount*(a0-a1)+a0, 0, 255) : (u32) a0;
		u32 r2 = (Maths::Abs<f32>(r0-r1) > threshold) ? (u32) Maths::Clamp<f32>(amount*(r0-r1)+r0, 0, 255) : (u32) r0;
		u32 g2 = (Maths::Abs<f32>(g0-g1) > threshold) ? (u32) Maths::Clamp<f32>(amount*(g0-g1)+g0, 0, 255) : (u32) g0;
		u32 b2 = (Maths::Abs<f32>(b0-b1) > threshold) ? (u32) Maths::Clamp<f32>(amount*(b0-b1)+b0, 0, 255) : (u32) b0;

		U32 v = (a2 << 24) | (r2 << 16) | (g2 << 8) | (b2);
		*pSrcTexel = (*pSrcTexel & ~channelMask) | (channelMask & v);

		pSrcTexel++;
		pTmpTexel++;
	}

	return ACT_OK;
}

ACTRESULT EnhanceMipContrast( ActTools::Texture* aTexture, u32 channelMask)
{
	if( _stricmp( aTexture->GetFormat()->GetId(), "A8R8G8B8" ) )
		return ACT_ERROR; 

#ifdef DEBUG_MIPPROCESSING
	Util::C_String str;
	Attk::C_Image image;
	image.NewPixelData(aTexture->GetMipmap(0, 0)->GetWidth(), aTexture->GetMipmap(0, 0)->GetHeight(), 1,  Attk::E_PixelFormat_B8G8R8A8U, aTexture->GetMipmap(0, 0)->GetTexels());
	str.Printf("z:\\textureproc\\before_0.tga");
	image.Write(str.Chars());
#endif

	for (U32 i=1; i<aTexture->GetNumMipMaps(); ++i)
	{
		for (u32 j=0; j<aTexture->GetDepth(); ++j)
		{
#ifdef DEBUG_MIPPROCESSING
			image.NewPixelData(aTexture->GetMipmap(j, i)->GetWidth(), aTexture->GetMipmap(j, i)->GetHeight(), 1,  Attk::E_PixelFormat_B8G8R8A8U, aTexture->GetMipmap(j, i)->GetTexels());			
			str.Printf("z:\\textureproc\\before_%d.tga", i);
			image.Write(str.Chars());
#endif
			UnsharpMask(aTexture->GetMipmap(j, i), channelMask, 1.f*i, 0.5f + 0.1f*i);

			//only adjust alphas
			if ((channelMask & MASK_A) && aTexture->GetMipmap(j, i)->GetWidth() > 8 && aTexture->GetMipmap(j, i)->GetHeight() > 8)
			{
				AdjustmentLUT lut;
				GenerateEnhancementLUT(lut, 1);
				AdjustChannels(aTexture->GetMipmap(j, i), lut, MASK_A);
			}
#ifdef DEBUG_MIPPROCESSING
			image.NewPixelData(aTexture->GetMipmap(j, i)->GetWidth(), aTexture->GetMipmap(j, i)->GetHeight(), 1,  Attk::E_PixelFormat_B8G8R8A8U, aTexture->GetMipmap(j, i)->GetTexels());
			str.Printf("z:\\textureproc\\after_%d.tga", i);
			image.Write(str.Chars());
#endif		
		}
	}

	return ACT_OK;
}

}
