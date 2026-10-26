#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/AreaLight.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"
#include <lux_common/SerialiseUtVectorToOst.h>

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

AreaLight::Triangle::Triangle(const float3 &v1, const float3 &v2, const float3 &v3, const float3 &vColour)
{
	m_v[0] = v1, m_v[1] = v2, m_v[2] = v3;
	m_vColour = vColour;
	m_vNormal = Normalize(CrossProduct(m_v[1]-m_v[0], m_v[2]-m_v[0]));
	m_vCentre = ( m_v[0] + m_v[1] + m_v[2] ) / 3.0f;
	m_fArea = Length( CrossProduct( m_v[1] - m_v[0] , m_v[2] - m_v[0]));
}

// estimate form factor
float AreaLight::Triangle::EdgeArea(const float3 &p, const float3 &q, const float3 &n) const
{ 
	// Numerically stable edge-area calculator. (Including degenerate cases.)
	// Andrew J. Willmott, 1992

	// 4 dots, 1 cross, 1 atan per edge.

	const float epsilon = 1e-6f;
	float sinFactor, qdotp, projArea;

	qdotp =  DotProduct(p, q);
	sinFactor = p.LengthSquared() * q.LengthSquared() - (qdotp * qdotp);

	projArea = DotProduct(-CrossProduct(p, q), n);

	if (sinFactor > 0)				// If SinFactor > 0 we may apply the formula without problem 
	{
		sinFactor = sqrtf(sinFactor);
		return (float)(projArea * (M_PI * 0.5f - atan(qdotp / sinFactor)) / (2.0f * M_PI * sinFactor));
	}								// If not, we have three remaining cases... 

	else if (qdotp > epsilon) 		// CASE 1: p and q point in the same direction }
		return(0);
	else if (qdotp < -epsilon) 		// CASE 2: Viewpoint lies on the edge }
		return(0.5);
	else
		return(0.125);				// CASE 3: Viewpoint lies at either end of the edge }
}

// Common terminology for this is the polygon-to-point form factor. 
// The 'form-factor' mentioned in radiosity papers is the average
// patch factor across a patch.
float AreaLight::Triangle::PatchFactor(const float3 &p, const float3 &np) const
{
	float3	x1, x2, x3;
	float  sum;

	x1 = m_v[0];
	x1 -= p;
	x2 = m_v[1];
	x2 -= p;
	x3 = m_v[2];
	x3 -= p;
	
	sum =  EdgeArea(x1, x2, np);
	sum += EdgeArea(x2, x3, np);
	sum += EdgeArea(x3, x1, np);

	if (sum > 0.0)
		return(sum);
	else
		return(0.0);
}

IMPLEMENT_PERSISTENCE(AreaLight, "AreaLight");

AreaLight::AreaLight() : Light()
{
	Initialize();	
}

AreaLight::~AreaLight()
{
}

void AreaLight::Initialize()
{
	Light::Initialize();

	m_type = LUX_LIGHT_TYPE_AREA;
}

// initialize an area light using an aftk area light
AreaLight::AreaLight(Aftk::C_SceneAreaLightNode *pAftkLight, Aftk::Matrix44 &mtx) : Light(pAftkLight, mtx)
{
	// create vertices for the area light's quad
	Position3 v[4];
	v[0] = Position3(-1, 1, 0 );
	v[1] = Position3( 1, 1, 0 );
	v[2] = Position3( 1,-1, 0 );
	v[3] = Position3(-1,-1, 0 );

	// transform the area light into world space
	for (int i=0; i<4; i++)
        v[i] = LPVS::ApplyMatrix(mtx, v[i] );

	float3 vColour = (*(float3*)&pAftkLight->GetColour());
	SetIntensity( pAftkLight->GetIntensity() * MAYA_LIGHT_GLOBAL_SCALE );

	// add the triangles
	AddTriangle((float3)v[3],(float3)v[1],(float3)v[0], vColour);
	AddTriangle((float3)v[3],(float3)v[2],(float3)v[1], vColour);
}

// do preprocessing - in this case precalculate the max radius
void AreaLight::PreProcess(Sampler *pSampler)
{
	// calculate area of each triangle
	float	fCurArea, fTotalArea = 0;

	// allocate our sample buffer
	float	x,y,z;
	m_nSamples = 0;

	for (int iTri=0; iTri<m_Triangles.Size(); iTri++)
	{
		fTotalArea += m_Triangles[iTri].m_fArea;

		// calculate largest axis
		float f, fMax;
		fMax = Length(m_Triangles[iTri].m_v[1] - m_Triangles[iTri].m_v[0]);
		f = Length(m_Triangles[iTri].m_v[1] - m_Triangles[iTri].m_v[0]);
		if (f > fMax) fMax = f;
		f = Length(m_Triangles[iTri].m_v[2] - m_Triangles[iTri].m_v[1]);
		if (f > fMax) fMax = f;

		int nSampForThisTri = (int)(0.4 * fMax);
		if (nSampForThisTri < 1) nSampForThisTri = 1;
		if (nSampForThisTri > 16) nSampForThisTri = 16;
		m_nSamples += nSampForThisTri;

		// generate the samples for this triangle
		float2	*buffer = new float2[nSampForThisTri];
		LDShuffleScrambled2D(1, nSampForThisTri, &buffer[0].x);	// 2d position

		for (int iSmp=0; iSmp<nSampForThisTri; iSmp++)
		{
			XSample newSmp;
			newSmp.m_iTriangle = iTri;

			// sample the triangle
			float b1, b2;
			UniformSampleTriangle(buffer[iSmp].x, buffer[iSmp].y, &b1, &b2);

			// Get triangle vertices in _p1_, _p2_, and _p3_
			const float3 &p1 = m_Triangles[iTri].m_v[0];
			const float3 &p2 = m_Triangles[iTri].m_v[1];
			const float3 &p3 = m_Triangles[iTri].m_v[2];
			newSmp.m_vPos = p1 * b1  + p2 * b2 + p3 * (1.f - b1 - b2);

			m_Samples.PushBack( newSmp );
		}
		delete [] buffer;
	}

	m_fColourScale = 5.0f * GetEmittance().y * m_Triangles.Size();
}

Spectrum AreaLight::Sample(const float3 &vPos, const float3 &vNormal, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	*pdf = 1.0f;
	visibility->SetSegment(vPos, m_Samples[iSample].m_vPos);
	*wi = visibility->ray.direction;

	// TODO: optimize this by only calling PatchFactor once per texel rather than once per sample
	float fFalloff = m_fColourScale * m_Triangles[m_Samples[iSample].m_iTriangle].PatchFactor(vPos, vNormal);

	return m_Triangles[ m_Samples[iSample].m_iTriangle ].m_vColour * fFalloff;
}

float3 AreaLight::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	*pdf = 1.0f;
	visibility->SetSegment(vPos, m_Samples[iSample].m_vPos);
	*wi = visibility->ray.direction;

	// TODO: optimize this by only calling PatchFactor once per texel rather than once per sample
	float fFalloff = m_fColourScale * m_Triangles[m_Samples[iSample].m_iTriangle].PatchFactor(vPos, *wi);

	return m_Triangles[ m_Samples[iSample].m_iTriangle ].m_vColour * fFalloff;
}

Spectrum AreaLight::Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const
{
	float fTotalArea = 0;
	for (int i=0; i<m_Triangles.Size(); i++)
		fTotalArea += m_Triangles[i].m_fArea;

	// select triangle to sample based on area
	float fSelect = vSample.z * fTotalArea;
	int iTri;
	for (iTri=0; iTri<m_Triangles.Size(); iTri++)
	{
		fSelect -= m_Triangles[iTri].m_fArea;
		if (fSelect<0)
			break;
	}

	// sample the triangle
	float b1, b2;
	UniformSampleTriangle(vSample.x, vSample.y, &b1, &b2);

	// Get triangle vertices in _p1_, _p2_, and _p3_
	const float3 &p1 = m_Triangles[iTri].m_v[0];
	const float3 &p2 = m_Triangles[iTri].m_v[1];
	const float3 &p3 = m_Triangles[iTri].m_v[2];

	float3 vPos = p1 * b1  + p2 * b2 + p3 * (1.f - b1 - b2);

	pRay->SetRay(vPos, m_Triangles[iTri].m_vNormal);

	*pdf = 1.0f;

	return m_Triangles[iTri].m_vColour * m_fColourScale;
}

float AreaLight::Pdf(const float3 &vPos, const float3 &vwi)
{
	return 1.0;
}

// calculate bounding box for this light's area of influence
box3f AreaLight::GetBBox(const LUX::SpatialHierarchy *pScene) const
{
	box3f bbox;

	for (int i=0; i<m_Triangles.Size(); i++)
	{
		// calc max radius from intensity of this tri
		float fMaxRadius = GetMaxRadius(m_Triangles[i].m_vColour, m_fCutoffThreshold);

		// put bounding box around this tri
		for (int j=0; j<3; j++)
		{
			float3 v = m_Triangles[i].m_v[j];

			bbox.UpdateBox( v - float3(fMaxRadius,fMaxRadius,fMaxRadius) );
			bbox.UpdateBox( v + float3(fMaxRadius,fMaxRadius,fMaxRadius) );
		}
	}

	return bbox;
}

void AreaLight::AddTriangle(const float3 &v1, const float3 &v2, const float3 &v3, const float3 &vColour)
{
	m_Triangles.PushBack(Triangle(v1,v2,v3,vColour));
}

void AreaLight::write(ost::Engine &ar)
{
	ar << m_Triangles;
	Light::write(ar);
}
void AreaLight::read(ost::Engine &ar)
{
	ar >> m_Triangles;
	Light::read(ar);
}
