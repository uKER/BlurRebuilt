#include "gravity/treebuilder/KDTree.h"

namespace Baking
{
	/*template <typename Serial>
	void KDTree::RecursiveSerialise(Serial& s, KDTreeNode* pNode)
	{
		//Write the bounding box...
		s << pNode->m_axis;
		s << pNode->m_dist;

		//Now write number of indices...
		if(pNode->m_leftNode)
		{
			//num indices = 0;
			s << 0;
			//Write offset to right node...
			s << pNode->m_leftNode->m_NumDescendants + 2;
			RecursiveSerialise(s, pNode->m_leftNode);
			RecursiveSerialise(s, pNode->m_rightNode);

		}
		else
		{
			//Leaf node
			//write number of triangles
			s << pNode->m_sizeIndice;
			//write start index for triangle
			s << pNode->m_startIndex;
		}
	}*/
};
