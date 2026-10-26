//-----------------------------------------------------------------------------
// File: BounceTexture.cpp
//
// Desc: Special texture designed to store the result of a bounce-pass
//-----------------------------------------------------------------------------

#include <stdafx.h>

#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/TextureTypes/DefaultTexture.h"
#include "Lux/TextureTypes/BounceTexture.h"
#include "Lux/ImageFilter.h"
#include "Lux/Sampler.h"
#include "Lux/RenderChannel.h"

#pragma intrinsic(fabs)

using namespace LUX;

// export RGBA values 
U32 *BounceTexture::GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool bAlphaScaling, bool bSwizzleSH)
{
	return DefaultTexture::GenerateRGBAData(iLayer, 1.0f, 1.0f, false, false);
}

// resolve this pass to intermediate texture
void BounceTexture::UpdateWeights(RenderChannel *pChannel, int iChannel)
{
	SLMapPixel *pData = m_DData[iChannel];

	switch (pChannel->m_target)
	{
	default:
		for (int i=0; i<m_cX*m_cY; i++)
		{
			if (!CheckPixelUsed(i) || pData[i].fWeight<0.000000001) continue;

			*(float3*)(m_pIntermediateBuffer+i) += pData[i].vColour / pData[i].fWeight;
		}
		break;
	case RenderChannel::Sun:
	case RenderChannel::Sky:
		for (int i=0; i<m_cX*m_cY; i++)
		{
			if (!CheckPixelUsed(i) || pData[i].fWeight<0.000000001) continue;

			float3 v = pData[i].vColour;
			m_pIntermediateBuffer[i].w += (v.x + v.y + v.z) / (pData[i].fWeight * 3);
		}
		break;
	}
}

// resolve buffer to channel
void BounceTexture::UpdateWeights(RenderChannel *pChannel)
{
	if (m_DData.Size() == 0)
		return;

	UpdateWeights(pChannel, 0);

	ClearChannels();
}

// resolve discontinuity buffer to channel
void BounceTexture::Filter(LUX::RenderChannel *pChannel)
{
	if (m_DData.Size() == 0)
		return;

	UpdateWeights(pChannel, 0);

	ClearChannels();
}

// resolve buffer to multiple channels
void BounceTexture::Filter(const Ut::Vector<LUX::RenderChannel*> &channels)
{
	if (m_DData.Size() == 0)
		return;

	for (int i=0; i<channels.Size(); i++)
		UpdateWeights(channels[i], i);

	ClearChannels();
}

// add a sample to this texture using a given filter
void BounceTexture::AddSample(float2 t, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent)
{
	InitializeChannels(1);

	static int ifx[16];
	static int ify[16];
	int x0, x1, y0, y1;

	PrecalcSampleOffsets(t, m_pFilter, x0, y0, x1, y1, ifx, ify);

	float *pfTable = m_pFilter->GetTable();
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x)
		{
			// Evaluate filter value at (x,y) pixel
			int offset = ify[y-y0]*FILTER_TABLE_SIZE + ifx[x-x0];
			float filterWt = pfTable[offset];
			// Update pixel values with filtered sample contribution
			SLMapPixel *pPixel = &GetPixel(0, x, y);
			pPixel->vColour += vSample * filterWt;
			pPixel->fWeight += filterWt;
		}
}

// add a sample to the dicontinuity buffer
void BounceTexture::AddDiscontinuitySample(int iX, int iY, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent)
{
	InitializeChannels(1);

	int x0 = iX-1;
	int x1 = iX+1;
	int y0 = iY-1;
	int y1 = iY+1;

	x0 = max(x0, m_window.x);
	x1 = min(x1, m_window.x + m_window.cx -1);
	y0 = max(y0, m_window.y);
	y1 = min(y1, m_window.y + m_window.cy -1);

	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			// Evaluate filter value at (x,y) pixel
			float filterWt = 1.0f;
			// Update pixel values with filtered sample contribution
			SLMapPixel *pPixel = &GetPixel(0, x, y);
			pPixel->vColour += vSample * filterWt;
			pPixel->fWeight += filterWt;
		}
	}
}

// add a sample to the dicontinuity buffer - multi channel
void BounceTexture::AddDiscontinuitySample(int iX, int iY, const Ut::Vector<float3> &samples, const float3 &vNormal, const HMat33 &mtxTangent)
{
	InitializeChannels(samples.Size());

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
				// Evaluate filter value at (x,y) pixel
				float filterWt = 1.0f;
				// Update pixel values with filtered sample contribution
				SLMapPixel *pPixel = &GetPixel(iChannel, x, y);
				pPixel->vColour += samples[iChannel] * filterWt;
				pPixel->fWeight += filterWt;
			}
		}
	}
}

// ensure _n_ channels
void BounceTexture::InitializeChannels(int nChannels)
{
	for (int i=m_DData.Size(); i<nChannels; i++)
	{
		m_DData.PushBack( new SLMapPixel[m_cX * m_cY] );
		memset(m_DData.Back(), 0, sizeof(SLMapPixel) * m_cX * m_cY);
	}
}

// free all channels
void BounceTexture::ClearChannels()
{
	for (int i=0; i<m_DData.Size(); i++)
		delete [] m_DData[i];

	m_DData.Destroy();
}
