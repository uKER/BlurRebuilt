#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/SpotLight.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(SpotLight, "SpotLight");

SpotLight::SpotLight() : Light()
{
	Initialize();
}

SpotLight::~SpotLight()
{
}

SpotLight::SpotLight(const Aftk::C_SceneSpotLightNode *pAftkLight, Aftk::Matrix44 &mtx) : Light(pAftkLight, mtx)
{
	Initialize();

	float f, fFalloffStart, fTotalAngle;

	SetColour( *(float3*)&pAftkLight->GetColour() );
	SetIntensity( pAftkLight->GetIntensity() * MAYA_LIGHT_GLOBAL_SCALE );

	// get cone angles from Aftk
	// handle the case of the penumbra angle being negative
	f = pAftkLight->GetPenumbraAngle();
	if (f<0)
		f = 0;

	fFalloffStart = pAftkLight->GetConeAngle()*0.5f-f;
	fTotalAngle   = fFalloffStart+f*2;
	if (fFalloffStart<0) fFalloffStart=0;

	m_fCosFalloffStart = cosf(fFalloffStart);
	m_fCosTotalWidth = cosf(fTotalAngle);

	// protect against falloff being absolute 0 because it can generate #IND
	if (m_fCosFalloffStart - m_fCosTotalWidth < 0.0001f)
		m_fCosTotalWidth = m_fCosFalloffStart + 0.0001f;

	// get decay type
	m_iDecayType = pAftkLight->GetDecayType();
	if (m_iDecayType == Aftk::E_LightDecay_None)
		S_WARN( g_Log, "Node '" << pAftkLight->GetPath() << "' has decay type set to 'none'\n" );

	m_type = LUX_LIGHT_TYPE_SPOTLIGHT;
	m_iDecayType = Aftk::E_LightDecay_Quadratic;

	// get world matrix
	m_worldMatrix = LPVS::ConvertMatrix(mtx);
	m_invWorldMatrix = m_worldMatrix.Inverse();
}

void SpotLight::Initialize()
{
	m_type = LUX_LIGHT_TYPE_SPOTLIGHT;
	m_iDecayType = Aftk::E_LightDecay_Quadratic;
}

// calculate falloff for a given direction vector
float SpotLight::FallOff(const float3 &vDir) const
{
	float3 v = vDir * (HMat33)m_invWorldMatrix;
	v.Normalize();

	float fCosTheta = -v.z;
	if (fCosTheta < m_fCosTotalWidth)
		return 0.;
 	if (fCosTheta > m_fCosFalloffStart)
		return 1.;

	// Compute falloff inside spotlight cone
	float delta = (fCosTheta - m_fCosTotalWidth) / (m_fCosFalloffStart - m_fCosTotalWidth);
	return delta*delta*delta*delta;
}

// Sample incoming radiance to a point p. u1 and u2 are 2d sampling positions on the light
// returns: wi is the incoming light direction, pdf is the probability function, visibility ray is in 'visibility'
Spectrum SpotLight::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	// calculate direction from sampling point to our position
	*wi = m_vPos - vPos;

	// calculate visibility ray
	visibility->SetSegment(vPos, m_vPos);

	// calculate reciprocal squared distance
	float fDistSq = wi->LengthSquared();
	float fInvDistSq = 1.0f / fDistSq;
	float fInvDist = 1.0f / sqrtf(fDistSq);

	*wi = *wi * fInvDist;	// normalize direction vector
	*pdf = 1.0f;

	return GetEmittance() * GetDecay(m_iDecayType, fDistSq) * FallOff(-(*wi));
}

// sample a randomly shot ray from the light
Spectrum SpotLight::Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const
{
	float3 vOrg, vDir;
	// calculate ray
	vDir = UniformSampleCone(float2(vSample.x, vSample.y), m_fCosTotalWidth);
	vDir = vDir * (HMat33)m_worldMatrix;

	vDir.Normalize();
	vDir = -vDir;
	pRay->SetRay(m_vPos, vDir);
	// calculate pdf
	*pdf = UniformConePdf(m_fCosTotalWidth);
	return GetEmittance() * FallOff(vDir);
}


// return total emissive power
Spectrum SpotLight::Power()
{
	return GetEmittance() * 2.f * PI * (1.f - .5f * (m_fCosFalloffStart + m_fCosTotalWidth));
}

float SpotLight::Pdf(const float3 &vPos, const float3 &vwi)
{
	return 0;
}

// return bounding box
box3f SpotLight::GetBBox(const LUX::SpatialHierarchy *pScene) const
{
	box3f bbox;

	float fSpotBoxExpandSize = 0.01f;
	float pdf;
	float fMaxRadius = GetMaxRadius(GetEmittance(), m_fCutoffThreshold);

	bbox.UpdateBox(m_vPos);

	// calculate maximum radius (pointlight-style)

	// shoot a few rays from our light and see where they end up - put the bounding box around there
	for (int i=0; i<40; i++)
	{
		float4 vSample( genrand_real1(), genrand_real1(), genrand_real1(), genrand_real1() );
		SD::Ray ray;
		SD::RayTraceResult result;
		ray.bShadowRay = false;
		ray.length = fMaxRadius;
		Sample(vSample, &ray, &pdf);

		pScene->RayTrace(ray, result);

		float3 vEnd;
		if (result.bHit)
			vEnd = result.GetPosition();
		else
			vEnd = ray.origin + (ray.direction * fMaxRadius);

		bbox.UpdateBox(vEnd);
//		AddLine(ray.origin, vEnd);
	}

	// expand the box a little
	bbox.vmin -= float3(fSpotBoxExpandSize);
	bbox.vmax += float3(fSpotBoxExpandSize);

//	AddBoundingBox(bbox);

	return bbox;
}

void SpotLight::write( ost::Engine &ar )
{
	ar << m_vPos << m_worldMatrix << m_fCosTotalWidth << m_fCosFalloffStart << (int)m_iDecayType;
	Light::write( ar );
}
void SpotLight::read( ost::Engine &ar )
{
	ar >> m_vPos >> m_worldMatrix >> m_fCosTotalWidth >> m_fCosFalloffStart >> *(int*)&m_iDecayType;
	m_invWorldMatrix = m_worldMatrix.Inverse();
	Light::read( ar );
}

void SpotLight::SetGIProperties(GlobalIlluminationProperties *pGIProp)
{
	ApplyGammaSettingsToIntensity(pGIProp);
}
