//-----------------------------------------------------------------------------
// File: DirectionTexture.cpp
//
// Desc: Store the lightmap data colour values, sizes, etc.
//       This is the default texture type which stores a basic RGB lightmap
//       No directional information is stored. The incoming sample normals
//       are discarded.
//-----------------------------------------------------------------------------

#include <stdafx.h>

#include "Lux/ImageFilter.h"
#include "Lux/Sampler.h"
#include "Lux/RenderChannel.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/TextureTypes/DirectionTexture.h"

using namespace LUX;

// export RGBA values 
U32 *DirectionTexture::GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool bAlphaScaling, bool bSwizzleSH)
{
	// TODO: contrast, etc.
	float f;
	float scale = fBrightness;

	// gamma correction
	float invGamma = 1.0f / 2.2f;

	// malloc new data
	U32 *m_pRGBA32Data = new U32[m_cX*m_cY];

	U32 *pOut = m_pRGBA32Data;

	for (int i=0; i<m_cX*m_cY; i++, pOut++)
	{
		int c,iColour = LMAP_UNUSED_SPACE_COLOUR; // untouched pixels are coloured bright green

		if (CheckPixelUsed(i))
		{
			float m = 1.0f;

			// get main buffer colour
			float3 vColour;
			float3 vGradient[3];

			switch (iLayer)
			{
			case 1:	// ambient colour
				vColour.x = m_pIntermediateBuffer[i].fSH[0][0];
				vColour.y = m_pIntermediateBuffer[i].fSH[1][0];
				vColour.z = m_pIntermediateBuffer[i].fSH[2][0];
				vColour *= fBrightness;
				break;
			case 2:	// directional vector (normalized gradients)
				for (int g=0; g<3; g++)
				{
					vGradient[g].x = m_pIntermediateBuffer[i].fSH[g][1];
					vGradient[g].y = m_pIntermediateBuffer[i].fSH[g][2];
					vGradient[g].z = m_pIntermediateBuffer[i].fSH[g][3];
				}

                // get normalized average gradient
				vColour = Normalize(vGradient[0] + vGradient[1] + vGradient[2]);
				vColour = (vColour * 0.5f) + 0.5f;
				break;
			case 0: // gradient scale
				for (int g=0; g<3; g++)
				{
					vGradient[g].x = m_pIntermediateBuffer[i].fSH[g][1];
					vGradient[g].y = m_pIntermediateBuffer[i].fSH[g][2];
					vGradient[g].z = m_pIntermediateBuffer[i].fSH[g][3];
				}

				// get average gradient length
				vColour.x = Length(vGradient[0]);
				vColour.y = Length(vGradient[1]);
				vColour.z = Length(vGradient[2]);
				vColour *= fBrightness;
				break;
			}

/*			if (bGamma)
			{
				m = vColour.Luma();

				if (m>1.0f)
				{
					m = 1.0f/m;
				}
				else
					m = 1.0f;
			}
*/
			// calculate the RGBA32 value from the float3
			for (int j=0; j<3; j++)
			{
				f = vColour[j];
/*
				if (bGamma)
				{
					f *= m;
					f = powf(f, invGamma);
				}
*/
				c = (int)(f*255.0f + 0.5f); // clamp
				if (c < 0)   c = 0;
				if (c > 255)
					c = 255;
				iColour = iColour<<8;
				iColour += c;
			}

			if (iColour == TEXEL_UNUSED_COLOUR_VALUE)
				iColour = TEXEL_UNUSED_COLOUR_VALUE_PLUS_ONE;

			*pOut = iColour;
		}
		else
			*pOut = TEXEL_UNUSED_COLOUR_VALUE;
	}

	return m_pRGBA32Data;
}
