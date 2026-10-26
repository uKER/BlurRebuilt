#include "stdafx.h"

#include "texture_export/Texture.h"

using namespace ActTools;

namespace Texutils
{

inline f32 ToSRGB(f32 linear)
{
	if (linear <=  0.0031308f)
		return linear*12.92f;
	else
		return pow(linear, 1.f/2.4f)*1.055f - 0.055f;
}

inline f32 ToLinear(f32 sRGB)
{
	if (sRGB <= 0.04045f)
		return sRGB/12.92f;
	else
		return pow((sRGB+0.055f)/1.055f, 2.4f);
}



ACTRESULT LinearToSRGB( ActTools::Texture* aTexture )
{
	if( _stricmp( aTexture->GetFormat()->GetId(), "A8R8G8B8" ) )
		return ACT_ERROR; 


	u8 lut[256];
	for (s32 i=0; i<256; ++i)
		lut[i] = (u8)(255.f*ToSRGB((f32)i/255.f) + 0.5f); //round to nearest


	for (u32 i=0; i<aTexture->GetDepth() * aTexture->GetNumMipMaps(); i++)
	{
		u8*	pTexels	= (u8*)aTexture->GetSurface(i).GetTexels();
		u32 nTexels = aTexture->GetSurface(i).GetWidth()*aTexture->GetSurface(i).GetHeight();

		for (u32 x=0; x<nTexels; ++x)
		{
			//convert rgb channels. leave alpha
			pTexels[x*4 + 0] = lut[pTexels[x*4 + 0]];
			pTexels[x*4 + 1] = lut[pTexels[x*4 + 1]];
			pTexels[x*4 + 2] = lut[pTexels[x*4 + 2]];
		}
	}

	return ACT_OK;
}

ACTRESULT SRGBToLinear( ActTools::Texture* aTexture )
{
	if( _stricmp( aTexture->GetFormat()->GetId(), "A8R8G8B8" ) )
		return ACT_ERROR; 

	u8 lut[256];
	for (s32 i=0; i<256; ++i)
		lut[i] = (u8)(255.f*ToLinear((f32)i/255.f) + 0.5f); //round to nearest


	for (u32 i=0; i<aTexture->GetDepth() * aTexture->GetNumMipMaps(); i++)
	{
		u8*	pTexels	= (u8*)aTexture->GetSurface(i).GetTexels();
		u32 nTexels = aTexture->GetSurface(i).GetWidth()*aTexture->GetSurface(i).GetHeight();

		for (u32 x=0; x<nTexels; ++x)
		{
			//convert rgb channels. leave alpha
			pTexels[x*4 + 0] = lut[pTexels[x*4 + 0]];
			pTexels[x*4 + 1] = lut[pTexels[x*4 + 1]];
			pTexels[x*4 + 2] = lut[pTexels[x*4 + 2]];
		}
	}

	return ACT_OK;
}

}
