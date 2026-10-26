#include "stdafx.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/Lights/Light.h"
#include "Lux/LightingModel.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationModel.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"
#include "Lux/ImageFilter.h"
#include "Lux/SurfaceIterator.h"
#include "Lux/ShadowRayBatch.h"
#include "Lux/GlobalIllumination/ClientSide.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/SamplePointIterator.h"
#include "Lux/RasterizedTriIterator.h"
#include "Lux/VtxLighting.h"
#include "Lux/LightmapRenderer.h"
#include "Lux/DataTypes/DataTypes.h"
#include "Lux/OutputTypeSelect.h"
#include "Lux/TextureTypes/ITexture.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/TextureTypes/DefaultTexture.h"
#include "Lux/TextureTypes/SHTexture.h"
#include "Lux/TextureTypes/DirectionTexture.h"
#include "Lux/TextureTypes/Basis3Texture.h"
#include "Lux/TextureTypes/BounceTexture.h"
#include "System/Time/Time.h"

#include <old_nightmare_maths/serializevecmat.h>

#include <sstream>
#include <vector>
#include <list>

#include "tools_common/memory_stream.hpp"
#include "Lux/SHLighting.h"


// this class implements rendering setup code on the client side
using namespace LUX;

IMPLEMENT_PERSISTENCE(ClientJobRenderer, "CTexIter");

// deconstruction
ClientJobRenderer::~ClientJobRenderer()
{
	if (m_pIntegrator)
		delete m_pIntegrator;
}

// initial client-side update function
void ClientJobRenderer::Initialize()
{
	m_pIntegrator->Preprocess();

	m_pIntegrator->m_pHierarchy->SetEpsilon( m_pIntegrator->m_fEpsilon );

	m_timer = Time::GetCurrentTime();
}

// process a lightmapping job
BaseObject *ClientJobRenderer::ProcessSamplePointJob(SamplePointJob *pJob)
{
	((SD::SpatialHierarchy*)m_pIntegrator->m_pHierarchy)->SetLayerBitArray( pJob->m_iLightingPassBits );
	((SD::SpatialHierarchy*)m_pIntegrator->m_pHierarchy)->SetLodLevel( 0 );

	I_JobRenderer *pIter = LUX::DataTypes::CreateJobRenderer(
			(LUX::DataTypes::RenderMode)pJob->m_iRenderMode,
			LUX::DataTypes::E_SamplePointIterator, pJob->m_bBouncePass, 
			pJob );

	BaseObject *pResult = pIter->Process(m_pIntegrator);

	delete pIter;

	// return the result
	return pResult;
}

// SH lighting job
BaseObject *ClientJobRenderer::ProcessSHLightingJob(SHLightingJob *pJob)
{
	((SD::SpatialHierarchy*)m_pIntegrator->m_pHierarchy)->SetLayerBitArray( pJob->m_iLightingPassBits );
	((SD::SpatialHierarchy*)m_pIntegrator->m_pHierarchy)->SetLodLevel( 0 );

	return pJob->Process(m_pIntegrator);
}

// vtx lighting job
BaseObject *ClientJobRenderer::ProcessVtxLightingJob(VtxLightingJob *pJob)
{
	((SD::SpatialHierarchy*)m_pIntegrator->m_pHierarchy)->SetLayerBitArray( pJob->m_iLightingPassBits );
	((SD::SpatialHierarchy*)m_pIntegrator->m_pHierarchy)->SetLodLevel( 0 );

	return pJob->Process(m_pIntegrator);
}

// client-side job processing function
// returns result
BaseObject *ClientJobRenderer::Process(BaseObject *pInJob)
{
#ifndef COMPILER_VC
	LOGDEBUG (g_NetLog, "clientjobrenderer: process\n");
#endif

	// is it a samplepoint rendering job?
	if (!strcmp(pInJob->getPersistenceID(), "SPointJob"))
		return ProcessSamplePointJob((SamplePointJob*)pInJob);
	// is it an SH lighting job? (light an object e.g. tree by attaching an SH node to it)
	if (!strcmp(pInJob->getPersistenceID(), "SHJob"))
		return ProcessSHLightingJob((SHLightingJob*)pInJob);
	// is it a vertex lighting job?
	if (!strcmp(pInJob->getPersistenceID(), "VtxJob"))
		return ProcessVtxLightingJob((VtxLightingJob*)pInJob);

	// it must be a lightmapping job
	assert(!strcmp(pInJob->getPersistenceID(), "LightmapJob"));
	return ProcessLightmapJob((LightmapJob*)pInJob);
}

// render a couple of charts and send the result back to the server
BaseObject *ClientJobRenderer::ProcessLightmapJob(LightmapJob *pJob)
{
	return pJob->Process(m_pIntegrator);
}

// do cleanup. simply prints out some stats
void ClientJobRenderer::CleanUp()
{
	// print out timings
	char buffer[512];
	float time = (float)(Time::GetCurrentTime()-m_timer) / 1000000.0f;
	int nRays = m_pIntegrator->m_pHierarchy->GetNumRaysCast();

    LOGDEBUG( g_Log, "%5.2fM rays cast in %5.2f secs - %5.2fK rays/sec\n", (float)nRays/1000000.0f, time, (float)nRays/(float)(time*1000.0f));
}

// serialize/deserialize
void ClientJobRenderer::write(ost::Engine &ar)
{
	ar << m_iBiggestArea << m_fDensity;
	ar << m_pIntegrator;
}
void ClientJobRenderer::read(ost::Engine &ar)
{
	MEMORY_TRACKER_PUSH("tools.lux.client.initialupdate");

	ar >> m_iBiggestArea >> m_fDensity;
	BaseObject *pObj;
	ar >> pObj;
	m_pIntegrator = (SurfaceIntegrator*)pObj;

	MEMORY_TRACKER_POP("tools.lux.client.initialupdate");
}

