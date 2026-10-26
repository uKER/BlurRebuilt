#include <stdafx.h>

#include "Lux/RenderChannel.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/LightingModel.h"
#include "Lux/GlobalIllumination/GlobalIlluminationModel.h"
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
#include "CServ/ClusterServHeaders.h"
#include "Lux/GlobalIllumination/ClientSide.h"
#include "Lux/Instantiate.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/SamplePoints.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/SamplePointIterator.h"
#include "lux_common/LightingPass.h"
#include "Lux/DebugCode.h"
#include "Lux/Common.h"
#include "Lux/SHLighting.h"
#include "Lux/VtxLighting.h"
#include "Lux/GlobalIllumination/ProgressiveBounceLighting.h"
#include <spatial_hierarchy/UserDataIterator.h>
#include "lux_common\TexPreprocess.h"

#include <algorithm>

using namespace std;

namespace LUX
{

ost::Engine& operator << (ost::Engine& ar, BounceChart &T)
{
	ar << T.m_transform.uTransform << T.m_transform.vTransform << T.m_iUVData << T.m_pTex;
	return ar;
}

ost::Engine& operator >> (ost::Engine& ar, BounceChart &T)
{
	ar >> T.m_transform.uTransform >> T.m_transform.vTransform >> T.m_iUVData >> T.m_pTex;
	return ar;
}

/// convert RGB16 to float3 and back
U16 Float3ToRGB16(const float3 &v)
{
	int r = v.x * 31;
	int g = v.y * 63;
	int b = v.z * 31;

	return r + (g*32) + (b*2048);
}

float3 RGB16ToFloat3(U16 rgb)
{
	int r = rgb & 31;
	int g = (rgb/32) & 63;
	int b = (rgb/2048) & 31;

	return float3(float(r)/31.f, float(g)/63.f, float(b)/31.f);
}

// generate a map that can map nodes GUIDs to chart indices
void BounceTexData::MapNodesToCharts(SceneUnwrapper *pUnwrapper)
{
	for (int i=0; i<pUnwrapper->m_ChartList.Size(); i++)
	{
		if (pUnwrapper->m_ChartList[i] &&
			pUnwrapper->m_ChartList[i]->m_pMatrixNode)
			m_ChartMap[pUnwrapper->m_ChartList[i]->m_pMatrixNode->GetUniqueId()] = i;
	}
}

// return the index of the chart that renders the given node
int BounceTexData::GetChartThatRendersNode(Aftk::C_SceneNode *pNode)
{
	Ut::UtMap<S32, int>::Iterator iter = m_ChartMap.Find(pNode->GetUniqueId());
	if (iter == m_ChartMap.End())
	{
		// couldn't find the instance. this object will not cast bounce lighting
		// TODO: check for vertex lit instance
		return -1;
	}
	else
		return iter->GetValue() | BOUNCE_LIGHTING_LIGHTMAPPED;
}

// store the chart index of each triangle into the spatial hierarchy
int BounceTexData::BounceGetChartIdx(Aftk::C_SceneNode *pNode, SceneUnwrapper *pUnwrapper, VtxLighting *pVtxLighting, int iFace)
{
	// if it's an instance, find the chart that renders it and return the index of that
	if (pNode->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance ))
		return GetChartThatRendersNode(pNode);

	// if it's normal mesh, store the chart index
	if (pNode->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		Aftk::C_SceneMeshNode *m_pMesh = (Aftk::C_SceneMeshNode*)pNode;
		if (iFace == -1)
			return -1;
		LMPolyAttributes *pAttr = POLY_DATA(iFace);

		if (!pAttr)
		{
			// this object was not lightmapped. check for vertex colours
			int id = pNode->GetUniqueId();

			// find vtx data index for this node
			Ut::UtMap<S32, U32>::Iterator iter = m_map.Find(id);
			if (iter == m_map.End())
				return -1; // no vertex data for this node

			// return vtx lit data index
			return BOUNCE_LIGHTING_VTXLIT | iter->GetValue() + 3*iFace;
		}
		else if (pAttr->iChart<0)	// this polygon has "do not light" on it
			return -1;

		CTextureChart *pChart = pUnwrapper->m_ChartList[pAttr->iChart];
		if (!pChart)
		{
			// it's been moved which means it's probably a base instance.
			assert(pNode->GetNumInstancesOfThisNode() > 0);

			// find the chart that renders the base instance and return the index of that
			return GetChartThatRendersNode(pNode);
		}

		// return the chart index - default behavior
		if (pAttr)
			return BOUNCE_LIGHTING_LIGHTMAPPED | pAttr->iChart;
	}

	// return -1 - this object will not cast bounce lighting
	return -1;
}

/// get the average colour of the texture on a particular polygon
U16 BounceGetColour(TexturePreprocessor &texpp, Aftk::C_SceneNode *pNode, int iFace)
{
	if (!pNode->HasType(Aftk::E_ObjType_ISceneMesh))
		return -1;

	Aftk::I_SceneMeshNode *pMesh = (Aftk::I_SceneMeshNode*)pNode;
	if (pMesh->GetNumPolygons(0)<iFace)
		return -1;

	TextureData *pData = texpp.GetTextureData(&pMesh->GetPolygon(iFace));
	if (!pData)
		return -1;

	return Float3ToRGB16(pData->GetAverageColour());
}

/// generate bounce lighting data for each primitive in the spatial hierarchy
void BounceTexData::SetBounceLightingUserData(LUX::SpatialHierarchy *pHierarchy, SceneUnwrapper *pUnwrapper, VtxLighting *pVtxLighting, bool bColouredBounceLighting)
{
	TexturePreprocessor texpp;

	int iVtxLightingItem = 0;

	// use the new SD user data iterator
	for (SD::UserDataIterator iter(pHierarchy); !iter.End(); ++iter)
	{
		LUX::PrimInfo info;

		if (iter.GetMode() == SD::UserDataIterator::Mode_BaseInstances)
		{
			// base instance primitive
			info.iRGB = 0;
			info.iChart = iter.GetFaceIndex();
		}
		else
		{
			// triangle primitive
			if (bColouredBounceLighting)
				info.iRGB = BounceGetColour(texpp, iter.GetSceneNode(), iter.GetFaceIndex());
			else
				info.iRGB = 0xffff;
			info.iChart = BounceGetChartIdx(iter.GetSceneNode(), pUnwrapper, pVtxLighting, iter.GetFaceIndex());
		}

		pHierarchy->SetAdditionalPrimInfo(iter.GetPrimitiveIndex(), info);
	}
}

/// sample this chart at the given UV coordinates
float4 BounceChart::Sample(float u, float v)
{
	if (!m_pTex)
		return float4(0,0,0,0);

	U32 *pData = m_pTex->GetRGBAData(0);
	int x = u * m_pTex->m_cX;
	int y = v * m_pTex->m_cY;

	if (x<0) x=0;
	if (y<0) y=0;
	if (x>=m_pTex->m_cX) x = m_pTex->m_cX-1;
	if (y>=m_pTex->m_cY) y = m_pTex->m_cY-1;

	float4 vRtn;
	U8 *p = (U8*)&pData[x + m_pTex->m_cX * y];
	vRtn.x = (float)p[0] / 255.0f;
	vRtn.y = (float)p[1] / 255.0f;
	vRtn.z = (float)p[2] / 255.0f;
	vRtn.w = (float)p[3] / 255.0f;

	return vRtn;
}

/// add the given texture to the current texture (additive)
void BounceChart::AddTexture(RGBTexture *pTex)
{
	U32 *pSrc = pTex->GetRGBAData(0);
	U32 *pDst = m_pTex->GetRGBAData(0);
	for (int i=0; i<(pTex->m_cX * pTex->m_cY); i++)
	{
		for (int j=0; j<4; j++)
		{
			// add the two RGBA's together and make sure to clamp to 255
			int a = (int)((U8*)(&pDst[i]))[j] + (int)((U8*)(&pSrc[i]))[j];
			if (a > 255)
				a = 255;
			((U8*)(&pDst[i]))[j] = a;
		}		
	}
}

/// destructor
BounceChart::~BounceChart()
{
}

// get index to UV data for a node, generate UV data if needed
U32 BounceTexData::GetUVData(Aftk::C_SceneMeshNode *pMesh, int iLightmapUVSet)
{
	int id = pMesh->GetUniqueId();
	// do we already have the UV data for this node?
	Ut::UtMap<S32, U32>::Iterator iter = m_map.Find(id);
	if (iter != m_map.End())
		return iter->GetValue();

	// generate new UV data reference
	int iStart = m_pBounceUVList->Size();
	m_map[id] = iStart;

	// store the lightmap UV coordinates for each triangle on this mesh
	for (int iPoly=0; iPoly<pMesh->GetNumPolygons(0); iPoly++)
	{
		Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(iPoly);
		for (int i=0; i<3; i++)
		{
			const Aftk::C_PolygonVertex &vert = pPoly->GetPolygonVertex(i);
			U32 idx = vert.GetTextureCoordinateIndex(iLightmapUVSet);
			const Aftk::TextureCoordinate4 &uv = pMesh->GetTextureCoordinate(iLightmapUVSet, idx);
			m_pBounceUVList->PushBack(float2(uv.x, uv.y));
		}
	}

	return iStart;
}

// get index to Vtx data for a node, generate vtx data if needed
U32 BounceTexData::GetVtxData(Aftk::C_SceneNode *pNode)
{
	// only handle meshnodes at the moment, not instances
	if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
		return -1;

	Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pNode;

	int id = pMesh->GetUniqueId();
	// do we already have the vtx data for this node?
	Ut::UtMap<S32, U32>::Iterator iter = m_map.Find(id);
	if (iter != m_map.End())
		return iter->GetValue();

	int iStart = m_BounceVtxList.Size();
	m_map[id] = iStart;

	// store an entry for each vertex for each polygon
	int nVerts = pMesh->GetNumPolygons(0) * 3;
	m_BounceVtxList.Resize(m_BounceVtxList.Size() + nVerts);
	memset(&m_BounceVtxList[0], 0, m_BounceVtxList.Size() * sizeof(float4));

	return iStart;
}

// generate bounce lighting data
void BounceTexData::Generate(LUX::SpatialHierarchy *pHierarchy, SceneUnwrapper *pUnwrapper, VtxLighting *pVtxLighting, bool bColouredBounceLighting)
{
	// generate bounce lightmap data (UV data)
	m_pBounceTexList = new TBounceChartList;
	m_pBounceUVList = new TBounceUVList;

	int nBounceCharts = pUnwrapper->m_ChartList.Size();
	m_pBounceTexList->Resize(nBounceCharts);
	for (int iChart=0; iChart<nBounceCharts; iChart++)
	{
		if (iChart >= pUnwrapper->m_ChartMatrixList.Size())
		{
			// no UV matrix. this is probably an instance. copy the UV data directly from the source object and reference that
			if (pUnwrapper->m_ChartList[iChart])
				m_pBounceTexList->At(iChart).m_iUVData = GetUVData(pUnwrapper->m_ChartList[iChart]->m_pMesh, pUnwrapper->m_ChartList[iChart]->m_iLMapUVSet);
			else
				m_pBounceTexList->At(iChart).m_iUVData = -2;	// can't generate bounce data for this object
		}
		else
		{
			float4 v = pUnwrapper->m_ChartMatrixList[iChart].uTransform;
			// normal mesh. use the UV matrix
			m_pBounceTexList->At(iChart).m_transform.uTransform = pUnwrapper->m_ChartMatrixList[iChart].uTransform;
			m_pBounceTexList->At(iChart).m_transform.vTransform = pUnwrapper->m_ChartMatrixList[iChart].vTransform;
			m_pBounceTexList->At(iChart).m_iUVData = -1;
		}
		m_pBounceTexList->At(iChart).m_pTex = NULL;
	}

	m_map.Clear();

	// generate bounce vtx lighting data. map VtxLighting items to our global vertex bounce lighting array
	int n = pVtxLighting->GetNumItems();
	m_BounceVtxItemList.Resize(n);
	for (int iItem=0; iItem<n; iItem++)
	{
		VtxLighting::Item *pItem = pVtxLighting->GetItem(iItem);
		U32 i = GetVtxData(pItem->pNode);
		m_BounceVtxItemList[iItem] = i;
	}

	MapNodesToCharts(pUnwrapper);

	SetBounceLightingUserData(pHierarchy, pUnwrapper, pVtxLighting, bColouredBounceLighting);

	m_map.Destroy();
	m_ChartMap.Destroy();
}

float DotProduct(float4 a, float4 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}


// sample the bounce lighting
float4 BounceTexData::Sample(SD::RayTraceResult &result, const LUX::SpatialHierarchy *pHierarchy)
{
	if (result.iGlobalPrim == -1)
		return float4(0,0,0,0); // no primitive index, no bounce lighting

	int iChart = pHierarchy->GetAdditionalPrimInfo(result.iGlobalPrim).iChart;
	U16 iRGB = pHierarchy->GetAdditionalPrimInfo(result.iGlobalPrim).iRGB;
	if (iChart == -1)
		return float4(0,0,0,0);	// no bounce lighting

	switch (iChart & BOUNCE_LIGHTING_FLAGS)
	{
	case BOUNCE_LIGHTING_LIGHTMAPPED:
		{
		if (m_pBounceTexList->Size() <= (iChart&~BOUNCE_LIGHTING_FLAGS))
		{
			LOGERROR( g_Log, "Internal error in BounceTexData::Sample (iChart = %x(%x), iGlobalPrim = %d, iGlobalSubPrim = %d)\n", iChart, m_pBounceTexList->Size(), result.iGlobalPrim, result.iGlobalSubPrim);
			return float4(0,0,0,0);
		}

		BounceChart *pChart = &m_pBounceTexList->At(iChart&~BOUNCE_LIGHTING_FLAGS);
		float u,v;

		if (pChart->m_iUVData == -2)
			return float4(0,0,0,0);
		if (pChart->m_iUVData == -1)
		{
			// calculate UV using the UV transform matrix (used for normal, lightmapped mesh)
			float3 vPos3 = result.GetPosition();
			float4 vPos4(vPos3.x, vPos3.y, vPos3.z, 1);

			u = DotProduct(pChart->m_transform.uTransform, vPos4);
			v = DotProduct(pChart->m_transform.vTransform, vPos4);
		}
		else if (result.iPolygon != -1)
		{
			// read UVs from the stored UV data (used for instances)
			float3 bary(result.u, result.v, 1.0f - result.u - result.v);

			// lerp between the three UVs using barycentric co-ordinates
			int iPolygon = pHierarchy->GetAdditionalPrimInfo(result.iGlobalPrim).iChart * 3;
			iRGB = pHierarchy->GetAdditionalPrimInfo(result.iGlobalPrim).iRGB;
			float2 uv[3];
			for (int i=0; i<3; i++)
				uv[i] = m_pBounceUVList->At(pChart->m_iUVData + iPolygon + i);

			u = uv[0].x * bary.x + uv[1].x * bary.y + uv[2].x * bary.z;
			v = uv[0].y * bary.x + uv[1].y * bary.y + uv[2].y * bary.z;
		}
		else
			return float4(0,0,0,0);

		float3 vTexCol = RGB16ToFloat3(iRGB);
		float4 _vTexCol(vTexCol.x, vTexCol.y, vTexCol.z, vTexCol.y);

		// sample texture at UV
		return pChart->Sample(u,v) * _vTexCol;
		}
	case BOUNCE_LIGHTING_VTXLIT:
		{
			// lerp between the three vertex colours using barycentric co-ordinates
			float3 bary(result.u, result.v, 1.0f - result.u - result.v);

			int iVtxDat = iChart&~BOUNCE_LIGHTING_FLAGS;
			iRGB = pHierarchy->GetAdditionalPrimInfo(result.iGlobalPrim).iRGB;
			float4 col[3];
			for (int i=0; i<3; i++)
				col[i] = m_BounceVtxList[iVtxDat+i];

			float4 vResult = (col[0] * (float)bary.x) + (col[1] * (float)bary.y) + (col[2] * (float)bary.z);

			return vResult;
		}
	default:
		assert(0);
		return float4(0,0,0,0); // should never happen
	}
}

bool BounceTexData::Validate()
{
	for (int i=0; i<m_BounceVtxList.Size(); i++)
	{
		if (CheckBuggeredFloat(m_BounceVtxList[i].x) ||
			CheckBuggeredFloat(m_BounceVtxList[i].y) ||
			CheckBuggeredFloat(m_BounceVtxList[i].z) ||
			CheckBuggeredFloat(m_BounceVtxList[i].w) )
		{
			return false;
		}
	}

	return true;
}

// set vertex bounce data for next bounce pass
void BounceTexData::SetVertexData(VtxLightingResult *pResult, int n, bool *pbFoundQNAN)
{
	Validate();

	int iStart = m_BounceVtxItemList[pResult->m_iItem];
	if (iStart == -1)
	{
		// this can happen if the object didn't have bounce vertex colours rendered for it (e.g. currently instances aren't supported)

		return;
	}

	if (pResult->m_iStride == 4)
	{
		// convert datatype to float4 (should in theory be float4 already)
		int iData = 0;
		for (int i=0; i<n; i++, iData += pResult->m_iStride)
		{
			float4 v;
			v.x = pResult->m_Data[iData+0];
			v.y = pResult->m_Data[iData+1];
			v.z = pResult->m_Data[iData+2];
			if (pResult->m_iStride > 3)
				v.w = pResult->m_Data[iData+3];
			else
				v.w = 0;
			m_BounceVtxList[iStart + i] = v;
		}
	}

	if (!Validate())
		*pbFoundQNAN = true;
}

// free all data
void BounceTexData::Purge()
{
	if (m_pBounceTexList)
	{
		for (int i=0; i<m_pBounceTexList->Size(); i++)
			delete m_pBounceTexList->At(i).m_pTex;

		m_pBounceTexList->Destroy();
		delete m_pBounceTexList;
		m_pBounceTexList = NULL;
	}
	if (m_pBounceUVList)
	{
		m_pBounceUVList->Destroy();
		delete m_pBounceUVList;
		m_pBounceUVList = NULL;
	}
}

BounceTexData::BounceTexData()
{
	m_pBounceTexList = NULL;
	m_pBounceUVList = NULL;
}

BounceTexData::~BounceTexData()
{
	Purge();
}

// read/write BounceTexData class
ost::Engine& operator << (ost::Engine& ar, BounceTexData &T)
{
	T.Validate();
	if (T.m_pBounceTexList && T.m_pBounceUVList)
	{
		ar << 1;
		ar << *T.m_pBounceTexList << *T.m_pBounceUVList;
	}
	else
		ar << 0;
	ar << T.m_BounceVtxList;
	return ar;
}

ost::Engine& operator >> (ost::Engine& ar, BounceTexData &T)
{
	int iOk;
	ar >> iOk;
	if (iOk)
	{
		T.m_pBounceTexList = new TBounceChartList;
		T.m_pBounceUVList = new TBounceUVList;
		ar >> *T.m_pBounceTexList >> *T.m_pBounceUVList;
	}
	else
	{
		T.m_pBounceTexList = NULL;
		T.m_pBounceUVList = NULL;
	}
	ar >> T.m_BounceVtxList;
	T.Validate();
	return ar;
}

} // namespace LUX
