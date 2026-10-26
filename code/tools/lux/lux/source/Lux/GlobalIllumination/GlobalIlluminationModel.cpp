#include <stdafx.h>

#include "Lux/Lights/Light.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/Lights/PointLight.h"
#include "Lux/RenderChannel.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/LightingModel.h"
#include "Lux/GlobalIllumination/GlobalIlluminationModel.h"
#include "Lux/GlobalIllumination/GlobalIlluminationNet.h"
#include "Lux/GlobalIllumination/ProgressiveBounceLighting.h"
#include "Lux/Lux.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/SceneUnwrapper.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/LightmapPacking/LightmapPacking.h"
#include "Lux/RasterizedTriIterator.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/Lights/Light.h"
#include "Lux/Lights/HemisphereLight2.h"
#include "Lux/Lights/BounceGatherLight.h"
#include "CServ/ClusterServHeaders.h"
#include "Lux/GlobalIllumination/ClientSide.h"
#include "Lux/Instantiate.h"
#include "Lux/SamplePoints.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/SamplePointIterator.h"
#include "lux_common/LightingPass.h"
#include "Lux/DebugCode.h"
#include "Lux/Common.h"
#include "Lux/SHLighting.h"
#include "Lux/VtxLighting.h"
#include "Lux/VertexIterator2.h"
#include "Lux/LightmapRenderer.h"
#include "System/Memory.h"
#include <system\misc\progress.h>

#include <algorithm>

using namespace LUX;
using namespace std;

SuperGroupBuilder *gpSuperGroups;

extern int GetUVSetIndexFromMeshNode(Aftk::I_SceneMeshNode *pMesh);
std::string g_clientExePath = "";	// for windows
std::string g_clientElfPath = "";	// for G5
bool g_useNetworkClients = false;
bool g_silentExit = false;

int giOnlyLightChart = -1;
int giOnlyLightFace  = -1;

char gpszOnlyLightObject[] = "";
int giLightOnlyNode = -1;

bool GlobalIlluminationModel::Light(
	LUX::SpatialHierarchy* pHierachy, 
	const LightingModelProperties* pProperties,
	Lux* pLux,
	Aftk::C_Scene* pAftkScene,
	Progress::I_ProgressBar *pProgress)
{
	SetDAZ();

	// There's to spatial representation of the scene!
	if (pHierachy == 0) return false;
	// There's nothing to light!
	if (pAftkScene == 0) return false;
	// Incorrect lighting model parameters have been supplied! Abort.
	if (pProperties->GetType() != LUX_MODEL_TYPE_GLOBALILLUMINATION) return false;

	m_pHierarchy = pHierachy;
	m_pAftkScene = pAftkScene;
	m_pLux = pLux;
	m_pProperties = pProperties;
	m_pGIProperties = (GlobalIlluminationProperties*)m_pProperties;

	// initialize the multi progress bar
	Progress::UpdateMulti multi(pProgress, "Lighting");
	m_pUnwrapProgress = multi.AddChild(780.f);
	for (int i=0; i<=m_pGIProperties->m_nProgressiveBouncePasses; i++)
	{
		if (i == 0)
			m_RenderProgress.PushBack(multi.AddChild(3000.f));
		else
			m_RenderProgress.PushBack(multi.AddChild(300.f));
	}
	m_pPackingProgress = multi.AddChild(240.f);
	m_pValidateProgress = multi.AddChild(120.f);

	// render the lighting
	return Render();
}


Ut::Vector<CTextureChart*> *g_pMyChartList;

struct TCItem
{
	int idx;
	bool operator < (const TCItem &b) const
	{
		CTextureChart *pA = g_pMyChartList->At(idx);
		CTextureChart *pB = g_pMyChartList->At(b.idx);
		int d = pA->m_pMesh->GetUniqueId() - pB->m_pMesh->GetUniqueId();
		if (d != 0)
			return d<0;
		d = pA->m_iFace - pB->m_iFace;
		return d<0;
	}
};

void SortTextureAndChartList(Ut::Vector<CTextureChart*> &myChartList, Ut::Vector<RGBTexture*> &myTexList)
{
	// setup for sorting
	g_pMyChartList = &myChartList;

	Ut::Vector<TCItem> sortlist;
	int n = myChartList.Size();
	sortlist.Resize(n);
	for (int i=0; i<n; i++)
		sortlist[i].idx = i;

	// sort charts by their mesh id and face id
	std::sort(&sortlist[0], &sortlist[0]+sortlist.Size());

	// copy texture + chart pointers
	Ut::Vector<CTextureChart*> tmpChartList = myChartList;
	Ut::Vector<RGBTexture*> tmpTexList = myTexList;

	// copy back and remap
	for (int i=0; i<n; i++)
	{
		int x = sortlist[i].idx;
		myChartList[i] = tmpChartList[ x ];
		myTexList[i] = tmpTexList[ x ];
	}
}


// function for sorting chart   groups by lighting pass
bool SeamGroupLightPassGreater( ChartSeamGroup group1, ChartSeamGroup group2)
{
	// sort lowest lighting pass first
	return group1.iLightingPass < group2.iLightingPass;
}

// generate one lightmapping job from our chartlist
BaseObject *GlobalIlluminationModel::GenerateJobStatic(void *pContext, int iChart)
{
	GlobalIlluminationModel *pThis = (GlobalIlluminationModel*)pContext;
	return pThis->GenerateJob(iChart);
}

// get UV coordinate given a mesh, triangle and edge no.
float2 GetTextureCoord(Aftk::C_SceneMeshNode *pMesh, int iTriangle, int iEdge, int iUVSet)
{
	int idx = pMesh->GetPolygon(iTriangle).GetPolygonVertex(iEdge).GetTextureCoordinateIndex(iUVSet);

	Aftk::TextureCoordinate4 uv = pMesh->GetTextureCoordinate(iUVSet, idx);

	return float2(uv.x, uv.y);
}

// get edge vertex positions
void GetEdge(const Aftk::C_SceneMeshNode *pMesh, int iTriangle, int iEdge, float3 &vA, float3 &vB)
{
	const Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(iTriangle);

	vA = pMesh->GetEvaluatedPolygonVertexPosition(pPoly->GetPolygonVertexIndex(iEdge));
	vB = pMesh->GetEvaluatedPolygonVertexPosition(pPoly->GetPolygonVertexIndex((iEdge+1)%3));
}

// returns the LOD level of the given node. returns 0 if the node is not a LOD
int LuxGetModelLodLevel(Aftk::C_SceneNode *pNode)
{
	// check for LODs
	Aftk::C_SceneNode *pParent = pNode->GetParentPtr();
	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		Aftk::C_SceneLodGroupNode *pLG = (Aftk::C_SceneLodGroupNode*)pParent;

		for (int i=0; i<(int)pLG->GetNumLodLevels(); i++)
		{
			Aftk::C_GroupLodLevelInfo &info = pLG->GetLodLevelInfo(i);
			if (info.GetSceneNodePtr() == pNode)
				return i;
		}
	}
	return 0;
}

// generate one lightmapping job from our chartlist
BaseObject *GlobalIlluminationModel::GenerateJob(int iID)
{
	// if it's a sample point job, generate it in the SamplePoints class
	int i = iID & ~JOB_INDEX_MASK;
	if (i == SAMPLEPOINT_JOB)
		return GetLux()->GetSamplePointsPtr()->GenerateJob(iID);

	// if it's an SH lighting job, generate it in the SHLighting class
	if (i == SHLIGHTING_JOB)
		return m_pSHLighting->GenerateJob(iID);

	// if it's a vertex lighting job, generate it in the VtxLighting class
	if (i == VTXLIGHTING_JOB)
		return m_pVtxLighting->GenerateJob(iID);

	// generate a lightmap job
	// TODO: move the rest of this function into it's own class and stick it in LightmapRenderer.cpp
	// loads of stuff like the unwrapper, lightmap packer, etc. could live there as well.
	SceneUnwrapper &unwrapper = *m_pUnwrapper;
	ChartSeamGroup *pSeamGroup = &unwrapper.m_SeamGroups[iID];

	Aftk::Matrix44 worldMatrix;
	Matrix44 temp, normalTransform;
	Aftk::C_SceneNode *pMtxMesh = NULL;

	Ut::UtMap<int,int> chartIndexMap;

	LightmapJob *pJob = new LightmapJob;
	pJob->m_bInstance = false;

	int iChart = pSeamGroup->iChart;
	int iFirstChart = iChart;
	while (1)
	{
		CTextureChart *pChart = unwrapper.m_ChartList[iChart];
		assert(pChart);

		pJob->m_Charts.PushBack();
		if (pChart->IsInstance())
			pJob->m_bInstance = true;
		LightmapChartJob *pCJob = &pJob->m_Charts.Back();

		int iOrgw = unwrapper.m_ChartList[iChart]->m_iTexWidth;
		int iOrgh = unwrapper.m_ChartList[iChart]->m_iTexHeight;
		if (m_bBouncePass)
		{
			iOrgw /= 4;
			iOrgh /= 4;
			if (iOrgw == 0) iOrgw = 1;
			if (iOrgh == 0) iOrgh = 1;
		}

		// get world matrix if we haven't already
		if (pMtxMesh != pChart->m_pMatrixNode)
		{
			pMtxMesh = pChart->m_pMatrixNode;
			worldMatrix = pChart->m_pMatrixNode->GetWorldMatrix();

			// calculate matrix to use to transform normals
			temp = worldMatrix;
			Transpose(&temp);
			Invert4x4(&normalTransform, temp);
		}

		// iterate through this chart's polygons
		int iFace = pChart->m_iFace;
		int iRT = 0;
		pCJob->m_cX = iOrgw;
		pCJob->m_cY = iOrgh;
		pCJob->m_iChart = iChart;
		pCJob->m_bbox = pChart->m_bbox;

		// for base instances, add an empty chart. nothing will be rendered but the UVs will be remapped
		if (pChart && !DoesBaseInstanceNeedExporting(pChart->m_pMatrixNode))
		{
			pChart->m_bBaseInstanceDoNotExport = true;
			goto next;
		}

		while (iFace!=-1)
		{
			if (giOnlyLightFace == -1 || giOnlyLightFace == iFace)
			{
				// add this face
				if (pCJob->AddFace(pChart->m_pMatrixNode, iFace, &worldMatrix, &normalTransform, GetUVSetIndexFromMeshNode(pChart->m_pMesh), m_pGIProperties->NeedTangentSpace()))
					chartIndexMap[iFace] = pJob->m_Charts.Size()-1;
			}

			// get next
			iFace = ((LMPolyAttributes*)pChart->m_pMesh->GetPolygon(iFace).GetAttachDataPtr())->iChartLink;
		}

		// add the job to the job scheduler (except if we didn't rasterize any tris)
		if (!pCJob->m_TriList.Size())
		{
			S_WARN( g_Log, "skipping chart on node '" << pChart->m_pMatrixNode->GetPath() << "' because all tris had bad normals.\n");
			pJob->m_Charts.PopBack();
			goto next;
		}

		// scale all textures to 1/4 size
		if (m_bBouncePass)
		{
			for (int i=0; i<pCJob->m_TriList.Size(); i++)
			{
				pCJob->m_TriList[i].m_x /= 4;
				pCJob->m_TriList[i].m_y /= 4;
				pCJob->m_TriList[i].m_cx /= 4;
				pCJob->m_TriList[i].m_cy /= 4;
				if (pCJob->m_TriList[i].m_cx == 0)
					pCJob->m_TriList[i].m_cx = 1;
				if (pCJob->m_TriList[i].m_cy == 0)
					pCJob->m_TriList[i].m_cy = 1;
			}
		}

	next:
		if (pChart->m_bLastInChartSeamGroup)
			break;
		iChart++;
	}

	// get the mesh for this group
	Aftk::C_SceneMeshNode *m_pMesh = unwrapper.m_ChartList[pSeamGroup->iChart]->m_pMesh;

	// find lightmap UV set
	int iLMapUVSet = 0;
	for (int i=0; i<m_pMesh->GetNumTextureCoordinateSets(); i++)
	{
		if ( m_pMesh->GetTextureCoordinateSetName(i) == LIGHT_MAP_UV_SET_NAME )
		{
			iLMapUVSet = i;
			break;
		}
	}

	// add the seams to the lighting job
	// ignore seams when rendering bounce passes
	if (m_pGIProperties->m_bFixSeams && !m_bBouncePass)
	{
		for (Ut::Vector<ChartSeam>::Iterator i=pSeamGroup->seams.Begin(); i != pSeamGroup->seams.End(); ++i)
		{
			SeamEdge seam;

			if (unwrapper.m_ChartList[iChart]->IsInstance() || unwrapper.m_ChartList[iChart]->m_bReferenceGeometry)
			{
				// for base instances, there is only one chart per job. so point both chart indices to chart 0 of the job
				assert(pJob->m_Charts.Size() == 1);

				seam.iChart1 = 0;
				seam.iChart2 = 0;
			}
			else
			{
				// for normal mesh...

				// check that both the charts were added to the job
				if (chartIndexMap.Find(i->iTriangle1) == chartIndexMap.End() ||
					chartIndexMap.Find(i->iTriangle2) == chartIndexMap.End())
					continue;

				seam.iChart1 = chartIndexMap[i->iTriangle1];
				seam.iChart2 = chartIndexMap[i->iTriangle2];
			}

			// find the index of the edge on the other triangle
			int iEdge1 = i->iEdge1;
			int iEdge2;
			float3 vEdgeA[2], vEdgeB[2];
			GetEdge(m_pMesh, i->iTriangle1, iEdge1, vEdgeA[0], vEdgeA[1]);
			for (iEdge2=0; iEdge2<3; iEdge2++)
			{
				GetEdge(m_pMesh, i->iTriangle2, iEdge2, vEdgeB[0], vEdgeB[1]);
				if (Length(vEdgeA[0] - vEdgeB[1]) < 0.0001f && Length(vEdgeB[0] - vEdgeA[1]) < 0.0001f)
					break;
			}
			if (iEdge2 == 3)
			{
				LOGDEBUG( LUX::g_Log, "problem on SeamEdge: opposite triangle adjacency information is weird.\n");
				continue;
			}

			// get the UVs of the edges
			seam.uv1a = GetTextureCoord(m_pMesh, i->iTriangle1, iEdge1, iLMapUVSet);
			seam.uv1b = GetTextureCoord(m_pMesh, i->iTriangle1, (iEdge1+1)%3, iLMapUVSet);
			seam.uv2b = GetTextureCoord(m_pMesh, i->iTriangle2, iEdge2, iLMapUVSet);
			seam.uv2a = GetTextureCoord(m_pMesh, i->iTriangle2, (iEdge2+1)%3, iLMapUVSet);
			seam.x1 = POLY_DATA(i->iTriangle1)->m_x;  seam.x2 = POLY_DATA(i->iTriangle2)->m_x;
			seam.y1 = POLY_DATA(i->iTriangle1)->m_y;  seam.y2 = POLY_DATA(i->iTriangle2)->m_y;
			seam.w1 = POLY_DATA(i->iTriangle1)->m_cx; seam.w2 = POLY_DATA(i->iTriangle2)->m_cx;
			seam.h1 = POLY_DATA(i->iTriangle1)->m_cy; seam.h2 = POLY_DATA(i->iTriangle2)->m_cy;

			pJob->m_Seams.PushBack(seam);
		}
	}

	pJob->m_iLightingPassBits = m_pLux->GetLightingPassBits(pSeamGroup->iLightingPass);
	pJob->m_iRenderMode = m_pGIProperties->GetOutputType();

	pJob->m_fOutputGamma = m_pGIProperties->m_fOutputGamma;
	pJob->m_fBrightness = m_pGIProperties->m_fBrightness;
	pJob->m_bAlphaScaling = m_pGIProperties->m_bAlphaScaling;
	pJob->m_bSwizzleSH = m_pGIProperties->m_bPrescaleSH;
	pJob->m_iLodLevel = LuxGetModelLodLevel(unwrapper.m_ChartList[pSeamGroup->iChart]->m_pMatrixNode);
	pJob->m_bBouncePass = m_bBouncePass;
	AftkUt::C_NodeFlagParser parser(*unwrapper.m_ChartList[pSeamGroup->iChart]->m_pMatrixNode);
	// if the chart has anti-light-leaking settings then use that, otherwise use the scene default
	bool bHaveAntiLightLeakingActorParameter = false;
	std::string sAntiLightLeaking;
	bHaveAntiLightLeakingActorParameter = parser.GetParameterValue("AntiLightLeaking", sAntiLightLeaking);
	if (bHaveAntiLightLeakingActorParameter)
		pJob->m_bAntiLightLeaking = atoi(sAntiLightLeaking.c_str())?true:false;
	else
		pJob->m_bAntiLightLeaking = m_pGIProperties->GetAntiLightLeaking();

	// TODO: this is obsolete. remove.
	if (m_bBouncePass)
		pJob->m_iTextureType = OUTPUT_TYPE_BOUNCE;
	else
		pJob->m_iTextureType = m_pGIProperties->GetOutputType();

	return pJob;
}

// my custom weld visitor which doesn't weld stuff that has "DoNotLight" or "SHLighting" on.
struct C_MyWeldVisitor
{
	static bool IsInteresting(Aftk::C_SceneNode* pSceneNode) 
	{
		return true;
	}

	Aftk::C_Status Visit(
		Aftk::C_SceneNode* pNode,
		Aftk::C_MeshWeldMemoryManager& memoryManager)
	{
		if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
			return Aftk::C_Status::E_Status_Success;

		switch(GetNodeLightingMode(m_pProperties, pNode))
		{
		case LMode_DoNotLight:
		case LMode_SH:
			return Aftk::C_Status::E_Status_Success;
		case LMode_Lightmap:
		case LMode_VtxColour:
			Aftk::C_MeshWeld::Weld(
				memoryManager,
				(Aftk::C_SceneMeshNode*)pNode,
				AFTK_EPSILON,
				AFTK_EPSILON,
				AFTK_EPSILON,
				AFTK_EPSILON);
			return Aftk::C_Status::E_Status_Success;
		default:
		case LMode_NotSet:
			assert(0);
			return Aftk::C_Status::E_Status_Success;
		}
	}

	const LightingModelProperties *m_pProperties;
};

// unwrap scene, render lighting into lightmaps, write out lightmaps
bool GlobalIlluminationModel::Render()
{
	SceneUnwrapper unwrapper;
	std::string lmFileName;
	bool bCancelling = false;
	int nCharts = 0;

	RunServerStruct net;
	Progress::UpdateStepped packprog(m_pPackingProgress, m_pLux->GetNumPasses(), "Packing");

	// setup network info structure 
	net.bUseNetworkClients = g_useNetworkClients;
	net.clientElfPath = g_clientElfPath;
	net.clientExePath = g_clientExePath;
	net.pLux = m_pLux;

	InitServerPaths(net);

	m_pUnwrapper = &unwrapper;

	// Chart the entire scene
	m_pLux->DoProgressCallback(0,"Unwrapping");
	LOGINFO( g_Log, "Unwrapping\n");

	// Unwrap UVs
	unwrapper.m_fMaxSize = m_pGIProperties->GetDefaultLightmapSize()/2;

	unwrapper.m_fScale = m_pGIProperties->GetTexelDensity();
	unwrapper.m_fTexelDensityRatio[0] = m_pGIProperties->GetTexelDensityRatio(0);
	unwrapper.m_fTexelDensityRatio[1] = m_pGIProperties->GetTexelDensityRatio(1);
	unwrapper.m_fTexelDensityRatio[2] = m_pGIProperties->GetTexelDensityRatio(2);
	unwrapper.m_fTexelDensityRatio[3] = m_pGIProperties->GetTexelDensityRatio(3);
	unwrapper.m_fTexelDensityRatio[4] = m_pGIProperties->GetTexelDensityRatio(4);
	unwrapper.SetDoubleSidedPolyflagMask( m_pGIProperties->GetDoubleSidedPolyflag() );
	unwrapper.SetDoNotLightPolyflagMask( m_pGIProperties->GetDoNotLightPolyflag() );
	unwrapper.SetLowResLightmapPolyflagMask( m_pGIProperties->GetLowResLightmapsPolyflag() );
	unwrapper.SetLowResLightmapBits( m_pGIProperties->GetLowResLightmapsBits() );
	unwrapper.SetCalculateTangentSpace( m_pGIProperties->NeedTangentSpace() );
	unwrapper.SetSeamSettings( m_pGIProperties->m_bFixSeams, m_pGIProperties->m_bFixAllSeams );

	// optionally convert all instances to mesh
	if (m_pGIProperties->m_iInstanceMode == INSTANCE_MODE_INSTANTIATE)
		ConvertRemappedInstancesToMesh(m_pAftkScene);

	if (!unwrapper.Unwrap(m_pAftkScene, m_pProperties, m_pLux->GetLightingPassVector(), m_pGIProperties->m_nProgressiveBouncePasses>0, m_pUnwrapProgress))
	{
		//Display an error here. todo.
		return false;
	}

	m_pAftkScene->Validate();

	switch (m_pGIProperties->m_iInstanceMode)
	{
	case INSTANCE_MODE_UV_OFFSET:
		PackInstancedMeshNodes((Lux*)m_pLux, &unwrapper, m_pProperties, m_pLux->GetLightingPassVector(), false);	// re-pack instanced meshnodes
		break;
	case INSTANCE_MODE_DELETE:
        m_pHierarchy->SetIgnoreInstances(true);					// ignore instances
		break;
	case INSTANCE_MODE_INSTANTIATE:
		PackInstancedMeshNodes((Lux*)m_pLux, &unwrapper, m_pProperties, m_pLux->GetLightingPassVector(), true);	// instantiate instanced meshnodes
		break;
	}

	// sort seam groups by lighting pass (route)
	if (unwrapper.m_SeamGroups.Size() > 0)
		sort(&unwrapper.m_SeamGroups[0], &unwrapper.m_SeamGroups[0] + unwrapper.m_SeamGroups.Size(), SeamGroupLightPassGreater);

	LOGINFO( g_Log, "Unwrapping done\n");

	// create a texture for every chart
	LOGINFO( g_Log, "Generating %d charts...\n",unwrapper.m_ChartList.Size());

	m_pGIProperties->AddAreaLightsFromScene(m_pAftkScene, &unwrapper.m_ChartList);

	InitializeLights();

	// find largest texture extents (we use one texture and resize it to fit the various chart sizes)
	TRGBTextureList texList;
	int iBiggestArea = 0;
	int nBounceCharts = 0;
	BounceTexData *pBounceTexData = new BounceTexData;
	m_pIntegrator->SetBounceTextures(pBounceTexData);

	for (int i=0; i<unwrapper.m_ChartList.Size(); i++)
	{
		if (!unwrapper.m_ChartList[i])
			continue;
		assert(unwrapper.m_ChartList[i]->m_iTexHeight > 0);
		int iArea = unwrapper.m_ChartList[i]->m_iTexWidth * unwrapper.m_ChartList[i]->m_iTexHeight;
		if (iArea > iBiggestArea) iBiggestArea = iArea;
	}

	MEMORY_TRACKER_PUSH("tools.lux.server.integrator.preprocess");
	// render SH sample points
	m_pIntegrator->Preprocess();
	MEMORY_TRACKER_POP("tools.lux.server.integrator.preprocess");

	// build the light tree, which now contains all lights
	m_pLightTree->Build();

	m_pIntegrator->m_pHierarchy->SetEpsilon( m_pGIProperties->GetEpsilon() );

	// initialize a list of SH lighting jobs
	m_pSHLighting = new SHLighting( m_pGIProperties->GetOutputType(), m_pGIProperties->m_fBrightness, m_pGIProperties->OutputGamma(), m_pLux->GetLightingPassBits(0) );

	// initialize vertex lighting job manager
	m_pVtxLighting = new VtxLighting( m_pAftkScene, m_pGIProperties->m_fBrightness, m_pGIProperties->OutputGamma(), m_pLux );
	m_pVtxLighting->SetRenderMode( m_pGIProperties->GetOutputType() );

	m_pIntegrator->m_iProgBouncePass = PROGBOUNCEPASS_FINALGATHER;

	RenderVertexAndSHLighting();

	// generate bounce texture data
	if (m_pGIProperties->m_nProgressiveBouncePasses > 0)
	{
		LOGINFO( g_Log, "Setting up bounce lighting...\n");
		m_pLux->DoProgressCallback(0,"Setting up bounce lighting...");
		pBounceTexData->Generate(m_pHierarchy, &unwrapper, m_pVtxLighting, m_pGIProperties->m_bColouredBounceLighting);
	}

	unwrapper.m_ChartMatrixList.Destroy();

	Memory::Tracker::Instance().DumpNodeStats(false, true);

	for (int iProgBouncePass=m_pGIProperties->m_nProgressiveBouncePasses; iProgBouncePass>=0; iProgBouncePass--)
	{
		m_bBouncePass = (iProgBouncePass > 0);
		if (m_pGIProperties->m_nProgressiveBouncePasses>0)
		{
			S_INFO( LUX::g_Log, 
				"\n"
				"Bounce pass " << m_pGIProperties->m_nProgressiveBouncePasses+1-iProgBouncePass << ":\n"
				"-------------\n"
				);
		}

		MEMORY_TRACKER_PUSH("tools.lux.server.initialupdate");

		SamplePoints *pSamplePoints = GetLux()->GetSamplePointsPtr();
		if (pSamplePoints)
		{
			pSamplePoints->SetBrightness( m_pGIProperties->m_fBrightness );
			pSamplePoints->SetGamma( m_pGIProperties->m_fOutputGamma );
			pSamplePoints->SetRenderMode( m_pGIProperties->GetOutputType(), m_bBouncePass, m_pLux->GetLightingPassBits(0));
		}

		// prepare the initial update structure to pass to the client
		ClientJobRenderer *pInitialUpdate = new ClientJobRenderer;
		pInitialUpdate->m_pIntegrator	= m_pIntegrator;
		pInitialUpdate->m_iBiggestArea	= iBiggestArea;
		pInitialUpdate->m_fDensity		= m_pGIProperties->GetTexelDensity();

		m_pVtxLighting->SetBouncePass(m_bBouncePass);

		if (iProgBouncePass == m_pGIProperties->m_nProgressiveBouncePasses)
			m_pIntegrator->m_iProgBouncePass = PROGBOUNCEPASS_SEED;
		else if (iProgBouncePass == 0)
			m_pIntegrator->m_iProgBouncePass = PROGBOUNCEPASS_FINALGATHER;
		else
			m_pIntegrator->m_iProgBouncePass = PROGBOUNCEPASS_BOUNCE;

		// start the server, pass the surface integrator class as the setup message
		LOGINFO( g_Log, "Preparing to start server...\n");
		m_pLux->DoProgressCallback(0,"Starting server");
		net.pServer = CreateServer( pInitialUpdate );

		// set spatial hierarchy to lighting pass one
		int nLayers = m_pAftkScene->GetNumLayers();
		for (int i=0; i<nLayers; i++)
		{
			if (!GetLux()->GetLightingPassVector()->At(0)->DoesLayerCastShadows( m_pAftkScene->GetLayerPtr(i)->GetName() ))
				((SD::SpatialHierarchy*)m_pHierarchy)->SetLayer( i, false );
			else
				((SD::SpatialHierarchy*)m_pHierarchy)->SetLayer( i, true );
		}

		// delete spatial hierarchy and stuff now to free up memory
		if (m_bBouncePass)
			pInitialUpdate->m_pIntegrator = NULL; // prevent it from destroying the integrator if there are more passes to go
		delete pInitialUpdate;

		MEMORY_TRACKER_POP("tools.lux.server.initialupdate");

		// prepare jobs
		LOGINFO( g_Log, "Preparing jobs...\n");

		m_pLux->DoProgressCallback(0, "Preparing jobs");

		JobScheduler *pScheduler = net.pServer->GetJobSchedulerPtr();

		pScheduler->SetGenerateJobCallbackFunction( &GenerateJobStatic, this );

		// iterate through the chart seam groups, add a job for each group
		int nJobsSent = 0;
		int nSeamGroups = unwrapper.m_SeamGroups.Size();
		for (int iGroup=0; iGroup<nSeamGroups; iGroup++)
		{
			// calculate cost of this group, which is the sum of the area of all the charts in the group
			// copy the charts in this group into our list
			int iChart = unwrapper.m_SeamGroups[iGroup].iChart;
			int iCost = 0;
			bool bSkip = true;
			bool bAnyRemap = false;
			while (1)
			{
				CTextureChart *pChart = unwrapper.m_ChartList[iChart];

				if (iChart == giOnlyLightChart)
					bSkip = false;

				bAnyRemap |= pChart->m_bRemap;

				iCost += pChart->m_iTexWidth * pChart->m_iTexHeight;
				if (unwrapper.m_ChartList[iChart]->m_bLastInChartSeamGroup)
					break;
				iChart++;
			}


			// add a job for this seam group
			if (giOnlyLightChart == -1 || !bSkip)
			{
				if (!m_bBouncePass || bAnyRemap)
					pScheduler->AddJob(iGroup, iCost);
			}
		}

		if (!m_bBouncePass)
		{
			// add jobs for sample points
			if (GetLux()->GetSamplePointsPtr())
				GetLux()->GetSamplePointsPtr()->AddJobs( pScheduler );

			// add jobs for SH lighting
			if (m_pSHLighting)
				m_pSHLighting->AddJobs( pScheduler );
		}
		// add jobs for vertex lighting
		if (m_pVtxLighting)
			m_pVtxLighting->AddJobs( pScheduler );

		printf("\n");

		if (!RunServer(net, m_RenderProgress[iProgBouncePass]))
			goto cancel;

		// collect the resulting textures to be used for the next pass
		if (m_bBouncePass)
		{
			nCharts = unwrapper.m_ChartList.Size();

			// retrieve the results
			JobScheduler *pScheduler = net.pServer->GetJobSchedulerPtr();
			for (int i=0; i<pScheduler->GetNumJobs(); i++)
			{
				BaseObject *pResult = pScheduler->GetResult(i);
				if (!pResult)
					continue;

				if (strcmp(pResult->getPersistenceID(), "LMapResult") == 0)
				{
					// it's a lightmap job result
					LightmapResult *pLMapResult = (LightmapResult*)pResult;
					// retrieve the RGB textures and take add them to the existing bounce textures
					for (int i=0; i<pLMapResult->m_Textures.Size(); i++)
					{
						RGBTexture *pTex = pLMapResult->m_Textures[i];
						if (!pBounceTexData->m_pBounceTexList->At(pTex->m_iAssociatedChart).m_pTex)
						{
							pBounceTexData->m_pBounceTexList->At(pTex->m_iAssociatedChart).m_pTex = pTex;
							pLMapResult->m_Textures[i] = NULL;
						}
						else
							pBounceTexData->m_pBounceTexList->At(pTex->m_iAssociatedChart).AddTexture(pTex);
						if (pTex->FoundQNAN())
						{
							CTextureChart *pChart = unwrapper.m_ChartList[pTex->m_iAssociatedChart];
							LOGDEBUG( g_Log, "QNAN found on lightmap for %s poly %d\n", 
								pChart->m_pMatrixNode->GetPath().c_str(),
								pChart->m_iFace);
						}
					}
					// delete the job result and it's textures
					delete pResult;
				}
				else if (strcmp(pResult->getPersistenceID(), "VtxResult") == 0)
				{
					// it's a vertex lighting job result
					VtxLightingResult *pVtxResult= (VtxLightingResult*)pResult;

					// copy the vertex colours into pBounceTexData
					VtxLighting::Item *pItem = m_pVtxLighting->GetItem(pVtxResult->m_iItem);
					bool bFoundQNAN = false;
					pBounceTexData->SetVertexData(pVtxResult, pItem->pNode->GetNumPolygons(0)*3, &bFoundQNAN);
					if (bFoundQNAN)
					{
						LOGDEBUG( g_Log, "Found QNAN on vertex data for node %s", pItem->pNode->GetPath().c_str());
					}
				}
				else
				{
					assert(0); // shouldn't happen
				}
			}

			LOGINFO( g_Log, "Waiting for server to finish...\n");
			net.pServer->join();
			delete net.pServer;

			if (net.pClient)
			{
				LOGINFO( g_Log, "Waiting for local client to finish...\n");
				net.pClient->join(); // TODO: add a timeout for this
				delete net.pClient;
			}
		}
	}

	Memory::Tracker::Instance().DumpNodeStats(false, true);

	// delete bounce textures
	delete pBounceTexData;

	// initialize an empty texlist
	nCharts = unwrapper.m_ChartList.Size();
	texList.Resize(nCharts);
	for (int i=0; i<nCharts; i++) texList[i] = NULL;

	// retrieve the results
	{
		JobScheduler *pScheduler = net.pServer->GetJobSchedulerPtr();
		for (int i=0; i<pScheduler->GetNumJobs(); i++)
		{
			BaseObject *pResult = pScheduler->GetResult(i);
			if (!pResult)
				continue;
			if (!strcmp(pResult->getPersistenceID(), "LMapResult"))
			{
				// it's a lightmap job result
				LightmapResult *pLMapResult = (LightmapResult*)pResult;
				// retrieve the RGB textures and store them in their charts
				for (int i=0; i<pLMapResult->m_Textures.Size(); i++)
				{
					RGBTexture *pTex = pLMapResult->m_Textures[i];
					texList[pTex->m_iAssociatedChart] = pTex;
					pLMapResult->m_Textures[i] = NULL;
					if (pTex->FoundQNAN())
					{
						CTextureChart *pChart = unwrapper.m_ChartList[pTex->m_iAssociatedChart];
						LOGDEBUG( g_Log, "QNAN found on lightmap for %s poly %d\n", 
							pChart->m_pMatrixNode->GetPath().c_str(),
							pChart->m_iFace);
					}
				}
				// delete the job result without deleting the textures it contains
				delete pResult;
			}
			else
			{
				// it's a sample point rendering result
				GetLux()->GetSamplePointsPtr()->StoreJobResult(pResult);

				// it's an SH lighting result
				m_pSHLighting->StoreJobResult(pResult);

				// it's a vertex lighting result
				m_pVtxLighting->StoreJobResult(pResult);
				
				delete pResult;
			}
		}
	}

	// optimize chart sizes
	if (m_pGIProperties->m_bOptimize)
	{
		for (int iTex=0; iTex<texList.Size(); iTex++)
		{
			RGBTexture *pTex = texList[iTex];

			// potentially optimize out flat coloured charts
			int iOrgX = pTex->m_cX;
			int iOrgY = pTex->m_cY;

			if (pTex->Optimize())
			{
				// TODO: the texture was optimized - we need to re-scale the UVs properly
				CTextureChart *pChart = unwrapper.m_ChartList[iTex];
				for (CTextureChart::PolyIterator iter(pChart); !iter.End(); iter.Next())
				{
					Aftk::C_Polygon *pPoly = iter.GetPoly();
					for (int iPV=0; iPV<3; iPV++)
					{
						Aftk::C_PolygonVertex *pv = &pPoly->GetPolygonVertex(iPV);
						S32 uvIndex = pv->GetTextureCoordinateIndex(pChart->m_iLMapUVSet);

						Aftk::TextureCoordinate4 *pUV = &iter.m_pMesh->GetTextureCoordinate(pChart->m_iLMapUVSet, uvIndex);

						pUV->x = 0.5f;
						pUV->y = 0.5f;
					}
				}
			}
		}
	}

	// save sample point data
	if (GetLux()->GetSamplePointsPtr())
	{
		GetLux()->GetSamplePointsPtr()->SaveSamplePoints(
			m_pGIProperties->m_fBrightness,
			m_pGIProperties->OutputGamma(),
			m_pGIProperties->m_bPrescaleSH,
			m_pGIProperties->GetSaveExtraSamplePointData()
			);
	}

	// run the packing process separately for each lighting pass
	for (int iLightingPass=m_pLux->GetNumPasses()-1; iLightingPass>=0; iLightingPass--)
	{
		packprog.Step();

		// create vectors that contains the textures and charts for this lighting pass
		Ut::Vector<CTextureChart*> myChartList;
		Ut::Vector<RGBTexture*> myTexList;

		// loop through and find which charts are in this pass
		for (TSeamGroupList::Iterator i=unwrapper.m_SeamGroups.Begin(); i!=unwrapper.m_SeamGroups.End(); ++i)
		{
			if (i->iLightingPass != iLightingPass)
				continue;
			
			// copy the charts in this group into our list
			int iChart = i->iChart;
			while (1)
			{
				myChartList.PushBack(unwrapper.m_ChartList[iChart]);
				myTexList.PushBack(texList[iChart]);
				if (unwrapper.m_ChartList[iChart]->m_bLastInChartSeamGroup)
					break;
				iChart++;
			}
		}

		if (myChartList.Size() == 0)
			continue;

		// sort the chart list and texture list by chart address
		SortTextureAndChartList(myChartList, myTexList);

		S_INFO( LUX::g_Log, "\n" );
		S_INFO( LUX::g_Log, "Lighting pass " << iLightingPass << ":\n" );

		LOGINFO( g_Log, "Packing lightmaps...\n");
		m_pLux->DoProgressCallback(100,"Packing lightmaps");

		// do the lightmap packing
		CLightmapPacking packer;
		packer.m_iTexturePackRes = m_pGIProperties->GetDefaultLightmapSize();
		packer.m_fMeterSize = m_pGIProperties->m_fMeterSize;
		packer.m_fMaxClusterSize = m_pGIProperties->m_fMaxClusterSize;
		packer.m_fMaxClusterArea = m_pGIProperties->m_fMaxClusterArea;
		packer.Pack(&myTexList, &myChartList, iLightingPass != 0);

		LOGINFO( g_Log, "Applying materials/remapping UVs...\n");
		m_pLux->DoProgressCallback(100,"Applying materials/remapping UVs");

		// remap the UVs and set the materials on the mesh
		if (packer.m_pPackingInfo)
		{
			lmFileName = m_pLux->GetLightMapFolder() + m_pLux->GetLightMapFilePrefix() + "_";
			packer.m_pPackingInfo->m_pszFilenamePrefix = lmFileName.c_str();
			packer.m_pPackingInfo->m_sLightingPassPrefix = m_pLux->GetLightingPassVector()->At(iLightingPass)->m_name;
			packer.m_pPackingInfo->m_filetype = m_pLux->GetLightMapFileType();

			if (m_pGIProperties->GetOutputFormat() != OUTPUT_FORMAT_DONTCARE)
			{
				packer.m_pPackingInfo->m_filetype = LUX_LIGHT_MAP_FILE_TYPE_DDS;
				packer.m_pPackingInfo->m_textureformat = m_pGIProperties->GetOutputFormat();
			}

			packer.m_pPackingInfo->m_iAtlasOffset = m_pLux->m_iAtlasOffset;

			gpSuperGroups = packer.m_pSuperGroups;

			packer.m_pPackingInfo->ApplyMappingToScene(m_pAftkScene, &myTexList, &myChartList, false, m_pGIProperties->GetTiled());
		}

		m_pLux->DoProgressCallback(100,"Saving textures");

		// generate and write out the texture atlases
		if (packer.m_pPackingInfo)
			packer.m_pPackingInfo->GenerateAtlases(&myTexList, m_pGIProperties->GetTiled(), m_pGIProperties->GetWhiteAlpha());

		m_pLux->m_iAtlasOffset += packer.m_iCurrentAtlas;

		// free up textures and charts
		for (int i=0; i<myTexList.Size(); i++)
			delete myTexList[i];
		myTexList.Destroy();
		for (int i=0; i<myChartList.Size(); i++)
			delete myChartList[i];
		myChartList.Destroy();
	}

	//	if (gbDebugMode)
	//		ColourMeshBySuperGroup(&unwrapper.m_ChartList);

	//	if (gbDebugMode)
	//		ColourMeshByAtlas(&unwrapper.m_ChartList);

	if (g_useNetworkClients)
	{
		Progress::UpdateSimple validateProg(m_pValidateProgress, "Validate");
		m_pAftkScene->Validate();
		validateProg.SetProgress(0.3f);

		m_pAftkScene->Write("c:\\test_temp.a3d", Aftk::E_FileType_BestGuess);

		validateProg.SetProgress(0.6f);

		LOGINFO(g_Log, "Welding all mesh nodes.\n");
		{
			//The class below contains memory pools we'll need.
			Aftk::C_MeshWeldMemoryManager weldMemoryManager;
			//This will be visiting our nodes.
			C_MyWeldVisitor visitor;
			visitor.m_pProperties = m_pProperties;

			//Perform work.
			Aftk::C_SceneNodeVisit::Traverse(
				m_pAftkScene, 
				visitor,
				weldMemoryManager, 
				0);
		}
		LOGINFO(g_Log, "done.\n");

		validateProg.SetProgress(0.9f);
	}

cancel:
	LOGINFO( g_Log, "Waiting for server to finish...\n");
	net.pServer->join();
	delete net.pServer;

	if (net.pClient)
	{
		LOGINFO( g_Log, "Waiting for local client to finish...\n");
		net.pClient->join(); // TODO: add a timeout for this
		delete net.pClient;
	}

	if (m_pLightTree)
		delete m_pLightTree;
	
	if (m_pSHLighting)
		delete m_pSHLighting;

	if (m_pVtxLighting)
		delete m_pVtxLighting;
	
	DeleteServerPaths(net);

	LOGINFO( g_Log, "Freeing up stuff...\n");
	return true;
}

// collect lights, create light-tree
void GlobalIlluminationModel::InitializeLights()
{
	// calculate minimum lighting cutoffpoint
	float fCutoff;
		fCutoff = 0.0005f;

//	fCutoff /= m_pGIProperties->m_fBrightness;

	// create lighting integrator
	m_pIntegrator = new SurfaceIntegrator(SurfaceIntegratorSetup(
		10.0f * m_pGIProperties->m_fMeterSize,			// minimum VPL distance
		m_pGIProperties->GetBounceScaleFactor(),			// indirect light scale
		fCutoff*4.0f ,		// russian roulette threshold
		fCutoff,			// cutoff threshold
		m_pGIProperties->GetNumAntiAliasingTaps(),	// number of samples per texel
		m_pGIProperties->GetTexelDensity(),
		m_pGIProperties->GetAntiLightLeaking(),
		m_pGIProperties->GetEpsilon()
		));

	m_pLightTree = new LightTree;

	m_pGIProperties->m_fCutoff = fCutoff;

	// add lights into the light-tree
	for (int i=0; i<m_pGIProperties->GetNumLights(); i++)
	{
		LUX::Light *pLight = m_pGIProperties->GetLightPtr(i);

		pLight->SetGIProperties(m_pGIProperties);

		// add global lights to the global list
		if (pLight->IsGlobalLight())
		{
			if (pLight->IsDeltaLight())
				m_pDirectionalLight = (DirectionalLight*)pLight;
			else
				m_GlobalLightList.PushBack(pLight);
			continue;            
		}

		pLight->SetCutoffThreshold(fCutoff);
		box3f bbox  = pLight->GetBBox(m_pHierarchy);
		// add this light to the tree
		m_pLightTree->AddLight(pLight,bbox);
	}

	// set the lights that the integrator will need
	m_pIntegrator->SetDirectionalLight(m_pDirectionalLight);
	m_pIntegrator->SetGlobalLightList(&m_GlobalLightList);
	m_pIntegrator->SetSpatialHierarchy(m_pHierarchy);
	m_pIntegrator->SetLightTree(m_pLightTree);
	m_pIntegrator->SetRenderChannels(m_pGIProperties->GetRenderChannelsPtr());
	m_pIntegrator->m_iBounceChannel = m_pGIProperties->m_iBounceChannel;

	// optionally force lighting to be split up into channels for gotham lighting
	if (m_pGIProperties->m_iForceGothamChannels)
		ForceGothamChannels();
	if (m_pGIProperties->m_bForceShooterChannels)
		ForceShooterChannels();
	else
	{
		// default light/gotham lighting -> if we're using alpha scaling, set to the alpha scaling output mode
		if (m_pGIProperties->m_iOutputType == OUTPUT_TYPE_RGB && m_pGIProperties->m_bAlphaScaling)
			m_pGIProperties->m_iOutputType = OUTPUT_TYPE_GOTHAM_NIGHT;
	}
}

// change channel settings so that directional = blue and everything else = green
void GlobalIlluminationModel::ForceGothamChannels()
{
	RenderChannel *pNewChannel;

	// create blue channel for directional light
	pNewChannel = new RenderChannel();
	pNewChannel->m_name = "directional";
	pNewChannel->m_target = RenderChannel::Blue;
	m_pGIProperties->AddRenderChannel(pNewChannel);

	// force directional into blue channel
	if (m_pDirectionalLight)
	{
		m_pDirectionalLight->m_iChannel = m_pGIProperties->GetRenderChannel("directional");
		m_pDirectionalLight->SetColour(float3(1,1,1));
		m_pDirectionalLight->SetIntensity(1);
	}

	// force all other lights into green channel by making the default channel green
	TRenderChannelVector *pChannels = m_pGIProperties->GetRenderChannelsPtr();
	pChannels->At(0)->m_target = RenderChannel::Green;
}

void GlobalIlluminationModel::ForceShooterChannels()
{
	// force hemisphere into green channel, directional into blue + all others into red channel
	// add lights into the light-tree
	RenderChannel *pNewChannel;

	// create channel for directional light
	pNewChannel = new RenderChannel();
	pNewChannel->m_name = "directional";
	pNewChannel->m_target = RenderChannel::Sun;
	m_pGIProperties->AddRenderChannel(pNewChannel);

	// create channel for hemisphere light
	pNewChannel = new RenderChannel();
	pNewChannel->m_name = "hemisphere";
	pNewChannel->m_target = RenderChannel::Sky;
	m_pGIProperties->AddRenderChannel(pNewChannel);

	// create channel for bounce light
	pNewChannel = new RenderChannel();
	pNewChannel->m_name = "bounce";
	pNewChannel->m_target = RenderChannel::RGB;
	m_pGIProperties->AddRenderChannel(pNewChannel);

	// find sun + sky light and force channels
	for (int i=0; i<m_pGIProperties->GetNumLights(); i++)
	{
		LUX::Light *pLight = m_pGIProperties->GetLightPtr(i);

		if (pLight == m_pDirectionalLight)
		{
			m_pDirectionalLight->m_iChannel = m_pGIProperties->GetRenderChannel("directional");
			m_pDirectionalLight->SetColour(float3(1,1,1));
			m_pDirectionalLight->SetIntensity(1);
		}
		else if (pLight->GetType() == LUX_LIGHT_TYPE_HEMISPHERE ||
				 pLight->GetType() == LUX_LIGHT_TYPE_HEMISPHERE2 ||
				 pLight->GetType() == LUX_LIGHT_TYPE_BOUNCEGATHER)
		{
			pLight->m_iChannel = m_pGIProperties->GetRenderChannel("hemisphere");
			if (pLight->GetType() == LUX_LIGHT_TYPE_BOUNCEGATHER)
			{
				LUX::BounceGatherLight *pBGLight = (LUX::BounceGatherLight*)pLight;
				pBGLight->m_iBounceChannel = m_pGIProperties->m_iBounceChannel;
			}
		}
	}
}

// CALLBACK FUNCTION FOR DEBUGGING PURPOSES
void DebugCallback(SD::RayTraceResult &result)
{
	const Aftk::C_Polygon *p = &result.pMeshNode->GetPolygon(result.iPolygon);
/*
	char buffer[512];
	int i=p->GetBasePolygonIndex();
	sprintf(buffer, "base index: %d\n",i);
	OutputDebugString(buffer);
*/
	giOnlyLightFace = result.iPolygon;
}

LuxModelTypeEnum GlobalIlluminationModel::GetType() const
{
	return LUX_MODEL_TYPE_GLOBALILLUMINATION;
}
GlobalIlluminationModel::GlobalIlluminationModel() : LightingModel()
{
	Reset();
}
GlobalIlluminationModel::~GlobalIlluminationModel()
{
	delete m_pUnwrapProgress;		
	delete m_pPackingProgress;		
	delete m_pValidateProgress;
	for (int i=0; i<m_RenderProgress.Size(); i++)
		delete m_RenderProgress[i];
}
void GlobalIlluminationModel::Reset()
{
	this->LightingModel::Reset();

	m_pProperties = 0;
	m_pDirectionalLight = NULL;
	m_pUnwrapProgress = NULL;
	m_pPackingProgress = NULL;
	m_pValidateProgress = NULL;
}

