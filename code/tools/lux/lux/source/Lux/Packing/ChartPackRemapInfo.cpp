//-----------------------------------------------------------------------------
// File: ChartPackRemapInfo.cpp
//
// Desc: Store information on how to build the atlas from the individual charts
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "Lux/LightmapPacking/PackingRectangle.h"
#include "Lux/LightmapPacking/LightmapPacking.h"
#include "Lux/LightmapPacking/ChartPackRemapInfo.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/LightmapPacking/ApplyMaterials.h"
#include "Lux/Parameterization/SuperGroups.h"
#include "Lux/DebugCode.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifndef YDL
#include <io.h>
#endif
#include <set>
#include <il/OpenIL.h>

using namespace LUX;

// initialize with default values
CChartPackRemapInfo::CChartPackRemapInfo(int iCharts)
{
	m_nCharts = iCharts;
	m_nAtlases = 0;
	m_iErrorCode = 0;
	m_filetype = LUX_LIGHT_MAP_FILE_TYPE_BMP;
	m_textureformat = TEXFMT_A8R8G8B8;
	m_iAtlasOffset = 0;
}

// add one remapped chart to the list
void CChartPackRemapInfo::Add(CMyImage *pImg, int iAtlas)
{
	TChartMapInfo cmi;

	cmi.iChart	= pImg->iChart;
	cmi.x		= pImg->x;
	cmi.y		= pImg->y;
	cmi.w		= pImg->w;
	cmi.h		= pImg->h;
	cmi.iAtlas	= iAtlas;

	if (iAtlas+1 > m_nAtlases) m_nAtlases = iAtlas+1;

	StoreChartMapping(&cmi);
}

// generate name for an atlas
char *CChartPackRemapInfo::GenerateLightmapFilename(int iAtlas, int iLayer)
{
	// does this lightmap have multiple layers?
	if (iLayer == -1 && m_pTextures->At(iAtlas)->GetNumLayers()>1)
		iLayer = 0;

	static char buffer[512];
	char *pszExt;
	switch (m_filetype)
	{
	case LUX_LIGHT_MAP_FILE_TYPE_BMP:
		pszExt = ".bmp";
		break;
	case LUX_LIGHT_MAP_FILE_TYPE_TGA:
		pszExt = ".tga";
		break;
	case LUX_LIGHT_MAP_FILE_TYPE_DDS:
		pszExt = ".dds";
		break;
	default:
		assert(0);
		break;
	}

	if (iLayer == -1)
	{
		sprintf(buffer,"%s%s_%03d%s", m_pszFilenamePrefix, m_sLightingPassPrefix.Chars(), iAtlas, pszExt);
		return buffer;
	}
	else
	{
		sprintf(buffer,"%s%s_%03d-%c%s", m_pszFilenamePrefix, m_sLightingPassPrefix.Chars(), iAtlas, 'A'+iLayer, pszExt);
		return buffer;
	}
}

// generate a new material from an old one + an added lightmap
const Aftk::C_MaterialResource *CChartPackRemapInfo::GenerateNewMaterial(Aftk::C_MaterialNode &newMaterial, int iAtlas )
{
	const Aftk::C_MaterialNode *pOriginalMat;

	// check whether this material already has a lightmap applied to it
	for (int i=0; i<newMaterial.GetNumShaderImageNodes(); i++)
	{
		const std::string &name = newMaterial.GetShaderImageNodePtr(i)->GetName();
		if (name == "lightmap0")
			return m_pAftkScene->AddMaterialNode(newMaterial);
	}

	// generate the filename for the atlas that this chart is in
	char *pszName = GenerateLightmapFilename( iAtlas );

	//2) Create image node
	Aftk::C_ShaderImageNode* pShaderImageNode = new Aftk::C_ShaderImageNode();
	pShaderImageNode->SetName("lightmap0");
	pShaderImageNode->GetFilePlug().SetValue(pszName);
	pShaderImageNode->GetImageTypePlug().SetValue("lightmap");
	//Add shader image node to material
	newMaterial.AddShaderNodePtr(pShaderImageNode);

	//3) Create main shader node
	Aftk::C_ShaderNode* pShaderNode = new Aftk::C_ShaderNode();
	pShaderNode->SetName("luxProperties");
	{
		Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
		pShaderNode->AddPlugPtr(pPlug);
		pPlug->SetName("lightmap0");

		//Connect lightmap0 plug to lightmap0 file node
		pPlug->SetSourceConnectionPtr(pShaderImageNode);
	}
	//Add shader node to material
	newMaterial.AddShaderNodePtr(pShaderNode);

	//4) Setup material properties
	{
		Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
		newMaterial.AddPlugPtr(pPlug);
		pPlug->SetName("lux");

		//Connect lux plug to luxProperties node
		pPlug->SetSourceConnectionPtr(pShaderNode);
	}

	newMaterial.SetName( newMaterial.GetName() + ".l" );

	// return ref
	return m_pAftkScene->AddMaterialNode(newMaterial);
}

// get a material from an old mtl + atlas combo
const Aftk::C_MaterialResource *CChartPackRemapInfo::GetMaterialCombo(int iMat, int iAtlas)
{
	bool	bNew;
	const Aftk::C_MaterialResource *pRef = m_pAttr->GetAttribute(iMat, iAtlas, &bNew);

	// if this is a new combination - create a new material
	if (bNew)
	{
		Aftk::C_MaterialNode newMaterial;

		// make copy of original material (if it exists)
		if (iMat != -1)
			newMaterial = m_MaterialList[iMat];

		pRef = GenerateNewMaterial(newMaterial, iAtlas);
		m_pAttr->SetAttribute(iMat, iAtlas, pRef);

		m_nNewMaterials++;
	}

	return pRef;
}

// assign a lightmap to this polygon's material
void CChartPackRemapInfo::SetLightmapMaterialToPolygon(Aftk::C_Polygon *pPoly, int iMat, int iAtlas)
{
	int		physicalIndex;
	const Aftk::C_MaterialResource *sharedPtr;
	const Aftk::C_MaterialResource *pRef = GetMaterialCombo(iMat, iAtlas);

	// set it to the current polygon
	SetMaterialResourcePtr(pPoly, pRef);
}

// remember for each polygon what material they are associated with (by ID)
void CChartPackRemapInfo::AssociatePolygonsWithMaterialIDs(TChartList *pCharts)
{
	int iID;
	int iFace, iChart;

	m_IntList.Clear();
	m_IntList.Reserve(100 * 1024); // reserve a nice big chunk

	for (int iRemap=0; iRemap<m_ChartMap.Size(); iRemap++)
	{
		TChartMapInfo *pRemap	   = &m_ChartMap[iRemap];
		iChart = pRemap->iChart;
		CTextureChart *pCurChart   = pCharts->At(iChart);

		// get the first face of this chart
		iFace					   = pCurChart->m_iFace;
		Aftk::C_SceneMeshNode *m_pMesh = pCurChart->m_pMesh;

		// skip instanced geometry
		if (pCurChart->IsInstance() || pCurChart->m_bReferenceGeometry)
			continue;

		// iterate thru polygons that make up this chart
        while (1)
		{
			// get poly
			Aftk::C_Polygon* pPoly = &m_pMesh->GetPolygon(iFace);

			// this this polygon's material ID to our list
			const Aftk::C_MaterialResource *pRes = GetMaterialResourcePtr(pPoly);
			if (pRes)
			{
				iID = pRes->GetIndex();
				// delete the manager's copy of the materials as we go
			}
			else
				iID = -1;
			m_IntList.PushBack( iID );

			if (iFace == pCurChart->m_iLastFace) break; // was this the last one?
			iFace = POLY_DATA(iFace)->iChartLink;		// get next
		}
	}

	// now go through and delete all the materials off the polygons
	for (int iRemap=0; iRemap<m_ChartMap.Size(); iRemap++)
	{
		TChartMapInfo *pRemap	   = &m_ChartMap[iRemap];
		iChart = pRemap->iChart;
		CTextureChart *pCurChart   = pCharts->At(iChart);

		// get the first face of this chart
		iFace					   = pCurChart->m_iFace;
		Aftk::C_SceneMeshNode *m_pMesh = pCurChart->m_pMesh;

		// skip instances and reference geometry - they are done separately
		if (pCurChart->IsInstance() || pCurChart->m_bReferenceGeometry)
			continue;

		// iterate thru polygons that make up this chart
        while (1)
		{
			// get poly
			Aftk::C_Polygon* pPoly = &m_pMesh->GetPolygon(iFace);

			pPoly->DeleteMaterial();

			if (iFace == pCurChart->m_iLastFace) break; // was this the last one?
			iFace = POLY_DATA(iFace)->iChartLink;		// get next
		}
	}

}

// copy materials from manager and delete the manager's copy
void CChartPackRemapInfo::GetMaterialsFromAftkManager()
{
	int n = m_pAftkScene->GetNumMaterialNodes();
	for (int i=0; i<n; i++)
	{
		const Aftk::C_MaterialResource* pRes = m_pAftkScene->GetMaterialResourcePtr(i);
		Aftk::C_MaterialNode *pMtl = const_cast<Aftk::C_MaterialNode*>(&pRes->GetContent());

		m_MaterialList.PushBack( *pMtl );

		m_mtlmap[ pMtl ] = pRes->GetIndex();
	}
}

extern SuperGroupBuilder *gpSuperGroups;

// apply the stored mapping to a scene
void CChartPackRemapInfo::ApplyMappingToScene(Aftk::C_Scene* pAftkScene, TRGBTextureList *pTextures, TChartList *pCharts, bool bRemapUVsOnly, bool bTiled)
{
	m_bTiled = bTiled;
	m_pTextures = pTextures;

	Ut::UtVector< Ut::DynamicVector<char> > piHashTable;
	int iMatFacelist = 0;

	m_pAftkScene = pAftkScene;

	// if none of the charts are set to remap, skip this whole function altogether
	bool bAnyRemap = false;
	for (int i=0; i<pCharts->Size(); i++)
		bAnyRemap |= pCharts->At(i)->m_bRemap;
	if (!bAnyRemap)
		return;

	// setup
	if (!bRemapUVsOnly)
	{
		m_pAttr = new CAttributeTable;
		m_pAttr->Initialize(m_pAftkScene->GetNumMaterialNodes(), m_nAtlases);

		S_DEBUG( g_Log, "Originally " << m_pAftkScene->GetNumMaterialNodes() << " materials.\n");

		GetMaterialsFromAftkManager();
		AssociatePolygonsWithMaterialIDs(pCharts);
	}

	m_nNewMaterials = 0;

	// remap faces (materials) and vertices (UVs)
	for (int iRemap=0; iRemap<m_ChartMap.Size(); iRemap++)
	{
		TChartMapInfo *pRemap	   = &m_ChartMap[iRemap];
		int iChart = pRemap->iChart;
		CTextureChart *pCurChart   = pCharts->At(iChart);
		TAtlasSizeInfo *pAtlasSize = &m_AtlasSizes[pRemap->iAtlas];

		// check whether this chart belongs to instanced geometry
		if (pCurChart->IsInstance())
		{
			assert(!bRemapUVsOnly);
			ApplyMappingToInstancedGeometry(pCurChart, pRemap, pAtlasSize);
			continue;
		}

		// get the first face of this chart
		int iFace				   = pCurChart->m_iFace;
		Aftk::C_SceneMeshNode *m_pMesh = pCurChart->m_pMesh;

		// initialize hash table
		int nUVVerts = m_pMesh->GetNumTextureCoordinates(pCurChart->m_iLMapUVSet);
		piHashTable.Resize( nUVVerts );
		memset(&piHashTable[0], 0, sizeof(char) * nUVVerts);

		// for reference geometry, set the offset to 0, then we'll add it back using the UV offset in blind data
		float xoffset=0, yoffset=0;
		int iOrgX = pRemap->x, iOrgY = pRemap->y;
		if (pCurChart->m_bReferenceGeometry)
		{
			SetInstanceParameters(m_pMesh, pRemap, pAtlasSize, pCurChart->m_iLMapUVSet, false);
			pRemap->x = 0;
			pRemap->y = 0;
		}
		else if (m_pMesh->GetNumInstancesOfThisNode()>0)
		{
			if (!bRemapUVsOnly)
			{
				// base instance that has been lit as mesh - getmeingame still needs instance blind data, so set it
				TChartMapInfo cmi;
				cmi.iAtlas = pRemap->iAtlas;
				cmi.x = 0;
				cmi.y = 0;
				SetInstanceParameters(m_pMesh, &cmi, pAtlasSize, pCurChart->m_iLMapUVSet, true);
			}
		}

		// get custom data from the chart's texture
		U8 *pCustomData = NULL;
		int nCustomData = 0;
		if (pTextures)
		{
			Ut::Vector<U8> &customData = pTextures->At(pRemap->iChart)->GetCustomData();
			nCustomData = customData.Size();
			if (nCustomData > 0)
				pCustomData = &customData[0];
		}
		Aftk::C_PolygonCluster *pCluster = NULL;
		if (pCustomData)
		{
			// find/generate a cluster on this mesh for this custom data
			int iCluster = GetPolygonClusterForCustomData(m_pMesh, pCustomData, nCustomData);
			pCluster = m_pMesh->GetPolygonClusterPtr(iCluster);
		}

		// iterate thru polygons that make up this chart
        while (1)
		{
			// get poly
			Aftk::C_Polygon* pPoly = &m_pMesh->GetPolygon(iFace);

			// set lightmap material
			if (!bRemapUVsOnly && !pCurChart->m_bReferenceGeometry)
			{
				SetLightmapMaterialToPolygon(pPoly, m_IntList[iMatFacelist], pRemap->iAtlas);
				if (pCluster)
					pCluster->AddPolygonIndex(0, iFace);
				iMatFacelist++;
			}

			if (pCurChart->m_bRemap)
			{
				// offset UVs
				for (int i=0; i<3; i++)
				{
					// get UV index
					Aftk::C_PolygonVertex *pv = &pPoly->GetPolygonVertex(i);
					S32 uvIndex = pv->GetTextureCoordinateIndex(pCurChart->m_iLMapUVSet);

					assert(uvIndex>=0 && uvIndex<nUVVerts);
					if (piHashTable[uvIndex]==1) continue; // don't process the same vertex twice

					Aftk::TextureCoordinate4 *pUV = &m_pMesh->GetTextureCoordinate(pCurChart->m_iLMapUVSet, uvIndex);

					RemapUV(&pUV->x, &pUV->y, pRemap, pAtlasSize);

					piHashTable[uvIndex] = 1;
				}

				// offset subchart
				POLY_DATA(iFace)->m_x += pRemap->x;
				POLY_DATA(iFace)->m_y += pRemap->y;

				int mx = POLY_DATA(iFace)->m_x;
				int my = POLY_DATA(iFace)->m_y;
				int mcx = POLY_DATA(iFace)->m_cx;
				int mcy = POLY_DATA(iFace)->m_cy;
				if (!(POLY_DATA(iFace)->m_x >= 0 && POLY_DATA(iFace)->m_x+POLY_DATA(iFace)->m_cx<=m_AtlasSizes[pRemap->iAtlas].cX) ||
					!(POLY_DATA(iFace)->m_y >= 0 && POLY_DATA(iFace)->m_y+POLY_DATA(iFace)->m_cy<=m_AtlasSizes[pRemap->iAtlas].cY))
				{
					static int nMessages = 10;
					if (nMessages > 0)
					{
						nMessages--;
						LOGDEBUG( g_Log, "Window out of bounds on face %d obj %s (%d, %d, %d, %d) - (%d, %d)\n",
							iFace, pCurChart->m_pMatrixNode->GetPath().c_str(), mx, mcx, my, mcy, m_AtlasSizes[pRemap->iAtlas].cX, m_AtlasSizes[pRemap->iAtlas].cY);
					}
				}

				POLY_DATA(iFace)->iAtlas = pRemap->iAtlas;
			}

			if (iFace == pCurChart->m_iLastFace) break; // was this the last one?
			iFace = POLY_DATA(iFace)->iChartLink;				  // get next
		}

		pRemap->x = iOrgX, pRemap->y = iOrgY;
	}

	if (!bRemapUVsOnly)
	{
		S_DEBUG( g_Log, "Generated " << m_nNewMaterials << " material combinations.\n" );
	}

	if (!bRemapUVsOnly)
	{
		
		if (gbDebugMode)
			DisplayMaterialSplits( m_pAftkScene, pCharts, m_pAttr->m_piConversionTable, m_pAttr->m_nMaterials, m_pAttr->m_nTextures);
		

		m_pAttr->Purge();
		delete m_pAttr;
	}
}

// set the blind data that contains UV offset and atlas index info for instanced geometry
void CChartPackRemapInfo::SetInstanceParameters(Aftk::C_SceneMeshRemappableNode *pNode, TChartMapInfo *pRemap, TAtlasSizeInfo *pAtlasSize, int iLMapUVSet, bool bBlindDataOnly)
{
	float fAtlascX = (float)pAtlasSize->cX;
	float fAtlascY = (float)pAtlasSize->cY;

	if (m_bTiled)
	{
		fAtlascX *= 2.0f;
		fAtlascY *= 2.0f;
	}

	//
	Aftk::C_BlindDataNode newBlindData;

	// copy existing blind data into our new blind data (if it's there)
	const Aftk::C_BlindDataResource *pBlindDataResource = pNode->GetBlindDataResourcePtr();
	if (pBlindDataResource)
	{
		const Aftk::C_BlindDataNode *pBlindData = &pNode->GetBlindDataResourcePtr()->GetContent();
		if (pBlindData)
			newBlindData = *pBlindData;
	}

	Aftk::Vector2F32 vOffset(pRemap->x / fAtlascX, pRemap->y / fAtlascY);
	char *pszLmapFilename = GenerateLightmapFilename(pRemap->iAtlas);

	// check whether blind data is already there
	bool bUVExists = false;
	bool bIndexExists = false;
	for (int i=0; i<newBlindData.GetNumAttributes(); i++)
	{
		const std::string &name = newBlindData.GetAttribute(i).GetName();

		if (name == "UVoffset")
		{
			bUVExists = true;
			newBlindData.GetAttribute(i).SetValue(vOffset);
		}
		else if (name == "AtlasName")
		{
			bIndexExists = true;
			newBlindData.GetAttribute(i).SetValue(pszLmapFilename);
		}
	}

	// --- set blind data for Ed

	if (!bUVExists)
	{
		// set UV offset
		Aftk::C_GenericAttribute uvOffsetAttr;
		uvOffsetAttr.SetName("UVoffset");
		uvOffsetAttr.SetValue(vOffset);
		newBlindData.AddAttribute(uvOffsetAttr);
	}

	if (!bIndexExists)
	{
		// set lightmap index
		Aftk::C_GenericAttribute atlasIndexAttr;
		atlasIndexAttr.SetName("AtlasName");
		atlasIndexAttr.SetValue(pszLmapFilename);
		newBlindData.AddAttribute(atlasIndexAttr);
	}

	// add new blind data into the scene and the scene node
	const Aftk::C_BlindDataResource* pNewBlindDataResource = 
		pNode->GetScenePtr()->AddBlindDataNode(newBlindData);
	pNode->SetBlindDataResourcePtr(pNewBlindDataResource);

	if (bBlindDataOnly)
		return;

	// --- set formalized aftk data for Ken
	// modify texture offset
	Aftk::TextureCoordinate4 newUV(vOffset.x, vOffset.y, 0, 0);
	pNode->SetTextureCoordinateOffset( iLMapUVSet, newUV );

	// modify materials
	// use GetNumPolygonMaterialMappings and SetPolygonMaterialMapping
	// to change the materials and set them to the new atlas
	for (int i=0; i<pNode->GetNumPolygonMaterialMappings(); i++)
	{
		const Aftk::C_MaterialResource *pMatRes = pNode->GetPolygonMaterialMappingPtr(i);
		const Aftk::C_MaterialResource *pRef;

//#define SLOW_METHOD // faster method appears to bugger things up - does the material index change when adding new materials?

#ifdef SLOW_METHOD
		Aftk::C_MaterialNode newMaterial = pMatRes->GetContent();

		pRef = GenerateNewMaterial(newMaterial, pRemap->iAtlas);
#else
		pRef = GetMaterialCombo( m_mtlmap[&pMatRes->GetContent()], pRemap->iAtlas);
#endif

		pNode->SetPolygonMaterialMapping(i, pRef);
	}
}

// remap a set of UV coordinates
void CChartPackRemapInfo::RemapUV(float *pX, float *pY, TChartMapInfo *pRemap, TAtlasSizeInfo *pAtlasSize)
{
	float u2 = *pX, v2 = *pY;

	float cX = pAtlasSize->cX;
	float cY = pAtlasSize->cY;

	if (m_bTiled)
	{
		cX *= 2.0f;
		cY *= 2.0f;
	}

	u2-=0.5/pRemap->w;
	v2-=0.5/pRemap->h;

	u2 *= (float)pRemap->w / cX;
	v2 *= (float)pRemap->h / cY;

	u2 += (float)pRemap->x / cX;
	v2 += (float)pRemap->y / cY;

	u2+=0.5/cX;
	v2+=0.5/cY;

//	v2+=0.5f;


	*pX = u2, *pY = v2;
}

// calculate the UV offset and lightmap index for a piece of instanced geometry and set it to the blind data
void CChartPackRemapInfo::ApplyMappingToInstancedGeometry(CTextureChart *pCurChart, TChartMapInfo *pRemap, TAtlasSizeInfo *pAtlasSize)
{
	SetInstanceParameters(pCurChart->m_pMatrixNode, pRemap, pAtlasSize, pCurChart->m_iLMapUVSet, false);
}


// copy a chart into an atlas
void CChartPackRemapInfo::BlitRGBA32(	U32 *pDest, int cDestX, int cDestY, int iDestX, int iDestY,
										U32 *pSrc,	int cSrcX,	int cSrcY)
{
	// don't blit null images
	if (iDestX >= 4096 || iDestY >= 4096)
		return;

	for (int y=0; y<cSrcY; y++)
		memcpy(pDest+(y+iDestY)*cDestX+iDestX, pSrc+y*cSrcX, sizeof(U32)*cSrcX);
}

// render atlas texture by copying the charts into them
void CChartPackRemapInfo::GenerateAtlases(TRGBTextureList *pInTextures, bool bTiled, bool bWhiteAlpha)
{
	m_bTiled = bTiled;

	// determine number of texture layers by looking at the first texture
	int nLayers = 0;
	for (int i=0; i<pInTextures->Size(); i++)
	{
		if (pInTextures->At(i))
		{
			nLayers = pInTextures->At(i)->GetNumLayers();
			break;
		}
	}

	if (!bTiled)
	{
		for (int iLayer=0; iLayer<nLayers; iLayer++)
		{
			// create remapped textures - individual .bmp files for each layer
			for (int iAtlas=0; iAtlas<m_nAtlases; iAtlas++)
			{
				TAtlasSizeInfo *pAtlasSize = &m_AtlasSizes[iAtlas];

				//Create lightmap for each mesh node
				U32 *pAtlasTex = (U32*)AllocAligned(pAtlasSize->cX * pAtlasSize->cY * sizeof(U32));

				for (int i=0; i<pAtlasSize->cX*pAtlasSize->cY; i++)
					pAtlasTex[i] = TEXEL_UNUSED_COLOUR_VALUE;

				// blit all the charts that are meant to go into this atlas
				for (int i=0; i<m_ChartMap.Size(); i++)
				{
					if (m_ChartMap[i].iAtlas != iAtlas)
						continue;

					RGBTexture *pChartTex = pInTextures->At(m_ChartMap[i].iChart); // get texture associated with the source chart

					BlitRGBA32(	pAtlasTex, pAtlasSize->cX, pAtlasSize->cY, m_ChartMap[i].x, m_ChartMap[i].y,	// destination
						pChartTex->GetRGBAData(iLayer), pChartTex->m_cX, pChartTex->m_cY);						// source

					if ( iLayer == nLayers-1 )
						pChartTex->FlushRGBAData();
				}

				char *pszName;
				if (nLayers == 1)
					pszName = GenerateLightmapFilename(iAtlas, -1);
				else
					pszName = GenerateLightmapFilename(iAtlas, iLayer);

				//Write lightmap to disk
				switch (m_filetype)
				{
				case LUX_LIGHT_MAP_FILE_TYPE_BMP:
					SaveTexture(pszName, pAtlasTex, pAtlasSize->cX, pAtlasSize->cY, TEXFMT_A8R8G8B8, bWhiteAlpha);
					break;
				case LUX_LIGHT_MAP_FILE_TYPE_DDS:
					SaveTexture(pszName, pAtlasTex, pAtlasSize->cX, pAtlasSize->cY, m_textureformat, bWhiteAlpha);
					break;
				}

				FreeAligned(pAtlasTex);
			}
		}
	}
	else
	{
		// create remapped textures - up to 4 layers are tiled into one texture
		for (int iAtlas=0; iAtlas<m_nAtlases; iAtlas++)
		{
			TAtlasSizeInfo *pAtlasSize = &m_AtlasSizes[iAtlas];

			//Create lightmap for each mesh node
			U32 *pAtlasTex = (U32*)AllocAligned(pAtlasSize->cX*2 * pAtlasSize->cY*2 * sizeof(U32));

			for (int i=0; i<pAtlasSize->cX*2*pAtlasSize->cY*2; i++)
				pAtlasTex[i] = TEXEL_UNUSED_COLOUR_VALUE;

			// blit all the charts that are meant to go into this atlas
			for (int i=0; i<m_ChartMap.Size(); i++)
			{
				if (m_ChartMap[i].iAtlas != iAtlas)
					continue;

				RGBTexture *pChartTex = pInTextures->At(m_ChartMap[i].iChart); // get texture associated with the source chart

				for (int iLayer=0; iLayer<nLayers; iLayer++)
				{
					int dx = (iLayer&1) ? pAtlasSize->cX : 0;
					int dy = (iLayer&2) ? pAtlasSize->cY : 0;

					BlitRGBA32(	pAtlasTex, pAtlasSize->cX*2, pAtlasSize->cY*2, m_ChartMap[i].x+dx, m_ChartMap[i].y+dy,	// destination
						pChartTex->GetRGBAData(iLayer), pChartTex->m_cX, pChartTex->m_cY);								// source
				}

				pChartTex->FlushRGBAData();
			}

			char *pszName = GenerateLightmapFilename(iAtlas);

			//Write lightmap to disk
			switch (m_filetype)
			{
			case LUX_LIGHT_MAP_FILE_TYPE_BMP:
				SaveTexture(pszName, pAtlasTex, pAtlasSize->cX*2, pAtlasSize->cY*2, 0, bWhiteAlpha);
				break;
			case LUX_LIGHT_MAP_FILE_TYPE_DDS:
				SaveTexture(pszName, pAtlasTex, pAtlasSize->cX*2, pAtlasSize->cY*2, 0, bWhiteAlpha);
				break;
			}

			FreeAligned(pAtlasTex);
		}
	}
}

// find any unused block of 4x4 pixels and fill them with the average of all surrounding 4x4 blocks
U32 *PadForDXT(U32 *pAtlasTex, int w, int h)
{
	U32 *pNewAtlasTex = new U32[w*h];
	memcpy(pNewAtlasTex, pAtlasTex, w*h*sizeof(U32));

	// pass 1 = put 1 pixel border into all unused texels
	for (int y=0; y<h; y++)
	{
		for (int x=0; x<w; x++)
		{
			// determine whether this block is empty
			if (pAtlasTex[x + w*y] != TEXEL_UNUSED_COLOUR_VALUE)
				continue;

			U32 nc = TEXEL_UNUSED_COLOUR_VALUE;

			// block of unused pixels - calculate the average of the surrounding blocks
			if (x>0)
				nc = pAtlasTex[w*y+x-1];
			if (y>0 && nc == TEXEL_UNUSED_COLOUR_VALUE)
				nc = pAtlasTex[w*(y-1)+x];
			if (x<w-1 && nc == TEXEL_UNUSED_COLOUR_VALUE)
				nc = pAtlasTex[w*y+x+1];
			if (y>h-1 && nc == TEXEL_UNUSED_COLOUR_VALUE)
				nc = pAtlasTex[w*(y+1)+x];

			// colour to texel
			if (nc != TEXEL_UNUSED_COLOUR_VALUE)
				pNewAtlasTex[x + w*y] = nc;
		}
	}

	// pass 2 = fill for DXT
	for (int y=0; y<h; y+=4)
	{
		for (int x=0; x<w; x+=4)
		{
			bool bUsed = false;

			int iCol = 0;
			// get a colour to fill this block with
			for (int i=0; i<16; i++)
			{
				int tx = x+(i%4);
				int ty = y+(i/4);

				if (tx>=w || ty>=h) continue;
				// check pixel used
				if (pNewAtlasTex[w*ty+tx] != TEXEL_UNUSED_COLOUR_VALUE)
				{
					iCol = pNewAtlasTex[w*ty+tx];
					bUsed = true;
					break;
				}
			}
			// none of the pixels in this square are used, so skip it
			if (!bUsed)
				continue;

			// fill any unused pixels with the fill colour
			for (int i=0; i<16; i++)
			{
				int tx = x+(i%4);
				int ty = y+(i/4);

				if (tx>=w || ty>=h) continue;
				if (pNewAtlasTex[w*ty+tx] == TEXEL_UNUSED_COLOUR_VALUE)
				{
					pNewAtlasTex[w*ty+tx] = iCol;
				}
			}

		}
	}

	return pNewAtlasTex;
}

void SaveTexture(char *pszName, U32 *pAtlasTex, int w, int h, int iFmt, bool bWhiteAlpha)
{
	U32 *pNewAtlasTex = PadForDXT(pAtlasTex, w, h);

	if (bWhiteAlpha)
	{
		for (int i=0; i<w*h; i++)
		{
			pNewAtlasTex[i] |= 0xff000000;
		}
	}

	// create a new image
	OpenIL::Image image(OpenIL::Image::R8G8B8A8, w, h, pNewAtlasTex);

	// save the new image
	// TODO: add support for 16-bit DDS format
	if (!image.Save(pszName))
		S_WARN( g_Log, "*** WARNING: Couldn't save texture '" << pszName << "'.\n")
	else
		S_DEBUG( g_Log, "Saved '" << pszName << "'\n");

	delete pNewAtlasTex;
}

void GetTextureName(const Aftk::C_MaterialResource *pMatRes, std::string &out)
{
	if (!pMatRes)
		return;

	const Aftk::C_MaterialNode *pMat = &pMatRes->GetContent();
	if (!pMat || !pMat->GetNumShaderImageNodes())
		return;

	const Aftk::C_ShaderImageNode *pImgNode = pMat->GetShaderImageNodePtr(0);
	if (!pImgNode)
		return;

	pImgNode->GetFilePlug().GetValue(out);
}

// debug print material splits
void CChartPackRemapInfo::DisplayMaterialSplits(Aftk::C_Scene *pScene, TChartList *pCharts, const Aftk::C_MaterialResource **m_piConversionTable, int nMaterials, int m_nTextures)
{
	int *piSplits = new int[nMaterials];
	int nMaterialsSplit = 0;

	for (int iMat=0; iMat<nMaterials; iMat++)
		piSplits[iMat] = -1;

	for (int iChart=0; iChart<pCharts->Size(); iChart++)
	{
		CTextureChart *pChart = pCharts->At(iChart);
		int iMtl = pChart->m_iMaterial;
		if (iMtl == -1)
			continue;	// skip instances

		int iAtlas = pChart->m_iAtlas;

		if (piSplits[iMtl] == -2)
			continue;

		if (piSplits[iMtl] == -1)
		{
			piSplits[iMtl] = iAtlas;
			continue;
		}
		else if (piSplits[iMtl] == iAtlas)
			continue;

		piSplits[iMtl] = -2;

		// split detected
		std::string texname = "<none>";

		if (!pChart)
		{
			S_DEBUG( g_Log, "pChart was NULL\n");
			continue;
		}

		if (!pChart->m_pMesh)
		{
			S_DEBUG( g_Log, "pChart was NULL\n");
			continue;
		}

		if (pChart->m_iFace == -1)
		{
			S_DEBUG( g_Log, "m_iFace == -1\n");
			continue;
		}

		GetTextureName( GetMaterialResourcePtr( &pChart->m_pMesh->GetPolygon(pChart->m_iFace) ), texname );

		nMaterialsSplit++;

		S_DEBUG( g_Log, "Texture (" << iMtl << ") " << texname << " is split.\n" );
	}
/*
	if (gbDebugMode)
	{

		// colour mesh
		int iPoly = 0;
		for (int iRemap=0; iRemap<m_ChartMap.Size(); iRemap++)
		{
			TChartMapInfo *pRemap	   = &m_ChartMap[iRemap];
			int iChart = pRemap->iChart;
			int iFace;
			CTextureChart *pCurChart   = pCharts->At(iChart);

			// get the first face of this chart
			iFace					   = pCurChart->m_iFace;
			Aftk::C_SceneMeshNode *m_pMesh = pCurChart->m_pMesh;

			// skip instanced geometry
			if (pCurChart->m_pMesh != pCurChart->m_pMatrixNode)
				continue;

			// iterate thru polygons that make up this chart
			while (1)
			{
				// get poly
				Aftk::C_Polygon* pPoly = &m_pMesh->GetPolygon(iFace);

				int iID = m_IntList[iPoly];

				int n = piSplits[iID];

#define FLOAT_PACK_RGBA32(r,g,b,a) ( ( ((int)(a*255))<<24 ) + ( ((int)(b*255))<<16 ) + ( ((int)(g*255))<<8 ) + ((int)(r*255)))

//				SetPolygonColour( pPoly, (n>1)?FLOAT_PACK_RGBA32(1,0,0,0):FLOAT_PACK_RGBA32(0,1,0,0));
				SetPolygonColour( pPoly, (iID == 1675)?FLOAT_PACK_RGBA32(1,0,0,0):FLOAT_PACK_RGBA32(0,1,0,0));

				iPoly++;

				if (iFace == pCurChart->m_iLastFace) break; // was this the last one?
				iFace = POLY_DATA(iFace)->iChartLink;		// get next
			}
		}
	}
*/
	S_INFO( g_Log, nMaterialsSplit << " individual textures were split\n");


	delete [] piSplits;
}

// find a cluster containing the given custom data, or create it if it doesn't exist yet
// returns the index of the cluster on the mesh
int CChartPackRemapInfo::GetPolygonClusterForCustomData(Aftk::C_SceneMeshNode *pMesh, U8 *pData, int c)
{
	// generate a string containing the data in hex - this will be the cluster's name
	Util::C_String text, x;
	for (int i=0; i<c; i++)
	{
		x.Printf("%02x", pData[i]);
		text = text + x;
	}
	std::string _text(text.Chars());

	// see if a polygon cluster with this name already exists
	for (int i=0; i<pMesh->GetNumPolygonClusters(); i++)
	{
		const Aftk::C_PolygonCluster *p = pMesh->GetPolygonClusterPtr(i);
		if (p->GetName() == _text)
		{
			// found it return the index of this cluster
			return i;
		}
	}

	// this cluster doesn't exist yet, so create it
	S32 i = pMesh->AddPolygonCluster();
	if (i == -1)
		return i;
	Aftk::C_PolygonCluster *p = pMesh->GetPolygonClusterPtr(i);
	p->SetName(_text);
	return i;
}
