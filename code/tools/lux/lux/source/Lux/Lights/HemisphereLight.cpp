#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/HemisphereLight.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(HemisphereLight, "HemiLight");

void HemisphereLight::Initialize()
{
	m_type = LUX_LIGHT_TYPE_HEMISPHERE;
	m_pSamples = NULL;
	m_nSamples = 32;
	m_bHemisphere = true;
}

HemisphereLight::HemisphereLight() : Light()
{
	Initialize();
}

HemisphereLight::HemisphereLight(const Aftk::C_SceneHemisphereLightNode *pAftkLight, Aftk::Matrix44 &mtx) : Light(pAftkLight, mtx)
{
	Initialize();

	SetColour( (*(float3*)&pAftkLight->GetColour()) );
	SetIntensity( pAftkLight->GetIntensity() * MAYA_LIGHT_GLOBAL_SCALE );
}

void HemisphereLight::SetXMLParameter(char *pName, char *pValue)
{
	if (!stricmp(pName, "hemisphere"))
	{
		if (stricmp(pValue, "false") == 0)
			m_bHemisphere = false;
		else
			m_bHemisphere = true;
	}
	else
		Light::SetXMLParameter(pName, pValue);
}

// true = hemisphere, false = full 360 degree sphere
void HemisphereLight::SetHemisphere(bool bOn)
{
	m_bHemisphere = bOn;
}

HemisphereLight::~HemisphereLight()
{
	FreeAligned(m_pSamples);
}

// returns the lighting intensity for a given direction
float3 HemisphereLight::Le(const float3 &vDir) const
{
	// samples pointing into to the ground are black
	if (vDir.y<=0 && m_bHemisphere)
		return float3(0,0,0);

	// return the same intensity for all angles
	return GetEmittance();
}

// generate our samples
void HemisphereLight::PreProcess(Sampler *pSampler)
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
			if (!m_bHemisphere && RandomFloat()<0.5f)
				z = -z;
			m_pSamples[i*m_nSamples + iSample] = float3(x,y,z);
		}

		SortVectorsBySigns(m_pSamples + i*m_nSamples, m_nSamples);
	}

	delete [] buffer;
}

// calculate scene bounding sphere
void HemisphereLight::SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize)
{
	box3f sceneBox = *pScene->GetBoundingBox();

	m_vWorldCentre = sceneBox.vmin * 0.5f + sceneBox.vmax * 0.5f;
	m_fWorldRadius = Length( m_vWorldCentre - sceneBox.vmax );
	m_fWorldRadius *= 1.01f;

	fMeterSize *= 100.0f;

	m_fDensity = 100.0f / (fMeterSize * fMeterSize);
}


// Sample incoming radiance to a point p. u1 and u2 are 2d sampling positions on the light
// returns: wi is the incoming light direction, pdf is the probability function, visibility ray is in 'visibility'
float3 HemisphereLight::Sample(const float3 &vPos, const float3 &vNormal,
		int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	// Sample cosine-weighted direction on unit sphere
	float x, y, z;
	float3 n = vNormal;

	assert(iSample<9*m_nSamples);
	*wi = m_pSamples[iSample];

	// Compute pdf for cosine-weighted infinite light direction
	*pdf = fabsf(wi->z) * INV_TWOPI;

	// Transform direction to world space
	float3 v1, v2;
	CoordinateSystem(n, &v1, &v2);
	*wi = float3(v1.x * wi->x + v2.x * wi->y + n.x * wi->z,
	             v1.y * wi->x + v2.y * wi->y + n.y * wi->z,
	             v1.z * wi->x + v2.z * wi->y + n.z * wi->z);

	visibility->SetRay(vPos, *wi);
	return Le(*wi);
}

// same as above, but without specifying a normal
float3 HemisphereLight::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	// generate uniform hemisphere sample
	float3 vSample = UniformSampleSphere(RandomFloat(),RandomFloat());
	// only sample the top hemisphere
	if (m_bHemisphere && vSample.y < 0)
		vSample.y = -vSample.y;

	*wi = vSample;

	// Compute pdf for uniform hemisphere sample
	*pdf = 1.0f / (4.f * M_PI);

	visibility->SetRay(vPos, *wi);
	return Le(*wi);
}

// cast a ray from the light
Spectrum HemisphereLight::Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const
{
	// Choose two points p1 and p2 on scene bounding sphere
	float3 p1 = m_vWorldCentre + UniformSampleSphere(vSample.x, vSample.y) * m_fWorldRadius;
	if (p1.y<0) p1.y=-p1.y;
	float3 p2 = m_vWorldCentre + UniformSampleSphere(vSample.z, vSample.w) * m_fWorldRadius;

	// Construct ray between p1 and p2
	pRay->SetRay(p1, Normalize(p2-p1));

	*pdf = 1.0f;

	return float3(1,1,1);
}

float HemisphereLight::Pdf(const float3 &vPos, const float3 &vwi)
{
	return 0;
}

// serialize/deserialize
void HemisphereLight::write(ost::Engine &ar)
{
	ar << m_bHemisphere;

	Light::write(ar);
}
void HemisphereLight::read(ost::Engine &ar)
{
	ar >> m_bHemisphere;

	Light::read(ar);
}

// the combined power of the whole light
Spectrum HemisphereLight::Power()
{
	return float3(1,1,1) * GetNumLightPaths() / m_fDensity;
}

// get number of light paths, which relates to the size of the world
float HemisphereLight::GetNumLightPaths() const
{
	return m_fWorldRadius * m_fWorldRadius * m_fDensity;
}

