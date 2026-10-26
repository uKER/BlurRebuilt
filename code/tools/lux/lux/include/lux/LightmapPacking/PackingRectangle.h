#pragma once

#include <vector>
#include <queue>

#include "MyPriorityQueue.hpp"
#include "Utility/MySortedList.hpp"

using namespace std;

class CMyImage;
class RGBTexture;

// rectangle structures keep track of the empty spaces in the atlas
class CMyRect
{
public:
	int	x,y,w,h;

	void	TryExtendRectW(CMyImage *pImg, RGBTexture *pTex);
	void	TryExtendRectH(CMyImage *pImg, RGBTexture *pTex);
};

typedef vector< CMyRect > TMyRectVector;
typedef CMyPriorityQueue< CMyRect > TMyRectPriorityQueue;
typedef CMySortedList< CMyRect > TMyRectList;
typedef TMyRectList::List_Iterator RectIterator;

// image that the charts get packed into
class CMyImage
{
public:
	int iChart;
	int	x,y,w,h;
	bool bBaseInstanceDoNotExport;

	TMyRectList m_FreeRects;
};

typedef CMyPriorityQueue< CMyImage* > TpMyImagePriorityQueue;
typedef CMySortedList< CMyImage* > TpMyImageList;
typedef TMyRectList::List_Iterator RectIterator;

