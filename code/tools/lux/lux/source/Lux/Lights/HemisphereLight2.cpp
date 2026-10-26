// same as hemispherelight, but with built in sun bounce lighting
#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/HemisphereLight2.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"
#include "lux_common/StringManipulation.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(HemisphereLight2, "HemiLight2");

HemisphereLight2::HemisphereLight2() : Light()
{
	m_type = LUX_LIGHT_TYPE_HEMISPHERE2;
	m_pSamples = NULL;
	m_nSamples = 8;
	m_fSunIntensity = 1.0f;
	m_vBounceColour = float3(1,1,1);
}

HemisphereLight2::~HemisphereLight2()
{
	FreeAligned(m_pSamples);
}

void HemisphereLight2::SetXMLParameter(char *pName, char *pValue)
{
	Light::SetXMLParameter(pName, pValue);

	std::stringstream s;
	if (stricmp(pName, "SunDirection") == 0)		// sun bounce direction
	{		
		Vector3F32 vec;
		StringUtility::StringToVector(pValue, vec);
		Normalise(&vec);
		m_vSunDirection[0] = vec[0];
		m_vSunDirection[1] = vec[1];
		m_vSunDirection[2] = vec[2];
	}
	else if (stricmp(pName, "SunIntensity") == 0)	// sun bounce intensity
	{
		float fIntensity;
		s << pValue;
		s >> m_fSunIntensity;
	}
}

void HemisphereLight2::SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize)
{
	m_pScene = pScene;
}

void HemisphereLight2::SetSunDirection(float3 vSunDir)
{
	m_vSunDirection = vSunDir;
}


// returns the lighting intensity for a given direction
float3 HemisphereLight2::Le(const float3 &vDir) const
{
	// samples pointing into to the ground are black
	if (vDir.y<=0)
		return float3(0,0,0);

	// return the same intensity for all angles
	return GetEmittance();
}

// set bounce light colour
void HemisphereLight2::SetBounceColour(float3 vColour)
{
	m_vBounceColour = vColour;
}

// generate our samples
void HemisphereLight2::PreProcess(Sampler *pSampler)
{
	// allocate our sample buffer
	m_pSamples = (float3*)AllocAligned( sizeof(float3) * m_nSamples * 9 );
	
	float2 *buffer = new float2[m_nSamples];
	float x,y,z;

	for (int i=0; i<9; i++)
	{
		// create samples into temporary buffer
		LDShuffleScrambled2D(1, m_nSamples, &buffer[0].x);

		// convert to sampling direction
		for (int iSample=0; iSample<m_nSamples; iSample++)
		{
			ConcentricSampleDisk(buffer[iSample].x, buffer[iSample].y, &x, &y);
			z = sqrtf(max(0.f, 1.f - x*x - y*y));
			m_pSamples[i*m_nSamples + iSample] = float3(x,y,z);
		}

		SortVectorsBySigns(m_pSamples + i*m_nSamples, m_nSamples);
	}

	delete [] buffer;
}


// Sample incoming radiance to a point p. u1 and u2 are 2d sampling positions on the light
// returns: wi is the incoming light direction, pdf is the probability function, visibility ray is in 'visibility'
float3 HemisphereLight2::Sample(const float3 &vPos, const float3 &vNormal,
		int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	// Sample cosine-weighted direction on unit sphere
	float x, y, z;
	float3 n = vNormal;

	assert(iSample<9*m_nSamples);
	*wi = m_pSamples[iSample];

	// Compute _pdf_ for cosine-weighted infinite light direction
	*pdf = fabsf(wi->z) * INV_TWOPI;

	// Transform direction to world space
	float3 v1, v2;
	CoordinateSystem(n, &v1, &v2);
	*wi = float3(v1.x * wi->x + v2.x * wi->y + n.x * wi->z,
	             v1.y * wi->x + v2.y * wi->y + n.y * wi->z,
	             v1.z * wi->x + v2.z * wi->y + n.z * wi->z);

	SD::Ray aoray;
	SD::RayTraceResult result;
	aoray.SetRay(vPos, *wi);
	aoray.bShadowRay = false;
	m_pScene->RayTrace(aoray, result);

	// unoccluded ray - add AO contribution
	if (!result.bHit)
	{
		visibility->ray.length = 0;
		return Le(*wi);
	}

	float fEpsilon = m_pScene->GetEpsilon();

	// calculate lambert shade between sunlight and the hit surface normal
/*
	float f = DotProduct( result.GetSurfaceNormal(), -m_vSunDirection );
	if (f<0)
		return float3(0,0,0);
*/
	float f = 1;

	// occluded ray - calculate bounce
	result.fDistance -= fEpsilon*100;
	if (result.fDistance <= fEpsilon)
		return float3(0,0,0);

	float3 vBounceRayStart = vPos + (*wi) * result.fDistance;

	visibility->SetRay(vBounceRayStart, -m_vSunDirection);
/*
	{
		aoray.SetRay(vBounceRayStart, -m_vSunDirection);
		m_pScene->RayTrace(aoray, result);
		if (!result.bHit)
		{
			AddLine(vBounceRayStart, vBounceRayStart - (m_vSunDirection));
		}
		else
			f = 0;
	}
*/
	return m_vBounceColour * m_fSunIntensity * f;
}

// same as above, but without specifying a normal
float3 HemisphereLight2::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	// generate uniform hemisphere sample
	float3 vSample = UniformSampleSphere(RandomFloat(),RandomFloat());
	// only sample the top hemisphere
	if (vSample.y < 0)
		vSample.y = -vSample.y;

	*wi = vSample;

	// Compute pdf for uniform hemisphere sample
//	*pdf = 1.0f / (4.f * M_PI);
	*pdf = 1.35f / (4.f * M_PI); // fudge to make SH nodes brightness match the textures

	visibility->SetRay(vPos, *wi);
	return Le(*wi);
}

// cast a ray from the light
Spectrum HemisphereLight2::Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const
{
//	assert(0);
	*pdf = 0;
	return float3(0,0,0);
}

float HemisphereLight2::Pdf(const float3 &vPos, const float3 &vwi)
{
	return 0;
}

// serialize/deserialize
void HemisphereLight2::write(ost::Engine &ar)
{
	ar << m_vSunDirection << m_fSunIntensity << m_vBounceColour;
	Light::write(ar);
}
void HemisphereLight2::read(ost::Engine &ar)
{
	ar >> m_vSunDirection >> m_fSunIntensity >> m_vBounceColour;
	Light::read(ar);
}
