#ifndef NEWKDTREE_H
#define NEWKDTREE_H

#include "gravity/treebuilder/GenericTree.h"
#include "gravity/narrowphase/OptimisedKDTree.h"

namespace Baking
{

/*enum Axis
{
	X	=	0,
	Y,
	Z,
	FORCE_DWORD = 1<<31
};

enum Status
{
	START	=	0,
	END
};

class KDTreeTriangle
{
public:
	U32 m_IndexA, m_IndexB, m_IndexC;
	U32 m_NormalIndex;
	U32 m_CollisionFlags;
	U16 m_MaterialIndex;

	template <typename Serial>
	void Serialise(Serial& s)
	{
		s << m_IndexA << m_IndexB << m_IndexC << m_NormalIndex << m_CollisionFlags << m_MaterialIndex;
	}

	void SetWorldBound(SharedPhysics::SortedArray<Maths::HVec3>& vertice)
	{
		const Maths::HVec3 a(vertice[m_IndexA]);
		const Maths::HVec3 b(vertice[m_IndexB]);
		const Maths::HVec3 c(vertice[m_IndexC]);
		m_min=Maths::Min(c, Min(a, b));
		m_max=Maths::Max(c, Max(a, b));
		Store(m_min, &m_minf);
		Store(m_max, &m_maxf);
	}

	MathsStorage::Float3 m_minf;
	MathsStorage::Float3 m_maxf;
	Maths::HVec3 m_min, m_max;
	
};

//This node is constructed from the bounding box of a triangle, and m_dist indicates
//the signed distance of a plane to the origin
class KDTreeEdgeNode
{
public:
	F32 m_dist;		//the position along an axis
	U32 m_index;	//which triangle this plane belong to
	Status	m_type;	//indicate this plane is the start or end plane of a bounding box

	bool operator > (KDTreeEdgeNode& other)
	{
		return m_dist > other.m_dist;
	}

	bool operator < (KDTreeEdgeNode& other)
	{
		return m_dist < other.m_dist;
	}

	bool operator == (KDTreeEdgeNode& other)
	{
		return m_dist == other.m_dist;
	}

	bool operator <= (KDTreeEdgeNode& other)
	{
		return m_dist <= other.m_dist;
	}

	bool operator >= (KDTreeEdgeNode& other)
	{
		return m_dist >= other.m_dist;
	}
};

class KDTreeNode: public GenericTreeNode<KDTreeNode>
{
public:
	
	KDTreeNode(U32 startIndex, U32 size):
										GenericTreeNode<KDTreeNode>(startIndex, size),
										m_NumDescendants(0)
	{
	}

	U32 m_axis;
	F32 m_dist;
	Maths::HVec3 min;
	Maths::HVec3 max;

	//Counts the number of descendant nodes this node has...
	U32 m_NumDescendants;

	friend class KDTree;
};


class KDTree : public GenericTree<KDTreeNode, KDTreeTriangle>
{
public:
	
	KDTree(F32 intersectCost = 80.f, F32 traversalCost =1.f, F32 eBonus = 0.5f, F32 splitRatio = 0.025f, U32 maxDepth = MAX_DEPTH): 
																			GenericTree<KDTreeNode, KDTreeTriangle>(), 
																			m_intersectCost(intersectCost), 
																			m_traversalCost(traversalCost),
																			m_eBonus(eBonus),
																			m_splitRatio(splitRatio),
																			m_maxDepth(maxDepth)
																			
	{
		m_otherAxis[0][0]=1; m_otherAxis[0][1]=2;
		m_otherAxis[1][0]=0; m_otherAxis[1][1]=2;
		m_otherAxis[2][0]=1; m_otherAxis[2][1]=0;
	}
													
	F32 PickEdgeNode(const SharedPhysics::SortedArray<KDTreeEdgeNode>& edges, const U32 axis, const F32 recipSA, const MathsStorage::Float3& d, const MathsStorage::Float3& min, const MathsStorage::Float3& max, F32& bestDist);
	KDTreeNode* CreateKDTreeNode(const U32 startIndex, const U32 numTriangle, const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF max);
	U32 GetSplitStatus(const U32 index, const U32 axis, const F32 dist, const Maths::HF32 PCREF threshold);
	U32 SplitTriangle(U32& index, const U32 axis, const F32 dist, const U32 startIndex, const U32 endIndex);

	void CreateEdgeNode(KDTreeTriangle& tri, U32 axis, U32 index, SharedPhysics::SortedArray<KDTreeEdgeNode>& edges);

	void SetCost(const F32 intersectCost, const F32 traversalCost)
	{
		this->m_intersectCost=intersectCost;
		this->m_traversalCost=traversalCost;
	}

	void SetBonus(const F32 eBonus)
	{
		m_eBonus=eBonus;
	}

	void setSplitRatio(F32 splitRatio)
	{
		m_splitRatio=splitRatio;
	}

	void setMaxDepth(U32 maxDepth)
	{
		m_maxDepth=maxDepth;
	}

	void BuildKDTree();

	template <typename Serial>
	void Serialise(Serial& s)
	{
		using namespace Maths;
		//Write this to the serial object...
		s << m_vertice.m_numElements;
		s << m_Normals.m_numElements;
		s << m_indice.m_numElements;
		//Count the number of nodes...
		const U32 numNodes = CountNumNodes(m_pRoot);
		s << numNodes;

		//Now write out the vertices (as Float3's)

		for(S32 a = 0; a < m_vertice.m_numElements; ++a)
		{
			const HVec3 v(m_vertice[a]);
			MathsStorage::Float3 f;
			Store(v, &f);
			s << f;
		}

		for(S32 a = 0; a < m_Normals.m_numElements; ++a)
		{
			const HVec3 n(m_Normals[a]);
			MathsStorage::Float3 f;
			Store(n, &f);
			s << f;
		}

		//Now store all the triangle data...
		for(S32 a = 0; a < m_indice.m_numElements; ++a)
		{
			m_indice[a].Serialise(s);
		}

		//AABB Tree nodes - this is going to be a recursive call for simplicity's sake
		CaclulateNumChildrenNodes();
		RecursiveSerialise(s, m_pRoot);
	}

private:

	void CaclulateNumChildrenNodes();

	U32 CountNumNodes(KDTreeNode* pNode);

	template <typename Serial>
	void RecursiveSerialise(Serial& s, KDTreeNode* pNode);

	KDTreeNode* BuildKDTree(U32 startIndex, U32 size, U32 depth);
	F32 m_intersectCost;
	F32 m_traversalCost;
	F32 m_eBonus;
	F32 m_splitRatio;
	U32 m_maxDepth;
	U32 m_otherAxis[3][2];
	SharedPhysics::SortedArray<KDTreeEdgeNode> edgesX;
	SharedPhysics::SortedArray<KDTreeEdgeNode> edgesY;
	SharedPhysics::SortedArray<KDTreeEdgeNode> edgesZ;
};
*/

};

#endif
