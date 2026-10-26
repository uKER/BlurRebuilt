#pragma once

#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/LightmapPacking/PackingRectangle.h"
#include "Lux/LightmapPacking/ChartPackRemapInfo.h"
#include "Lux/Parameterization/SuperGroups.h"

// free image class - stores a downsampled version of the image with usage information
class CFreeImage
{
public:
    int			w, h;
	char		*pData;
	bool		m_bHasFreeSpace;

				CFreeImage(int w, int h);
				~CFreeImage();
	void		Set(int x, int y, bool bUsed);
	bool		Get(int x, int y);
};

class GroupResult;
class SuperGroupBuilder;

typedef enum
{
	LMPM_Simple,					// pack lightmaps from big to small
	LMPM_MixedCell,					// pack lightmaps per cell (cells are assigned by supergroup), empty spaces are filled with other cells
	LMPM_Cell,						// pack lightmaps per cell (cells are assigned by supergroup)
	LMPM_SuperGroup,				// pack lightmaps per cell, ordered by supergroup
	LMPM_Clustering,				// pack lightmaps using clustering
};

// class that generates packed atlases from a list of CTextureCharts
class CLightmapPacking
{
public:
							CLightmapPacking();
							~CLightmapPacking();

	int						m_iTexturePackRes;
	float					m_fMeterSize;
	bool					m_bSuccess;
	int						m_iCurrentAtlas;
	float					m_fSmallestArea;

	// these variables govern different ways of packing lightmaps
	int						m_iPackMode;		// mode
	bool					m_bRepack;			// true to turn on re-packing (not advised!)
	float					m_fGridSize;		// size of the individual cells (3D)
	float					m_fMaxClusterArea;
	float					m_fMaxClusterSize;
	float					m_fTreeRadius;		// active radius of the tree search algorithm
	//

	TMyRectList				m_FreeRects;		// list of rectanbles with empty space
	TpMyImageList			*m_pImageLists;	// list of charts to be packed - one list for each gridspace
	CMyImage				*m_pImageArray;		// array of image pointers
	TChartList				*m_pCharts;

	CChartPackRemapInfo		*m_pPackingInfo;	// resulting output (packing info)
	SuperGroupBuilder		*m_pSuperGroups;

	float3					m_vNodeOffset, m_vNodeSize;
	int						m_iCurCell;
	int						m_iGridSize[3];
	int						m_nCells;

	bool					Pack(TRGBTextureList *pTextures, TChartList *pCharts, bool bSilent=false); // main processing function
	bool					DebugPack(TRGBTextureList *pInTextures, TChartList *pInCharts);

private:
	void					PackImageList();
	void					DoPackImageList(TpMyImageList *pTmpImageList);
	void					PackImageListSimple(TpMyImageList *pTmpImageList);
	void					PackImageListBySuperGroup(TpMyImageList *pTmpImageList);
	void					PackImageListTree();
	bool					TryInsertImage(CMyImage *pImg, bool bTest);
	void					ClipFreeRectList(CMyImage *pImg);
	void					CalculatePackingQuality();
	void					RemoveEmptyChartsFromList(TRGBTextureList &texlist, TChartList &charts);
	bool					TestPackImageList(TpMyImageList *pList, bool bTest);

	void					AddNewFreeRectList(TMyRectList *pFree, TMyRectList *pList);
	void					AddNewFreeRect(TMyRectList *pFree, CMyRect *pNewRect);
	void					AddNewFreeRectVector(TMyRectList *pFree, TMyRectVector *pList);

	int						FindFreeRectanglesInChart(CMyImage *pImg, RGBTexture *pTex);
	void					AddFreeRects(TMyRectVector *pList, CFreeImage *pImg, int x, int y, int iPixelSize);
	void					FindColumns(TMyRectVector *pList, CFreeImage *pImg, int x, int y, int h, int iPixelSize);
	bool					ScanColumn(CFreeImage *pImg, int x, int y, int h);
	void					ExtendFreeRects(CMyImage *pImg, RGBTexture *pTex);
	void					ExtendImageRects(TMyRectList *pList);
	void					AllocImageLists();
	void					InitializeFreeRectsList(bool bTest = false);
	bool					TestPackGroupList(Ut::Vector<SuperGroup*> &grouplist, bool btest, int *piArea);

	//
	bool					AddGroupsUntilFull(TGroupList &groups, Ut::Vector<GroupResult> &result, Ut::Vector<SuperGroup*> &outlist);
	void					PackLightmapsUsingClustering(TChartList &charts, SuperGroupBuilder *pSuperGroups, bool bSilent);

	TpMyImageList			*GetImageList(float3 vPos);
};
