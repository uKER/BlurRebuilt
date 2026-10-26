#include <StdAfx.h>

#include <spatial_hierarchy/HierachyNode.h>
#include <spatial_hierarchy/HierachyNodeIterator.h>

using namespace SD;

HierachyNodeIterator::HierachyNodeIterator()
{
	Reset();
}

HierachyNodeIterator::HierachyNodeIterator(HierachyNode* pHierachyNode)
{
	m_pHierarchyNode = pHierachyNode;
}

HierachyNodeIterator::HierachyNodeIterator(HierachyNodeIterator& rhs)
{
	*this = rhs;
}

HierachyNodeIterator::~HierachyNodeIterator()
{
	Clear();
}

const HierachyNodeIterator& HierachyNodeIterator::operator=(const HierachyNodeIterator& rhs)
{
	m_pHierarchyNode = rhs.m_pHierarchyNode;

	return *this;
}

HierachyNodeIterator::operator HierachyNode*()
{
	return m_pHierarchyNode;
}

HierachyNode *HierachyNodeIterator::operator++()
{
	return (*this)++;
}

HierachyNode *HierachyNodeIterator::operator++(int dummy)
{/*
	if (m_pHierarchyNode != 0)
	{
		GTL::TreeNodeIterator<HierachyNode*> it(m_pHierarchyNode);
		pTreeNode = it++;
		if (pTreeNode != 0)
		{
			return pTreeNode->GetContent();
		}
		else
		{
			return 0;
		}
	} 
		else
		{
			// There are no more nodes to iterate over
		m_pHierarchyNode = 0;
		}
	}*/
	return m_pHierarchyNode;
}

void HierachyNodeIterator::Clear()
{
	Reset();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
void HierachyNodeIterator::Reset()
{
	m_pHierarchyNode = 0;
}

#endif
