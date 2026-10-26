//-----------------------------------------------------------------------------
// File: LightmapPacking.cpp
//
// Desc: Pack individual charts into atlases
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "Lux/LightmapPacking/PackingRectangle.h"
#include "Lux/LightmapPacking/LightmapPacking.h"
#include "Lux/Parameterization/SuperGroups.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include <spatial_hierarchy/SpatialDividerHeaders.h>
#include <spatial_hierarchy/Kd-TreeBuilder.h>
#include "Lux/LightmapPacking/AABBBuilder.h"
#include "Lux/DebugCode.h"
#include "Lux/LightmapPacking/Clustering.h"
#include <algorithm>

using namespace std;

//#define DISABLE_PACKING
#define LIGHTMAP_DEFAULT_ATLAS_SIZE			1024.0f
#define DEFAULT_PACKING_ALGORITHM			LMPM_Clustering
#define DEFAULT_CELL_SIZE					200.0f
//#define ALIGN_4

// ----------------------------------------------------------------------------

CFreeImage::CFreeImage(int iw, int ih)
{
	// create freeImage data
	w = iw, h = ih;
	pData = new char[w * h];
	memset(pData, 0, sizeof(char) * w * h);
	m_bHasFreeSpace = false;
}

// delete freeimage data
CFreeImage::~CFreeImage()
{
	delete [] pData;
	pData = NULL;
}

// set pixel on or off
void CFreeImage::Set(int x, int y, bool bUsed)
{
	assert(x>=0 && x<w && y>=0 && y<h);

	pData[x + y*w] = bUsed;
	m_bHasFreeSpace |= !bUsed;
}
// get state of pixel
bool CFreeImage::Get(int x, int y)
{
	assert(x>=0 && x<w && y>=0 && y<h);

	return pData[x + y*w] != 0;
}

// ----------------------------------------------------------------------------

CLightmapPacking::CLightmapPacking()
{
	m_fSmallestArea		= 4.0f;
	m_iTexturePackRes	= (int)LIGHTMAP_DEFAULT_ATLAS_SIZE;
	m_bSuccess			= true;
	m_iPackMode			= DEFAULT_PACKING_ALGORITHM;
	m_fGridSize			= DEFAULT_CELL_SIZE;
	m_bRepack			= false;
	m_pSuperGroups		= NULL;
	m_pPackingInfo		= NULL;
	m_fTreeRadius		= 1000.0f;
}

// destroy packing info
CLightmapPacking::~CLightmapPacking()
{
	if (m_pPackingInfo)
	{
		delete m_pPackingInfo;
		m_pPackingInfo = NULL;
	}
	if (m_pSuperGroups)
	{
		delete m_pSuperGroups;
		m_pSuperGroups = NULL;
	}
}

// add a free rectangle to the list that's already there
// check for redundant overlapping rectangles, etc.
void CLightmapPacking::AddNewFreeRect(TMyRectList *pFree, CMyRect *pNewRect)
{
	bool bAdd = true;

	assert(!(pNewRect->x&3) && !(pNewRect->y&3));

	// check the new rectangle against all the existing ones
	for (RectIterator i1=pFree->begin(); i1!=pFree->end(); )
	{
		CMyRect *pRect = &i1->data;

		// do the two overlap at all?
		if ((pNewRect->x + pNewRect->w) <= pRect->x)	{ ++i1; continue; }
		if ((pNewRect->y + pNewRect->h) <= pRect->y)	{ ++i1; continue; }
		if ((pRect->x + pRect->w) <= pNewRect->x)		{ ++i1; continue; }
		if ((pRect->y + pRect->h) <= pNewRect->y)		{ ++i1; continue; }

		// Rects are overlapping..
		if (pRect->x >= pNewRect->x)
		{
			if ((pRect->x + pRect->w) > (pNewRect->x + pNewRect->w))	{ ++i1; continue; }
			if (pRect->y < pNewRect->y)									{ ++i1; continue; }
			if ((pRect->y + pRect->h) > (pNewRect->y + pNewRect->h))	{ ++i1; continue; }
		}
		else
		{
			if ((pNewRect->x + pNewRect->w) > (pRect->x + pRect->w))	{ ++i1; continue; }
			if (pNewRect->y < pRect->y)									{ ++i1; continue; }
			if ((pNewRect->y + pNewRect->h) > (pRect->y + pRect->h))	{ ++i1; continue; }
		}

		// Ok.. One overlaps the other..
		if ((pRect->w * pRect->h) < (pNewRect->w * pNewRect->h))
		{
			i1 = pFree->erase(i1);
			bAdd = true;
		}
		else
		{
			bAdd = false;
			++i1;
		}

		break;
	}

	// add the new rectangle to our free list
	if (bAdd)
		pFree->push( *pNewRect, -(pNewRect->w+pNewRect->x) * (pNewRect->y+pNewRect->h) );
}

// add a vector of free rectangles to our existing list
void CLightmapPacking::AddNewFreeRectList(TMyRectList *pFree, TMyRectList *pList)
{
	bool bAdd;

	for (TMyRectList::List_Iterator i=pList->begin(); i!=pList->end(); ++i)
	{
		CMyRect *pNewRect = &i->data;

		AddNewFreeRect(pFree, pNewRect);
	}
}

// add a list of free rectangles to our existing list
void CLightmapPacking::AddNewFreeRectVector(TMyRectList *pFree, TMyRectVector *pList)
{
	bool bAdd;

	for (TMyRectVector::iterator i=pList->begin(); i!=pList->end(); ++i)
	{
		CMyRect *pNewRect = &(*i); // (JAN: I'm too stupid to know a better way to do this)

		AddNewFreeRect(pFree, pNewRect);
	}
}



// clip all existing free rectangles against the newly added chart
void CLightmapPacking::ClipFreeRectList(CMyImage *pImg)
{
	CMyRect			newRect;
	TMyRectVector	newRectList;

	newRectList.reserve(100);

	for (RectIterator i=m_FreeRects.begin(); i!=m_FreeRects.end(); )
	{
		CMyRect *pRect = &i->data;

		int iw = (pImg->w+3)&~3;
		int ih = (pImg->h+3)&~3;

		// check whether the two areas touch at all
		if ((pRect->x + pRect->w) <= pImg->x) { ++i; continue; }
		if ((pRect->y + pRect->h) <= pImg->y) { ++i; continue; }
		if ((pImg->x + iw) <= pRect->x) { ++i; continue; }
		if ((pImg->y + ih) <= pRect->y) { ++i; continue; }

		// generate free rect to the left of the chart
		if (pImg->x > pRect->x)
		{
			newRect.x = pRect->x;
			newRect.y = pRect->y;
			newRect.w = pImg->x - pRect->x;
			newRect.h = pRect->h;

		assert(!(newRect.x&3) && !(newRect.y&3));

			newRectList.push_back( newRect );
		}
		// generate free rect to the right of the chart
		if ((pImg->x + iw) < (pRect->x + pRect->w))
		{
			newRect.x = pImg->x + iw;
			newRect.y = pRect->y;
			newRect.w = (pRect->x + pRect->w) - (pImg->x + iw);
			newRect.h = pRect->h;

		assert(!(newRect.x&3) && !(newRect.y&3));

			newRectList.push_back( newRect );
		}
		// generate free rect above the chart
		if (pImg->y > pRect->y)
		{
			newRect.x = pRect->x;
			newRect.y = pRect->y;
			newRect.w = pRect->w;
			newRect.h = pImg->y - pRect->y;

		assert(!(newRect.x&3) && !(newRect.y&3));

			newRectList.push_back( newRect );
		}
		// generate free rect below the chart
		if ((pImg->y + ih) < (pRect->y + pRect->h))
		{
			newRect.x = pRect->x;
			newRect.y = pImg->y + ih;
			newRect.w = pRect->w;
			newRect.h = (pRect->y + pRect->h) - (pImg->y + ih);

		assert(!(newRect.x&3) && !(newRect.y&3));

			newRectList.push_back( newRect );
		}

		i = m_FreeRects.erase(i);
	}
	AddNewFreeRectVector(&m_FreeRects, &newRectList);    
}

// check whether a column of pixels consist entirely of empty pixels
bool CLightmapPacking::ScanColumn(CFreeImage *pImg, int x, int y, int h)
{
	int i;
	for (i=0; i<h; ++i)
	{
		if (pImg->Get(x, i+y)) return true;
	}
	return false;
}

// find a row columns of empty pixels in an image. stops when the next pixel is solid
void CLightmapPacking::FindColumns(TMyRectVector *pList, CFreeImage *pImg, int x, int y, int h, int iPixelSize)
{
	int i;
	int w;
	CMyRect newRect;

	for (i=x, w=0; i<pImg->w; ++i, ++w)
	{
		if (ScanColumn(pImg, i, y, h))
			break;
	}
	if (!w)
		return;

	// add a rectangle using the maximum width of the row of pixels, 
	// using the minimum column height that we found
	newRect.x = x * iPixelSize;
	newRect.y = y * iPixelSize;
	newRect.w = w * iPixelSize;
	newRect.h = h * iPixelSize;

	assert(!(newRect.x&3) && !(newRect.y&3));
	assert(!(newRect.w&3) && !(newRect.h&3));

	pList->push_back(newRect);
}

// add all free rectangles given a particular top-right corner for a starting position
void CLightmapPacking::AddFreeRects(TMyRectVector *pList, CFreeImage *pImg, int x, int y, int iPixelSize)
{
	int i;
	int h;

	// scan various different possible rectangle heights from this position
	for (i=y, h=1; i<pImg->h; ++i, ++h)
	{
		if (pImg->Get(x, i))
			break;
		FindColumns(pList, pImg, x, y, h, iPixelSize);
	}
}


// see if we can extend our rectangle downwards or to the right... (this is a per-pixel check as opposed to using iPixelSize)
void CLightmapPacking::ExtendFreeRects(CMyImage *pImg, RGBTexture *pTex)
{
	TMyRectList::List_Iterator i;
	CMyRect *pRect;

	for(i = pImg->m_FreeRects.begin(); i!=pImg->m_FreeRects.end(); ++i)
	{
		pRect = &i->data;

		pRect->TryExtendRectW(pImg, pTex);
		pRect->TryExtendRectW(pImg, pTex);

#ifdef ALIGN_4
		pRect->w &= ~3;
		pRect->h &= ~3;
#endif
	}
}

// find a list of free rectangles in a chart
// this function uses a scaled-down representation of the image, which just stores whether a pixel is used or not
int CLightmapPacking::FindFreeRectanglesInChart(CMyImage *pImg, RGBTexture *pTex)
{
	int i, j;
	CFreeImage freeImage(	(int)floor(pImg->w / m_fSmallestArea),
							(int)floor(pImg->h / m_fSmallestArea));

	unsigned int *src;
	bool bHasFreeSpace;
	int iPixelSize;
	struct gfx_texture *txtr;
	TMyRectVector tmpRectList;

	tmpRectList.reserve(100);

	int iUsedSpace = 0;

	// setup 
	iPixelSize = (int)floor(m_fSmallestArea);
	assert(iPixelSize);

	if (freeImage.w < 2 || freeImage.h < 2)
		return freeImage.w * freeImage.h * iPixelSize * iPixelSize;

	// make a map of free space in the image
	for (i=0; i<freeImage.h; ++i)
	{
		for (j=0; j<freeImage.w; ++j)
		{
			bool bUsed = pTex->CheckImageBlockUsed(j * iPixelSize, i * iPixelSize, iPixelSize, iPixelSize);
			freeImage.Set(j,i, bUsed);
			if (bUsed)
				iUsedSpace++;
		}
	}
	//
	if (freeImage.m_bHasFreeSpace)
	{
		// Try all pixel starts
		for (i=0; i<freeImage.h; ++i)
		{
			for (j=0; j<freeImage.w; ++j)
			{
				if (!freeImage.Get(j,i))
				{
					AddFreeRects(&tmpRectList, &freeImage, j, i, iPixelSize);
				}
			}
		}
		AddNewFreeRectVector(&pImg->m_FreeRects, &tmpRectList);
	}
	ExtendFreeRects(pImg, pTex);

	return iUsedSpace * iPixelSize * iPixelSize;
}

// extend free rectangles from the newly place image across existing free rectangles if possible
void CLightmapPacking::ExtendImageRects(TMyRectList *pList)
{
	CMyRect *freerect;
	CMyRect *imagerect;

	for (RectIterator imagenode=pList->begin(); imagenode!=pList->end(); ++imagenode)
	{
		imagerect = &imagenode->data;
		for (RectIterator freenode=m_FreeRects.begin(); freenode!=m_FreeRects.end(); ++freenode)
		{
			freerect = &freenode->data;
			if((imagerect->x + imagerect->w) == freerect->x) // Match-up in X ?
			{
				if((imagerect->y >= freerect->y) && (imagerect->y + imagerect->h) <= (freerect->y + freerect->h))
					imagerect->w += freerect->w;
			}

			if((imagerect->y + imagerect->h) == freerect->y) // Match-up in X ?
			{
				if((imagerect->x >= freerect->x) && (imagerect->x + imagerect->w) <= (freerect->x + freerect->w))
					imagerect->h += freerect->h;
			}
#ifdef ALIGN_4
			imagerect->w &= ~3;
			imagerect->h &= ~3;
#endif
		}
	}
}

// find a place where we can fit this image in the current atlas
bool CLightmapPacking::TryInsertImage(CMyImage *pImg, bool bTest)
{
	// if it's a base instance image we don't have to do anything
	if (pImg->bBaseInstanceDoNotExport)
	{
		pImg->x = 4096;
		pImg->y = 4096;
		if (!bTest)
		{
			m_pPackingInfo->Add(pImg, m_iCurrentAtlas);
			m_pCharts->At(pImg->iChart)->m_iAtlas = m_iCurrentAtlas;
		}
		return true;
	}

	// look through our 'free rectangle' list to find a fit
	for (RectIterator i=m_FreeRects.begin(); i!=m_FreeRects.end(); ++i)
	{
		CMyRect *pRect = &i->data;

		if((pImg->w <= pRect->w) && (pImg->h <= pRect->h))
		{
			// This fits :)
			pImg->x = pRect->x;
			pImg->y = pRect->y;

			if (!bTest)
			{
				m_pPackingInfo->Add(pImg, m_iCurrentAtlas);
				m_pCharts->At(pImg->iChart)->m_iAtlas = m_iCurrentAtlas;
			}

			// clip all existing free rectangles against the newly added image
			ClipFreeRectList( pImg );

			// get the new free rectangles from the image
			TMyRectList tmpList;
			TMyRectList *pList = &pImg->m_FreeRects;
			// offset them into the atlas
			for (RectIterator i=pList->begin(); i!=pList->end(); ++i)
			{
				CMyRect rect;
				rect = i->data;
				rect.x+=pImg->x, rect.y+=pImg->y;
				rect.w = (rect.w+3) & ~3;
				rect.h = (rect.h+3) & ~3;

			assert(!(rect.x&3) && !(rect.y&3));
			assert(!(rect.w&3) && !(rect.h&3));

				tmpList.push_back(rect);
			}
			// extend them into adjacent free rectangles
			ExtendImageRects(&tmpList);
			// add them to our 'free list'
			AddNewFreeRectList(&m_FreeRects, &tmpList);

			return true;
		}
	}

	return false;
}

int giExtraFit;

// pack charts into the current atlas until it's full
void CLightmapPacking::PackImageList()
{
	TpMyImageList tmpList;

	giExtraFit=0;

	if (!m_bRepack)
	{
		// create a new atlas with one big free rect
		// (normal method)
		InitializeFreeRectsList();
		DoPackImageList(NULL);
	}
	else
	{
		// (repacking)
		DoPackImageList(&tmpList);

		// repacking - sometimes produces WORSE packing - so it's not worth the hassle

		// restart the packing, this time in cell order rather than in cell order
		InitializeFreeRectsList();
		for (TpMyImageList::List_Iterator i=tmpList.begin(); i!=tmpList.end(); ++i)
		{
			CMyImage *pImg = i->data;

			if (!TryInsertImage(pImg,false))
			{
				assert(0); // this shouldn't happen - must program something to handle this
				printf("*** INTERNAL ERROR: REPACKING ERROR\n");
				break;
			}
		}
		// if we can fit in any more then that's great

		DoPackImageList(NULL);
	}
}

// this function calls the appropriate packing function
void CLightmapPacking::DoPackImageList(TpMyImageList *pTmpImageList)
{
	if (m_iPackMode == LMPM_SuperGroup)
		PackImageListBySuperGroup(pTmpImageList);
	else
		PackImageListSimple(pTmpImageList);

}

// pack charts one cell at a time until no more will fit
// this works in a per-cell, biggest-first, order
void CLightmapPacking::PackImageListSimple(TpMyImageList *pTmpImageList)
{
	box3f bbox, testbox;
	float fPackedArea = 0;
	float fCurArea;
	bool bFirst = true;
	bool bSomeLeft = false;
	float fOrgArea;

	m_iCurCell = 0;

	while (1)
	{
		if (m_iCurCell >= m_nCells)
		{
			if (bSomeLeft)
				m_iCurCell = 0;
			return;
		}

		TpMyImageList *pImageList = &m_pImageLists[m_iCurCell];

		if (pImageList->begin() == pImageList->end())
		{
			m_iCurCell++;
			continue;
		}

		// go through all the images, in large to small order. try to fit as many as possible in this atlas
		for (TpMyImageList::List_Iterator i=pImageList->begin(); i!=pImageList->end(); )
		{
			CMyImage *pImg = i->data;

			if (TryInsertImage(pImg, pTmpImageList!=NULL))
			{
				if (pTmpImageList)
					pTmpImageList->push( i->data, i->data->w*256 + i->data->h );
				else
					giExtraFit++;
				i = pImageList->erase(i);
					
				continue;
			}
			else
				bSomeLeft = true;

			// couldn't fit this one in - try next
			if (m_iPackMode == LMPM_Cell)
				return;

			++i;
		}

		// move on to pack the next bit of map
		m_iCurCell++;
	}
}

// pack charts one cell at a time until no more will fit
// this works in a per-cell, biggest-first, order
void CLightmapPacking::PackImageListTree()
{
	box3f bbox, testbox;
	float fPackedArea = 0;
	float fCurArea;
	bool bFirst = true;
	bool bSomeLeft = false;
	float fOrgArea;

	TpMyImageList *pImageList;

	while (1)
	{
		pImageList = &m_pImageLists[m_iCurCell];
		if (pImageList->begin() == pImageList->end())
		{
			m_iCurCell++;

			if (m_iCurCell >= m_nCells)
				return;
		}
		else
			break;
	}

	// go through all the images, in large to small order. try to fit as many as possible in this atlas
	for (TpMyImageList::List_Iterator i=pImageList->begin(); i!=pImageList->end(); )
	{
		CMyImage *pImg = i->data;

		if (TryInsertImage(pImg, false))
		{
			i = pImageList->erase(i);
				
			continue;
		}
		else
			bSomeLeft = true;

		++i;
	}

	if (!bSomeLeft)
		m_iCurCell++;
}


// this works in a per-cell, per material, biggest-first, order
void CLightmapPacking::PackImageListBySuperGroup(TpMyImageList *pTmpImageList)
{
	box3f bbox, testbox;
	float fPackedArea = 0;
	float fCurArea;
	bool bFirst = true;
	bool bMissed;
	bool bExpand;
	float fOrgArea;

	while (1)
	{
		bMissed = false;
		bExpand = false;

		if (m_iCurCell >= m_nCells)
			return;

		TpMyImageList *pImageList = &m_pImageLists[m_iCurCell];

		// get the first (biggest) image to pack
		TpMyImageList::List_Iterator i = pImageList->begin();
		// if there are no images left in this cell move on to the next cell
		if (i == pImageList->end())
		{
			m_iCurCell++;
			continue;
		}

		// get the supergroup index associated with this image
		CMyImage *pImg = i->data;
		int iSuperGroup = m_pCharts->At(pImg->iChart)->m_iSuperGroup;

		// pack it
		if (!TryInsertImage(pImg, pTmpImageList!=NULL))
			return;

		if (pTmpImageList)
			pTmpImageList->push( i->data, i->data->w*256 + i->data->h );
		else
			giExtraFit++;
		i = pImageList->erase(i);

		// now pack all other images (charts) in the same supergroup
		// go through all the images, in large to small order. try to fit as many as possible in this atlas
		for (i=pImageList->begin(); i!=pImageList->end(); )
		{
			pImg = i->data;

			if (m_pCharts->At(pImg->iChart)->m_iSuperGroup != iSuperGroup)
			{
				++i;
				continue;
			}

			if (TryInsertImage(pImg, pTmpImageList!=NULL))
			{
				if (pTmpImageList)
					pTmpImageList->push( i->data, i->data->w*256 + i->data->h );
				else
					giExtraFit++;
				i = pImageList->erase(i);
					
				continue;
			}

			// couldn't fit this one in - stop
			return;
		}
	}
}



// calculate where to stick the grid and allocate it
void CLightmapPacking::AllocImageLists()
{
	float3 vc;

	box3f bound;

	// calculate bound for whole map
	for (int i=0; i<m_pCharts->Size(); i++)
	{
		if (!m_pCharts->At(i)) continue;
		bound = box3f(bound, m_pCharts->At(i)->m_bbox);
	}

	// get bound for the whole map
 	vc.x = (int)(bound.GetSize().x / m_fGridSize);
	vc.y = (int)(bound.GetSize().y / m_fGridSize);
	vc.z = (int)(bound.GetSize().z / m_fGridSize);

	// in simple mode we don't sort by cell - so only have one cell
	if (m_iPackMode == LMPM_Simple)
		vc = float3(1,1,1);

	// have at least one cell
	int n[3];
	for (int i=0; i<3; i++)
	{
		if (vc[i] < 1) vc[i] = 1;
		if (vc[i] > 15) vc[i] = 15;
		m_iGridSize[i] = (int)vc[i];
	}

	// calculate grid position/dimensions
	m_vNodeOffset = bound.vmin;
	m_vNodeSize = bound.GetSize() / vc;
	
	m_nCells = m_iGridSize[0] * m_iGridSize[1] * m_iGridSize[2];
	m_iCurCell = 0;

	m_pImageLists = new TpMyImageList [m_nCells];
}

// calculate which image list is associated with a world-space position
TpMyImageList *CLightmapPacking::GetImageList(float3 vPos)
{
	float3 v;
	int ix[3];

	v = (vPos - m_vNodeOffset) / m_vNodeSize;

	ix[0] = (int)v.x;
	ix[1] = (int)v.y;
	ix[2] = (int)v.z;

	// clamp to grid
	for (int i=0; i<3; i++)
	{
		if (ix[i] < 0) ix[i] = 0;
		if (ix[i] >= m_iGridSize[i]) ix[i] = m_iGridSize[i]-1;
	}

	int iImage = ix[0]*1 + ix[1]*m_iGridSize[0] + ix[2]*m_iGridSize[0]*m_iGridSize[1];
	assert(iImage>=0 && iImage<m_nCells);

	return &m_pImageLists[iImage];
}

// initialize free rect list by emtpying it and putting in one big free rect
void CLightmapPacking::InitializeFreeRectsList(bool bTest)
{
	CMyRect rect;
	TAtlasSizeInfo atlas;

	m_FreeRects.clear();

	rect.x = 0;
	rect.y = 0;
	atlas.cX = rect.w = m_iTexturePackRes;
	atlas.cY = rect.h = m_iTexturePackRes;

	if (!bTest)
		m_pPackingInfo->AddAtlas(atlas);

	m_FreeRects.push( rect, rect.w * rect.h );
}

// remove empty charts generated by the instancing
// remove empty textures that contained degenerate charts
void CLightmapPacking::RemoveEmptyChartsFromList(TRGBTextureList &texlist, TChartList &charts)
{
	for (int i=0; i<charts.Size(); )
	{
		if (charts[i] == NULL || texlist[i] == NULL)
		{
			delete charts[i];
			charts[i] = charts[charts.Size()-1];
			charts.PopBack();

			texlist[i] = texlist[texlist.Size()-1];

			texlist.PopBack();
		}
		else
			i++;
	}
}


// generates packed atlases from a list of CTextureCharts + associated textures
bool CLightmapPacking::Pack(TRGBTextureList *pInTextures, TChartList *pInCharts, bool bSilent)
{
#ifdef DISABLE_PACKING
	return DebugPack(pInTextures, pInCharts);
#endif

	if (m_iPackMode == LMPM_Clustering)
		RemoveEmptyChartsFromList(*pInTextures, *pInCharts);

	if (pInCharts->Size() == 0)
		return false;

	// convert to references for easy access
	TRGBTextureList &pTextures = *pInTextures;
	TChartList &pCharts		= *pInCharts;
	m_pCharts = pInCharts;
	TpMyImageList *pImageList;

	assert(pTextures.Size() == pCharts.Size()); // check input

	if (m_iPackMode == LMPM_Clustering)
	{
		m_pSuperGroups = new SuperGroupBuilder(
			pInCharts->At(0)->m_pMesh->GetScenePtr(),	// aftk scene pointer
			*pInCharts,									// chart list
			m_fMaxClusterSize * m_fMeterSize,			// max clustering radius
			(float)(m_iTexturePackRes * m_iTexturePackRes) * m_fMaxClusterArea * 0.01f // max surface area is % of whole atlas
			);
	}

	if (!bSilent)
	{
		printf("Charts:      %5d\n",pInCharts->Size());
		if (m_pSuperGroups)
			printf("Supergroups: %5d\n",m_pSuperGroups->GetNumGroups());
	}

	// allocate the remap class
	m_pPackingInfo = new CChartPackRemapInfo(pCharts.Size());

	// allocate the array to permanently store the images
	m_pImageArray = new CMyImage [pCharts.Size()];

	// iterate through our charts - calculate empty space inside them
	for (int i=0; i<pCharts.Size(); i++)
	{
		if (!pInTextures->At(i)) continue;

		// create a new image and stick it on the priority queue
		CMyImage *pNewImage = &m_pImageArray[i];

		pNewImage->iChart = i;
		pNewImage->x = 0;
		pNewImage->y = 0;
		pNewImage->w = pInTextures->At(i)->m_cX;// pCharts[i]->m_iTexWidth;
		pNewImage->h = pInTextures->At(i)->m_cY;// pCharts[i]->m_iTexHeight;

		// base instances don't get packed
		pNewImage->bBaseInstanceDoNotExport = m_pCharts->At(i)->m_bBaseInstanceDoNotExport;

		// can't pack charts that are bigger than our atlas res
		if (pNewImage->w>m_iTexturePackRes || pNewImage->h>m_iTexturePackRes)
			return false;

		// make a list of rectangles covering the empty space inside the image
		pCharts[i]->m_iUsedSpace = FindFreeRectanglesInChart( pNewImage, pInTextures->At(i) );

	}

	// pack the scene using clustering - uses a completely different function
	if (m_iPackMode == LMPM_Clustering)
	{
		PackLightmapsUsingClustering(pCharts, m_pSuperGroups, bSilent);

		delete [] m_pImageArray;

		return m_bSuccess;
	}

	// allocate a 2D array of image lists
	AllocImageLists();

	// assign each chart to a particular image list based on location
	for (int i=0; i<pCharts.Size(); i++)
	{
		if (!pInTextures->At(i)) continue;

		// create a new image and stick it on the priority queue
		CMyImage *pNewImage = &m_pImageArray[i];

		// add the object to the first list
		pImageList = &m_pImageLists[0];

		pImageList->push( pNewImage, pNewImage->w*256 + pNewImage->h );
	}

	m_iCurrentAtlas = 0;

	// keep making new atlases until all the charts have been packed
	while (m_iCurCell < m_nCells)
	{
		// pack images into the new atlas
		PackImageList();

		if (!bSilent)
			printf("\rPacking atlas %2d",m_iCurrentAtlas);

		m_iCurrentAtlas++;
	}
	if (!bSilent)
		printf("\n");

	delete [] m_pImageLists;
	delete [] m_pImageArray;

	return m_bSuccess;
}

// generates one atlas per chart at exactly the chart's original size
// (i.e. disable packing for debugging purposes)
bool CLightmapPacking::DebugPack(TRGBTextureList *pInTextures, TChartList *pInCharts)
{
	// convert to references for easy access
	TRGBTextureList &pTextures = *pInTextures;
	TChartList &pCharts		= *pInCharts;

	assert(pTextures.Size() == pCharts.Size()); // check input

	// allocate the remap class
	m_pPackingInfo = new CChartPackRemapInfo(pCharts.Size());

	// allocate the array to permanently store the images
	m_pImageArray = new CMyImage [pCharts.Size()];

	// keep making new atlases until all the charts have been packed
	for (int i=0; i<pCharts.Size(); i++)
	{
		m_FreeRects.clear();

		// create a new atlas the size of the chart
		CMyImage img;
		TAtlasSizeInfo atlas;

		img.iChart = i;
		img.x = 0;
		img.y = 0;
		atlas.cX = img.w = pCharts[i]->m_iTexWidth;
		atlas.cY = img.h = pCharts[i]->m_iTexHeight;

		m_pPackingInfo->AddAtlas(atlas);
		m_pPackingInfo->Add(&img, i);
	}

	delete [] m_pImageArray;

	return m_bSuccess;
}

// pack an image list, returns false if it doesn't fit or true if it does
bool CLightmapPacking::TestPackImageList(TpMyImageList *pImageList, bool bTest)
{
	InitializeFreeRectsList(bTest);

	// go through all the images, in large to small order. try to fit as many as possible in
	for (TpMyImageList::List_Iterator i=pImageList->begin(); i!=pImageList->end(); ++i)
	{
		CMyImage *pImg = i->data;

		if (!TryInsertImage(pImg, bTest))
			return false;
	}
	return true;
}


