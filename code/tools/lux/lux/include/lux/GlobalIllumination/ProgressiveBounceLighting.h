#ifndef __PROGRESSIVE_BOUNCE_LIGHTING_H__
#define __PROGRESSIVE_BOUNCE_LIGHTING_H__

class RGBTexture;

namespace SD
{
	class SD::RayTraceResult;
};

namespace LUX
{

class SceneUnwrapper;
class VtxLighting;
class VtxLightingResult;

#define BOUNCE_LIGHTING_LIGHTMAPPED	0	
#define BOUNCE_LIGHTING_VTXLIT		0x80000000
#define BOUNCE_LIGHTING_FLAGS		0x80000000

// 4x2 matrix to transform world-space co-ordinates to texture uv's
struct UVTransform
{
	float4		uTransform;
	float4		vTransform;
};

class BounceChart
{
public:
	int			m_iUVData;
	UVTransform m_transform;
	RGBTexture	*m_pTex;		// texture

						/// destructor
						~BounceChart();
						/// sample this chart at the given UV coordinates
	float4				Sample(float u, float v);
						/// add the given texture to the current texture (additive)
	void				AddTexture(RGBTexture *pTex);
};

class BounceVtxItem
{
	float4 v;
};

typedef Ut::Vector<BounceChart> TBounceChartList;
typedef Ut::Vector<float2> TBounceUVList;
typedef Ut::Vector<int> TBounceVtxItemList;
typedef Ut::Vector<float4> TBounceVtxList;

class BounceTexData
{
public:
						BounceTexData();
						~BounceTexData();
	void				Generate(LUX::SpatialHierarchy *pHierarchy, SceneUnwrapper *pUnwrapper, VtxLighting *pVtxLighting, bool bColouredBounceLighting);
	void				Purge();
	float4				Sample(SD::RayTraceResult &result, const LUX::SpatialHierarchy *p);
	U32					GetUVData(Aftk::C_SceneMeshNode *pMesh, int iLightmapUVSet);
	U32					GetVtxData(Aftk::C_SceneNode *pNode);
	int					BounceGetChartIdx(Aftk::C_SceneNode *pNode, SceneUnwrapper *pUnwrapper, VtxLighting *pVtxLighting, int iFace);
	void				SetBounceLightingUserData(LUX::SpatialHierarchy *pHierarchy, SceneUnwrapper *pUnwrapper, VtxLighting *pVtxLighting, bool bColouredBounceLighting);
	void				SetVertexData(VtxLightingResult *pResult, int n, bool *pbFoundQNAN);
	bool				Validate();
	void				MapNodesToCharts(SceneUnwrapper *pUnwrapper);
	int					GetChartThatRendersNode(Aftk::C_SceneNode *pNode);

	TBounceChartList	*m_pBounceTexList;
	TBounceUVList		*m_pBounceUVList;
	TBounceVtxItemList	m_BounceVtxItemList;	// for each vtx lit node, an index into m_BounceVtxList
	TBounceVtxList		m_BounceVtxList;		// vertex lighting data for each polygon for each vtx lit node

	Ut::UtMap<S32, U32> m_map;		// map Aftk SceneNode GUIDs to UV data reference index
	Ut::UtMap<S32, int>	m_ChartMap;	// map Aftk SceneNode GUIDs to chart index
};

ost::Engine& operator << (ost::Engine& ar, BounceChart &T);
ost::Engine& operator >> (ost::Engine& ar, BounceChart &T);

ost::Engine& operator << (ost::Engine& ar, BounceTexData &T);
ost::Engine& operator >> (ost::Engine& ar, BounceTexData &T);

int BounceChartIdxCallback(Aftk::C_SceneNode *m_pMesh, void *pContext, int iFace);
void SetBounceLightingUserData(LUX::SpatialHierarchy *pHierarchy, SceneUnwrapper *pUnwrapper, VtxLighting *pVtxLighting);

}

#endif
