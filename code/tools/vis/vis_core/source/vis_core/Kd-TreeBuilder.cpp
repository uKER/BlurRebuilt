#include "stdafx.h"

#include "Vis_Core/Kd-TreeBuilder.h"
#include <algorithm>

using namespace std;

// utility function
inline int Log2Int(float v) {
	return ((*(int *) &v) >> 23) - 127;
}

// this structure stores a possible split position
BoundEdge::BoundEdge(float tt, int pn, bool starting)
{
	t = tt;
	iPrimNum = pn;
	type = starting ? START : END;
}

bool BoundEdge::operator<(const BoundEdge &e) const
{
	if (t == e.t)
		return (int)type < (int)e.type;
	else
		return t < e.t;
}

int gnLeafNodes = 0;
int gnTreeNodes = 0;
int gnMailboxen = 0;

// the kd-tree builder node
// TODO: as an optimization - if there's only one primitive use this leafnode as the mailbox itself
void KdNode::InitLeaf(int *primNums, int np, MailboxVector *pmailboxes)
{
	iFlagAndNumPrimitives = np | 0x80000000;

	if (np == 0)
	{
		iPrimitive = 0;
		return;
	}

	gnLeafNodes++;
	gnMailboxen+=np;

	iPrimitive = pmailboxes->Size();
	
	// create the mailboxes
	for (int i=0; i<np; i++)
	{
		KdMailbox mailbox;

		mailbox.iPrimitive = primNums[i];

		// flag the last primitive
		if (i==np-1) 
			mailbox.iPrimitive|=0x80000000;

		pmailboxes->PushBack(mailbox);
	}
}

// init an internal splitnode
void KdNode::InitSplit(int iAxis, float f, int iChildren)
{
	fSplitCoordinate = f;
	iFlagDimAndOffset = iAxis + iChildren*8;
	gnTreeNodes++;
}



// the main kd-tree builder

// this builds the tree using a primitive list
// primitive list can be built from an aftkscenenode using CollectPolygonsFromScene
KdBuilder::KdBuilder(Box3fVector &primBounds, int icost, int tcost, int maxp, int iMaxDepth )
	: m_iIsectCost(icost), m_iTraversalCost(tcost), m_iMaxPrims(maxp)
{
	int nPrims = primBounds.Size();
	gnTreeNodes = 0;
	gnLeafNodes = 0;
	gnMailboxen = 0;

	m_pMailboxes = new MailboxVector;

	// calculate default max depth
	if (iMaxDepth <= 0)
		iMaxDepth =
		    (int)(8 + 1.3f * Log2Int(float(nPrims)));

	// Compute bounds for kd-tree construction
	for (int i = 0; i < nPrims; ++i)
		m_Bounds = box3f(m_Bounds, primBounds[i]);

	// Allocate working memory for kd-tree construction
	BoundEdge *edges[3];
	for (int i = 0; i < 3; ++i)
		edges[i] = new BoundEdge[2*nPrims];
	int *prims0 = new int[nPrims];
	int *prims1 = new int[(iMaxDepth+1) * nPrims];

	// Initialize _primNums_ for kd-tree construction
	int *primNums = new int[nPrims];
	for (int i = 0; i < nPrims; ++i)
		primNums[i] = i;

	// Start recursive construction of kd-tree
	KdNode newNode;
	m_Nodes.PushBack(newNode);
	BuildTree(0, m_Bounds, primBounds, primNums, nPrims, iMaxDepth, edges, prims0, prims1);

	// Free working memory for kd-tree construction
	delete[] primNums;
	for (int i = 0; i < 3; ++i)
		delete[] edges[i];
	delete[] prims0;
	delete[] prims1;
}

// recursive tree builder
// this is called by the KdBuilder initialization code
void KdBuilder::BuildTree(int iNodeNum, const box3f &nodeBounds, Box3fVector &allPrimBounds, int *piPrimNums, int nPrims, int iDepth,
		BoundEdge *edges[3], int *prims0, int *prims1, int nBadRefines)
{
	// create a new node
	KdNode newNode;

	// Initialize leaf node if termination criteria met
	if (nPrims <= m_iMaxPrims || iDepth == 0)
	{
		m_Nodes[iNodeNum].InitLeaf(piPrimNums, nPrims, m_pMailboxes);
		return;
	}

	// Initialize interior node and continue recursion
	// Choose split axis position for interior node
	int	iBestAxis = -1, iBestOffset = -1;
	float fBestCost = FLT_MAX;
	float fOldCost = m_iIsectCost * float(nPrims);
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

retrySplit:
	// Initialize edges for iAxis
	int i;
	for (i = 0; i < nPrims; ++i)
	{
		int np = piPrimNums[i];
		const box3f &bbox = allPrimBounds[np];
		edges[iAxis][i] = BoundEdge(bbox.vmin[iAxis], np, true);
	}

	float f;

	sort(&edges[iAxis][0], &edges[iAxis][nPrims]);

	box3f bestbounds0, bestbounds1;

	box3f bounds;
	// manually calculate bbox edges
	for (int x=0; x<i; x++)
	{
		bounds.UpdateBox( allPrimBounds[edges[iAxis][x].iPrimNum].vmin );
		bounds.UpdateBox( allPrimBounds[edges[iAxis][x].iPrimNum].vmax );
	}

	float fEdgeL = bounds.vmin[iAxis];
	float fEdgeR = bounds.vmax[iAxis];

	// Compute cost of all splits for iAxis to find best
	int nBelow = 0, nAbove = nPrims;
	for (int i = 1; i < nPrims-1; ++i)
	{
		// update left bound edge
		f = allPrimBounds[edges[iAxis][i-1].iPrimNum].vmax[iAxis];
		if (f > fEdgeL) fEdgeL = f;
		// update right bound edge
		f = allPrimBounds[edges[iAxis][i].iPrimNum].vmin[iAxis];
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

		// Update best split if this is lowest cost so far
		if (fCost < fBestCost)
		{
			fBestCost = fCost;
			iBestAxis = iAxis;
			iBestOffset = i;
			bestbounds0 = bounds0;
			bestbounds1 = bounds1;
		}

		nBelow++;
		nAbove--;
	}
//	assert(nBelow == nPrims && nAbove == 0);

	// if no split was found try a different axis
	if (iBestAxis == -1 && iRetries < 2)
	{
		++iRetries;
		iAxis = (iAxis+1) % 3;
		goto retrySplit;
	}

	// if the previous couple splits were ok then do a bad split anyway
	if (fBestCost > fOldCost) ++nBadRefines;

	// otherwise create a leaf here
	if ((fBestCost > 4.f * fOldCost && nPrims < 16) ||
		iBestAxis == -1 || nBadRefines == 3)
	{
		// protect against bad refines creating leafs with too many polygons for a 16-bit index buffer
		if ( nPrims < 10000)
		{
			m_Nodes[iNodeNum].InitLeaf(piPrimNums, nPrims, m_pMailboxes);
			return;
		}
	}

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
