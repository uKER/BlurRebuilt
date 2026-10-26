#ifndef AABBTREE_INL
#define AABBTREE_INL

#include "gravity/treebuilder/AABBTree.h"
//#include "gravity/narrowphase/OptimisedAABBTree.inl"

namespace Baking
{
template <typename Serial>
void AABBTree::RecursiveSerialise(Serial& s, AABBTreeNode* pNode)
{
	//Write the bounding box...
	Collision::C_AxisBoxMinMax& aabb = pNode->aabb;
	Collision::C_QuantizedAxisBoxMinMax quantAABB(aabb, this->m_pRoot->aabb.m_min, m_pRoot->aabb.m_max);
	//s << aabb.m_min;
	//s << aabb.m_max;
	quantAABB.Serialise(s);

	//Now write number of indices...
	if(pNode->m_leftNode)
	{
		//num indices = 0;
		s << 0;
		//Write offset to right node...
		s << pNode->m_leftNode->m_NumDescendants + 2;
		s << 0 << 0;
		RecursiveSerialise(s, pNode->m_leftNode);
		RecursiveSerialise(s, pNode->m_rightNode);

	}
	else
	{
		//Leaf node
		//write number of triangles
		s << pNode->m_TriangleSizeIndice;
		//write start index for triangle
		s << pNode->m_TriangleStartIndex;
		//write start index for vertices
		s << pNode->m_VerticeStartIndex;
		s << pNode->m_NumVertices;
	}
}


template <typename Serial>
void AABBTree::Serialise(Serial& s)
{
	//Write this to the serial object...
	//s << (U32)Collision::E_TriangleMesh_Shape;
	using namespace Maths;

	s << m_vertice.m_numElements;
	s << m_Normals.m_numElements;
	//s << m_indice.m_numElements;
	s << m_StreamSize;
	//Count the number of nodes...
	const U32 numNodes = CountNumNodes(m_pRoot);
	s << numNodes;

	//Write range here!
	if(m_pRoot)
	{
		s << m_pRoot->aabb.m_min << m_pRoot->aabb.m_max;
	}
	else
	{
		s << Maths::Constants<Maths::HVec3>::Zero() << Maths::Constants<Maths::HVec3>::Zero();
	}

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
		MathsStorage::HenDN3 f;
		Store(n, &f);
		s << (U32&)f;
	}

	//AABB Tree nodes - this is going to be a recursive call for simplicity's sake
	if(m_pRoot)
	{
		CaclulateNumChildrenNodes();
		RecursiveSerialise(s, m_pRoot);

		//Now store all the triangle data...
		/*for(S32 a = 0; a < m_indice.m_numElements; ++a)
		{
			m_indice[a].Serialise(s);
		}*/
		//Iterate over nodes and write out at the end...
		SharedPhysics::C_SafeStack<true> stack(16000);
		U32 size = 1;
		stack.Push(m_pRoot, 0);
		while(size--)
		{
			AABBTreeNode* node(stack.Pop<AABBTreeNode>(size));
			if(node->m_leftNode)
			{
				stack.Push(node->m_rightNode, size++);
				stack.Push(node->m_leftNode, size++);
				continue;
			}

			//Root node...
			U32 offset = node->m_TriangleStartIndex;
			Collision::C_TriangleHeader* header = m_Stream.GetHeader(offset);
			header->Serialise(s);
			offset += sizeof(Collision::C_TriangleHeader);
			U32 headerIterator = 0;
			for(U32 a = 0; a < node->m_TriangleSizeIndice; ++a, ++headerIterator)
			{
				if(header->m_NumTriangles <= headerIterator)
				{
					header = m_Stream.GetHeader(offset);
					header->Serialise(s);
					offset += sizeof(Collision::C_TriangleHeader);
					headerIterator = 0;
				}
				Collision::AABBTreeTriangle* tri(m_Stream.GetTriangles(offset));
				tri->Serialise(s);
				//assert(this->MatchNormal(*tri));
				offset += sizeof(Collision::AABBTreeTriangle);
			}
		}
	}

	//Last bit - encode additional triangle edge data...

	s << this->m_TriangleEdgeNormals.m_numElements;
	for(S32 a = 0; a < m_TriangleEdgeNormals.m_numElements; ++a)
	{
		m_TriangleEdgeNormals[a].Serialise(s);
	}
	//m_Stream.Serialise(s, m_StreamSize);

	

}

};

#endif
