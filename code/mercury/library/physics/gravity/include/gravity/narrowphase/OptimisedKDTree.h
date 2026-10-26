#ifndef OPTIMISEDKDTREE_H
#define OPTIMISEDKDTREE_H

namespace SharedPhysics
{
	template <bool bFixedSize> class C_SafeStack;
};

namespace Collision
{
class C_Ray;
class KDTreeEdgeNode;
class KDTreeTriangle
{
public:
	U32 m_IndexA, m_IndexB, m_IndexC;
	U32 m_NormalIndex;
	U32 m_CollisionFlags;
	U16 m_MaterialIndex;

	template <typename Serial>
	void DeSerialise(Serial& s);	
};

class OptimisedKDNode
{
public:
	//AABBMinMax m_BoundingBox;
	U32 m_axis;
	F32 m_dist;

	union
	{
		U32 m_StartIndex;
		U32 m_RightChildOffset;
	};
	U32 m_NumIndices;
	template <typename Serial>
	void DeSerialise(Serial& s);
};

class OptimisedKDTree
{
public:
	MathsStorage::Float3* m_pVertices;
	MathsStorage::Float3* m_pNormals;
	KDTreeTriangle* m_pTriangles;
	OptimisedKDNode* m_pNodes;
	U32 m_NumVertices, m_NumTriangles, m_NumNodes, m_NumNormals;
	Maths::HVec3 axes[3];

	OptimisedKDTree() : m_pVertices(NULL), m_pNormals(NULL), m_pTriangles(NULL), m_pNodes(NULL), m_NumVertices(0),
		m_NumTriangles(0), m_NumNodes(0), m_NumNormals(0)
	{
		Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		axes[0] = Maths::HVec3(one, zero, zero);
		axes[1] = Maths::HVec3(zero, one, one);
		axes[2] = Maths::HVec3(zero, zero, one);
	}

	~OptimisedKDTree()
	{
		if(m_pVertices)
			delete [] m_pVertices;
		if(m_pNormals)
			delete [] m_pNormals;
		m_pVertices = NULL;
		if(m_pTriangles)
			delete [] m_pTriangles;
		m_pTriangles = NULL;
		if(m_pNodes)
			delete [] m_pNodes;
		m_pNodes = NULL;
	}

	NOALIAS __forceinline U32 Side(const Maths::HVec3 PCREF point, const OptimisedKDNode* const __restrict pNode) const
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Epsilon2());
		const Maths::HVec3 axis(axes[pNode->m_axis]);
		const Maths::HF32 dist(Maths::Load<Maths::HF32>(&pNode->m_dist));
		const Maths::HF32 val((axis | point) + dist);
		return (val > -zero) - (val < zero);
	}

	template <typename Serial>
	void DeSerialise(Serial& s);

	template<bool (*TriCollisions)(const MathsStorage::Float3* const __restrict verts, const MathsStorage::Float3* const __restrict normals, const Collision::KDTreeTriangle* const __restrict tris, const U32 numTris, Collision::C_Ray& ray)>
	bool RayTracing(Collision::C_Ray &ray, SharedPhysics::C_SafeStack<true>& stack);
};

};

#endif
