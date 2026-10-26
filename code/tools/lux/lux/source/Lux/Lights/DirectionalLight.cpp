#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"
#include "lux_common/StringManipulation.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(DirectionalLight, "DirLight");

DirectionalLight::DirectionalLight() : Light()
{
	Initialize();
}

DirectionalLight::~DirectionalLight()
{
	if (m_pSamples)
		FreeAligned(m_pSamples);
}

DirectionalLight::DirectionalLight(Aftk::C_SceneDirectionalLightNode *pLight, Aftk::Matrix44 &mtx) : Light(pLight, mtx)
{
	Initialize();

	SetColour( (*(float3*)&pLight->GetColour()) );
	SetIntensity( pLight->GetIntensity() );

	// get direction from world matrix
	Normal3 vDir = Maths::Constants<Normal3>::Z();
	m_vDirection = LPVS::ApplyMatrix( mtx, vDir );

	// this code does not make sense in the context of having a separate directional occlusion channel
//	float f = pAftkLight->GetIntensity();
//	pDirLight->m_vIntensity = (*(float3*)&pAftkLight->GetColour()) * f;
	// so don't set it - leave it to white
}

void DirectionalLight::Initialize()
{
	m_type = LUX_LIGHT_TYPE_DIRECTIONAL;

	m_vDirection[0] = 0;
	m_vDirection[1] = -1;
	m_vDirection[2] = 0;

	m_fDensity = 100.0f;
	m_fPenumbra = 1.0f;
	m_pSamples = NULL;
	m_bLambert = true;
}

void DirectionalLight::SetXMLParameter(char *pName, char *pValue)
{
	Light::SetXMLParameter(pName, pValue);

	if (stricmp(pName, "Direction") == 0)		// set direction
	{
		// get attribute type
		Vector3F32 vec;
		StringUtility::StringToVector(pValue, vec);
		Normalise(&vec);
		m_vDirection[0] = -vec[0];
		m_vDirection[1] = -vec[1];
		m_vDirection[2] = -vec[2];
	}
	else if (stricmp(pName, "penumbra") == 0)	// set penumbra
	{
		// get intensity
		float f;
		std::stringstream ss;
		ss << pValue;
		ss >> f;
		m_fPenumbra = f;
	}
	else if (stricmp(pName, "lambert") == 0)	// set lambert shading on/off
	{
		if (stricmp(pValue, "Off") == 0 || stricmp(pValue, "No") == 0)
		{
			m_bLambert = false;
		}
	}
}

// get scene area projection onto directional light plane
void DirectionalLight::SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize)
{
	box3f sceneBox = *pScene->GetBoundingBox();

	float3 v1, v2;
	CoordinateSystem(m_vDirection, &v1, &v2);

	float3 vProj;

	// project the scene bounding box into the plane of the directional light
	for (int i=0; i<8; i++)
	{
		float3 v;
		v.x = (i&1) ? sceneBox.vmin.x : sceneBox.vmax.x;
		v.y = (i&2) ? sceneBox.vmin.y : sceneBox.vmax.y;
		v.z = (i&4) ? sceneBox.vmin.z : sceneBox.vmax.z;

		vProj.x = DotProduct(v, v1);
		vProj.y = DotProduct(v, v2);
		vProj.z = DotProduct(v, m_vDirection);

		m_AreaBox.UpdateBox(vProj);
	}

	fMeterSize *= 100.0f;

	m_fDensity = 100.0f / (fMeterSize * fMeterSize);
}

// the power per path is constant
Spectrum DirectionalLight::Power()
{
	return float3(1,1,1) * GetNumLightPaths() / m_fDensity;
}

// calculate number of light paths needed from the area of the scene
float DirectionalLight::GetNumLightPaths() const
{
	float3 vSize = m_AreaBox.GetSize();
	float  fArea = vSize.x * vSize.z;

    return fArea * m_fDensity;
}

// generate our jittered direction samples
void DirectionalLight::PreProcess(Sampler *pSampler)
{
	if (m_vDirection.x == 0) m_vDirection.x = 0;
	if (m_vDirection.y == 0) m_vDirection.y = 0;
	if (m_vDirection.z == 0) m_vDirection.z = 0;

	if (m_nSamples < pSampler->m_nSamplesPerTexel)
		m_nSamples = pSampler->m_nSamplesPerTexel;

	// allocate our sample buffer
	m_pSamples = (float3*)AllocAligned( sizeof(float3) * m_nSamples );
	
	float2 *buffer = new float2[m_nSamples];
	float x,y,z;

	// create samples into temporary buffer
	LDShuffleScrambled2D(1, m_nSamples, &buffer[0].x);

	// convert to sampling direction
	for (int iSample=0; iSample<m_nSamples; iSample++)
	{
		// jitter the sample using the penumbra
		float3 vDir = UniformSampleCone(buffer[iSample], m_fPenumbra);

		// Transform to world space
		float3 v1, v2;
		CoordinateSystem(m_vDirection, &v1, &v2);
		vDir = float3(	v1.x * vDir.x + v2.x * vDir.y + m_vDirection.x * vDir.z,
						v1.y * vDir.x + v2.y * vDir.y + m_vDirection.y * vDir.z,
						v1.z * vDir.x + v2.z * vDir.y + m_vDirection.z * vDir.z);

		m_pSamples[iSample] = vDir;
	}

	delete [] buffer;
}

// Sample incoming radiance to a point p. u1 and u2 are 2d sampling positions on the light
// returns: wi is the incoming light direction, pdf is the probability function, visibility ray is in 'visibility'
Spectrum DirectionalLight::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	Spectrum spec;
	float3 vDir = m_pSamples[iSample];

	*pdf = 1.0f;
	*wi = vDir;
	visibility->SetRay(vPos, vDir);

	spec = float3(1,1,1);

	return GetEmittance();
}

Spectrum DirectionalLight::Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const
{
	float3 vPos;
	
	vPos.x = m_AreaBox.vmin.x + (m_AreaBox.vmax.x - m_AreaBox.vmin.x) * vSample.x;
	vPos.y = m_AreaBox.vmin.y + (m_AreaBox.vmax.y - m_AreaBox.vmin.y) * vSample.y;
	vPos.z = m_AreaBox.vmax.z;

	// Transform to world space
	float3 v1, v2;
	CoordinateSystem(m_vDirection, &v1, &v2);
	vPos = float3(v1.x * vPos.x + v2.x * vPos.y + m_vDirection.x * vPos.z,
	              v1.y * vPos.x + v2.y * vPos.y + m_vDirection.y * vPos.z,
	              v1.z * vPos.x + v2.z * vPos.y + m_vDirection.z * vPos.z);

	*pdf = 1.0f;
	pRay->SetRay(vPos, -m_vDirection);

	return GetEmittance();
}

float DirectionalLight::Pdf(const float3 &vPos, const float3 &vwi)
{
	return 0;
}

void DirectionalLight::write(ost::Engine &ar)
{
	ar << m_vDirection << m_fPenumbra << m_bLambert;
	Light::write(ar);
}
void DirectionalLight::read(ost::Engine &ar)
{
	ar >> m_vDirection >> m_fPenumbra >> m_bLambert;
	m_pSamples = NULL;
	Light::read(ar);
}

bool DirectionalLight::GetLambert() const
{
	return m_bLambert;
}
