#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/PointLight.h"
#include "Lux/Lights/SpotLight.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(PointLight, "PointLight");

PointLight::PointLight() : Light()
{
	Initialize();
}

PointLight::PointLight(const Aftk::C_ScenePointLightNode *pAftkLight, Aftk::Matrix44 &mtx) : Light(pAftkLight, mtx)
{
	Initialize();

	SetColour( (*(float3*)&pAftkLight->GetColour()) );
	SetIntensity( pAftkLight->GetIntensity() * MAYA_LIGHT_GLOBAL_SCALE );

	// get decay type
	m_iDecayType = pAftkLight->GetDecayType();
	if (m_iDecayType == Aftk::E_LightDecay_None)
		S_WARN( g_Log, "Node '" << pAftkLight->GetPath() << "' has decay type set to 'none'\n");

	m_fCutoffRadius = FLT_MAX;
}

void PointLight::Initialize()
{
	m_type = LUX_LIGHT_TYPE_POINT;
	m_fCutoffRadius = FLT_MAX;
	m_iDecayType = Aftk::E_LightDecay_Quadratic;
}

PointLight::~PointLight()
{
}

// do preprocessing - in this case precalculate the max radius
void PointLight::PreProcess(Sampler *pSampler)
{
	float fMaxRadius;

	switch (m_iDecayType)
	{
	case Aftk::E_LightDecay_Quadratic:
		fMaxRadius = GetMaxRadius(GetEmittance(), m_fCutoffThreshold);
		m_fMaxRadiusSq = fMaxRadius*fMaxRadius;
		break;
	case Aftk::E_LightDecay_Linear:
		m_fMaxRadiusSq = GetMaxRadius(GetEmittance(), m_fCutoffThreshold);
		break;
	default:
		m_fMaxRadiusSq = FLT_MAX;
		break;
	}

	float fCut = m_fCutoffRadius * m_fCutoffRadius;
	if (fCut < m_fMaxRadiusSq) m_fMaxRadiusSq = fCut;
}

// Sample incoming radiance to a point p. u1 and u2 are 2d sampling positions on the light
// returns: wi is the incoming light direction, pdf is the probability function, visibility ray is in 'visibility'
float3 PointLight::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	float3 vMyPos = m_vPos; //= m_vPos + UniformSampleSphere(vSample.x,vSample.y) * 0.002f;
	// calculate direction from sampling point to our position
	*wi = vMyPos - vPos;

	*pdf = 1.0f;

	// calculate visibility ray
	visibility->SetSegment(vPos, vMyPos);

	// calculate 1/squared distance
	float fDistSq = wi->LengthSquared();

	// calculate attenuation to fade-out by fMaxRadius
	float f = (fDistSq/m_fMaxRadiusSq)*6.0f-5.0f;
	if (f<0)
		f = 1.0f;
	else if (f>1)
	{
		return float3(0,0,0);
	}
	else
		f = ::Lerp(f,1.0f,0.0f);

	float fInvDist = 1.0f / sqrtf(fDistSq);
	float fDecay = f * GetDecay(m_iDecayType, fDistSq);

	*wi = *wi * fInvDist;	// normalize direction vector

	return GetEmittance() * fDecay;
}

Spectrum PointLight::Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const
{
	pRay->SetRay(m_vPos, UniformSampleSphere(vSample.x,vSample.y));
	*pdf = 1.f / (4.f * PI);
	return GetEmittance();
}

float PointLight::Pdf(const float3 &vPos, const float3 &vwi)
{
	return 1.0;
}

box3f PointLight::GetBBox(const LUX::SpatialHierarchy *pScene) const
{
	float fMaxRadius = GetMaxRadius(GetEmittance(), m_fCutoffThreshold);

	box3f bbox(m_vPos-float3(fMaxRadius), m_vPos+float3(fMaxRadius));

	return bbox;
}

void PointLight::write(ost::Engine &ar)
{
	ar << m_vPos << (int)m_iDecayType;
	Light::write(ar);
}
void PointLight::read(ost::Engine &ar)
{
	ar >> m_vPos >> *(int*)&m_iDecayType;
	Light::read(ar);
}

void PointLight::SetGIProperties(GlobalIlluminationProperties *pGIProp)
{
	ApplyGammaSettingsToIntensity(pGIProp);
}
