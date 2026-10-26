#include <stdafx.h>

#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/SceneUnwrapper.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/Parameterization/SuperGroups.h"
#include "Lux/Parameterization/ChartSeams.h"
#include "Lux/Common.h"
#include "lux_common/LightingPass.h"
#include <system\misc\progress.h>

using namespace LUX;

extern int giLightOnlyNode;

SceneUnwrapper::SceneUnwrapper()
{
	m_fScale = 1.0f;
	m_fMaxSize = 1024.0f;
	m_iDoubleSidedPolyflagMask = (1<<8);
	m_bFixSeams = false;
	m_bFixAllSeams = false;
}

SceneUnwrapper::~SceneUnwrapper()
{
}

bool SceneUnwrapper::Unwrap(Aftk::C_Scene* pAftkScene, const LightingModelProperties *pProp, const Ut::Vector<LightingPass*> *pLightingPasses, bool bStoreChartMatrices, Progress::I_ProgressBar *pProgress)
{
	MEMORY_TRACKER_PUSH("tools.lux.server.unwrapper");

	U32 n = pAftkScene->GetNumMeshPolygons();

	m_ChartList.Reserve(n/10); // take a guess at the amount of charts there will be

	// count the number of nodes that need unwrapping
	int nNodesThatNeedProcessing = 0;
	int nNodesProcessed = 0;
	int nMaxVertPos = 0;

	Progress::UpdateSimple progress(pProgress, "Unwrapping");

	// iterate all the scene's nodes
	for (int iPass=1; iPass<=2; iPass++)
	{
		if (iPass == 2)
		{
			g_pVerts = new Position3[nMaxVertPos];
		}

		for (Aftk::C_SceneNodeIterator it(pAftkScene->GetRootSceneNodePtr()); it; it++)
		{
			Aftk::C_SceneNode* pAftkSceneNode = it;
			Aftk::E_ObjType type = pAftkSceneNode->GetType();

			// only interested in mesh nodes
			if (type != Aftk::E_ObjType_SceneMesh)
				continue;

			// only unwrap nodes that need to be lit using lightmaps
			if (GetNodeLightingMode(pProp, pAftkSceneNode) != LMode_Lightmap)
				continue;

			if (!DoesNodeNeedProcessing(pAftkSceneNode))
				continue;

			if (giLightOnlyNode != -1 && pAftkSceneNode->GetUniqueId() != giLightOnlyNode)
				continue;

			// find a lightingpass that lights this node
			int lp;
			int iLightingPassForThisNode = -1;
			bool bFirst = true;
			for (lp=0; lp<pLightingPasses->Size(); lp++)
			{
				if ((*pLightingPasses)[lp]->DoesNodeNeedLighting(pAftkSceneNode))
				{
					// for each lighting pass that this node is in, generate a separate set of charts
					iLightingPassForThisNode = lp;

					Aftk::C_SceneMeshNode *pMeshNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;

					if (iPass==1)
					{
						// pass 1 simply counts the number of objects to unwrap for our progress counter
						nNodesThatNeedProcessing++;

						int nVP = pMeshNode->GetNumVertexPositions();
						if (nVP > nMaxVertPos) nMaxVertPos = nVP;

						continue;
					}

					if (m_bCalculateTangentSpace)
					{
						MEMORY_TRACKER_PUSH("tools.aftk");
		//				LOGINFO( g_Log, "Calculating tangents and Binormals.\n");
						Aftk::C_MeshCalculateTangents::CalculateTangents(
							pMeshNode,
							true,
							false);
		//				LOGINFO( g_Log, "CalculateTangentsAndBinormals done.\n");
			
						Aftk::C_MeshExpandPolygonVertices::ExpandPolygonVertices(
							pMeshNode);
						MEMORY_TRACKER_POP("tools.aftk");
					}

					Aftk::Matrix44 worldMatrix = pMeshNode->GetWorldMatrix();

					// create texture atlas
					CTextureAtlas texAtlas;
					texAtlas.m_iLowResLightmapPolyflag = m_iLowResLightmapPolyflag;
					texAtlas.m_iDoNotLightPolyflagMask = m_iDoNotLightPolyflagMask;
					texAtlas.m_iLowResLightmapBits = m_iLowResLightmapBits;
					texAtlas.m_bFixAllSeams = m_bFixAllSeams;

					nNodesProcessed ++;
					printf("\runwrapping node %5d/%5d...", nNodesProcessed, nNodesThatNeedProcessing);
					texAtlas.ParameterizeMeshUVs(
						pMeshNode, &m_ChartList, m_fScale, m_fMaxSize, 
						&worldMatrix, iLightingPassForThisNode, m_fTexelDensityRatio, m_SeamGroups,
						bStoreChartMatrices?&m_ChartMatrixList:NULL,
						bFirst
						);

					progress.SetProgress(100.f*(float)nNodesProcessed/(float)nNodesThatNeedProcessing);

					bFirst = false;
				}
			}
		}
	}
	printf("\n");

	delete [] g_pVerts;

	SetChartNumbersToPolygons();

	MEMORY_TRACKER_POP("tools.lux.server.unwrapper");

	return true;
}

void SceneUnwrapper::SetDoubleSidedPolyflagMask(int iDoubleSidedPolyflagMask)
{
	m_iDoubleSidedPolyflagMask = iDoubleSidedPolyflagMask;
}

void SceneUnwrapper::SetDoNotLightPolyflagMask(int iDoNotLightPolyflagMask)
{
	m_iDoNotLightPolyflagMask = iDoNotLightPolyflagMask;
}

void SceneUnwrapper::SetLowResLightmapPolyflagMask(int iLowResLightmapPolyflag)
{
	m_iLowResLightmapPolyflag = iLowResLightmapPolyflag;
}

void SceneUnwrapper::SetLowResLightmapBits(int iBits)
{
	m_iLowResLightmapBits = iBits;
}

void SceneUnwrapper::SetCalculateTangentSpace(bool bOn)
{
	m_bCalculateTangentSpace = bOn;
}

void SceneUnwrapper::SetSeamSettings(bool bFixSeams, bool bFixAllSeams)
{
	m_bFixSeams = bFixSeams;
	m_bFixAllSeams = bFixAllSeams;
}

// transfer chart numbers to polygons
void SceneUnwrapper::SetChartNumbersToPolygons()
{
	int i, nCharts = m_ChartList.Size();

	for (i=0; i<nCharts; i++)
		m_ChartList[i]->SetChartNumberToPolygons(i);
}

// clear all charts, etc.
void SceneUnwrapper::Purge()
{
	int nCharts = m_ChartList.Size();

	for (int i=0; i<nCharts; i++)
	{
		m_ChartList[i]->Free();
		
		delete m_ChartList[i];
	}

}

