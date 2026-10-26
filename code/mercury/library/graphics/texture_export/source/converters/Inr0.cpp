#include "stdafx.h"

#include "texture_export/Texture.h"

using namespace ActTools;

namespace Texutils
{

/// convert the given texture from an IPR0 to an INR0, given a neon texture
ACTRESULT Ipr0ToInr0( ActTools::Texture* aTexture, ActTools::Texture* aNeonTexture )
{
	// convert to A8R8G8B8
	ActTools::A8R8G8B8 a8r8g8b8;
	aTexture->ConvertTo(&a8r8g8b8);

	ActTools::L8 l8;
	aNeonTexture->ConvertTo(&l8);

	if (aNeonTexture->GetWidth() != aTexture->GetWidth() ||
		aNeonTexture->GetHeight() != aTexture->GetHeight() ||
		aNeonTexture->GetDepth() != aTexture->GetDepth() ||
		aNeonTexture->GetNumMipMaps() != aTexture->GetNumMipMaps())
	{
		return ACT_ERROR;
	}

	for (U32 i=0; i<aTexture->GetDepth() * aTexture->GetNumMipMaps(); i++)
	{
		U8*	pIPR0Texels	= (U8*)aTexture->GetSurface(i).GetTexels();
		U8*	pNeonTexels	= (U8*)aNeonTexture->GetSurface(i).GetTexels();

		int iWidth = aTexture->GetWidth();
		int iHeight = aTexture->GetHeight();

		for (int x=0; x<iWidth * iHeight; x++)
		{
			pIPR0Texels[x * 4 + 1] = pNeonTexels[x];
		}
	}

	return ACT_OK;
}

}
