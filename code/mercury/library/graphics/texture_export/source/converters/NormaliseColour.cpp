#include "stdafx.h"

#include "texture_export/Texture.h"

using namespace ActTools;

namespace Texutils
{

void GenerateNormalisedTexture(U32* data, int width, int height)
{
	for (int y = 0; y < height; y++)
    {
		for (int x = 0; x < width; x++)
		{
			const U32 nInput = data[y * width + x];
			const Maths::C_RGBA_F32	vInput			= Maths::C_RGBA_F32(Maths::C_RGBA_U8(nInput));
			const Maths::HVec3		vRGB			= Maths::HVec3(vInput.R(), vInput.G(), vInput.B());
			const Maths::HF32		vRGBLength		= vRGB.Length();
			const Maths::HVec3		vRGBNormalised	= Maths::Normalise(vRGB);
			const Maths::C_RGBA_F32	vOutput			= Maths::C_RGBA_F32(vRGBNormalised.X(), vRGBNormalised.Y(), vRGBNormalised.Z(), vRGBLength);

			data[y * width + x] = Maths::C_RGBA_U8(vOutput.Saturate());
		}
    }
}

/// normalise the colour of the given texture, storing the length in the alpha channel
ACTRESULT NormaliseColour( ActTools::Texture* aTexture )
{
	// convert to A8R8G8B8
	ActTools::A8R8G8B8 a8r8g8b8;
	aTexture->ConvertTo(&a8r8g8b8);

	// normalise the colour
	for (U32 i=0; i<aTexture->GetDepth() * aTexture->GetNumMipMaps(); i++)
	{
		GenerateNormalisedTexture((U32*)aTexture->GetSurface(i).GetTexels(), aTexture->GetWidth(), aTexture->GetHeight());
	}

	return ACT_OK;
}

}
