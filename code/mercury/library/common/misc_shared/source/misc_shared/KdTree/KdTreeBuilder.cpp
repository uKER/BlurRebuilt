#include "MiscSharedPch.h"

#include "misc_shared/KdTree/KdTree.h"
#include "misc_shared/KdTree/KdTreeBuilder.h"
#include <algorithm>

using namespace std;

namespace Util
{

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
	{
		if (type == e.type)
			return iPrimNum < e.iPrimNum;
		else
			return (int)type < (int)e.type;
	}
	else
		return t < e.t;
}

int CompareFunc(const void *A, const void *B)
{
	BoundEdge *pA = (BoundEdge*)A;
	BoundEdge *pB = (BoundEdge*)B;

	if (pA->t == pB->t)
	{
		if (pA->type == pB->type)
		{
			return (pA->iPrimNum < pB->iPrimNum)?-1:1;
		}
		else
			return ((int)pA->type < (int)pB->type)?-1:1;
	}
	else
	{
		return (pA->t < pB->t)?-1:1;
	}
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

	// sort the primitives so that they are ordered sequentially in memory
	sort(&primNums[0], &primNums[np]);
	
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


/// add a primitive to the tree. returns the index of the primitive
int KdBuilder::AddPrimitive(const Maths::C_AxisBox &bbox)
{
	int x = m_PrimBounds.Size();
	m_PrimBounds.PushBack(bbox);
	return x;
}

/// add a primitive to the tree. returns the index of the primitive
int KdBuilder::AddPrimitive(const Maths::HVec3 &point)
{
	int x= m_PrimBounds.Size();
	m_PrimBounds.PushBack(Maths::C_AxisBox(point, point));
	return x;
}

/// set heuristic parameters
void KdBuilder::SetParameters(int icost, int tcost, float ebonus, int maxp, int iMaxDepth)
{
	m_iIsectCost = icost;
	m_iTraversalCost = tcost;
	m_iMaxPrims = maxp;
	m_fEmptyBonus = ebonus;
	m_iMaxDepth = iMaxDepth;
}

/// get reference to primbounds vector
Box3Vector &KdBuilder::GetPrimBounds()
{
	return m_PrimBounds;
}

/// the main kd-tree builder
/// build the tree. returns a new tree data structure

/// this builds the tree using a primitive list
KdTreeData KdBuilder::Build()
{
	int iMaxDepth = m_iMaxDepth;

	int nPrims = m_PrimBounds.Size();
	gnTreeNodes = 0;
	gnLeafNodes = 0;
	gnMailboxen = 0;

	// calculate default max depth
	if (iMaxDepth <= 0)
		iMaxDepth =
		    (int)(8 + 2.0f * log(float(nPrims)));

	// Compute bounds for kd-tree construction
	Maths::C_AxisBox bbox( &m_Bounds );
	for (int i = 0; i < nPrims; ++i)
	{
		Maths::C_AxisBox axisbox(
			Maths::HVec3(m_PrimBounds[i].vmin.x,m_PrimBounds[i].vmin.y,m_PrimBounds[i].vmin.z),
			Maths::HVec3(m_PrimBounds[i].vmax.x,m_PrimBounds[i].vmax.y,m_PrimBounds[i].vmax.z));
		bbox.UpdateExtents( axisbox );
	}
	m_Bounds = bbox;

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
	BuildTree(0, m_Bounds, m_PrimBounds, primNums, nPrims, iMaxDepth, edges, prims0, prims1);

	// Free working memory for kd-tree construction
	delete[] primNums;
	for (int i = 0; i < 3; ++i)
		delete[] edges[i];
	delete[] prims0;
	delete[] prims1;

	return *this;
}

// recursive tree builder
// this is called by the KdBuilder initialization code
void KdBuilder::BuildTree(int iNodeNum, const MathsStorage::Box3 &nodeBounds, Box3Vector &allPrimBounds, int *piPrimNums, int nPrims, int iDepth,
		BoundEdge *edges[3], int *prims0, int *prims1, int nBadRefines)
{
	// create a new node
	KdNode newNode;

	// Initialize leaf node if termination criteria met
	if (nPrims <= m_iMaxPrims || iDepth == 0)
	{
		m_Nodes[iNodeNum].InitLeaf(piPrimNums, nPrims, &m_Mailboxes);
		return;
	}

	// Initialize interior node and continue recursion
	// Choose split axis position for interior node
	int	iBestAxis = -1, iBestOffset = -1;
	float fBestCost = FLT_MAX;
	float fOldCost = m_iIsectCost * float(nPrims);
	MathsStorage::Float3 vd = Maths::C_AxisBox(&nodeBounds).Diagonal();
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
	for (int i = 0; i < nPrims; ++i)
	{
		int np = piPrimNums[i];
		const MathsStorage::Box3 &bbox = allPrimBounds[np];
		edges[iAxis][2*i] =	  BoundEdge(bbox.vmin.v[iAxis], np, true);
		edges[iAxis][2*i+1] = BoundEdge(bbox.vmax.v[iAxis], np, false);
	}
	qsort(&edges[iAxis][0], 2*nPrims, sizeof(BoundEdge), CompareFunc);

	// Compute cost of all splits for iAxis to find best
	int nBelow = 0, nAbove = nPrims;
	for (int i = 0; i < 2*nPrims; ++i)
	{
		if (edges[iAxis][i].type == BoundEdge::END) --nAbove;

		float fEdget = edges[iAxis][i].t;
		if (fEdget > nodeBounds.vmin.v[iAxis] && fEdget < nodeBounds.vmax.v[iAxis])
		{
			// Compute cost for split at _i_th edge
			int otherAxis[3][2] = { {1,2}, {0,2}, {0,1} };
			int otherAxis0 = otherAxis[iAxis][0];
			int otherAxis1 = otherAxis[iAxis][1];
			float fBelowSA = 2 * (vd.v[otherAxis0] * vd.v[otherAxis1] + (fEdget - nodeBounds.vmin.v[iAxis]) * (vd.v[otherAxis0] + vd.v[otherAxis1]));
			float fAboveSA = 2 * (vd.v[otherAxis0] * vd.v[otherAxis1] + (nodeBounds.vmax.v[iAxis] - fEdget) * (vd.v[otherAxis0] + vd.v[otherAxis1]));
			float fBelow = fBelowSA * iInvTotalSA;
			float fAbove = fAboveSA * iInvTotalSA;
			float feb = (nAbove == 0 || nBelow == 0) ? m_fEmptyBonus : 0.f;
			float fCost = m_iTraversalCost + m_iIsectCost * (1.f - feb) * (fBelow * nBelow + fAbove * nAbove);

			// Update best split if this is lowest cost so far
			if (fCost < fBestCost)
			{
				fBestCost = fCost;
				iBestAxis = iAxis;
				iBestOffset = i;
			}
		}
		if (edges[iAxis][i].type == BoundEdge::START) ++nBelow;
	}
	assert(nBelow == nPrims && nAbove == 0);

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
		m_Nodes[iNodeNum].InitLeaf(piPrimNums, nPrims, &m_Mailboxes);
		return;
	}

	// copy primitives into different lists depending on which side of the split they are
	int n0 = 0, n1 = 0;
	for (int i=0; i<iBestOffset; ++i)
		if (edges[iBestAxis][i].type == BoundEdge::START)
			prims0[n0++] = edges[iBestAxis][i].iPrimNum;
	for (int i=iBestOffset+1; i<2*nPrims; ++i)
		if (edges[iBestAxis][i].type == BoundEdge::END)
			prims1[n1++] = edges[iBestAxis][i].iPrimNum;

	// push a dummy-node on the list to make sure all child pairs are aligned
	// (after adding a leafnode)
	float fSplit = edges[iBestAxis][iBestOffset].t;

	// allocate space for left childnode
	assert(m_Nodes.Size() == iNodeNum+1);
	m_Nodes.PushBack(newNode);

	// initialize childnodes
	MathsStorage::Box3 bounds0 = nodeBounds, bounds1 = nodeBounds;
	bounds0.vmax.v[iBestAxis] = bounds1.vmin.v[iBestAxis] = fSplit;
	BuildTree(iNodeNum+1,   bounds0, allPrimBounds, prims0, n0, iDepth-1, edges, prims0, prims1 + nPrims, nBadRefines);

	// set the parameters for this node
	int iNewNode = m_Nodes.Size();
	m_Nodes.PushBack(newNode);
	m_Nodes[iNodeNum].InitSplit(iBestAxis, fSplit, iNewNode - (iNodeNum + 1));
	BuildTree(iNewNode, bounds1, allPrimBounds, prims1, n1, iDepth-1, edges, prims0, prims1 + nPrims, nBadRefines);
}

}
