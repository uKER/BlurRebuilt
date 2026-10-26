#include "stdafx.h"
#include "Lux/RenderChannel.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/Lights/Light.h"
#include "Lux/Lights/AreaLight.h"
#include "Lux/LightingModel.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationModel.h"
#include "Lux/GlobalIllumination/ProgressiveBounceLighting.h"
#include "Lux/Lights/BounceGatherLight.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/ImageFilter.h"
#include "Lux/SurfaceIterator.h"
#include "Lux/ShadowRayBatch.h"
#include "Lux/RasterizedTriIterator.h"
#include "lux_common/TexPreprocess.h"
#include <algorithm>

using namespace LUX;

const int SurfaceIntegrator::m_iMaxTriangleSamples = 1000;

IMPLEMENT_PERSISTENCE(SurfaceIntegrator, "SurfIntr");

// get renderchannel pointer by index
RenderChannel *SurfaceIntegrator::GetRenderChannel(int iChannel)
{
	return (*m_pChannels)[iChannel];
}

void SurfaceIntegrator::GetRenderChannelTarget(int iChannel, LightSample &sample)
{
	RenderChannel *p = (*m_pChannels)[iChannel];
	if (p)
	{
		sample.target = p->m_target;
		sample.iSubChannel = p->m_iSubChannel;
	}
	else
	{
		sample.target = RenderChannel::RGB;
		sample.iSubChannel = 0;
	}
}

// calculate lighting data for a set of points/normals specified by the lighting iterator
void SurfaceIntegrator::LightData(SurfaceIterator *pIterator)
{
	SetIterator(pIterator);

	// debug: rasterize using whitespace - for testing packing
	if (gbDebugMode)
	{
		m_pIterator->Go(SurfaceIterator::SampleFirst, 1, SampleCallbackWhite, this);
		m_pIterator->End(GetRenderChannel(0));
		return;
	}

	// sample directional light
	if (m_pDirectionalLight)
		RenderLight(m_pDirectionalLight);

	// sample local lights
//	SampleLightListParallel( &m_pLightTree->m_LightPtrVec );
	SampleLightList( &m_pLightTree->m_LightPtrVec );

	// sample global lights
	SampleLightList( m_pGlobalLightList );
}

// set the surface iterator to use
void SurfaceIntegrator::SetIterator(SurfaceIterator *pIterator)
{
	m_pIterator = pIterator;

	pIterator->SetSampler(m_pSampler);
}

void SurfaceIntegrator::SampleLightListSorted(LightGroupItem *pItem, int nItems)
{
	m_pLightGroupItem = pItem;
	m_nLightGroupItems = nItems;

	int nTotalSamples = 0;
	for (int i=0; i<nItems; i++)
		nTotalSamples += pItem[i].nSamples;
	m_fTotalSamples = (float)nTotalSamples;

	// when rendering textures, the light decides whether we want lambert shading or not
	SurfaceIteratorCallbackPtr pCallback = SampleCallbackParallel;

	// if there are no surface normals, force the callback to the one used for SH point sampling
	if (!m_pIterator->SurfaceNormals())
		pCallback = SampleCallbackParallelPoint;

	// iterate using callback
	ShadowRayBatch batch(m_pIterator, m_pHierarchy);
	m_pBatch = &batch;

	m_pIterator->Go(SurfaceIterator::SampleFirst, pItem->nSamples, pCallback, this);

	batch.Flush();
	m_pIterator->End(NULL);
}

// sample an entire light list with one sweep of the iterator
void SurfaceIntegrator::SampleLightListParallel(LightPtrVector *pLightList)
{
	if (pLightList->Size() == 0)
		return;

	// sort the lights by number of samples
	Ut::Vector<LightGroupItem> sortlist;
	for (int i=0; i<pLightList->Size(); i++)
	{
		LightGroupItem item;
		item.pLight = pLightList->At(i);		
		assert(!item.pLight->UseDiscontinuitySampling());
		assert(item.pLight->IsDeltaLight());
		if (!m_pIterator->SurfaceNormals())
			item.nSamples = 1;	// no surface normal -> always 1
		else
		{
			item.nSamples = item.pLight->m_nSamples; // use light nsamples
			if (item.nSamples == -1)
				item.nSamples = m_pSampler->m_nSamplesPerTexel; // if light doesn't specify, use default from antialiasing setting
		}
		sortlist.PushBack(item);
	}

	// sort the list
	std::sort(&sortlist[0], &sortlist[0]+sortlist.Size());

	int nCurSamples = sortlist[0].nSamples;
	int i=1;
	int iFirst = 0;
	while (1)
	{
		if (i>=sortlist.Size() || sortlist[i].nSamples != nCurSamples)
		{
			SampleLightListSorted(&sortlist[iFirst], i-iFirst);	
			if (i>=sortlist.Size())
				break;
			iFirst = i+1;
		}
		i++;
	}
}

// walk through a list of light, sampling each
void SurfaceIntegrator::SampleLightList(LightPtrVector *pLightList)
{
	for (int i=0; i<pLightList->Size(); i++)
	{
		RenderLight(pLightList->At(i));
	}
}

// special function to render the bounce-gather light
// this function can output light into multiple channels simultaneously
void SurfaceIntegrator::RenderBounceGatherLight(Light *pLight)
{
	if (m_pIterator->SurfaceNormals())
		m_pIterator->Go(SurfaceIterator::DiscontinuitySampling, pLight->m_nSamples, SampleCallbackBounceGather, this);
	else
		m_pIterator->Go(SurfaceIterator::DiscontinuitySampling, pLight->m_nSamples, SampleCallbackBounceGatherPoint, this);

	Ut::Vector<LUX::RenderChannel*> channels;
	channels.Resize(2);
	channels[0] = GetRenderChannel(m_iBounceChannel);
	channels[1] = GetRenderChannel(pLight->m_iChannel);
	m_pIterator->End(channels);
}

// render one light
void SurfaceIntegrator::RenderLight(Light *pLight)
{
	m_pLight = pLight;

	if (pLight->GetType() == LUX_LIGHT_TYPE_BOUNCEGATHER)
	{
		// render the bounce gather light
		RenderBounceGatherLight(pLight);
		return;
	}
	else
	{
		// if we're doing an intermediate bounce pass, then only render the bounce gather light
		if (m_iProgBouncePass == PROGBOUNCEPASS_BOUNCE)
			return;
	}

	// when rendering textures, the light decides whether we want lambert shading or not
	SurfaceIteratorCallbackPtr pCallback = pLight->GetLambert()?SampleCallbackLambert:SampleCallbackFlat;

	// if there are no surface normals, force the callback to the one used for SH point sampling
	if (!m_pIterator->SurfaceNormals())
		pCallback = SampleCallbackPoint;

	// iterate using callback
	RenderChannel *pChannel = GetRenderChannel(pLight->m_iChannel);
	ShadowRayBatch batch(m_pIterator, m_pHierarchy);
	m_pBatch = &batch;

	if (pLight->UseDiscontinuitySampling())
	{
		if (pLight->m_nSamples == 0)
			return;
		m_pIterator->Go(SurfaceIterator::DiscontinuitySampling, pLight->m_nSamples, pCallback, this);
	}
	else if (pLight->IsDeltaLight())
	{
		// calculate number of samples
		int nSamples;
		if (!m_pIterator->SurfaceNormals())
			nSamples = 1;	// no surface normal -> always 1
		else
		{
			nSamples = pLight->m_nSamples; // use light nsamples
			if (nSamples == -1)
				nSamples = m_pSampler->m_nSamplesPerTexel; // if light doesn't specify, use default from antialiasing setting
		}
		
		m_pIterator->Go(SurfaceIterator::SampleFirst, nSamples, pCallback, this);
	}
	else
	{
		m_pIterator->Go(SurfaceIterator::SampleFirstNoAA, pLight->m_nSamples, pCallback, this);
	}

	batch.Flush();
	m_pIterator->End(pChannel);
}

// constructor
SurfaceIntegrator::SurfaceIntegrator(const SurfaceIntegratorSetup &settings)
{
	m_settings	  = settings;
	m_fMinDistSq  = settings.m_fMinDist * settings.m_fMinDist;
	m_nSamplerPerTexel = settings.m_nSamplerPerTexel;
	m_bInitializedThroughDeserialization = false;
	m_fTexelDensity = settings.m_fTexelDensity;
	m_bAntiLightLeaking = settings.m_bAntiLightLeaking;
	m_fEpsilon = settings.m_fEpsilon;
	m_pSampler = NULL;
	m_p3x3BoxFilter = NULL;
	m_pGaussianFilter = NULL;
	m_pBounceTextures = NULL;
}

// destructor
SurfaceIntegrator::~SurfaceIntegrator()
{
	if (m_pSampler)
		delete m_pSampler;

	if (m_p3x3BoxFilter)
		delete m_p3x3BoxFilter;
	if (m_pGaussianFilter)
		delete m_pGaussianFilter;

	if (m_bInitializedThroughDeserialization)
	{
		// if we were initialized through deserialization on the client, we'll need to destroy some extra things
		for (int i=0; i<m_pLightTree->m_LocalLights.Size(); i++)
			delete m_pLightTree->m_LocalLights[i];
		m_pLightTree->m_LocalLights.Destroy();
		for (int i=0; i<m_pGlobalLightList->Size(); i++)
			delete m_pGlobalLightList->At(i);
		m_pGlobalLightList->Destroy();
		delete m_pGlobalLightList;
		delete m_pLightTree;
		delete m_pBounceTextures;

		delete m_pHierarchy;
		if (m_pDirectionalLight)
			delete m_pDirectionalLight;

		for (int i=0; i<m_pChannels->Size(); i++)
			delete m_pChannels->At(i);
		m_pChannels->Destroy();
		delete m_pChannels;
	}
}

void SurfaceIntegrator::SampleCallbackParallel(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample)
{
	SurfaceIntegrator *me = (SurfaceIntegrator*)pThis;

	LightGroupItem *pItem = me->m_pLightGroupItem;
	Ut::Vector<LightSample> samples;
	float3 vDir, d;
	float fPDF;
	Visibility vis;
	SD::RayTraceResult result;
	samples.Reserve(me->m_nLightGroupItems);

	// TODO: rewrite this using SSE? process 4 lights simultaneously?
	// for each light in the group
	for (int i=0; i<me->m_nLightGroupItems; i++, pItem++)
	{
		Light *pLight = pItem->pLight;

		// sample the light
		float3 vLight = pLight->Sample(vPos, vNormal, iCurSample, &vDir, &fPDF, &vis);

		if (pLight->GetLambert())
		{
			// perform lambert shading
			float fLambert = DotProduct(vDir, vNormal);
			if (fLambert<0)
				d = float3(0,0,0);
			else
			{
				d = vLight * fLambert / fPDF;
			}
		}

		if (d.x>0 || d.y>0 || d.z>0)
		{
			// trace the visibility ray
			me->m_pBatch->m_pHierarchy->RayTrace(vis.ray, result);

			if (result.bHit)
				d = float3(0,0,0);
		}

		// add the sample to our list
		LightSample sample;
		sample.vDir = vDir;
		sample.vColour = d;
		me->GetRenderChannelTarget(pLight->m_iChannel, sample);
		samples.PushBack(sample);			
	}

	// send the sample back to the iterator
	me->m_pIterator->AddSample(samples);
}

void SurfaceIntegrator::SampleCallbackParallelPoint(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample)
{
	SurfaceIntegrator *me = (SurfaceIntegrator*)pThis;

	LightGroupItem *pItem = me->m_pLightGroupItem;
	Ut::Vector<LightSample> samples;
	float3 vDir, d;
	float fPDF;
	LightSample sample;
	Visibility vis;
	SD::RayTraceResult result;
	samples.Reserve(me->m_nLightGroupItems);
	// for each light in the group
	for (int i=0; i<me->m_nLightGroupItems; i++)
	{
		Light *pLight = pItem->pLight;

		// sample the light
		float3 vLight = pLight->Sample(vPos, iCurSample, &vDir, &fPDF, &vis);

		// trace the visibility ray
		if (d.x > 0 || d.y > 0 || d.z > 0)
		{
			me->m_pBatch->m_pHierarchy->RayTrace(vis.ray, result);

			// add the sample to our list
			sample.vDir = vDir;
			if (!result.bHit)
				sample.vColour = d;
			else
				sample.vColour = float3(0.f,0.f,0.f);
		}
		else
			sample.vColour = float3(0.f,0.f,0.f);
		me->GetRenderChannelTarget(pLight->m_iChannel, sample);
		samples.PushBack(sample);			
	}

	// send the sample back to the iterator
	me->m_pIterator->AddSample(samples);
}

// callback to render bounce gather light for a surface with normals
void SurfaceIntegrator::SampleCallbackBounceGather(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample)
{
	SurfaceIntegrator *me = (SurfaceIntegrator*)pThis;
	BounceGatherLight *pLight = (BounceGatherLight*)me->m_pLight;

	Ut::Vector<LightSample> samples;
	float3 vDir;
	float pdf;
	pLight->Sample(vPos, vNormal, &pdf, iCurSample, samples);

	pdf = 1.f/pdf;

	for (int i=0; i<samples.Size(); i++)
	{
		// Compute pdf for cosine-weighted infinite light direction
		float fLambert = DotProduct(samples[i].vDir, vNormal);
		if (fLambert<0)
			fLambert = 0;

		samples[i].vColour = samples[i].vColour * (fLambert * pdf);
		me->GetRenderChannelTarget(samples[i].iChannel, samples[i]);
	}

	me->m_pIterator->AddSample(samples);
}

// callback to render bounce gather light for a surface without normals
void SurfaceIntegrator::SampleCallbackBounceGatherPoint(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample)
{
	SurfaceIntegrator *me = (SurfaceIntegrator*)pThis;
	BounceGatherLight *pLight = (BounceGatherLight*)me->m_pLight;

	Ut::Vector<LightSample> samples;
	float3 vDir;
	float pdf;
	pLight->Sample(vPos, &pdf, iCurSample, samples);

	pdf = 1.f/pdf;

	for (int i=0; i<samples.Size(); i++)
	{
		samples[i].vColour = samples[i].vColour * pdf;
		me->GetRenderChannelTarget(samples[i].iChannel, samples[i]);
	}

	me->m_pIterator->AddSample(samples);
}

// callback to rasterize whitespace for debugging
void SurfaceIntegrator::SampleCallbackWhite(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample)
{
	SurfaceIntegrator *me= (SurfaceIntegrator *)pThis;

	LightSample sample;
	sample.vColour = float3(1,1,1);
	sample.vDir = float3(1,1,1);
	me->GetRenderChannelTarget(0, sample);
	me->m_pIterator->AddSample(sample);
}

// callback function that is called by the iterator - samples without applying any shading
void SurfaceIntegrator::SampleCallbackPoint(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample)
{
	float3 vDir, d;
	float fPDF;
	Visibility vis;

	SurfaceIntegrator *me= (SurfaceIntegrator *)pThis;

	float3 vLight = me->m_pLight->Sample(vPos, iCurSample, &vDir, &fPDF, &vis);

	d = vLight / fPDF;

	// skip raytracing for low light values below
	const static float fCutoff = 0.001f;
	if (d.x < fCutoff && d.y < fCutoff && d.z < fCutoff)
		d = float3(0,0,0);

	LightSample sample;
	sample.vColour = d;
	sample.vDir = vDir;
	me->GetRenderChannelTarget(me->m_pLight->m_iChannel, sample);

	// add this sample to our batch
	me->m_pBatch->AddRay(sample, vis.ray);
}

// callback function that is called by the iterator - samples without applying any shading
void SurfaceIntegrator::SampleCallbackFlat(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample)
{
	float3 vDir, d;
	float fPDF;
	Visibility vis;

	SurfaceIntegrator *me= (SurfaceIntegrator *)pThis;

	float3 vLight = me->m_pLight->Sample(vPos, vNormal, iCurSample, &vDir, &fPDF, &vis);

	// no BRDF is used
	float fLambert = DotProduct(vDir, vNormal);
	if (fLambert<0)
		fLambert=0;
	else
		fLambert = 1;

	d = vLight * fLambert / fPDF;

	// add this sample to our batch
	LightSample sample;
	sample.vColour = d;
	sample.vDir = vDir;
	me->GetRenderChannelTarget(me->m_pLight->m_iChannel, sample);

	me->m_pBatch->AddRay(sample, vis.ray);
}

// callback function that is called by the iterator - samples applying lambert shading
void SurfaceIntegrator::SampleCallbackLambert(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample)
{
	float3 vDir, d;
	float fPDF;
	Visibility vis;

	SurfaceIntegrator *me= (SurfaceIntegrator *)pThis;

	float3 vLight = me->m_pLight->Sample(vPos, vNormal, iCurSample, &vDir, &fPDF, &vis);

	// multiply by BRDF (i.e. lambert for now)
	float fLambert = DotProduct(vDir, vNormal);
	if (fLambert<0)
		d = float3(0,0,0);
	else
	{
		d = vLight * fLambert / fPDF;
	}

	// add this sample to our batch
	LightSample sample;
	sample.vColour = d;
	sample.vDir = vDir;
	me->GetRenderChannelTarget(me->m_pLight->m_iChannel, sample);

	me->m_pBatch->AddRay(sample, vis.ray);
}

const float2 &SurfaceIntegrator::GetSample(int i)
{
	i = i%RAND_SEQ_SIZE;
	return m_Sample[i];
}

// initialize random-number sequence
void SurfaceIntegrator::GenerateSequence()
{
	// always use the same seed, so all clients have the same set of samples
	init_genrand(5489UL);

	// TODO: find a better way to generate these samples
	for (int i=0; i<9; i++)
		LDShuffleScrambled2D(1, 32, (float*)&m_Sample[32*i]);
}

// initialize by shooting rays from all lights and storing virtual pointlights for later use
void SurfaceIntegrator::Preprocess()
{
	// initialize sampler
	m_pSampler = new Sampler(m_nSamplerPerTexel);
	m_pSampler->GenerateSamples();

	// initialize filters
//	m_pGaussianFilter = new BoxFilter(0.5,0.5);
	m_pGaussianFilter = new GaussianFilter(2.0f,2.0f,0.8f);
	m_pGaussianFilter->InitializeTable();

	m_p3x3BoxFilter = new BoxFilter(1.5,1.5);
	m_p3x3BoxFilter->InitializeTable();

	// call preprocessing function on all lights
	LightPtrVector *pLightList = m_pGlobalLightList;
	for (int i=0; i<pLightList->Size(); i++)
	{
		Light *pLight = pLightList->At(i);
		pLight->SetCutoffThreshold(m_settings.m_fCutoffThreshold);
		pLight->SetSpatialHierarchy(m_pHierarchy, 1.0f);
		pLight->PreProcess(m_pSampler);
		// if it's a bounce gather light it needs to know about the bounce textures
		if (pLight->GetType() == LUX_LIGHT_TYPE_BOUNCEGATHER)
		{
			((BounceGatherLight*)pLight)->SetBounceTextureData(m_pBounceTextures);
			((BounceGatherLight*)pLight)->SetBouncePass(m_iProgBouncePass);
		}
	}
	// call preprocessing function on all local lights
	pLightList = &m_pLightTree->m_LocalLights;
	for (int i=0; i<pLightList->Size(); i++)
	{
		pLightList->At(i)->SetCutoffThreshold(m_settings.m_fCutoffThreshold);
		pLightList->At(i)->SetSpatialHierarchy(m_pHierarchy, 1.0f);
		pLightList->At(i)->PreProcess(m_pSampler);
	}
	if (m_pDirectionalLight)
	{
		m_pDirectionalLight->PreProcess(m_pSampler);
		m_pDirectionalLight->SetSpatialHierarchy(m_pHierarchy, 1.0f);
	}

	GenerateSequence();

	PrecalculateTriangleSamplePoints();
}

// can potentially cause some glitches
#define TEST_LIGHTS_USING_RAYTRACING

// test one light against all the tris in a chart
bool SurfaceIntegrator::TestLight(Light *pLight, Ut::Vector<NmRawTriangle> *pTriList)
{
	float3 Llight;
	float3 vDir;
	float pdf;

	Visibility vis;

	if (pLight->GetType() == LUX_LIGHT_TYPE_SPOTLIGHT)
		return true;

	float fCutoff = m_settings.m_fCutoffThreshold;

#ifdef TEST_LIGHTS_USING_RAYTRACING
	OcclusionRayBatch batch( m_pHierarchy );
#endif

	// test each tri
	for (int i=0; i<pTriList->Size(); i++)
	{
		NmRawTriangle *pTri = &(*pTriList)[i];

		// number of samples depends on the area of the triangle
		float fArea = Length( CrossProduct(	pTri->vert[1] - pTri->vert[0] , 
											pTri->vert[2] - pTri->vert[0] ));
		fArea *= 0.5f;
		// compare area to texel density
		int nSamples = (int)(m_fTexelDensity * fArea * 0.25f);
		if (nSamples<4) nSamples=4;
		// for area lights, make sure we have a minimum number of samples
		if (pLight->GetType() == LUX_LIGHT_TYPE_AREA && nSamples<pLight->m_nSamples)
			nSamples = pLight->m_nSamples;

		if (nSamples>m_iMaxTriangleSamples) nSamples = m_iMaxTriangleSamples;

		// calculate this triangle's face normal
		float3 e1 = pTri->vert[1] - pTri->vert[0];
		float3 e2 = pTri->vert[2] - pTri->vert[0];
		float3 vFaceNormal = CrossProduct(e1,e2);

		int nLightSamples = pLight->m_nSamples;
		if (nLightSamples<=0)
			nLightSamples = 1;

		// sample each tri a certain amount of times
		for (int iSample=0; iSample<nSamples; iSample++)
		{
			// sample the triangle
			float3 s = m_TriangleSamples[iSample];

			float3 vPos;
			vPos.x = s.x*pTri->vert[0].x + s.y*pTri->vert[1].x + s.z*pTri->vert[2].x;
			vPos.y = s.x*pTri->vert[0].y + s.y*pTri->vert[1].y + s.z*pTri->vert[2].y;
			vPos.z = s.x*pTri->vert[0].z + s.y*pTri->vert[1].z + s.z*pTri->vert[2].z;

			// sample the light
			float3 vCol = pLight->Sample(vPos, iSample%nLightSamples, &vDir, &pdf, &vis);

			// if contribution < epsilon, continue
			if (fabs(vCol.x) < fCutoff && fabs(vCol.y) < fCutoff && fabs(vCol.z) < fCutoff)
				continue;

			// reject light rays that are facing the same way as the triangle
			if (DotProduct(vFaceNormal, vDir) < 0.0f)
				continue;

#ifdef TEST_LIGHTS_USING_RAYTRACING			
			// trace the ray
			LightSample sample;
			sample.vColour = vCol;
			sample.vDir = vCol;
			sample.target = RenderChannel::RGB;

			batch.AddRay(sample, vis.ray);

			// if ray was unoccluded return true
			if (batch.m_bUnoccludedRay)
				return true;
#else
			return true;
#endif
		}
	}

#ifdef TEST_LIGHTS_USING_RAYTRACING			
	// flush any rays left in the batch
	batch.Flush();
	if (batch.m_bUnoccludedRay)
		return true;
#endif

	return false;
}

// precalculate triangle samples for light testing
void SurfaceIntegrator::PrecalculateTriangleSamplePoints()
{
	// initialize 2d samples
	Ut::Vector<float2> m_Samples;
	m_Samples.Resize(m_iMaxTriangleSamples);
	LDShuffleScrambled2D(1, m_iMaxTriangleSamples, &m_Samples[0].x);

	// convert 2d samples to triangle barycentric coordinates
	m_TriangleSamples.Resize(m_iMaxTriangleSamples);
	for (int i=0; i<m_iMaxTriangleSamples; i++)
	{
		// sample the triangle
		float3 s;
		UniformSampleTriangle(m_Samples[i].x, m_Samples[i].y, &s.x, &s.y);
		s.z = 1.0f - (s.x+s.y);

		m_TriangleSamples[i] = s;
	}
}

// calculate which lights are near enough to our bounding box that they need rendering
void SurfaceIntegrator::GetLights(const box3f &bbox, Ut::Vector<NmRawTriangle> *pTriList)
{
//	assert(	bbox.vmin.x<=bbox.vmax.x && bbox.vmin.y<=bbox.vmax.y && bbox.vmin.z<=bbox.vmax.z);

	m_pLightTree->GetLights(bbox);

	if (!pTriList)
		return;

	// cull local lights by tracing a few rays
	LightPtrVector *pLights = &m_pLightTree->m_LightPtrVec;
	for (int i=0; i<pLights->Size();)
	{
		if (TestLight((*pLights)[i], pTriList))
			i++;
		else
		{
			pLights->At(i) = pLights->Back();
			pLights->PopBack();
		}
	}

}

// serialize light tree
ost::Engine& operator << (ost::Engine& ar, LightTree &t)
{
	// save kd nodes
	ar << t.m_nNodes;
	for (int i=0; i<t.m_nNodes; i++)
		ar << t.m_pNodes[i].iFlagDimAndOffset << t.m_pNodes[i].fSplitCoordinate;

	// save mailboxes
	int n = t.m_pMailboxes->Size();
	ar << n;
	for (int i=0; i<n; i++)
		ar << t.m_pMailboxes->At(i).iPrimitive;

	// save actual lights
	ar << t.m_LocalLights;

	return ar;
}

// deserialize light tree
ost::Engine& operator >> (ost::Engine& ar, LightTree &t)
{
	// read kd nodes
	ar >> t.m_nNodes;
	if (t.m_nNodes)
	{
		t.m_pNodes = (KdNode*)AllocAligned(sizeof(KdNode) * t.m_nNodes);
		for (int i=0; i<t.m_nNodes; i++)
			ar >> t.m_pNodes[i].iFlagDimAndOffset >> t.m_pNodes[i].fSplitCoordinate;
	}

	// read mailboxes
	int n;
	ar >> n;
	if (n)
	{
		t.m_pMailboxes = new MailboxVector;
		t.m_pMailboxes->Resize(n);
		for (int i=0; i<n; i++)
			ar >> t.m_pMailboxes->At(i).iPrimitive;
	}

	// read actual lights
	ar >> *(Ut::Vector<BaseObject*>*)&t.m_LocalLights;

	// allocate space for mailboxes
	t.m_pRealMailBoxes = new int[t.m_LocalLights.Size()];

	return ar;
}


// serialize/deserialize
void SurfaceIntegrator::write(ost::Engine &ar)
{
	// write
	ar << m_fMinDistSq << m_bUseFiltering << m_nSamplerPerTexel << m_fTexelDensity << m_bAntiLightLeaking;

	ar << m_pDirectionalLight;
	ar << *m_pGlobalLightList;
	ar << *m_pLightTree;
	ar << *m_pChannels;
	ar << *m_pBounceTextures;

	ar << m_settings.m_fRRThreshold;
	ar << m_settings.m_fCutoffThreshold;
	ar << m_iBounceChannel;
	ar << m_fEpsilon;
	ar << (int&)m_iProgBouncePass;

	((LUX::SpatialHierarchy*)m_pHierarchy)->write(ar);
}
void SurfaceIntegrator::read(ost::Engine &ar)
{
	MEMORY_TRACKER_PUSH("tools.lux.client.integrator");
	// read
	ar >> m_fMinDistSq >> m_bUseFiltering >> m_nSamplerPerTexel >> m_fTexelDensity >> m_bAntiLightLeaking;

	MEMORY_TRACKER_PUSH("tools.lux.client.lights");
	BaseObject *pObj; ar >> pObj; m_pDirectionalLight = (DirectionalLight*)pObj;
	m_pGlobalLightList = new Ut::Vector<Light*>;	ar >> *(Ut::Vector<BaseObject*>*)m_pGlobalLightList;
	m_pLightTree = new LightTree;					ar >> *m_pLightTree;
	m_pChannels = new Ut::Vector<RenderChannel*>;	ar >> *(Ut::Vector<BaseObject*>*)m_pChannels;
	MEMORY_TRACKER_POP("tools.lux.client.lights");
	MEMORY_TRACKER_PUSH("tools.lux.client.bouncetexdata");
	m_pBounceTextures = new BounceTexData;			ar >> *m_pBounceTextures;
	MEMORY_TRACKER_POP("tools.lux.client.bouncetexdata");

	ar >> m_settings.m_fRRThreshold;
	ar >> m_settings.m_fCutoffThreshold;
	ar >> m_iBounceChannel;
	ar >> m_fEpsilon;
	ar >> (int&)m_iProgBouncePass;

	MEMORY_TRACKER_PUSH("tools.lux.client.spatialhierarchy");
	LUX::SpatialHierarchy *pHierarchy = new LUX::SpatialHierarchy;
	pHierarchy->read(ar);
	m_pHierarchy = pHierarchy;
	MEMORY_TRACKER_POP("tools.lux.client.spatialhierarchy");

	m_bInitializedThroughDeserialization = true;
	MEMORY_TRACKER_POP("tools.lux.client.integrator");
}

typedef Ut::Vector<float2> TFloat2Vector;

bool operator <(const LightGroupItem &t, const LightGroupItem &other)
{
	return t.nSamples < other.nSamples;
}
