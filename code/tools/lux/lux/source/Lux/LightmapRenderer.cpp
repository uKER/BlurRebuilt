#include "stdafx.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/Lights/Light.h"
#include "Lux/LightingModel.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationModel.h"
#include "Lux/Sampler.h"
#include "Lux/DebugDraw.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/ImageFilter.h"
#include "Lux/SurfaceIterator.h"
#include "Lux/ShadowRayBatch.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/GlobalIllumination/ClientSide.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/SamplePointIterator.h"
#include "Lux/RasterizedTriIterator.h"
#include "Lux/VtxLighting.h"
#include "Lux/TextureTypes/ITexture.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/TextureTypes/DefaultTexture.h"
#include "Lux/TextureTypes/SHTexture.h"
#include "Lux/TextureTypes/DirectionTexture.h"
#include "Lux/TextureTypes/Basis3Texture.h"
#include "Lux/TextureTypes/BounceTexture.h"
#include "Lux/TextureTypes/FlexibleTexture.h"
#include "Lux/FixChartSeams.h"
#include "Lux/DataTypes.h"
#include "Lux/OutputTypeSelect.h"
#include "System/Time/Time.h"
#include "Lux/LightmapRenderer.h"
#include "Lux/DataTypes/DataTypes.h"

#include <old_nightmare_maths/serializevecmat.h>

#include <sstream>
#include <vector>
#include <list>

#include "tools_common/memory_stream.hpp"
#include "Lux/SHLighting.h"


// this class implements rendering setup code on the client side
using namespace LUX;

extern bool gbBlueInRedHack;

IMPLEMENT_PERSISTENCE(LightmapJob, "LightmapJob");
IMPLEMENT_PERSISTENCE(LightmapResult, "LMapResult");

// add a polygon to a lightmap job
bool LightmapChartJob::AddFace(Aftk::I_SceneMeshNode *pMesh, int polygonIndex, Aftk::Matrix44 *pWorldMatrix, Aftk::Matrix44 *pNormalmatrix, int m_iLMapUVSet, bool bTangents)
{
	Rasterizer r2;

	if (!r2.LoadPolygon(pMesh, polygonIndex, pWorldMatrix, pNormalmatrix, m_iLMapUVSet, bTangents))
		return false;
	m_TriList.PushBack();
	memcpy(&m_TriList.Back(), &r2.m_tri, sizeof(NmRawTriangle));
	return true;
}

/// process a job, returns the result
BaseObject *LightmapJob::Process(SurfaceIntegrator *pIntegrator)
{
	I_JobRenderer *pIter = new LightmapRenderer((LightmapJob*)this);

	// retrieve the results object
	LightmapResult *pResult = (LightmapResult*)pIter->Process(pIntegrator);

	delete pIter;

	return pResult;
	
}

// --- serialisation function for a LightmapResult

void LightmapResult::write(ost::Engine &ar)
{
	int nTextures = m_Textures.Size();
	ar << nTextures;
	for (int i=0; i<nTextures; i++)
		m_Textures[i]->write(ar);
}

void LightmapResult::read(ost::Engine &ar)
{
	int nTextures;
	ar >> nTextures;
	m_Textures.Resize(nTextures);
	for (int i=0; i<nTextures; i++)
	{
		m_Textures[i] = new RGBTexture();
		m_Textures[i]->read(ar);
	}
}

LightmapResult::~LightmapResult()
{
	for (int i=0; i<m_Textures.Size(); i++)
	{
		if (m_Textures[i])
			delete m_Textures[i];
	}
}

/// constructor
LightmapRenderer::LightmapRenderer(LightmapJob *pJob)
{
	m_pJob = pJob;
}

// render a couple of charts and send the result back to the server
BaseObject *LightmapRenderer::Process(SurfaceIntegrator *pIntegrator)
{
	m_pIntegrator = pIntegrator;

	LightmapResult *pResult = new LightmapResult;

	// create a temporary texture, so we can query it's properties
	ITexture *pTex = LUX::DataTypes::CreateSpecializedTexture(
								(LUX::DataTypes::RenderMode)m_pJob->m_iRenderMode,
								m_pJob->m_bBouncePass,
								4, 4,
								m_pIntegrator);

	RasterizedTriIterator *pIterator = new RasterizedTriIterator;
	if (pTex)
		pIterator->SetNeedTangentSpace( pTex->NeedTangentSpace() );
	pIterator->m_pSampler = NULL;
	pIterator->m_nChannels = m_pIntegrator->m_nVertexOutputChannels;
	pIterator->m_bAntiLightLeaking = m_pJob->m_bAntiLightLeaking;
//	if (m_pJob->m_bInstance)
//		pIterator->m_bAntiLightLeaking = false;

	delete pTex;

	Ut::Vector<ITexture*> textures;
	int nCharts = m_pJob->m_Charts.Size();
	textures.Reserve(nCharts);
	pResult->m_Textures.Reserve(nCharts);

	TexelIteratorInput	ti;

	// for each chart in the job
	for (int iChart=0; iChart<nCharts; iChart++)
	{
		LightmapChartJob *pChart = &m_pJob->m_Charts[iChart];

		// get the list of triangles to process
		Rasterizer r2;

		// setup the texture for rendering into
		ITexture *pTex = LUX::DataTypes::CreateSpecializedTexture(
								(LUX::DataTypes::RenderMode)m_pJob->m_iRenderMode,
								m_pJob->m_bBouncePass,
								pChart->m_cX, pChart->m_cY,
								m_pIntegrator);
		textures.PushBack(pTex);

		// rasterize the triangles into a TexelIteratorInput
		ti.Clear();
		ti.SetTexture(pTex);
		ti.m_iChart		= pChart->m_iChart;
		ti.m_iChartWidth	= pChart->m_cX;
		ti.m_iChartHeight = pChart->m_cY;

		for (int i=0; i<pChart->m_TriList.Size(); i++)
			r2.Rasterize(&ti, &pChart->m_TriList[i], 0 );
		ti.m_Tris.Resize(0);
		for (int i=0; i<pChart->m_TriList.Size(); i++)
			r2.Rasterize(&ti, &pChart->m_TriList[i], 1 );

		pIterator->SetTriList(&ti);
		((SD::SpatialHierarchy*)m_pIntegrator->m_pHierarchy)->SetLayerBitArray( m_pJob->m_iLightingPassBits );
		((SD::SpatialHierarchy*)m_pIntegrator->m_pHierarchy)->SetLodLevel( m_pJob->m_iLodLevel );

		// calculate the lighting
		m_pIntegrator->GetLights(pChart->m_bbox, &pChart->m_TriList);
		m_pIntegrator->LightData(pIterator);
	}

	// process the chart seams
	for (int iSeam=0; iSeam<m_pJob->m_Seams.Size(); iSeam++)
	{
		SeamEdge *pEdge = &m_pJob->m_Seams[iSeam];

		float2 c1(m_pJob->m_Charts[pEdge->iChart1].m_cX, m_pJob->m_Charts[pEdge->iChart1].m_cY);
		float2 c2(m_pJob->m_Charts[pEdge->iChart2].m_cX, m_pJob->m_Charts[pEdge->iChart2].m_cY);

		FixChartSeam(
			textures[pEdge->iChart1], pEdge->uv1a * c1, pEdge->uv1b * c1,
			textures[pEdge->iChart2], pEdge->uv2a * c2, pEdge->uv2b * c2,
			pEdge->x1, pEdge->y1, pEdge->w1, pEdge->h1, pEdge->x2, pEdge->y2, pEdge->w2, pEdge->h2);
	}

	// convert the floating point textures we just rendered to RGBA and store in the job result
	for (int iTex=0; iTex<nCharts; iTex++)
	{
		RGBTexture *pRGBTex = new RGBTexture(textures[iTex],
			m_pJob->m_bBouncePass?1.f:m_pJob->m_fOutputGamma, m_pJob->m_bBouncePass?1.f:m_pJob->m_fBrightness);
		pRGBTex->m_iAssociatedChart = m_pJob->m_Charts[iTex].m_iChart;
		pResult->m_Textures.PushBack(pRGBTex);
		delete textures[iTex];
	}

	delete pIterator;

	// return the resulting textures
	return pResult;
}
