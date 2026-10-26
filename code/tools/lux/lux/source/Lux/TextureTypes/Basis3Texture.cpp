//-----------------------------------------------------------------------------
// File: Basis3Texture.cpp
//
// Desc: Store the lightmap data colour values, sizes, etc.
//       Texture type which uses 3 basis vectors in tangent space to store
//		 directional light information.
//-----------------------------------------------------------------------------

#include <stdafx.h>

#include "Lux/ImageFilter.h"
#include "Lux/Sampler.h"
#include "Lux/RenderChannel.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/TextureTypes/Basis3Texture.h"

using namespace LUX;

extern bool gbAlternativeGamma;

// constructor
Basis3Texture::Basis3Texture(int x, int y, ImageFilter *pFilter, bool bHemi3v) :
		ITexture(x,y,pFilter)
{
	Create(x,y);

	m_bHemi3v = bHemi3v;
}

// calculate per 4x4 texel block scale value and put it in the alpha
void Basis3Texture::CalculateAlphaScaling(int iChannel)
{
	// convert RGB to rgb ratio + luminance
	for (int d=0; d<m_cX*m_cY; d++)
	{
		float4 v = m_pIntermediateBuffer[d].f[iChannel];
		float4 vOut;
		float fMax = v.x;
		if (v.y > fMax) fMax = v.y;
		if (v.z > fMax) fMax = v.z;
		if (fMax > 1.0f)
			fMax = 1.0f;

		float fInv = 1.0f / fMax;

		vOut.x = v.x * fInv;
		vOut.y = v.y * fInv;
		vOut.z = v.z * fInv;
		vOut.w = fMax;

		m_pIntermediateBuffer[d].f[iChannel] = vOut;
	}
}

// export RGBA values 
U32 *Basis3Texture::GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool bAlphaScaling, bool bSwizzleSH)
{
	// TODO: contrast, etc.
	float f;

	// gamma correction
	float invGamma = 1.0f / fGamma;

	bool bGamma = (fGamma != 1.0f);

	// malloc new data
	U32 *m_pRGBA32Data = new U32[m_cX*m_cY];

	U32 *pOut = m_pRGBA32Data;

	// apply brightness
	for (int d=0; d<m_cX*m_cY; d++)
		m_pIntermediateBuffer[d].f[iLayer] = m_pIntermediateBuffer[d].f[iLayer] * fBrightness;

	if (iLayer < 3)
	{
		// apply alpha scaling to quadrants 1-3
		if (bAlphaScaling)
			CalculateAlphaScaling(iLayer);
		else
		{
			// if alpha scaling is off, fill quadrants 1-3 with white alpha
			for (int i=0; i<m_cX*m_cY; i++)
				m_pIntermediateBuffer[i].f[iLayer].w = 1.0f;
		}
	}

	// when the first layer gets grabbed, swizzle the SH nodes
	int iAvgLayer = 0;
	for (int i=0; i<m_cX*m_cY; i++, pOut++)
	{
		int c,iColour = LMAP_UNUSED_SPACE_COLOUR; // untouched pixels are coloured bright green
		U8 *pColour = (U8*)&iColour;

		if (CheckPixelUsed(i))
		{
			float m = 1.0f;
			float m2 = 1.0f;

			// get main buffer colour
			float4 vColour;

			vColour = float4(	m_pIntermediateBuffer[i].f[iLayer].x, 
								m_pIntermediateBuffer[i].f[iLayer].y,
								m_pIntermediateBuffer[i].f[iLayer].z,
								m_pIntermediateBuffer[i].f[iLayer].w);

			// for hemisphere 3-basis RGB, prevent clipping on the 
			if (iLayer == 3 && m_bHemi3v)
			{
				float fHighest = m_pIntermediateBuffer[i].f[iLayer].x;
				if (m_pIntermediateBuffer[i].f[iLayer].y > fHighest)
					fHighest = m_pIntermediateBuffer[i].f[iLayer].y;
				if (m_pIntermediateBuffer[i].f[iLayer].z > fHighest)
					fHighest = m_pIntermediateBuffer[i].f[iLayer].z;

				if (fHighest > 1.0f)
				{
					float fScale = 1.0f / fHighest;
					m_pIntermediateBuffer[i].f[iLayer].x *= fScale;
					m_pIntermediateBuffer[i].f[iLayer].y *= fScale;
					m_pIntermediateBuffer[i].f[iLayer].z *= fScale;
				}
			}

			// calculate the RGBA32 value from the float3
			for (int j=0; j<4; j++)
			{
				f = vColour[j];

				if (bGamma)
				{
					if (!gbAlternativeGamma)
					{
						if (iLayer<3)
						{
							// apply gamma curve to first three quadrants, all components
							f = powf(vColour[j], invGamma);
						}
					}
					else
					{
						// alternative - apply gamma curve to Q1-3 alpha and Q4 green + alpha
						if (iLayer == 3 || j == 3)
							f = powf(vColour[j], invGamma);
					}

				}

				c = (int)(f*255.0f + 0.5f); // clamp
				if (c < 0)   c = 0;
				if (c > 255)
					c = 255;
				pColour[j] = c;
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

// create a new texture
void Basis3Texture::Create(int x, int y)
{
	m_pData = new SLMapPixel[x*y];
	m_pIntermediateBuffer = new SLMapPixel[x*y];
}

// clear the contents of the texture
void Basis3Texture::Clear()
{
	int c = m_cX * m_cY;
	memset(m_pData, 0, sizeof(SLMapPixel) * c);
	memset(m_pIntermediateBuffer, 0, sizeof(SLMapPixel) * c);

	ITexture::Clear();
}

// free this texture
void Basis3Texture::Free()
{
	delete [] m_pData;
	ClearIntermediateBuffer();
	for (int i=0; i<m_DData.Size(); i++)
		delete m_DData[i];
	delete [] m_pIntermediateBuffer;

	m_bInitialized = false;

	ITexture::Free();
}

// remove all the channels on the intermediate buffer
void Basis3Texture::InitializeIntermediateBuffer(int nChannels)
{
	MEMORY_TRACKER_PUSH("tools.lux.client.texture");

	for (int i=m_DData.Size(); i<nChannels; i++)
	{
		m_DData.PushBack( new SLMapPixel[m_cX*m_cY] ); 
		memset(m_DData[i], 0, sizeof(SLMapPixel) * m_cX*m_cY);
		assert(i == m_DData.Size()-1);
	}

	MEMORY_TRACKER_POP("tools.lux.client.texture");
}

// initialize n layers on the intermediate buffer
void Basis3Texture::ClearIntermediateBuffer()
{
	for (int i=0; i<m_DData.Size(); i++)
		delete [] m_DData[i];

	m_DData.Destroy();
}

#define SQRT2 1.4142135623730950488016887242097
#define SQRT3 1.7320508075688772935274463415059
#define SQRT6 2.4494897427831780981972840747059
#define SQRT2OVER3 0.81649658092772603273242802490196

#define LMAPAXIS1 SQRT2OVER3, 0, 1.0/SQRT3
#define LMAPAXIS2 -1.0/SQRT6, 1.0/SQRT2, 1.0/SQRT3
#define LMAPAXIS3 -1.0/SQRT6,-1.0/SQRT2, 1.0/SQRT3

const float3 Basis3Texture::m_vAxis[3] = { float3(LMAPAXIS1), float3(LMAPAXIS2), float3(LMAPAXIS3) };

// add a sample to the normal buffer
void Basis3Texture::AddSample(float2 t, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent)
{
	static int ifx[16];
	static int ify[16];
	int x0, x1, y0, y1;
	float *pfTable = m_pFilter->GetTable();
	float f;

	PrecalcSampleOffsets(t, m_pFilter, x0, y0, x1, y1, ifx, ify);

	HVec3 _vNormal(vNormal.x, vNormal.y, vNormal.z);

	_vNormal = _vNormal * mtxTangent;

	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x)
		{
			// Evaluate filter value at (x,y) pixel
			int offset = ify[y-y0]*FILTER_TABLE_SIZE + ifx[x-x0];
			float filterWt = pfTable[offset];
			// Update pixel values with filtered sample contribution
			SLMapPixel *pPixel = &GetPixel(x, y);

			f = DotProduct(_vNormal, m_vAxis[0]);
			if (f > 0)
				(float3&)pPixel->f[0] += vSample * f * filterWt;
			f = DotProduct(_vNormal, m_vAxis[1]);
			if (f > 0)
				(float3&)pPixel->f[1] += vSample * f * filterWt;
			f = DotProduct(_vNormal, m_vAxis[2]);
			if (f > 0)
				(float3&)pPixel->f[2] += vSample * f * filterWt;

			// store original value in 4th channel
			(float3&)pPixel->f[3] += vSample * filterWt;

			pPixel->fWeight += filterWt;
		}
}

// add a sample to the dicontinuity buffer (multiple channels)
void Basis3Texture::AddDiscontinuitySample(int iX, int iY, const Ut::Vector<float3> &samples, const float3 &vNormal, const HMat33 &mtxTangent)
{
	InitializeIntermediateBuffer(samples.Size());

	HVec3 _vNormal(vNormal.x, vNormal.y, vNormal.z);
	_vNormal = _vNormal * mtxTangent;

	int x0 = iX-1;
	int x1 = iX+1;
	int y0 = iY-1;
	int y1 = iY+1;

	x0 = max(x0, m_window.x);
	x1 = min(x1, m_window.x + m_window.cx -1);
	y0 = max(y0, m_window.y);
	y1 = min(y1, m_window.y + m_window.cy -1);

	for (int iChannel=0; iChannel<samples.Size(); iChannel++)
	{
		for (int y = y0; y <= y1; ++y)
		{
			for (int x = x0; x <= x1; ++x)
			{
				SLMapPixel *pPixel = &m_DData[iChannel][x + y*m_cX];
				float3 vSample = samples[iChannel];
				if (vSample.x != 0 || vSample.y != 0 || vSample.z != 0)
				{
					float f;

					f = DotProduct(_vNormal, m_vAxis[0]);
					if (f > 0)
						(float3&)pPixel->f[0] += vSample * f;
					f = DotProduct(_vNormal, m_vAxis[1]);
					if (f > 0)
						(float3&)pPixel->f[1] += vSample * f;
					f = DotProduct(_vNormal, m_vAxis[2]);
					if (f > 0)
						(float3&)pPixel->f[2] += vSample * f;

					(float3&)pPixel->f[3] += vSample;
				}

				pPixel->fWeight += 1.0f;
			}
		}
	}
}

// add a sample to the dicontinuity buffer (one channel)
void Basis3Texture::AddDiscontinuitySample(int iX, int iY, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent)
{
	InitializeIntermediateBuffer(1);

	int x0 = iX-1;
	int x1 = iX+1;
	int y0 = iY-1;
	int y1 = iY+1;

	x0 = max(x0, m_window.x);
	x1 = min(x1, m_window.x + m_window.cx -1);
	y0 = max(y0, m_window.y);
	y1 = min(y1, m_window.y + m_window.cy -1);

	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x)
		{
			SLMapPixel *pPixel = &m_DData[0][x + y*m_cX];
			if (vSample.x != 0 || vSample.y != 0 || vSample.z != 0)
			{
				float f;

				if (m_bHemi3v)
				{
					HVec3 _vNormal(vNormal.x, vNormal.y, vNormal.z);
					_vNormal = _vNormal * mtxTangent;

					f = DotProduct(_vNormal, m_vAxis[0]);
					if (f > 0)
						(float3&)pPixel->f[0] += vSample * f;
					f = DotProduct(_vNormal, m_vAxis[1]);
					if (f > 0)
						(float3&)pPixel->f[1] += vSample * f;
					f = DotProduct(_vNormal, m_vAxis[2]);
					if (f > 0)
						(float3&)pPixel->f[2] += vSample * f;
				}

				(float3&)pPixel->f[3] += vSample;
				pPixel->fWeight += 1.0f;
			}

		}
}

// convolve all added samples into the intermediate buffer
void Basis3Texture::UpdateWeights(LUX::RenderChannel *pChannel, SLMapPixel *pData)
{
	if (pChannel->m_target == RenderChannel::Sun)
	{
		for (int i=0; i<m_cX*m_cY; i++)
		{
			if (!CheckPixelUsed(i) || pData[i].fWeight<0.000000001) continue;

			// divide the accumulated result by the weight and add it to the total buffer
			m_pIntermediateBuffer[i].f[3].w += m_pData[i].f[3].y / m_pData[i].fWeight;

			memset(&pData[i], 0, sizeof(SLMapPixel));		// reset accumulation buffer
			pData[i].fWeight = 0;
		}	
	}
	else if (pChannel->m_target == RenderChannel::Sky)
	{
		for (int i=0; i<m_cX*m_cY; i++)
		{
			if (!CheckPixelUsed(i) || pData[i].fWeight<0.000000001) continue;

			// divide the accumulated result by the weight and add it to the total buffer
			if (m_bHemi3v)
			{
				m_pIntermediateBuffer[i].f[3].x += pData[i].f[0].y / pData[i].fWeight;
				m_pIntermediateBuffer[i].f[3].y += pData[i].f[1].y / pData[i].fWeight;
				m_pIntermediateBuffer[i].f[3].z += pData[i].f[2].y / pData[i].fWeight;
			}
			else
			{
				m_pIntermediateBuffer[i].f[3].y += pData[i].f[3].y / pData[i].fWeight;
			}

			memset(&pData[i], 0, sizeof(SLMapPixel));		// reset accumulation buffer
			pData[i].fWeight = 0;
		}	
	}
	else
	{
		for (int i=0; i<m_cX*m_cY; i++)
		{
			if (!CheckPixelUsed(i) || pData[i].fWeight<0.000000001) continue;

			// divide the accumulated result by the weight and add it to the total buffer
			for (int c=0; c<3; c++)
				(float3&)m_pIntermediateBuffer[i].f[c] += (float3&)pData[i].f[c] / pData[i].fWeight;

			memset(&pData[i], 0, sizeof(SLMapPixel));		// reset accumulation buffer
			pData[i].fWeight = 0;
		}	
	}
}

// convolve all added samples into the intermediate buffer
void Basis3Texture::UpdateWeights(LUX::RenderChannel *pChannel)
{
	UpdateWeights(pChannel, m_pData);
}

// convolve all added discontinuity samples into the intermediate buffer (one layer)
void Basis3Texture::Filter(LUX::RenderChannel *pChannel)
{
	Ut::Vector<LUX::RenderChannel*> channels;
	channels.PushBack(pChannel);
	Filter(channels);
	ClearIntermediateBuffer();
}

// convolve all added discontinuity samples into the intermediate buffer (multiple layers)
void Basis3Texture::Filter(const Ut::Vector<LUX::RenderChannel*> &channels)
{
	for (int iChannel=0; iChannel<channels.Size() && iChannel<m_DData.Size(); iChannel++)
	{
		LUX::RenderChannel *pChannel = channels[iChannel];

		UpdateWeights(channels[iChannel], m_DData[iChannel]);
	}
}

// get number of texture layers needed
int	Basis3Texture::GetNumLayers()
{
	return 4;
}

bool Basis3Texture::NeedTangentSpace()
{
	return true;
}

// merge the values of two texels. this function is used by the anti chart seam code
void Basis3Texture::MergeTexels(float2 t1, ITexture *_pOtherTexture, float2 t2)
{
	Basis3Texture *pOtherTexture = (Basis3Texture*)_pOtherTexture;

	// check that both the positions aren in bounds
	if (int(t1.x)<0 || int(t1.x)>=m_cX ||
		int(t1.y)<0 || int(t1.y)>=m_cY ||
		int(t2.x)<0 || int(t2.x)>=pOtherTexture->m_cX ||
		int(t2.y)<0 || int(t2.y)>=pOtherTexture->m_cY)
	{
		assert(0); // shouldn't happen in theory
		return;
	}

	// fetch texels
	SLMapPixel  *p1 = m_pIntermediateBuffer + (m_cX * (int)t1.y + (int)t1.x);
	SLMapPixel  *p2 = pOtherTexture->m_pIntermediateBuffer + (pOtherTexture->m_cX * (int)t2.y + (int)t2.x);

	// average
	SLMapPixel vAvg;
	for (int i=0; i<4; i++)
		vAvg.f[i] = (p1->f[i] + p2->f[i]) * 0.5f;
	*p1 = vAvg;
	*p2 = vAvg;
}
