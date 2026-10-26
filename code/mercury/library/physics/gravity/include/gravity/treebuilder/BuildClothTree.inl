#ifndef BUILD_CLOTH_TREE_INL
#define BUILD_CLOTH_TREE_INL

#include "gravity/treebuilder/BuildClothTree.h"

namespace Baking
{

template <typename Serial>
void C_BuildSoftBodyTree::Export(Serial& serialiser)
{
	//Write out the cloth tree!
	//Now we need to construct and export the tree!
	CalculateNumDescendants();

	//Number vertices, Number edges, Number triangles, Number nodes!
	serialiser << m_SoftBodyVertices.m_numElements << m_SoftBodyEdges.m_numElements << m_SoftBodyTriangles.m_numElements
		<<(m_pRootNode->m_NumDescendants + 1);

	//Loop and output cloth vertices, edges and triangles!
	for(S32 a = 0; a < m_SoftBodyVertices.m_numElements; ++a)
	{
		serialiser << m_SoftBodyVertices[a];
	}

	for(S32 a = 0; a < m_SoftBodyEdges.m_numElements; ++a)
	{
		serialiser << m_SoftBodyEdges[a];
	}

	for(S32 a = 0; a < m_SoftBodyTriangles.m_numElements; ++a)
	{
		serialiser << m_SoftBodyTriangles[a];
	}

	

	//Now we know how many items there are, we need to export out the right data!
	this->m_pRootNode->RecursiveSerialise(serialiser);
}

template <typename Serial>
void C_BuildSoftBodyNode::RecursiveSerialise(Serial& s)
{
	//Write the bounding box...
	s << m_BoundingBox;

	//Now write number of indices...
	if(m_pLeftNode)
	{
		//num indices = 0;
		s << 0;
		//Write offset to right node...
		s << m_pLeftNode->m_NumDescendants + 2;
		s << m_ParentIndex;
		s << 0;
		s << 0;
		//RecursiveSerialise(s, pNode->m_leftNode);
		//RecursiveSerialise(s, pNode->m_rightNode);
		m_pLeftNode->RecursiveSerialise(s);
		m_pRightNode->RecursiveSerialise(s);

	}
	else
	{
		//Leaf node
		//write number of triangles
		s << m_NumTriangles;
		//write start index for triangle
		s << m_StartIndex;
		s << m_ParentIndex;
		s << m_StartVertexIndex;
		s << m_NumVertices;
	}
}
}

#endif
