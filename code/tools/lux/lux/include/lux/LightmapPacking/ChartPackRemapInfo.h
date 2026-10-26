#pragma once

#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/LightmapPacking/PackingRectangle.h"

// store chart mapping
typedef struct
{
	int iChart, x, y, w, h, iAtlas;
} TChartMapInfo;

typedef struct
{
	int cX, cY;
} TAtlasSizeInfo;

class CAttributeTable;
class RGBTexture;

typedef Ut::UtVector< Ut::DynamicVector<TChartMapInfo> > TChartMapInfoList;
typedef Ut::UtVector< Ut::DynamicVector<TAtlasSizeInfo> > TAtlasSizeInfoList;
typedef Ut::UtVector< Ut::DynamicVector<U32> > TMatIDList;
typedef Ut::UtVector< Ut::DynamicVector<Aftk::C_MaterialNode> > TMaterialList;
typedef Ut::UtVector< Ut::DynamicVector<Aftk::C_MaterialResource*> > TMatRestPtrList;


// class that stores the result of the packing algorithm -- move this into a separate CPP file
class CChartPackRemapInfo
{
public:
	// data
	int						m_nAtlases;
	int						m_nCharts;

	int						m_iErrorCode;
	int						m_nNewMaterials;
	int						m_iAtlasOffset;

	TChartMapInfoList		m_ChartMap;
	TAtlasSizeInfoList		m_AtlasSizes;
	Aftk::C_Scene*				m_pAftkScene;
	const char				*m_pszFilenamePrefix;
	Util::C_String			m_sLightingPassPrefix;
	TRGBTextureList			*m_pTextures;

	// temporary members for remapping
	TMatIDList						m_IntList;
	TMaterialList					m_MaterialList;
	CAttributeTable					*m_pAttr;

	// map a material pointer to a unique material ID in our attribute table
	std::map<const Aftk::C_MaterialNode *, int> m_mtlmap;

	TMatRestPtrList					m_MatRestPtrList;
	LUX::LuxLightMapFileTypeEnum	m_filetype;
	LUX::TextureFormat				m_textureformat;
	bool							m_bTiled;

	//
					CChartPackRemapInfo(int iCharts);

	void			RememberMaterial(const Aftk::C_MaterialNode *pMtl, int iID);
	int				GetMaterialID(const Aftk::C_MaterialNode *pMtl);

	void			ApplyMappingToScene(Aftk::C_Scene* pAftkScene);
	void			StoreChartMapping(TChartMapInfo *mapping)		{ m_ChartMap.PushBack(*mapping); }
	void			Add(CMyImage *pImg, int iAtlas);
	void			AddAtlas(TAtlasSizeInfo &info)					{ m_AtlasSizes.PushBack(info); }
	void			GenerateAtlases(TRGBTextureList *pInTextures, bool bTiled, bool bWhiteAlpha);
	void			ApplyMappingToScene(Aftk::C_Scene* pAftkScene, TRGBTextureList *pTextures, TChartList *pCharts, bool bUVsOnly = false, bool bTiled = false);
	void			SetLightmapMaterialToPolygon(Aftk::C_Polygon *pPoly, int iMat, int iAtlas);
	const Aftk::C_MaterialResource *GenerateNewMaterial(Aftk::C_MaterialNode &newMaterial, int iAtlas );
	const Aftk::C_MaterialResource *GetMaterialCombo(int iMat, int iAtlas);

	void			GetMaterialsFromAftkManager();
	void			AssociatePolygonsWithMaterialIDs(TChartList *pCharts);

	void			ApplyMappingToInstancedGeometry(CTextureChart *pCurChart, TChartMapInfo *pRemap, TAtlasSizeInfo *pAtlasSize);
	void			RemapUV(float *pX, float *pY, TChartMapInfo *pRemap, TAtlasSizeInfo *pAtlasSize);
	void			SetInstanceParameters(Aftk::C_SceneMeshRemappableNode *pNode, TChartMapInfo *pRemap, TAtlasSizeInfo *pAtlasSize, int iLMapUVSet, bool bBlindDataOnly);

	char			*GenerateLightmapFilename(int iAtlas, int iLayer = -1);

	void			DisplayMaterialSplits(Aftk::C_Scene *pScene, TChartList *pCharts, const Aftk::C_MaterialResource	**, int nMaterials, int nTex);

	void			BlitRGBA32(	U32 *pDest, int cDestX, int cDestY, int iDestX, int iDestY,
								U32 *pSrc,	int cSrcX,	int cSrcY);

private:
	int				GetPolygonClusterForCustomData(Aftk::C_SceneMeshNode *pMesh, U8 *pData, int c);
};

// TODO: move these utility functions somewhere else
void SaveTexture(char *pszName, U32 *pAtlasTex, int w, int h, int iFmt, bool bWhiteAlpha);
