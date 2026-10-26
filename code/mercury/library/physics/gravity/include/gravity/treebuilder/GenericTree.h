#ifndef GENERICTREE_H
#define GENERICTREE_H

#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/narrowphase/Primitive.h"


namespace Baking
{

#define MAX_DEPTH		22 
#define	MIN_LEAF_SIZE	12

enum SPLITSTATUS
{
	POINT_IN_FRONT_PLANE,
	POINT_IN_BACK_PLANE,
	POINT_STRADDLE_PLANE
};

template <typename Node>
class GenericTreeNode
{
public:

	GenericTreeNode(U32 TriangleStartIndex, U32 size):
												m_TriangleStartIndex(TriangleStartIndex), m_TriangleSizeIndice(size),
												m_VerticeStartIndex(0), m_leftNode(NULL), m_rightNode(NULL)
	{
	}
	
	virtual ~GenericTreeNode()
	{
		if(m_leftNode)
			delete m_leftNode;
		if(m_rightNode)
			delete m_rightNode;
	}

	U32 m_TriangleStartIndex;
	U32 m_TriangleSizeIndice;

	U32 m_VerticeStartIndex;
	U32 m_NumVertices;
	Node* m_leftNode, *m_rightNode;
};

template <typename Node, typename Triangle>
class GenericTree
{
public:

	GenericTree():
				m_pRoot(NULL){}

	virtual ~GenericTree()
	{
#if !CFG_CONVERTER
		BIZ_ASSERT(m_pRoot == NULL);
#endif

		if(m_pRoot)
			delete m_pRoot;
	}
	

	NOALIAS __forceinline void AddVertices(Maths::HVec3* __restrict vertices, const U32 numVertices) __restrict
	{
		for(U32 a= 0; a < numVertices; ++a)
			m_vertice.AddToEnd(vertices+a);
	}

	NOALIAS __forceinline void AddVertex(Maths::HVec3 PCREF vertex) __restrict
	{
		m_vertice.AddToEnd(&vertex);
	}

	U32 GetNumVertices()
	{
		return m_vertice.m_numElements;
	}

	Node* GetRoot()
	{
		return m_pRoot;
	}

	SharedPhysics::SortedArray<Maths::HVec3>& GetVertice()
	{
		return m_vertice;
	}

	SharedPhysics::SortedArray<Maths::HVec3>& GetNormals()
	{
		return m_Normals;
	}
	
	void RemoveDuplicatedVertice();


	
protected:

	SharedPhysics::SortedArray<Maths::HVec3> m_vertice;
	SharedPhysics::SortedArray<Maths::HVec3> m_Normals;
	//SharedPhysics::SortedArray<Triangle> m_indice;
	Node* m_pRoot;
	

};

};



#endif
