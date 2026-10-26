#ifndef BUILD_CLOTH_TREE_H
#define BUILD_CLOTH_TREE_H

#include "gravity/cloths/cloth.h"
#include "gravity/narrowphase/AxisBox.h"
#include "gravity/treebuilder/aabbtree.h"
#include "system/serialise/BinarySerialiseAutoEndian.h"

namespace Baking
{

	class C_RemapSoftBodyEdge
	{
	public:
		Collision::C_SoftBodyEdge m_Edge;
		U32 m_Index;
		U32 m_RemappedIndex;

		bool operator > (const C_RemapSoftBodyEdge& edge) const
		{
			return m_Edge > edge.m_Edge;
		}
		bool operator <= (const C_RemapSoftBodyEdge& edge) const
		{
			return m_Edge <= edge.m_Edge;
		}
		bool operator < (const C_RemapSoftBodyEdge& edge) const
		{
			return m_Edge < edge.m_Edge;
		}
		bool operator == (const C_RemapSoftBodyEdge& edge) const
		{
			return m_Edge == edge.m_Edge;
		}
		
		C_RemapSoftBodyEdge()
		{
			m_Index = m_RemappedIndex = 0;
		}
	};

	class C_BuildSoftBodyNode
	{
	public:
		C_BuildSoftBodyNode() : m_pLeftNode(NULL), m_pRightNode(NULL)
		{
		}

		~C_BuildSoftBodyNode()
		{
			if(m_pLeftNode)
			{
				delete m_pLeftNode;
				delete m_pRightNode;
			}
		}

		template <typename Serial>
		void RecursiveSerialise(Serial& s);

		U32 CalculateNumDescendants(U32 index);

		Collision::C_AxisBoxMinMax m_BoundingBox;
		U32 m_StartIndex;
		U32 m_NumTriangles;
		U32 m_StartVertexIndex;
		U32 m_NumVertices;
		U32 m_NumDescendants;
		S32 m_ParentIndex;
		C_BuildSoftBodyNode* m_pLeftNode;
		C_BuildSoftBodyNode* m_pRightNode;
	};

class C_SoftBodyTreeVertex
{
public:
	Collision::C_GravityParticle m_Vertex;
	U32 m_Index;
	U32 m_RemappedIndex;

	C_SoftBodyTreeVertex()
	{
	}

	bool operator < (const C_SoftBodyTreeVertex& vertex)
	{
		const Maths::Scalar_Bool xEqX(m_Vertex.m_CurPosition.X().EQ(vertex.m_Vertex.m_CurPosition.X()));
		const Maths::Scalar_Bool yEqY(m_Vertex.m_CurPosition.Y().EQ(vertex.m_Vertex.m_CurPosition.Y()));

		return m_Vertex.m_CurPosition.X().LT(vertex.m_Vertex.m_CurPosition.X()) | 
			(xEqX & m_Vertex.m_CurPosition.Y().LT(vertex.m_Vertex.m_CurPosition.Y())) | 
			(xEqX & yEqY & m_Vertex.m_CurPosition.Z().LT(vertex.m_Vertex.m_CurPosition.Z()));
	}
	
	bool operator <= (const C_SoftBodyTreeVertex& vertex)
	{
		const Maths::Scalar_Bool xEqX(m_Vertex.m_CurPosition.X().EQ(vertex.m_Vertex.m_CurPosition.X()));
		const Maths::Scalar_Bool yEqY(m_Vertex.m_CurPosition.Y().EQ(vertex.m_Vertex.m_CurPosition.Y()));

		return m_Vertex.m_CurPosition.X().LT(vertex.m_Vertex.m_CurPosition.X()) | 
			(xEqX & m_Vertex.m_CurPosition.Y().LT(vertex.m_Vertex.m_CurPosition.Y())) | 
			(xEqX & yEqY & m_Vertex.m_CurPosition.Z().LE(vertex.m_Vertex.m_CurPosition.Z()));
	}

	bool operator > (const C_SoftBodyTreeVertex& vertex)
	{
		const Maths::Scalar_Bool xEqX(m_Vertex.m_CurPosition.X().EQ(vertex.m_Vertex.m_CurPosition.X()));
		const Maths::Scalar_Bool yEqY(m_Vertex.m_CurPosition.Y().EQ(vertex.m_Vertex.m_CurPosition.Y()));

		return m_Vertex.m_CurPosition.X().GT(vertex.m_Vertex.m_CurPosition.X()) | 
			(xEqX & m_Vertex.m_CurPosition.Y().GT(vertex.m_Vertex.m_CurPosition.Y())) | 
			(xEqX & yEqY & m_Vertex.m_CurPosition.Z().GT(vertex.m_Vertex.m_CurPosition.Z()));
	}
	
	bool operator >= (const C_SoftBodyTreeVertex& vertex)
	{
		const Maths::Scalar_Bool xEqX(m_Vertex.m_CurPosition.X().EQ(vertex.m_Vertex.m_CurPosition.X()));
		const Maths::Scalar_Bool yEqY(m_Vertex.m_CurPosition.Y().EQ(vertex.m_Vertex.m_CurPosition.Y()));

		return m_Vertex.m_CurPosition.X().GT(vertex.m_Vertex.m_CurPosition.X()) | 
			(xEqX & m_Vertex.m_CurPosition.Y().GT(vertex.m_Vertex.m_CurPosition.Y())) | 
			(xEqX & yEqY & m_Vertex.m_CurPosition.Z().GE(vertex.m_Vertex.m_CurPosition.Z()));
	}

	bool operator == (const C_SoftBodyTreeVertex& vertex)
	{
		const Maths::HVec3 dif(m_Vertex.m_CurPosition - vertex.m_Vertex.m_CurPosition);
		const Maths::HF32 d2(dif | dif);
		return (d2 < Maths::Constants<Maths::HF32>::Epsilon()) != 0;
	}
};

	class C_BuildSoftBodyTree
	{
	public:
		SharedPhysics::SortedArray<Collision::C_SoftBodyTriangle> m_SoftBodyTriangles;
		SharedPhysics::SortedArray<Collision::C_SoftBodyEdge> m_SoftBodyEdges;
		SharedPhysics::SortedArray<Collision::C_GravityParticle> m_SoftBodyVertices;
		U32* m_pSoftBodyVertexRemapArray;
		C_BuildSoftBodyNode* m_pRootNode;
		U32 m_MinTriangles;
	
		C_BuildSoftBodyTree(U32 minTriangles = 1) : m_pRootNode(NULL), m_MinTriangles(minTriangles), m_pSoftBodyVertexRemapArray(NULL)
		{
		}

		~C_BuildSoftBodyTree()
		{
			if(m_pRootNode)
				delete m_pRootNode;
			delete [] m_pSoftBodyVertexRemapArray;
		}

		void AddTriangle(Maths::HVec3 PCREF vertA, Maths::HVec3 PCREF vertB, Maths::HVec3 PCREF vertC, 
			Maths::HF32 PCREF massA, Maths::HF32 PCREF massB, Maths::HF32 PCREF massC);
		void BuildTree();
		template <typename Serial>
		void Export(Serial& serialiser);
	private:
		void RemoveDuplicateVertices();
		void GenerateSupportEdges();
		void RemapTriangles();
		void RemoveDuplicateEdges();
		void CalculateNumDescendants();
		void CalculateParticleMasses();
		Maths::HVec3 AveragePoint(U32 triangleIndex);
		C_BuildSoftBodyNode* BuildNode( U32 startIndex, U32 numIndices, U32 vertexStartIndex, U32 numVertexIndices );
		C_BuildSoftBodyNode* Subdivide( U32 startIndex, U32 numIndices, U32 vertexStartIndex, U32 numVertexIndices );
		Collision::C_Plane PickSplitAxis( C_BuildSoftBodyNode* pNode );
		F32 PickBestEdgePoint(SharedPhysics::SortedArray<C_EdgeEvent>& edgeEvents, F32& score);
		void ExtractEdgeEvents(SharedPhysics::SortedArray<C_EdgeEvent>& edgeEvents, U32 axis, U32 startIndex, U32 endIndex);
	};

};

#endif
