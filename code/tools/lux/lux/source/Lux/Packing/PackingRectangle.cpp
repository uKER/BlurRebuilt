//-----------------------------------------------------------------------------
// File: PackingRectangle.cpp
//
// Desc: Class for keeping track of rectangles for packing of lightmaps
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "Lux/LightmapPacking/PackingRectangle.h"
#include "Lux/TextureTypes/RGBTexture.h"


// check whether this free rect can be extended to the bottom end of the image (yes or no)
void CMyRect::TryExtendRectW(CMyImage *pImg, RGBTexture *pTex)
{
	int i;
	int j;
	int iEndPos;

	iEndPos = y + h;

	for (i=y; i<iEndPos; ++i)
	{
		for (j=(x+w); j<pImg->w; ++j)
		{
			if (pTex->CheckPixelUsed(j, i)) // check whether this pixel is empty
				return;
		}
	}

	w = pImg->w - x;
}


// check whether this free rect can be extended to the right end of the image (yes or no)
void CMyRect::TryExtendRectH(CMyImage *pImg, RGBTexture *pTex)
{
	int i;
	int j;
	int iEndPos;

	iEndPos = x + w;

	for (i=(y+h); i<pImg->h; ++i)
	{
		for (j=x; j<iEndPos; ++j)
		{
			if (pTex->CheckPixelUsed(j, i)) // check whether this pixel is empty
				return;
		}
	}
	h = pImg->h - y;
}
