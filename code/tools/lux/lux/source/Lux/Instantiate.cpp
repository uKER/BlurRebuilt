#include <stdafx.h>
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/SceneUnwrapper.h"
#include "Lux/LightSample.h"
#include "Lux/ImageFilter.h"
#include "Lux/DataTypes.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/TextureTypes/FlexibleTexture.h"
#include "Lux/Instantiate.h"
#include "Lux/LightmapPacking/LightmapPacking.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/Rasterize.h"
#include "Lux/RasterizedTriIterator.h"
#include "Lux/Lux.h"
#include "Lux/Common.h"
#include "lux_common/LightingPass.h"
#include "tools_common/StringProc.h"

using namespace LUX;

bool DoesNodeHaveInstanceActorProperties(Aftk::C_SceneNode *pNode)
{
	AftkUt::C_NodeFlagParser parser(*pNode);

	std::string strClass, strName;

	strClass = parser.GetFlagGroupCategory();
	Tu::ToLower(strClass);

	strName = parser.GetFlagGroupName();
	Tu::ToLower(strName);

	if (!(strClass == "system" && strName == "instance"))
		return false;

	// yes it has instance properties

	// get the switch value
	std::string result;
	parser.GetParameterValue("LightAllInstSame", result);
	return atof(result.c_str()) != 0;
}

// 
int AddNewChartToChartList(SceneUnwrapper *pUnwrapper, CTextureChart *pNewChart, int iLightingPass, const Ut::Vector<ChartSeam> *pSeams)
{
	// add our new chart into the chart list
	int iNewChart = pUnwrapper->m_ChartList.Size();
	pUnwrapper->m_ChartList.PushBack(pNewChart);

	int iSeamGroup = pUnwrapper->m_SeamGroups.Size();

	// create a new seam group for this chart
	pUnwrapper->m_SeamGroups.PushBack();
	pUnwrapper->m_SeamGroups.Back().iChart = iNewChart;
	pUnwrapper->m_SeamGroups.Back().iLightingPass = iLightingPass;
	pNewChart->m_bLastInChartSeamGroup = true;

	// duplicate the old seam list from the original charts into this one
	if (pSeams)
	{
		for (int i=0; i<pSeams->Size(); i++)
			pUnwrapper->m_SeamGroups.Back().seams.PushBack(pSeams->At(i));
	}
	
	pNewChart->RecalcBoundingBox();

	return iSeamGroup;
}

// collect the charts associated with a particular mesh node
// if there's more than one chart, pack them together to form just one and re-assign it and return that
int GetPackedChartFromMeshNode(Lux *pLux, Aftk::C_SceneMeshNode *pNode, SceneUnwrapper *pUnwrapper, int *piReturnSeamGroup, int iLightingPass)
{
	int iError = PCFM_OK;

	Aftk::C_Scene *pAftkScene = pLux->GetAftkScene();
	Aftk::C_SceneMeshNode *m_pMesh = pNode;
	std::string lmFileName;

	// make a list of charts and seam groups that are associated with the given node
	TChartList chartList;
	Ut::Vector<int> chartIndexList;
	Ut::Vector<int> groupIndexList;
	TRGBTextureList texList;

	int iMaxSize = 0;
	int iBiggestArea = 0;
	int i;
	int iNewChart = -1, iSeamGroup = -1;

	Ut::Vector<ChartSeam> m_Seams;

	// walk through all the seam groups
	for (int iGroup=0; iGroup<pUnwrapper->m_SeamGroups.Size(); iGroup++)
	{
		bool bGroupAdded = false;
		// walk through all the charts
		int i = pUnwrapper->m_SeamGroups[iGroup].iChart;
		while (1)
		{
			CTextureChart *pCurChart = pUnwrapper->m_ChartList[i];

			// check for a match
			if (!pCurChart || (pCurChart->m_pMesh != pNode))
				break;

			// is this a reference geometry node? (this means it was already re-packed)
			if (pCurChart->m_bReferenceGeometry)
			{
				// yes - return the previously re-packed chart
				*piReturnSeamGroup = iGroup;
				return PCFM_OK;
			}

			// not repacked - make a list of all the charts that need re-packing
			chartList.PushBack( pCurChart );	// 
			chartIndexList.PushBack( i );
			if (!bGroupAdded)
			{
				groupIndexList.PushBack(iGroup);
				bGroupAdded = true;
			}
			
			// add the seams to our list so we can add them to the base-instance chart later
			for (int j=0; j<pUnwrapper->m_SeamGroups[iGroup].seams.Size(); j++)
				m_Seams.PushBack(pUnwrapper->m_SeamGroups[iGroup].seams[j]);

			int iArea = pCurChart->m_iTexWidth * pCurChart->m_iTexHeight;
			if (iArea > iBiggestArea) iBiggestArea = iArea;

			// go to next chart in this seam group
			i++;
			if (pCurChart->m_bLastInChartSeamGroup)
				break;
		}
	}

	if (chartList.Size() == 0)		// this can happen if we're instancing a meshnode that was flagged as "donotlight"
	{
		return PCFM_NO_CHARTS;
	}

	// if there was only one chart to begin with we don't need to repack
	if (chartList.Size() == 1)
	{
		assert(groupIndexList.Size() == 1);
		chartList[0]->m_bReferenceGeometry = true;
		*piReturnSeamGroup = groupIndexList[0];
		return PCFM_OK;
	}

	// here when there is more than one chart associated with this node
	// this means we'll have to pack them into one

	Aftk::Matrix44 mtx = pNode->GetWorldMatrix();

	// find largest texture extents (we use one texture and resize it to fit the various chart sizes)
	FlexibleTexture<LUX::DataTypes::Scalar> lmaptex(iBiggestArea,1,NULL);

	// rasterize the charts into individual textures
	TexelIteratorInput dummytii;
	Rasterizer r2;
	texList.Resize(chartList.Size());
	for (i=0; i<chartList.Size(); i++)
	{
		// create texture
		CTextureChart *pChart = chartList[i];
		lmaptex.Resize(pChart->m_iTexWidth, pChart->m_iTexHeight);
		dummytii.m_pTex = &lmaptex;
		lmaptex.Clear();

		if (iMaxSize < pChart->m_iTexWidth) iMaxSize = pChart->m_iTexWidth;
		if (iMaxSize < pChart->m_iTexHeight) iMaxSize = pChart->m_iTexHeight;

		// calculate matrix to use to transform normals
		LUX::Matrix44 temp, normalTransform;
		temp = mtx;
		Transpose(&temp);
		Invert4x4(&normalTransform, temp);

		// render the faces into the texture
		int iFace = pChart->m_iFace;
		while (iFace != -1)
		{
			if (r2.LoadPolygon(pChart->m_pMesh, iFace, &mtx, &normalTransform, pChart->m_iLMapUVSet, false))
			{
				r2.Rasterize(&dummytii);
			}

			iFace = POLY_DATA(iFace)->iChartLink;
		}

		// convert the texture to RGBA
		texList[i] = new RGBTexture(&lmaptex,false,1.0f);
	}

	// pack the textures we rendered out into a mini-atlas
	CLightmapPacking packer;
	packer.m_pPackingInfo = NULL;
	int iAtlasRes = iMaxSize;
	int iLastFace;
	CTextureChart *pNewChart = NULL;

	// abort if the biggest chart is bigger than the atlas size
	if (iMaxSize > pUnwrapper->m_fMaxSize)
	{
		iError = PCFM_DOES_NOT_FIT;
		goto abort;
	}

	do
	{
		// clean out packer class
        if (packer.m_pPackingInfo)
		{
			delete packer.m_pPackingInfo;
			packer.m_pPackingInfo = NULL;
		}

		// keep trying a slightly higher res each time
		iAtlasRes += 4;
		if (iAtlasRes > pUnwrapper->m_fMaxSize)
		{
			iError = PCFM_DOES_NOT_FIT;
			goto abort;
		}
		packer.m_iTexturePackRes = iAtlasRes;
		packer.m_iPackMode		 = LMPM_Simple;
		if (!packer.Pack(&texList, &chartList, true))
			continue;
	}
	while (packer.m_pPackingInfo->m_AtlasSizes.Size() != 1);

	// remap the UVs according to the new packing
	packer.m_pPackingInfo->m_filetype = pLux->GetLightMapFileType();
	lmFileName = pLux->GetLightMapFolder() + pLux->GetLightMapFilePrefix() + "_";
	packer.m_pPackingInfo->m_pszFilenamePrefix = lmFileName.c_str();
	packer.m_pPackingInfo->ApplyMappingToScene(pAftkScene, NULL, &chartList, true);

	// check that all the charts packed into one atlas
	assert(packer.m_pPackingInfo->m_AtlasSizes.Size() == 1);
	if (packer.m_pPackingInfo->m_AtlasSizes.Size() != 1)
	{
		printf("WARNING: Instance '%s' didn't fit into a single atlas. Lighting won't be correct.\n",
			pNode->GetName().c_str() );
	}

	// create a single new chart for the whole node
	pNewChart = new CTextureChart( chartList[0] );
	pNewChart->m_pMatrixNode	= pNode;	// default, can be overwritten by calling function
	pNewChart->m_iTexWidth		= packer.m_pPackingInfo->m_AtlasSizes[0].cX; 
	pNewChart->m_iTexHeight		= packer.m_pPackingInfo->m_AtlasSizes[0].cY;

	// go through all the old charts and map their faces to our new chart
	pNewChart->m_iFace = chartList[0]->m_iFace;
	pNewChart->m_pMesh = pNode;
	iLastFace = chartList[0]->m_iLastFace;

	// point the POLY_DATA to the base_instance chart
	iNewChart = pUnwrapper->m_ChartList.Size();
	for (i=0; i<chartList.Size()-1; i++)
	{
		// make the last face of this chart point to the first face of the next chart
		int iFace = chartList[i]->m_iLastFace;
		POLY_DATA(iFace)->iChartLink = chartList[i+1]->m_iFace;
		POLY_DATA(iFace)->iChart = iNewChart;
	}

	pNewChart->m_iLastFace = chartList[i]->m_iLastFace;

	// disable the old charts from the chart list
	// we can't delete them properly because it would mess up the supergroup indices
	// + physically delete the old charts
	for (int i=0; i<chartList.Size(); i++)
	{
		delete chartList[i];
		pUnwrapper->m_ChartList[chartIndexList[i]] = NULL;
	}
	// delete the old seam groups from the list as well
	for (int i=0; i<groupIndexList.Size(); i++)
		pUnwrapper->m_SeamGroups.EraseByIndex(groupIndexList[i]-i);

	pNewChart->m_bReferenceGeometry = true;

	iSeamGroup = AddNewChartToChartList(pUnwrapper, pNewChart, iLightingPass, &m_Seams);

	// delete the textures we created earlier
	for (int i=0; i<texList.Size(); i++)
		delete texList[i];

	// DONE
	*piReturnSeamGroup = iSeamGroup;
	return PCFM_OK;

abort:
	// it won't fit.
	// delete the textures we created before
	for (int i=0; i<texList.Size(); i++)
		delete texList[i];
	// throw an error
	return iError;
}

// pack mesh nodes that have been instanced into one chart, then duplicate that chart for each instance
void LUX::PackInstancedMeshNodes(Lux *pLux, SceneUnwrapper *pUnwrapper, const LightingModelProperties *pProp, const Ut::Vector<LightingPass*> *pLightingPasses, bool bInstantiate)
{
	Aftk::C_Scene *pAftkScene = pLux->GetAftkScene();

	// create a list of all instance nodes in the scene
	Ut::Vector<Aftk::C_SceneNode*> instancelist;

	// iterate all the scene's nodes
	for (Aftk::C_SceneNodeIterator it(pAftkScene->GetRootSceneNodePtr()); it; ++it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;

		// only interested in instance nodes
		if (!pAftkSceneNode->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance ))
			continue;

		// only pack nodes that need to be lit using lightmaps
		if (GetNodeLightingMode(pProp, pAftkSceneNode) != LMode_Lightmap)
			continue;

		// if it's flagged "light all instances of this node the same as the reference" then skip it
		if (DoesNodeHaveInstanceActorProperties(pAftkSceneNode))
			continue;

		// check that it's the highest LOD
//		if (!CheckModelIsHighestLod(pAftkSceneNode))
//			continue;

		Aftk::C_SceneMeshRemappedInstanceNode *pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)pAftkSceneNode;

		Aftk::C_SceneMeshNode *pOrgNode = pInstance->GetSourceNodePtr();

		// skip things that don't need lighting
		// find a lightingpass that lights this node
		const Ut::Vector<LightingPass*> *pLightingPasses = pLux->GetLightingPassVector();
		int lp;
		for (lp=0; lp<pLightingPasses->Size() && !(*pLightingPasses)[lp]->DoesNodeNeedLighting(pOrgNode); lp++) { ; }
		if (lp == pLightingPasses->Size())
			continue; // does not get lit by any pass

		if (!LUX::DoesNodeNeedProcessing(pOrgNode))
			continue;

		instancelist.PushBack( pInstance );
	}

	// iterate through the instances we found - create a chart for each one
	for (int iInst=0; iInst<instancelist.Size(); iInst++)
	{
		Aftk::C_SceneMeshRemappedInstanceNode *pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)instancelist[iInst];

		Aftk::C_SceneMeshNode *pOrgNode = (Aftk::C_SceneMeshNode*)pInstance->GetSourceNodePtr();

		assert(pOrgNode->GetType() == Aftk::E_ObjType_SceneMesh);

		Aftk::Matrix44 mtxOrg = pOrgNode->GetWorldMatrix();
		Aftk::Matrix44 mtxInst = pInstance->GetWorldMatrix();
		if (DoesMatrixHaveNegativeScaling(mtxOrg) != 
			DoesMatrixHaveNegativeScaling(mtxInst))
		{
			LOGWARN( g_Log, "Instance '%s' has a flipped matrix relative to '%s' and will appear inside out.\n", pInstance->GetPath().c_str(), pOrgNode->GetName().c_str() );
		}
		
		// find a lightingpass that lights this node
		int lp;
		int iLightingPassForThisNode = -1;
		for (lp=0; lp<pLightingPasses->Size(); lp++)
		{
			if ((*pLightingPasses)[lp]->DoesNodeNeedLighting(pInstance))
			{
				if (iLightingPassForThisNode == -1)
					iLightingPassForThisNode = lp;
			}
		}
		if (iLightingPassForThisNode == -1)
			continue; // does not get lit by any pass

		// we've got an instance of a mesh node that needs instantiating
		int iSeamGroup = -1;
		int iResult;

		if (bInstantiate)
			iResult = PCFM_DOES_NOT_FIT;
		else
			iResult = GetPackedChartFromMeshNode(pLux, pOrgNode, pUnwrapper, &iSeamGroup, iLightingPassForThisNode);

		switch (iResult)
		{
		case PCFM_OK:
			{
				// create another instance of the chart, but with the instance's matrix
				CTextureChart *pNewChart = new CTextureChart(pUnwrapper->m_ChartList[pUnwrapper->m_SeamGroups[iSeamGroup].iChart]);
				pNewChart->m_pMatrixNode = pInstance;

				const Ut::Vector<ChartSeam> mySeams = pUnwrapper->m_SeamGroups[iSeamGroup].seams;
				AddNewChartToChartList(pUnwrapper, pNewChart, iLightingPassForThisNode, &mySeams);

				break;
			}
		case PCFM_NO_CHARTS:
			// the reference meshnode does not have any charts in it - do nothing
			break;
		case PCFM_DOES_NOT_FIT:
			{
				// here if the re-packing failed (because the surface of the instance is too big.
				// when this happens, we convert the instance into a normal meshnode
				// create a new meshnode
				Aftk::C_SceneMeshNode* pNewNode;

				assert(!bInstantiate);
				if (pInstance->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
				{
					LOGWARN( g_Log, "Boned instance '%s' will not be lit because it is too big to fit into a single lightmap.\n",pInstance->GetPath().c_str());
				}
				else
				{
					LOGWARN( g_Log, "Remapped instance '%s' will not be lit because it is too big to fit into a single lightmap.\n",pInstance->GetPath().c_str());
				}
/*
				pNewNode = ConvertRemappedInstanceToMesh(pInstance);

				// duplicate the charts from the reference mesh for this instance
				for (int i=0; i<pUnwrapper->m_ChartList.Size(); i++)
				{
					CTextureChart *pCurChart = pUnwrapper->m_ChartList[i];

					// check for a match
					if (!pCurChart || (pCurChart->m_pMesh != pOrgNode))
						continue;

					// is this a reference geometry node? (this means it was already re-packed -
					assert(pCurChart->m_bReferenceGeometry == false);

					// duplicate this chart with a different matrix node
					CTextureChart *pNewChart = new CTextureChart( pCurChart );
					pNewChart->m_pMatrixNode	= pNewNode;	// default, can be overwritten by calling function
					pNewChart->m_pMesh			= pNewNode;

					pNewChart->m_bReferenceGeometry = false;
					AddNewChartToChartList(pUnwrapper, pNewChart, iLightingPassForThisNode, NULL);
					// TODO: pass in a seam list here -^
				}
*/
			}
			break;
		}
	}    
}

