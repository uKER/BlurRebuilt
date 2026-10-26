#ifndef GENERICTREE_INL
#define GENERICTREE_INL

#include "gravity/treebuilder/GenericTree.h"
#include "gravity/treebuilder/AABBTree.h"


//using namespace SharedPhysics;

namespace Baking
{
template <typename Node, typename Triangle>
void GenericTree<Node, Triangle>::RemoveDuplicatedVertice()
{
	/*U32 size=m_vertice.m_numElements;
	U32* vertsIndex=new U32[size];
	SharedPhysics::SortedArray<Maths::HVec3> temp;
	

	for(S32 i=0; i<m_vertice.m_numElements; ++i)
	{
		vertsIndex[i] = temp.m_numElements;
		for(S32 j=0; j<temp.m_numElements; ++j)
		{
			if(m_vertice[i].All_EQ(temp[j]))
			{
				vertsIndex[i] = j;
				break;
			}
		}

		if(vertsIndex[i] == temp.m_numElements)
		{
			temp.AddToEnd(&m_vertice[i]);
		}
	}

	for(S32 i = 0; i < m_indice.m_numElements; ++i)
	{
		Triangle& tri = m_indice[i];
		tri.m_IndexA = vertsIndex[tri.m_IndexA];
		tri.m_IndexB = vertsIndex[tri.m_IndexB];
		tri.m_IndexC = vertsIndex[tri.m_IndexC];
		//m_indice.m_pElems[i] = vertsIndex[m_indice[i]];
		//m_indice.AddToEnd(&vertsIndex[m_indice[i]]);
	}

	delete [] vertsIndex;
	delete [] m_vertice.m_pElems;

	m_vertice.m_pElems=temp.m_pElems;
	m_vertice.m_numElements = temp.m_numElements;
	m_vertice.m_size = temp.m_size;
	temp.m_pElems = NULL;*/
}

};

#endif

