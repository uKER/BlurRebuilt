//-----------------------------------------------------------------------------
// File: RGBTexture.cpp
//
// Desc: Store the lightmap data colour values, sizes, etc. in final format
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "Lux/TextureTypes/ITexture.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/ImageFilter.h"
#include "Lux/Sampler.h"
#include "Lux/RenderChannel.h"

using namespace LUX;

IMPLEMENT_PERSISTENCE(RGBTexture, "RGBTexture");

// create a new texture - placeholder for RGBA32 data only
void RGBTexture::CreateRGBATexture(ITexture *pOriginal, float fGamma, float fBrightness)
{
	m_cX = pOriginal->m_cX;
	m_cY = pOriginal->m_cY;
	bool bFoundQNAN;

	m_nLayers = pOriginal->GetNumLayers();
	m_pRGBA32Data = new U32*[m_nLayers];
	for (int i=0; i<m_nLayers; i++)
	{
		m_pRGBA32Data[i] = pOriginal->GenerateRGBAData(i, fGamma, fBrightness, &m_bFoundQNAN);

		FillForDXT(m_pRGBA32Data[i]);
	}

	pOriginal->GetCustomData(m_CustomData);

	m_bInitialized = true;
}

// free this texture
void RGBTexture::Free()
{
	m_bInitialized = false;

	if (m_pRGBA32Data)
	{
		for (int i=0; i<m_nLayers; i++)
			delete [] m_pRGBA32Data[i];

		delete [] m_pRGBA32Data;
		m_pRGBA32Data = NULL;
	}
}

// flush RGBA buffer
void RGBTexture::FlushRGBAData()
{
	Free();
}

// export RGBA values 
U32 *RGBTexture::GetRGBAData(int iChannel)
{
	// data already exists
	assert(m_pRGBA32Data && iChannel < m_nLayers);
	return m_pRGBA32Data[iChannel];
}

// returns true when a pixel was written to (for when we only have the RGBA data)
bool RGBTexture::CheckPixelUsed(int x, int y)
{
	assert(x>=0 && x<m_cX && y>=0 && y<m_cY);
	int i = x + y*m_cX;
	return m_pRGBA32Data[0][i] != TEXEL_UNUSED_COLOUR_VALUE;
}

// check whether an area of this texture has any used pixels in (false if it's totally empty)
bool RGBTexture::CheckImageBlockUsed(int x, int y, int w, int h)
{
	int i;
	int j;
	unsigned int *src;

	for(i = 0; i < h; ++i)
	{
		for(j = 0; j < w; ++j)
		{
			if (CheckPixelUsed(j + x, i + y)) // check whether this pixel has been written to
				return true;
		}
	}
	return false;
}

// fill empty pixels in the DXT blocks with neighbouring stuff
void RGBTexture::FillForDXT(U32 *pData)
{
	for (int y=0; y<m_cY; y+=4)
	{
		for (int x=0; x<m_cX; x+=4)
		{
			bool bUsed = false;

			int iCol = 0;
			// get a colour to fill this block with
			for (int i=0; i<16; i++)
			{
				int tx = x+(i%4);
				int ty = y+(i/4);

				if (tx>=m_cX || ty>=m_cY) continue;
				// check pixel used
				if (pData[m_cX*ty+tx] != TEXEL_UNUSED_COLOUR_VALUE)
				{
					iCol = pData[m_cX*ty+tx];
					bUsed = true;
					break;
				}
			}
			// none of the pixels in this square are used, so skip it
			if (!bUsed)
				continue;

			// fill any unused pixels with the fill colour
			for (int i=0; i<16; i++)
			{
				int tx = x+(i%4);
				int ty = y+(i/4);

				if (tx>=m_cX || ty>=m_cY) continue;
				if (pData[m_cX*ty+tx] == TEXEL_UNUSED_COLOUR_VALUE)
				{
					pData[m_cX*ty+tx] = iCol;
				}
			}

		}
	}
}

// serialize/deserialize
void RGBTexture::write( ost::Engine& ar )
{
	ar << m_cX << m_cY << m_iAssociatedChart << m_nLayers << m_CustomData << m_bFoundQNAN;

	for (int i=0; i<m_nLayers; i++)
	{
		if (m_cX * m_cY)
			ar.writeBinary(m_pRGBA32Data[i], m_cX * m_cY * sizeof(U32));
	}
}
void RGBTexture::read( ost::Engine& ar )
{
	MEMORY_TRACKER_PUSH("tools.lux.server.rgbtexture");

	m_bInitialized = true;

	ar >> m_cX >> m_cY >> m_iAssociatedChart >> m_nLayers >> m_CustomData >> m_bFoundQNAN;

	m_pRGBA32Data = new U32*[m_nLayers];
	for (int i=0; i<m_nLayers; i++)
	{
		m_pRGBA32Data[i] = new U32[m_cX*m_cY];

		ar.readBinary(m_pRGBA32Data[i], m_cX * m_cY * sizeof(U32));
	}

	MEMORY_TRACKER_POP("tools.lux.server.rgbtexture");
}

// returns true if a QNAN was found in the output data
bool RGBTexture::FoundQNAN()
{
	return m_bFoundQNAN;
}


// Create an RGBTexture from a floating point one. also copies over the vertex data
RGBTexture::RGBTexture(ITexture *pOrg, float fGamma, float fBrightness)
{
	CreateRGBATexture(pOrg, fGamma, fBrightness);
}

// return custom 'context' data
Ut::Vector<U8> &RGBTexture::GetCustomData()
{
	return m_CustomData;
}

// potentially optimize RGB Texture by scaling it down if it's flat coloured
// returns true if we optimized the texture size
bool RGBTexture::Optimize()
{
	// automatically reject multi-channel textures
	if (m_nLayers != 1)
		return false;

	// iterate over pixels, check all colours match
	int iCol = -1;
	for (int y=0; y<m_cY; y++)
	{
		int ip = y*m_cX;
		for (int x=0; x<m_cX; x++, ip++)
		{
			if (!CheckPixelUsed(x,y))
				continue;

			int iPixel = m_pRGBA32Data[0][ip];

			if (iCol == -1)
				iCol = iPixel;
			else if (iCol != iPixel)
				return false;			// not all once colour, can't optimize
		}
	}

	// optimize by scaling down the texture to 4x4

	// delete original
	delete m_pRGBA32Data[0];

	// allocate new and initialize with the flat colour
	m_pRGBA32Data[0] = new U32[4*4];
	for (int i=0; i<16; i++)
		m_pRGBA32Data[0][i] = iCol;

	// set dimensions
	m_cX = m_cY = 4;

	return true;
}
