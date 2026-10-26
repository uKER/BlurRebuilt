#ifndef __MISCSHARED_KDTREEBUILDER_H__
#define __MISCSHARED_KDTREEBUILDER_H__

#include "misc_shared/KdTree/KdNode.h"

namespace Util
{

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
class KdBuilder : public KdTreeData
{
public:
	/// add a primitive to the tree. returns the index of the primitive
	int AddPrimitive(const Maths::C_AxisBox &bbox);

	/// add a primitive to the tree. returns the index of the primitive
	int AddPrimitive(const Maths::HVec3 &point);

	/// set heuristic parameters
	void SetParameters(	int icost = 1,			// intersect cost
						int tcost = 10,			// traversal cost
						float ebonus = 5.5f,	// empty bonus
						int maxp = 4,			// min primitives per leafnode
						int iMaxDepth = -1);	// max depth

	/// get reference to primbounds vector
	Box3Vector &GetPrimBounds();

	/// build the tree. returns a new tree data structure
	KdTreeData Build();

private:
	void	BuildTree(int iNodeNum, const MathsStorage::Box3 &bounds, Box3Vector &primBounds, int *piPrimNums, int nPrims, int iDepth,
		BoundEdge *edges[3], int *prims0, int *prims1, int badRefines = 0);

	MathsStorage::Box3	m_Bounds;
	Box3Vector		m_PrimBounds;

	int				m_iIsectCost;
	int				m_iTraversalCost;
	int				m_iMaxPrims;
	float			m_fEmptyBonus;
	int				m_iMaxDepth;
};

}

#endif

