#include "stdafx.h"
#include "Lux/Rasterize.h"
#include "Lux/RasterizedTriIterator.h"
#include "Lux/TextureTypes/ITexture.h"
#include "Lux/Sampler.h"
#include "Lux/ImageFilter.h"
#include "Lux/RenderChannel.h"
#include "Lux/2DGeometryMaths.h"

using namespace LUX;

//#define USE_PASS2_TO_SKIP_TEXELS

// copy rasterization info into our structure
void RasterizedTriIterator::SetInfo(RasterizedTriInfo *pInfo)
{
	memcpy(&this->m_b0, pInfo, sizeof(int) * 56);
	if (m_nEdges>0)
		m_pEdges = &m_pInput->m_Edges[m_iRasterizedEdge];
	else
		m_pEdges = NULL;
}

// give us a list of triangles to rasterize
void RasterizedTriIterator::SetTriList(TexelIteratorInput *pInput)
{
	m_pInput = pInput;
	m_nTris = pInput->m_Tris.Size();
	m_pTex = pInput->m_pTex;
}


//---functions for iterating through a pre-rasterized polygon
void RasterizedTriIterator::End(RenderChannel *pChannel)
{
	if (m_mode != DiscontinuitySampling)
		m_pTex->UpdateWeights(pChannel);
	else
		m_pTex->Filter(pChannel);
}

// end a multi-channel light render
void RasterizedTriIterator::End(const Ut::Vector<RenderChannel*> &channels)
{
	if (m_mode != DiscontinuitySampling)
	{
		assert(0); // not supported yet
	}
	else
		m_pTex->Filter(channels);
}

// load next triangle
void RasterizedTriIterator::GetNextTri()
{
	SetInfo(&m_pInput->m_Tris[m_iCurTri]);

	m_pTex->SetWindow(	m_pInput->m_Tris[m_iCurTri].m_x,
						m_pInput->m_Tris[m_iCurTri].m_y,
						m_pInput->m_Tris[m_iCurTri].m_cx,
						m_pInput->m_Tris[m_iCurTri].m_cy);
}

bool RasterizedTriIterator::ClampToEdge(float3 vPlane, float2 &s)
{
	float fDotS = vPlane.x * s.x + vPlane.y * s.y;

	// clamp fDotS to fDotP if needed
	if (fDotS>vPlane.z)
	{
		float f = fDotS-vPlane.z;
		s.x = s.x - vPlane.x * f;
		s.y = s.y - vPlane.y * f;
		return true;
	}
	return false;
}

void RasterizedTriIterator::RememberSample()
{
	// store sample position in circular buffer
	m_OldSampleList[m_iOldSampleIn].x = m_iX;
	m_OldSampleList[m_iOldSampleIn].y = m_iRealY;
	m_OldSampleList[m_iOldSampleIn].vt = m_vCurUV;
	m_OldSampleList[m_iOldSampleIn].iTri = m_iCurTri;
	m_OldSampleList[m_iOldSampleIn].mtxTangentSpace = m_TangentMtx;
	if (++m_iOldSampleIn >= MAX_OLD_SAMPLES) m_iOldSampleIn = 0;
	assert(m_iOldSampleIn != m_iOldSampleOut);
}

#define NEW_METHOD

bool RasterizedTriIterator::CalcSample()
{
#ifdef NEW_METHOD
	// calculate current tu,tv position
	float2 vt = float2(m_iX, m_iRealY);
	if ((m_mode != DiscontinuitySampling && m_mode != SampleFirstNoAA && m_mode != DiscontinuitySampling2))
		vt = vt + m_pSampler->GetImageSample(m_iSample);// - float2(0.5f,0.5f); // I THINK IN THEORY THIS SHOULDN'T BE THERE NOT 100% SURE
	else
		vt = vt + float2(0.5f,0.5f);
#else
	// calculate current tu,tv position
	float2 vt = float2(m_iX, m_iRealY);
	if ((m_mode != DiscontinuitySampling && m_mode != SampleFirstNoAA && m_mode != DiscontinuitySampling2)
		&& m_nSamples>1)
		vt = vt + m_pSampler->GetImageSample(m_iSample);// - float2(0.5f,0.5f); // I THINK IN THEORY THIS SHOULDN'T BE THERE NOT 100% SURE
#endif

	m_vCurUV = vt;
	float sX = vt.x, sY = vt.y;

	// calculate barycentric coordinates
	m_b.x = ((m_x2-sX) * (m_y3-sY) - (m_x3-sX) * (m_y2-sY)) / m_b0;
	m_b.y = ((m_x3-sX) * (m_y1-sY) - (m_x1-sX) * (m_y3-sY)) / m_b0;
	m_b.z = ((m_x1-sX) * (m_y2-sY) - (m_x2-sX) * (m_y1-sY)) / m_b0;

	return true;
}

bool RasterizedTriIterator::CalcAntiLLSample(float2 vt)
{
	m_vCurUV = vt;
	float sX = vt.x, sY = vt.y;

	// calculate barycentric coordinates
	m_b.x = ((m_x2-sX) * (m_y3-sY) - (m_x3-sX) * (m_y2-sY)) / m_b0;
	m_b.y = ((m_x3-sX) * (m_y1-sY) - (m_x1-sX) * (m_y3-sY)) / m_b0;
	m_b.z = ((m_x1-sX) * (m_y2-sY) - (m_x2-sX) * (m_y1-sY)) / m_b0;

	return true;
}

bool RasterizedTriIterator::ValidateSample()
{
	if (m_bAntiLightLeaking)
	{
#define ANTILL_EPSILON (0.00001f)

	// discard any samples with barycentric coordinates outside of the triangle
	// (FIXME: maybe this should this check whether the samples is outside all other triangles in the chart as well?)
	if (m_b.x<ANTILL_EPSILON || m_b.y<ANTILL_EPSILON || m_b.z<ANTILL_EPSILON)
		return false;
	}

	return true;
}

// calculate a valid sample, which is guaranteed to be both on the current texel and on the current chart
// the sample could be on a different triangle in this chart. remember to set the current triangle back one the sample
// has been calculated
bool RasterizedTriIterator::CalcValidSample()
{
	assert(m_iCurTri < m_nTris);
	SetInfo(&m_pInput->m_Tris[m_iCurTri]);

	if (!CalcSample())
		return false;

	if (!m_bAntiLightLeaking)
		return true;

	if (ValidateSample())
	{
		if (m_bNeedTangentSpace)
			GetTangentSpace();

		return true;
	}

	float2 vOrgUV = float2(m_iX, m_iRealY);

#define TRY_ALL_TRIANGLES

	// for each triangle
	int iTri = m_iCurTri;
	while (1)
	{
		float2 v;
		RasterizedTriInfo *pOut = &m_pInput->m_Tris[iTri];

		// calculate bounding box for this triangle
		// TODO: this is the slowest bit of the entire function. optimize it by using SSE min/max
		float2 vMin(pOut->m_x1,pOut->m_y1), vMax(pOut->m_x1, pOut->m_y1);
		if (pOut->m_x2 < vMin.x) vMin.x = pOut->m_x2;
		if (pOut->m_x3 < vMin.x) vMin.x = pOut->m_x3;
		if (pOut->m_y2 < vMin.y) vMin.y = pOut->m_y2;
		if (pOut->m_y3 < vMin.y) vMin.y = pOut->m_y3;

		if (pOut->m_x2 > vMax.x) vMax.x = pOut->m_x2;
		if (pOut->m_x3 > vMax.x) vMax.x = pOut->m_x3;
		if (pOut->m_y2 > vMax.y) vMax.y = pOut->m_y2;
		if (pOut->m_y3 > vMax.y) vMax.y = pOut->m_y3;

		// clamp against the bounding box of this texel
		float2 vMinCurUV = vOrgUV - float2(0.0f,0.0f);
		float2 vMaxCurUV = vOrgUV + float2(1.0f,1.0f);
		if (vMinCurUV.x > vMin.x) vMin.x = vMinCurUV.x;
		if (vMinCurUV.y > vMin.y) vMin.y = vMinCurUV.y;
		if (vMaxCurUV.x < vMax.x) vMax.x = vMaxCurUV.x;
		if (vMaxCurUV.y < vMax.y) vMax.y = vMaxCurUV.y;

		// they don't overlap
		if (vMin.x >= vMax.x || vMin.y >= vMax.y)
		{
#ifdef TRY_ALL_TRIANGLES
			goto nextTri;
#else
			return false;
#endif
		}

		{
		// create a polygon structure from the triangle
		Geom2D::Polygon poly(	float2(pOut->m_x1, pOut->m_y1),
								float2(pOut->m_x2, pOut->m_y2),
								float2(pOut->m_x3, pOut->m_y3));

		// clip the polygon against the texel area
		poly.Clip(-vMinCurUV.x, float2(-1.f,0));
		poly.Clip(vMaxCurUV.x, float2(1.f,0));
		poly.Clip(-vMinCurUV.y, float2(0,-1.f));
		poly.Clip(vMaxCurUV.y, float2(0,1.f));
		if (poly.nVerts == 0)
			goto nextTri;

		// generate sample points within this bounding box
#ifdef TRY_ALL_TRIANGLES
		assert(iTri < m_nTris);
		memcpy(&this->m_b0, &m_pInput->m_Tris[iTri].m_b0, sizeof(int) * 56);
#endif

		// try one of the vertices of the clipped polygon
		float2 vCentroid(0,0);
		for (int i=0; i<poly.nVerts; i++)
		{
			vCentroid = vCentroid + poly.vert[i];
			if (CalcAntiLLSample(poly.vert[i]) && ValidateSample())
			{
				if (m_bNeedTangentSpace)
					GetTangentSpace();

				return true;
			}
		}

		// try the centroid
		vCentroid = (vCentroid * (1.0f/(float)poly.nVerts));
		if (CalcAntiLLSample(vCentroid) && ValidateSample())
		{
			if (m_bNeedTangentSpace)
				GetTangentSpace();

			return true;
		}

		}

nextTri:

#ifdef TRY_ALL_TRIANGLES
		iTri++;
		if (iTri >= m_nTris) iTri = 0;
		if (iTri == m_iCurTri)
		{
			return false;
		}
#else
		SetInfo(&m_pInput->m_Tris[m_iCurTri]);
		return false;
#endif
	}
}

bool RasterizedTriIterator::GetSample()
{
	if (!CalcValidSample())
		return false;

	RememberSample();

	return true;
}
//---

// use barycentric coordinates to interpolate the world postion from the current tu/tv
float3 RasterizedTriIterator::GetWorldPosition()
{
	// TODO: optimize this so it caches the result when it gets called for different samples for the same position
	float3 vOut;

	vOut.x = (m_vPos[0].x * m_b.x) + (m_vPos[1].x * m_b.y) + (m_vPos[2].x * m_b.z);
	vOut.y = (m_vPos[0].y * m_b.x) + (m_vPos[1].y * m_b.y) + (m_vPos[2].y * m_b.z);
	vOut.z = (m_vPos[0].z * m_b.x) + (m_vPos[1].z * m_b.y) + (m_vPos[2].z * m_b.z);

	return vOut;
}
// use barycentric coordinates to interpolate the normal from the current tu/tv
float3 RasterizedTriIterator::GetNormal()
{
	// TODO: optimize this so it caches the result when it gets called for different samples for the same position
	float3 vOut;

	vOut.x = (m_vNormal[0].x * m_b.x) + (m_vNormal[1].x * m_b.y) + (m_vNormal[2].x * m_b.z);
	vOut.y = (m_vNormal[0].y * m_b.x) + (m_vNormal[1].y * m_b.y) + (m_vNormal[2].y * m_b.z);
	vOut.z = (m_vNormal[0].z * m_b.x) + (m_vNormal[1].z * m_b.y) + (m_vNormal[2].z * m_b.z);

	vOut.Normalize();

	return vOut;
}

// interpolate tangent space from barycentric coordinates
void RasterizedTriIterator::GetTangentSpace()
{
	HVec3 vTangent, vBinormal;

	vTangent.x = (m_vTangent[0].x * m_b.x) + (m_vTangent[1].x * m_b.y) + (m_vTangent[2].x * m_b.z);
	vTangent.y = (m_vTangent[0].y * m_b.x) + (m_vTangent[1].y * m_b.y) + (m_vTangent[2].y * m_b.z);
	vTangent.z = (m_vTangent[0].z * m_b.x) + (m_vTangent[1].z * m_b.y) + (m_vTangent[2].z * m_b.z);

	vBinormal.x = (m_vBinormal[0].x * m_b.x) + (m_vBinormal[1].x * m_b.y) + (m_vBinormal[2].x * m_b.z);
	vBinormal.y = (m_vBinormal[0].y * m_b.x) + (m_vBinormal[1].y * m_b.y) + (m_vBinormal[2].y * m_b.z);
	vBinormal.z = (m_vBinormal[0].z * m_b.x) + (m_vBinormal[1].z * m_b.y) + (m_vBinormal[2].z * m_b.z);

	m_TangentMtx = HMat33(vTangent, vBinormal, GetNormal());
	m_TangentMtx = m_TangentMtx.Transposed();
}

// add a sample to the texture
void RasterizedTriIterator::AddSample(const LightSample &sample)
{
	assert(m_pTex);

	// get sample position from circular buffer
	OldSample *p = &m_OldSampleList[m_iOldSampleOut];
	if (++m_iOldSampleOut >= MAX_OLD_SAMPLES) m_iOldSampleOut  = 0;

	switch (m_mode)
	{
	case SampleFirst:
	case TexelFirst:
	case SampleFirstNoAA:
		m_pTex->AddSample(p->vt, sample.vColour, sample.vDir, p->mtxTangentSpace);
		break;
	case DiscontinuitySampling:
	case DiscontinuitySampling2:
		m_pTex->AddDiscontinuitySample(p->x, p->y, sample.vColour, sample.vDir, p->mtxTangentSpace);
		break;
	}
}

// add a multi-channel sample to the texture
void RasterizedTriIterator::AddSample(const Ut::Vector<LightSample> &samples)
{
	assert(m_pTex);

	// get sample position from circular buffer
	OldSample *p = &m_OldSampleList[m_iOldSampleOut];
	if (++m_iOldSampleOut >= MAX_OLD_SAMPLES) m_iOldSampleOut  = 0;

	Ut::Vector<float3> _samples;
	_samples.Resize(samples.Size());
	for (int i=0; i<samples.Size(); i++)
		_samples[i] = samples[i].vColour;

	switch (m_mode)
	{
	case SampleFirst:
	case TexelFirst:
	case SampleFirstNoAA:
		assert(0); // not supported yet
		break;
	case DiscontinuitySampling:
	case DiscontinuitySampling2:
		m_pTex->AddDiscontinuitySample(p->x, p->y, _samples, samples[0].vDir, p->mtxTangentSpace);
		break;
	}
}

void RasterizedTriIterator::SetNeedTangentSpace(bool bOn)
{
	m_bNeedTangentSpace = bOn;
}

//---functions for iterating through a pre-rasterized polygon
void RasterizedTriIterator::Go(t_mode mode, int nSamples, SurfaceIteratorCallbackPtr pCallback, void *pContext)
{
	m_nSamples	= nSamples;
	m_pCallback = pCallback;
	m_pContext	= pContext;
	m_mode		= mode;
	m_iOldSampleIn = 0;
	m_iOldSampleOut = 0;

	//[Ken] If seems we can get here with no triangles to process. Abort in this case.
	if (m_nTris == 0)
		return;

	switch (mode)
	{
	case SampleFirst:
	case SampleFirstNoAA:
		GoSampleFirst();
		break;
	case DiscontinuitySampling:
		GoDiscontinuitySampling();
		break;
	}
}

// iterate sample-first through a pre-rasterized polygon
void RasterizedTriIterator::GoSampleFirst()
{
	// loop through triangles
	for (m_iCurTri=0; m_iCurTri<m_nTris; m_iCurTri++)
	{
		GetNextTri();

		// loop through edges
		for (m_iY=0; m_iY<m_nEdges; m_iY++)
		{
			m_iRealY = m_iY + m_iFirstEdgeY;

			// loop through texels
			int top = m_pEdges[m_iY].x2;
			for (m_iX=m_pEdges[m_iY].x1; m_iX<=top; m_iX++)
			{
				// loop through samples
				for (m_iSample=0; m_iSample<m_nSamples; m_iSample++)
				{
					// calculate world pos + normal
					if (!GetSample())
					{
						SetInfo(&m_pInput->m_Tris[m_iCurTri]);
						continue;
					}

					float3 vPos = GetWorldPosition();
					float3 vNormal = GetNormal();

					(*m_pCallback)(m_pContext, vPos, vNormal, m_iSample);

					SetInfo(&m_pInput->m_Tris[m_iCurTri]);
				}            
			}
		}
	}
}

// iterate through a pre-rasterized polygon using discontinuity sampling
void RasterizedTriIterator::GoDiscontinuitySampling()
{
	// loop through triangles
	for (m_iCurTri=0; m_iCurTri<m_nTris; m_iCurTri++)
	{
		GetNextTri();

		// loop through edges
		for (m_iY=0; m_iY<m_nEdges; m_iY++)
		{
			m_iRealY = m_iY + m_iFirstEdgeY;

			m_iX=m_pEdges[m_iY].x1;
			int top = m_pEdges[m_iY].x2;

			int iSubPixelX = (m_iX%3);
			int iSubPixelY = 3*((m_iRealY)%3);

			// loop through texels
			for (; m_iX<=top; m_iX++)
			{
				// calculate world pos + normal
				if (CalcValidSample())
				{
					float3 vPos = GetWorldPosition();
					float3 vNormal = GetNormal();
					int iSampleOffset = (iSubPixelX + iSubPixelY) * m_nSamples;
					int iLastSample = iSampleOffset + m_nSamples;
					SetInfo(&m_pInput->m_Tris[m_iCurTri]);

					// loop through samples
					m_iSample = iSampleOffset;
					for (m_iSample = iSampleOffset; m_iSample<iLastSample; m_iSample++)
					{
						RememberSample();
						(*m_pCallback)(m_pContext, vPos, vNormal, m_iSample);
					}
				}
				else
					SetInfo(&m_pInput->m_Tris[m_iCurTri]);

				if (++iSubPixelX >= 3)
					iSubPixelX = 0;
			}
		}
	}
}
