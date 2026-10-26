//-----------------------------------------------------------------------------
// File: ITexture.cpp
//
// Desc: Interface and common functionality for all texture types
//       Base class for different types of intermediate texture
//
//       Initially, the lighting is rendered into a texture derived from this
//       class. Once the rendered is complete, the texture gets converted
//       into an RGBTexture.
//-----------------------------------------------------------------------------

#include <stdafx.h>

#include "Lux/TextureTypes/ITexture.h"
#include "Lux/ImageFilter.h"
#include "Lux/Sampler.h"
#include "Lux/RenderChannel.h"

using namespace LUX;

// create a new texture
void ITexture::Create(int x, int y)
{
	m_cX	= x;
	m_cY	= y;
	m_iPass = 1;
	m_bInitialized = true;

	m_pTouchedBuf = (STouched*) AllocAligned( sizeof(STouched) * x*y);
	if (!m_pTouchedBuf) throw std::bad_alloc();
}

// resize texture data
void ITexture::Resize(int x, int y)
{
	m_cX = x; m_cY = y;
	Clear();
}

// clear texture data
void ITexture::Clear()
{
	int c = m_cX * m_cY;
	memset(m_pTouchedBuf, 0, sizeof(STouched) * c);
}

// free this texture
void ITexture::Free()
{
	if ( m_pTouchedBuf )
	{
		FreeAligned( m_pTouchedBuf );
		m_pTouchedBuf = NULL;
	}

	m_bInitialized = false;
}

// touch a pixel
void ITexture::TouchPixel(int x, int y)
{
	assert( x>=0 && x<m_cX && y>=0 && y<m_cY );
	m_pTouchedBuf[ x + y*m_cX ].bTouched1 = 1;
}

// returns true when a pixel was written to
bool ITexture::CheckPixelUsed(int x, int y)
{
	assert( x>=0 && x<m_cX && y>=0 && y<m_cY );
	return CheckPixelUsed( x + y*m_cX );
}

// returns true when a pixel was written to
bool ITexture::CheckPixelUsed(int i)
{
	return m_pTouchedBuf[ i ].bTouched1;
}

// touch a pixel (channel 2)
void ITexture::TouchPixel2(int x, int y)
{
	assert( x>=0 && x<m_cX && y>=0 && y<m_cY );
	m_pTouchedBuf[ x + y*m_cX ].bTouched2 = 1;
}

// returns true when a pixel was written to (channel 2)
bool ITexture::CheckPixelUsed2(int x, int y)
{
	assert( x>=0 && x<m_cX && y>=0 && y<m_cY );
	return m_pTouchedBuf[ x + y*m_cX ].bTouched2;
}

// set a subwindow within the texture. rendering will be clamped to this window
void ITexture::SetWindow(int x, int y, int cx, int cy)
{
	m_window.x = x;
	m_window.y = y;
	m_window.cx = cx;
	m_window.cy = cy;
}

// utility function - precalculate sample offsets for AddSample
void ITexture::PrecalcSampleOffsets(float2 t, ImageFilter *pFilter,
							  int &x0, int &y0, int &x1, int &y1, int *ifx, int *ify)
{
	static int xfilterbuffer[16];
	static int yfilterbuffer[16];

	// texture sub-window could be set here
	int xPixelStart=m_window.x, yPixelStart=m_window.y, xPixelCount=m_window.cx, yPixelCount=m_window.cy;
	float *pfTable = pFilter->GetTable();

	float dImageX = t.x - 0.5f;
	float dImageY = t.y - 0.5f;

	x0 = Ceil2Int (dImageX - pFilter->xWidth);
	x1 = Floor2Int(dImageX + pFilter->xWidth);
	y0 = Ceil2Int (dImageY - pFilter->yWidth);
	y1 = Floor2Int(dImageY + pFilter->yWidth);
	x0 = max(x0, xPixelStart);
	x1 = min(x1, xPixelStart + xPixelCount - 1);
	y0 = max(y0, yPixelStart);
	y1 = min(y1, yPixelStart + yPixelCount - 1);

	// Loop over filter support and add sample to pixel arrays
	// Precompute x and y filter table offsets
	for (int x = x0; x <= x1; ++x)
	{
		float fx = fabs((x - dImageX) *
		                 pFilter->invXWidth * FILTER_TABLE_SIZE);
		ifx[x-x0] = min(Floor2Int(fx), FILTER_TABLE_SIZE-1);
	}
	for (int y = y0; y <= y1; ++y)
	{
		float fy = fabs((y - dImageY) *
		                 pFilter->invYWidth * FILTER_TABLE_SIZE);
		ify[y-y0] = min(Floor2Int(fy), FILTER_TABLE_SIZE-1);
	}
}

// returns true if this texture needs tangent space to render
bool ITexture::NeedTangentSpace()
{
	return false;
}
