// same as hemispherelight, but with built in sun bounce lighting
#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/LightSample.h"
#include "Lux/Lights/BounceGatherLight.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"
#include "Lux/GlobalIllumination/ProgressiveBounceLighting.h"
#include "Lux/TextureTypes/RGBTexture.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(BounceGatherLight, "BounceGather");

BounceGatherLight::BounceGatherLight() : Light()
{
	m_type = LUX_LIGHT_TYPE_BOUNCEGATHER;
	m_pSamples = NULL;
	m_pSamplesPoint = NULL;
	m_nSamples = 8;
	m_fBounceScale = 0.3f;
	m_bHemisphere = true;
	m_iBounceChannel = 0;
	m_bCalcLightTransfer = false;
}

BounceGatherLight::~BounceGatherLight()
{
	FreeAligned(m_pSamples);
	FreeAligned(m_pSamplesPoint);
}

void BounceGatherLight::SetXMLParameter(char *pName, char *pValue)
{
	Light::SetXMLParameter(pName, pValue);

	std::stringstream s;
	if (stricmp(pName, "BounceScale") == 0)
	{
		s << pValue;
		s >> m_fBounceScale;
	}
	else if (stricmp(pName, "CalcLightTransfer") == 0)
	{
		if ((stricmp(pValue, "true") == 0) ||
			(stricmp(pValue, "on") == 0))
			m_bCalcLightTransfer = true;
		else
			m_bCalcLightTransfer = false;
	}
	else if (stricmp(pName, "Hemisphere") == 0)
	{
		if (stricmp(pValue, "false") == 0)
			m_bHemisphere = false;
		else
			m_bHemisphere = true;
	}
}

void BounceGatherLight::SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize)
{
	m_pScene = pScene;
}

// returns the lighting intensity for a given direction
float3 BounceGatherLight::Le(const float3 &vDir) const
{
	// samples pointing into to the ground are black
	if (vDir.y<=0 && m_bHemisphere)
		return float3(0,0,0);

	// return the same intensity for all angles
	return GetEmittance();
}

// generate our samples
// TODO: share this function between Hemisphere, Hemisphere2 and BounceGather
void BounceGatherLight::PreProcess(Sampler *pSampler)
{
	MEMORY_TRACKER_PUSH("tools.lux.client.bouncegatherlight");
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

	// sample for SH nodes, samplepoints etc.
	int nSamples = 128;
	m_pSamplesPoint = (float3*)AllocAligned( sizeof(float3) * nSamples);

	buffer = new float2[nSamples];

	LDShuffleScrambled2D(1, nSamples, &buffer[0].x);

	for (int iSample=0; iSample<nSamples; iSample++)
		m_pSamplesPoint[iSample] = UniformSampleSphere(buffer[iSample].x, buffer[iSample].y);

	delete [] buffer;

	MEMORY_TRACKER_POP("tools.lux.client.bouncegatherlight");
}

// dummy default sample function
float3 BounceGatherLight::Sample(const float3 &vPos, const float3 &vNormal,
		int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	assert(0);
	return float3(0,0,0);
}

// bounce light sample function which can return multiple samples simultaneously
void BounceGatherLight::Sample(const float3 &vPos, const float3 &vNormal, float *pdf,
			int iSample, Ut::Vector<LightSample> &samples) const
{
	// Sample cosine-weighted direction on unit sphere
	float x, y, z;
	float3 n = vNormal;

	assert(iSample<9*m_nSamples);
	float3 wi = m_pSamples[iSample];

	// Compute pdf for cosine-weighted infinite light direction
	*pdf = fabsf(wi.z) * INV_TWOPI;

	// Transform direction to world space
	float3 v1, v2;
	CoordinateSystem(n, &v1, &v2);
	wi = float3(v1.x * wi.x + v2.x * wi.y + n.x * wi.z,
	            v1.y * wi.x + v2.y * wi.y + n.y * wi.z,
	            v1.z * wi.x + v2.z * wi.y + n.z * wi.z);

	DoSample(vPos, wi, samples);
}

// same as above, but without specifying a normal
void BounceGatherLight::Sample(const float3 &vPos, float *pdf, int iSample, Ut::Vector<LightSample> &samples) const
{
	// generate uniform hemisphere sample
	float3 vDir;
	if (iSample<128)
		vDir = m_pSamplesPoint[iSample];
	else
		vDir = UniformSampleSphere(RandomFloat(),RandomFloat());

	// Compute pdf for uniform hemisphere sample
//	*pdf = 1.0f / (4.f * M_PI);
	if (m_bHemisphere)
		*pdf = 0.5f * (1.0f / (4.f * M_PI));
	else
		*pdf = 0.5f * (1.0f / (4.f * M_PI));

	DoSample(vPos, vDir, samples);
}

// perform bounce light gather sampling
void BounceGatherLight::DoSample(const float3 &vPos, const float3 &vDir, Ut::Vector<LightSample> &samples) const
{
	SD::Ray aoray;
	SD::RayTraceResult result;
	aoray.SetRay(vPos, vDir);
	aoray.bShadowRay = false;
	m_pScene->RayTrace(aoray, result);
	samples.Resize(1);

	if (!m_bCalcLightTransfer)
	{
		// normal bounce lighting - return the colour
		samples[0].vDir = vDir;

		// unoccluded ray - add AO contribution
		if (!result.bHit)
		{
			// normal bounce lighting

			// if we're doing an intermediate bounce, ignore the AO
			if (m_iBouncePass == PROGBOUNCEPASS_BOUNCE)
				samples[0].vColour = float3(0,0,0);
			else
			{
				samples[0].vColour = Le(vDir);
			}
			// return the hemisphere colour
			samples[0].iChannel = m_iChannel;
			return;
		}

		// occluded ray, return 0
		if (m_iBouncePass == PROGBOUNCEPASS_SEED)
		{
			samples[0].vColour = float3(0,0,0);
			samples[0].iChannel = m_iChannel;
			return;
		}

		// occluded ray - lookup bounce texture and add the contribution from that
		samples[0].vColour = m_pBounceTexData->Sample(result, m_pScene) * m_fBounceScale;
		samples[0].iChannel = m_iBounceChannel;
	}
	else
	{
		// light transfer - return the direction of the light as an SH
		samples[0].iChannel = m_iChannel;

		// unoccluded ray - add AO contribution
		if (!result.bHit)
		{
			samples[0].vDir = vDir;
			samples[0].iChannel = m_iChannel;

			if (m_iBouncePass == PROGBOUNCEPASS_BOUNCE)
			{
				samples[0].vColour = float4(0,0,0,0);
				return;
			}

			// light transfer - return light SH direction as RGB
			float3 v = Le(vDir);
			float fAvg = (v.x + v.y + v.z)/3.f;
			float tmp[3][4];

			SHEvalDirectionalLight( 2, &vDir.x,
				fAvg, fAvg, fAvg, tmp[0], tmp[1], tmp[2]);

			samples[0].vColour = float4(tmp[0][0], tmp[0][1], tmp[0][2], tmp[0][3]);
			return;
		}

		// occluded ray, return 0
		if (m_iBouncePass == PROGBOUNCEPASS_SEED)
		{
			samples[0].vColour = float4(0,0,0,0);
			samples[0].vDir = vDir;
			return;
		}

		// occluded ray - lookup bounce texture and add the contribution from that
		samples[0].vColour = m_pBounceTexData->Sample(result, m_pScene) * m_fBounceScale;
		samples[0].vDir = vDir;
	}
}

// same as above, but without specifying a normal
float3 BounceGatherLight::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	assert(0);
	return float3(0,0,0);
}

// cast a ray from the light
Spectrum BounceGatherLight::Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const
{
//	assert(0);
	*pdf = 0;
	return float3(0,0,0);
}

float BounceGatherLight::Pdf(const float3 &vPos, const float3 &vwi)
{
	return 0;
}

// serialize/deserialize
void BounceGatherLight::write(ost::Engine &ar)
{
	ar << m_fBounceScale << m_bCalcLightTransfer << m_bHemisphere << m_iBounceChannel;
	Light::write(ar);
}
void BounceGatherLight::read(ost::Engine &ar)
{
	ar >> m_fBounceScale >> m_bCalcLightTransfer >> m_bHemisphere >> m_iBounceChannel;
	Light::read(ar);
}

void BounceGatherLight::SetBounceTextureData(BounceTexData *pBounceTextureData)
{
	m_pBounceTexData = pBounceTextureData;
}

void BounceGatherLight::SetBouncePass(int i)
{
	m_iBouncePass = i;
}
