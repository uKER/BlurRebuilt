#pragma once

#include "kd-node.h"

typedef Ut::Vector<int> IntVector;
typedef Ut::Vector<KdNode> NodeVector;
typedef Ut::Vector<box3f> Box3fVector;

// the kd-tree builder class
class AABBBuilder
{
public:
			AABBBuilder(TChartList &primBounds, int icost, int tcost, int maxp, int maxDepth);

	void	BuildTree(int iNodeNum, const box3f &bounds, TChartList &primBounds, int *piPrimNums, int nPrims, int iDepth,
		BoundEdge *edges[3], int *prims0, int *prims1, int badRefines = 0);

	void	ConvertToCompactTree();
	void	ConvertRecursive(int iInNode, int iOutNode, int iOutSubNode);
	int		AreaCostHeuristic(int iArea, int iBiggestArea);

	NodeVector		m_Nodes;
	MailboxVector	*m_pMailboxes;

	box3f			m_Bounds;

	int				m_iIsectCost;
	int				m_iTraversalCost;
	int				m_iMaxPrims;
	int				m_iCurMailboxId;
	int				m_nMailboxes;
	int				m_iMaxDepth;
	int				m_nLeafNodes;

	int				m_iCurNode2;
	int				m_iCurCompactNode;
};
