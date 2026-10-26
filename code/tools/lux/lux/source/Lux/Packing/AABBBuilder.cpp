#include "stdafx.h"

#include "Lux/LightmapPacking/PackingRectangle.h"
#include "Lux/LightmapPacking/LightmapPacking.h"
#include "Lux/Parameterization/SuperGroups.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include <spatial_hierarchy/SpatialDividerHeaders.h>
#include <spatial_hierarchy/Kd-TreeBuilder.h>
#include "Lux/LightmapPacking/AABBBuilder.h"
#include <algorithm>

using namespace std;

// the main tree builder

// this builds the tree using a primitive list
// primitive list can be built from an aftkscenenode using CollectPolygonsFromScene
AABBBuilder::AABBBuilder(TChartList &primBounds, int icost, int tcost, int maxp, int iMaxDepth )
	: m_iIsectCost(icost), m_iTraversalCost(tcost), m_iMaxPrims(maxp)
{
	int nPrims = primBounds.Size();

	m_pMailboxes = new MailboxVector;
	m_nLeafNodes = 0;

	// calculate default max depth
	if (iMaxDepth <= 0)
		iMaxDepth =
		    Round2Int(8 + 1.3f * Log2Int(float(nPrims)));

	iMaxDepth *= 3;

	// Compute bounds for kd-tree construction
	for (int i = 0; i < nPrims; ++i)
	{
		if (primBounds[i])
			m_Bounds = box3f(m_Bounds, primBounds[i]->m_bbox);
	}

	// Allocate working memory for kd-tree construction
	BoundEdge *edges[3];
	for (int i = 0; i < 3; ++i)
		edges[i] = new BoundEdge[2*nPrims];
	int *prims0 = new int[nPrims];
	int *prims1 = new int[(iMaxDepth+1) * nPrims];

	// Initialize _primNums_ for kd-tree construction
	int *primNums = new int[nPrims];
	int nRealPrims = 0;
	for (int i = 0; i < nPrims; i++)
	{
		if (primBounds[i])
		{
			primNums[nRealPrims] = i;
			nRealPrims++;
		}
	}

	// Start recursive construction of kd-tree
	KdNode newNode;
	m_Nodes.PushBack(newNode);
	BuildTree(0, m_Bounds, primBounds, primNums, nRealPrims, iMaxDepth, edges, prims0, prims1);

	// Free working memory for kd-tree construction
	delete[] primNums;
	for (int i = 0; i < 3; ++i)
		delete[] edges[i];
	delete[] prims0;
	delete[] prims1;
}

//#define USE_TOTAL_AREA

// calculate a cost heuristic from a given area
// the heuristic is designed to try to end up with leafnodes containing close the m_iMaxPrims in area
int AABBBuilder::AreaCostHeuristic(int iArea, int iBiggestArea)
{
	iBiggestArea = 0;

	return (iArea+m_iMaxPrims+(iBiggestArea)-1)/m_iMaxPrims;
}

// recursive tree builder
// this is called by the AABBBuilder initialization code
void AABBBuilder::BuildTree(int iNodeNum, const box3f &nodeBounds, TChartList &allPrimBounds, int *piPrimNums, int nPrims, int iDepth,
		BoundEdge *edges[3], int *prims0, int *prims1, int nBadRefines)
{
	// create a new node
	KdNode newNode;

	// calculate sum of area of all nodes
	int iAreaSum = 0;
	int iArea;
	int iBiggestArea = 0;
	for (int i=0; i<nPrims; i++)
	{
		CTextureChart *pChart = allPrimBounds[ piPrimNums[i] ];
#ifdef USE_TOTAL_AREA
		iArea = pChart->m_iTexWidth * pChart->m_iTexHeight; 
#else
		iArea = pChart->m_iUsedSpace;
#endif
		iAreaSum += iArea;
		if (iArea > iBiggestArea) iBiggestArea = iArea;
	}

	// Initialize leaf node if termination criteria met
	if (iAreaSum <= m_iMaxPrims || iDepth == 0)
	{
		LOGDEBUG( LUX::g_Log, "making leafnode with area %d (target %d)\n", iAreaSum, m_iMaxPrims);
		m_Nodes[iNodeNum].InitLeaf(piPrimNums, nPrims, m_pMailboxes);
		m_nLeafNodes++;
		return;
	}

	// Initialize interior node and continue recursion
	// Choose split axis position for interior node
	int	iBestAxis = -1, iBestOffset = -1;
	float fBestCost = FLT_MAX;
	int iBestCost = 0;
	float3 vd = nodeBounds.GetSize();
	float fTotalSA = (2.f * (vd.x*vd.y + vd.x*vd.z + vd.y*vd.z));
	float iInvTotalSA = 1.f / fTotalSA;
	// Choose which axis to split along
	int iAxis;
	if (vd.x > vd.y && vd.x > vd.z)
		iAxis = 0;
	else
		iAxis = (vd.y > vd.z) ? 1 : 2;
	int iRetries = 0;

	int iMax = m_iMaxPrims - (iAreaSum/m_iMaxPrims) * iBiggestArea;
	int	iCost, iOldCost = ((iAreaSum+iMax-1)/iMax);

retrySplit:
	// Initialize edges for iAxis
	for (int i = 0; i < nPrims; ++i)
	{
		int np = piPrimNums[i];
		const box3f &bbox = allPrimBounds[np]->m_bbox;
		edges[iAxis][i] = BoundEdge(bbox.vmin[iAxis], np, true);
	}

	float f;

	sort(&edges[iAxis][0], &edges[iAxis][nPrims]);

	box3f bestbounds0, bestbounds1;

	box3f bounds;
	// manually calculate bbox edges
	for (int x=0; x<nPrims; x++)
	{
		bounds.UpdateBox( allPrimBounds[edges[iAxis][x].iPrimNum]->m_bbox.vmin );
		bounds.UpdateBox( allPrimBounds[edges[iAxis][x].iPrimNum]->m_bbox.vmax );
	}

	float fEdgeL = bounds.vmin[iAxis];
	float fEdgeR = bounds.vmax[iAxis];

	// Compute cost of all splits for iAxis to find best
	int nBelow = 0, nAbove = iAreaSum;

	int nBestBelow, nBestAbove;
	for (int i = 1; i < nPrims-1; ++i)
	{
		// update above/below
#ifdef USE_TOTAL_AREA
		int iArea = allPrimBounds[ edges[iAxis][i-1].iPrimNum ]->m_iTexWidth * allPrimBounds[ edges[iAxis][i-1].iPrimNum ]->m_iTexHeight;
#else
		int iArea = allPrimBounds[ edges[iAxis][i-1].iPrimNum ]->m_iUsedSpace;
#endif
		nBelow += iArea;
		nAbove -= iArea;

		// update left bound edge
		f = allPrimBounds[edges[iAxis][i-1].iPrimNum]->m_bbox.vmax[iAxis];
		if (f > fEdgeL) fEdgeL = f;
		// update right bound edge
		f = allPrimBounds[edges[iAxis][i].iPrimNum]->m_bbox.vmin[iAxis];
		fEdgeR = f;

		box3f bounds0 = bounds, bounds1 = bounds;

		bounds0.vmax[iAxis] = fEdgeL;
		bounds1.vmin[iAxis] = fEdgeR;

		float3 v;

		// Compute cost for split at _i_th edge
		static const int otherAxis[3][2] = { {1,2}, {0,2}, {0,1} };
		int otherAxis0 = otherAxis[iAxis][0];
		int otherAxis1 = otherAxis[iAxis][1];
		v = bounds0.GetSize();
		float fBelowSA = 2 * (v.x * v.x + v.y * v.y + v.z * v.z);
		v = bounds1.GetSize();
		float fAboveSA = 2 * (v.x * v.x + v.y * v.y + v.z * v.z);
		float fBelow = fBelowSA * iInvTotalSA;
		float fAbove = fAboveSA * iInvTotalSA;
		float fCost = m_iTraversalCost + m_iIsectCost * (fBelow * nBelow + fAbove * nAbove);

		iCost = ((nBelow+iMax-1)/iMax) + 
				((nAbove+iMax-1)/iMax);

		fCost = fCost;// + abs(nAbove-nBelow)*0.01;

		// Update best split if this is lowest cost so far
		if (fCost < fBestCost && iCost<=iOldCost)
		{
			fBestCost = fCost;
			iBestCost = iCost;
			iBestAxis = iAxis;
			iBestOffset = i;
			bestbounds0 = bounds0;
			bestbounds1 = bounds1;
			nBestBelow = nBelow;
			nBestAbove = nAbove;
		}
	}


	// if no split was found try a different axis
	if (iBestAxis==-1 && iRetries < 2)
	{
		++iRetries;
		iAxis = (iAxis+1) % 3;
		goto retrySplit;
	}



	LOGDEBUG( LUX::g_Log, "generating split: %d(%d) -> %d(%d), %d(%d)\n",
		iAreaSum,	((iAreaSum+iMax-1)/iMax),
		nBestBelow, ((nBestBelow+iMax-1)/iMax),
		nBestAbove, ((nBestAbove+iMax-1)/iMax));

	// copy primitives into different lists depending on which side of the split they are
	int n0 = 0, n1 = 0;
	for (int i=0; i<iBestOffset; ++i)
		prims0[n0++] = edges[iBestAxis][i].iPrimNum;

	for (int i=iBestOffset; i<nPrims; ++i)
		prims1[n1++] = edges[iBestAxis][i].iPrimNum;

	float fSplit = edges[iBestAxis][iBestOffset].t;

	// allocate space for left childnode
	assert(m_Nodes.Size() == iNodeNum+1);
	m_Nodes.PushBack(newNode);

	// initialize childnodes
	BuildTree(iNodeNum+1, bestbounds0, allPrimBounds, prims0, n0, iDepth-1, edges, prims0, prims1 + nPrims, nBadRefines);

	// set the parameters for this node
	int iNewNode = m_Nodes.Size();
	m_Nodes.PushBack(newNode);
	m_Nodes[iNodeNum].InitSplit(iBestAxis, fSplit, iNewNode - (iNodeNum + 1));
	BuildTree(iNewNode, bestbounds1, allPrimBounds, prims1, n1, iDepth-1, edges, prims0, prims1 + nPrims, nBadRefines);
}
