//-----------------------------------------------------------------------------
// File: DefaultTexture.cpp
//
// Desc: Store the lightmap data colour values, sizes, etc.
//       This is the default texture type which stores a basic RGB lightmap
//       No directional information is stored. The incoming sample normals
//       are discarded.
//-----------------------------------------------------------------------------

#include <stdafx.h>

#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/TextureTypes/DefaultTexture.h"
#include "Lux/ImageFilter.h"
#include "Lux/Sampler.h"
#include "Lux/RenderChannel.h"

using namespace LUX;

bool gbBlueInRedHack = false;

// create a new texture
void DefaultTexture::Create(int x, int y)
{
	m_pData = (SLMapPixel*) AllocAligned( sizeof(SLMapPixel) * x*y);
	m_pDData = (SLMapPixel*) AllocAligned( sizeof(SLMapPixel) * x*y);
	m_pIntermediateBuffer = new float4[x*y];
	if (!m_pIntermediateBuffer || !m_pData || !m_pDData) throw std::bad_alloc();
}

// clear texture data
void DefaultTexture::Clear()
{
	int c = m_cX * m_cY;
	memset(m_pData, 0, sizeof(SLMapPixel) * c);
	memset(m_pDData, 0, sizeof(SLMapPixel) * c);
	memset(m_pIntermediateBuffer, 0, sizeof(float4) * c);

	ITexture::Clear();
}

// free this texture
void DefaultTexture::Free()
{
	FreeAligned( m_pData );
	FreeAligned( m_pDData );
	delete [] m_pIntermediateBuffer;

	m_bInitialized = false;
}

