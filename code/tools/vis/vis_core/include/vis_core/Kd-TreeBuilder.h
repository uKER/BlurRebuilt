#pragma once

#include "kd-node.h"

typedef Ut::UtVector< Ut::DynamicVector<int> > IntVector;
typedef Ut::UtVector< Ut::DynamicVector<KdNode> > NodeVector;
typedef Ut::UtVector< Ut::DynamicVector<box3f> > Box3fVector;

// this structure stores a possible split position
struct BoundEdge
{
	BoundEdge() {};
	BoundEdge(float tt, int pn, bool starting);
	bool operator<(const BoundEdge &e) const;

	float t;
	int iPrimNum;
	enum { START, END } type;
};

// the kd-tree builder class
class KdBuilder
{
public:
			KdBuilder(Box3fVector &primBounds, int icost, int tcost, int maxp, int maxDepth);

	void	BuildTree(int iNodeNum, const box3f &bounds, Box3fVector &primBounds, int *piPrimNums, int nPrims, int iDepth,
		BoundEdge *edges[3], int *prims0, int *prims1, int badRefines = 0);

	void	ConvertToCompactTree();
	void	ConvertRecursive(int iInNode, int iOutNode, int iOutSubNode);

	NodeVector		m_Nodes;
	MailboxVector	*m_pMailboxes;

	box3f			m_Bounds;

	int				m_iIsectCost;
	int				m_iTraversalCost;
	int				m_iMaxPrims;
	int				m_iCurMailboxId;
	int				m_nMailboxes;
	int				m_iMaxDepth;

	int				m_iCurNode2;
	int				m_iCurCompactNode;
};
