#ifndef __KDTREE_BUILDER_H__
#define __KDTREE_BUILDER_H__

//#include "spatial_hierarchy/Ray.h"
//#include "spatial_hierarchy/RayTraceResult.h"
//#include "spatial_hierarchy/Primitive.h"

struct KdNode;

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
			KdBuilder(Box3Vector &primBounds, int icost, int tcost, float ebonus, int maxp, int maxDepth);

	void	BuildTree(int iNodeNum, const Box3 &bounds, Box3Vector &primBounds, int *piPrimNums, int nPrims, int iDepth,
		BoundEdge *edges[3], int *prims0, int *prims1, int badRefines = 0);

	NodeVector		m_Nodes;
	MailboxVector	*m_pMailboxes;

	Box3			m_Bounds;

	int				m_iIsectCost;
	int				m_iTraversalCost;
	int				m_iMaxPrims;
	float			m_fEmptyBonus;
};

#endif
