#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/HDRLight.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"

#include "Lux/HDRImage.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"

#include <algorithm>

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(HDRLight, "HDRLight");

HDRLight::HDRLight() : Light()
{
	m_type = LUX_LIGHT_TYPE_HEMISPHERE;
	m_nSamples = 8;
	m_pHDRMap = NULL;
	m_pSamples = NULL;
	m_fYaw = 0;
}

HDRLight::~HDRLight()
{
	if (m_pSamples)
	{
		FreeAligned(m_pSamples);
		m_pSamples = NULL;
	}
	if (m_pHDRMap)
	{
		delete m_pHDRMap;
		m_pHDRMap = NULL;
	}
}

void HDRLight::ReadImage(char *pszFilename)
{
	if (!pszFilename || !pszFilename[0])
		return;

	m_pHDRMap = new HDRImage;
	bool ret = HDRLoader::Load(pszFilename, *m_pHDRMap); 
	if (!ret)
	{
		printf("WARNING: Failed to load HDR Image '%s'.\n",pszFilename);
		delete m_pHDRMap;
		m_pHDRMap = NULL;
	}
}

void HDRLight::SetXMLParameter(char *pName, char *pValue)
{
	Light::SetXMLParameter(pName, pValue);

	if (stricmp(pName, "HDRMap") == 0)			// read intensity map
	{
		ReadImage(pValue);
	}
	else if (stricmp(pName, "rotation") == 0)	// read intensity map
	{
		
		std::stringstream s;
		float f;
		s << pValue;
		s >> f;
		m_fYaw = DEG2RAD(f);
	}
}

// generate our samples
void HDRLight::PreProcess(Sampler *pSampler)
{
	if (!m_pHDRMap)
	{
		// TODO: handle this better
		m_nSamples = 0;
		return;
	}

	int width = m_pHDRMap->GetWidth();
	int height = m_pHDRMap->GetHeight();

	Distribution1D *uDistrib, **vDistribs;

	// Compute scalar-valued image from environment map
	float filter = 1.f / max(width, height);
	int nu = width, nv = height;
	float *img = new float[width*height];
	for (int x = 0; x < nu; ++x)
	{
		float xp = (float)x / (float)nu;
		for (int y = 0; y < nv; ++y)
		{
			float yp = (float)y / (float)nv;
			
			img[y+x*nv] = m_pHDRMap->Lookup(xp, yp).Luma();
		}
	}
	// precalculate sin values
	float *sinVals = new float[nv];
	for (int i = 0; i < nv; ++i)
		sinVals[i] = sin(M_PI * float(i+.5)/float(nv));

	// Initialize sampling PDFs for infinite area light
	float *func = new float[max(nu,nv)];
	vDistribs = new Distribution1D *[nu];
	for (int u = 0; u < nu; ++u)
	{
		// Compute sampling distribution for column _u_
		for (int v = 0; v < nv; ++v)
			func[v] = img[u*nv+v] *= sinVals[v];
		vDistribs[u] = new Distribution1D(func, nv);
	}

	// Compute sampling distribution for columns of image
	for (int u = 0; u < nu; ++u)
		func[u] = vDistribs[u]->funcInt;
	uDistrib = new Distribution1D(func, nu);

	// use the sampling distribution to precalculate the samples

	// allocate our sample buffer
	m_pSamples = (HDRSample*)AllocAligned( sizeof(HDRSample) * m_nSamples );

	float2 *buffer = new float2[m_nSamples];
	float x,y,z;

	// create samples into temporary buffer
	LDShuffleScrambled2D(1, m_nSamples, &buffer[0].x);

	// convert to sampling direction
	HDRSample *pSample = m_pSamples;
	for (int iSample=0; iSample<m_nSamples; iSample++, pSample++)
	{
		float u1 = buffer[iSample].x;
		float u2 = buffer[iSample].y;

		// Find floating-point u,v sample coordinates
		float pdfs[2];
		float fu = uDistrib->Sample(u1, &pdfs[0]);
		int u = (int)Clamp(Float2Int(fu), 0, uDistrib->count-1);
		float fv = vDistribs[u]->Sample(u2, &pdfs[1]);

		// Convert sample point to direction on the unit sphere
		float theta = fv * vDistribs[u]->invCount * M_PI;
		float phi = fu * uDistrib->invCount * 2.f * M_PI;
		phi -= m_fYaw;
		float costheta = cos(theta), sintheta = sin(theta);
		float sinphi = sin(phi), cosphi = cos(phi);

		// calculate world-space direction vector
		float3 vDir	= float3(sintheta * cosphi, sintheta * sinphi, costheta);
		// TODO: allow for rotating of the HDR map
		
		pSample->vDir = float3( vDir.x, vDir.z, vDir.y );
		pSample->pdf  = (pdfs[0] * pdfs[1]) / (2. * M_PI * M_PI * sintheta);
		pSample->vCol = GetEmittance() * m_pHDRMap->Lookup(u,(int)fv);

		pSample->vCol.x = powf(pSample->vCol.x, m_fInputGamma);
		pSample->vCol.y = powf(pSample->vCol.y, m_fInputGamma);
		pSample->vCol.z = powf(pSample->vCol.z, m_fInputGamma);
	}

//	SortVectorsBySigns(m_pSamples + i*m_nSamples, m_nSamples);
	delete [] img;
	delete [] sinVals;
	for (int i = 0; i < nu; i++)
		delete vDistribs[i];
	delete [] func;
	delete uDistrib;
	delete [] vDistribs;
	delete [] buffer;

}



// Sample incoming radiance to a point p. u1 and u2 are 2d sampling positions on the light
// returns: wi is the incoming light direction, pdf is the probability function, visibility ray is in 'visibility'
float3 HDRLight::Sample(const float3 &vPos, const float3 &vNormal,
		int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	// TEST
	if (m_nSamples == 1)
	{
		visibility->SetRay(vPos, *wi);
		*wi = vNormal;
		*pdf = 1;

		float3 wh;
		wh.x = vNormal.x;
		wh.y = vNormal.z;
		wh.z = vNormal.y;

		float s = SphericalPhi(wh) * INV_TWOPI;
		float t = SphericalTheta(wh);
		return GetEmittance() * m_pHDRMap->Lookup(s, t * INV_PI);
	}

	// samples facing away from the normal will be rejected by the shader
	return Sample(vPos, iSample, wi, pdf, visibility);
}

// same as above, but without specifying a normal
float3 HDRLight::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	// Sample using precalculated sample directions
	assert(iSample<m_nSamples);
	HDRSample *pSample = &m_pSamples[iSample];

	*wi = pSample->vDir;
	*pdf = pSample->pdf;

	visibility->SetRay(vPos, *wi);
	return pSample->vCol;
}

// cast a ray from the light
Spectrum HDRLight::Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const
{
//	assert(0);
	*pdf = 0;
	return float3(0,0,0);
}

float HDRLight::Pdf(const float3 &vPos, const float3 &vwi)
{
	return 0;
}

void HDRLight::SetGIProperties(GlobalIlluminationProperties *pGI)
{
	m_fInputGamma = pGI->InputGamma();
}

// serialize/deserialize
void HDRLight::write(ost::Engine &ar)
{
	Light::write(ar);

	if (m_pHDRMap)
	{
		ar << true;
		ar << *m_pHDRMap;
	}
	else
		ar << false;

	ar << m_fYaw << m_fInputGamma;
}
void HDRLight::read(ost::Engine &ar)
{
	Light::read(ar);

	bool bHDRMap;
	ar >> bHDRMap;
	if (bHDRMap)
	{
		m_pHDRMap = new HDRImage;
		ar >> *m_pHDRMap;
	}

	ar >> m_fYaw >> m_fInputGamma;
}

// distribution utility class
Distribution1D::Distribution1D(float *f, int n)
{
	func = new float[n];
	cdf = new float[n+1];
	count = n;
	memcpy(func, f, n*sizeof(float));
	ComputeStep1dCDF(func, n, &funcInt, cdf);
	invFuncInt = 1.f / funcInt;
	invCount = 1.f / count;
}

float Distribution1D::Sample(float u, float *pdf)
{
	// Find surrounding cdf segments
	float *ptr = std::lower_bound(cdf, cdf+count+1, u);
	int offset = (int) (ptr-cdf-1);
	// Return offset along current cdf segment
	u = (u - cdf[offset]) / (cdf[offset+1] - cdf[offset]);
	*pdf = func[offset] * invFuncInt;
	return offset + u;
}

Distribution1D::~Distribution1D()
{
	delete [] func;
	delete [] cdf;
}
