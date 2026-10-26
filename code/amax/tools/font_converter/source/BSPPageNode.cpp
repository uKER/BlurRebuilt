
#include "stdafx.h"
#include "BSPPageNode.h"

namespace Font
{


C_BSPPageNode::C_BSPPageNode()
{
	memset(this,0,sizeof(*this));
	m_Full = false;
}

C_BSPPageNode::~C_BSPPageNode()
{
	delete m_Child[0];
	delete m_Child[1];
	memset(this,0,sizeof(*this));
}

// Try and insert a rectangle of rectSize 'rectSize' into the C_PageNode
int C_BSPPageNode::Insert(struct Rect *result, struct Rect *rectSize)
{
	U8 nReturn = 0, i = 0;
	Rect rectCurr = {0,0,0,0};
	int spareWidth = 0,spareHeight = 0;

	if(m_Full)
		return 1;	///< Page Node is Full

	if(m_Child[0] && m_Child[1])
	{
		// Try inserting into m_Child nodes
		for(i = 0; i < 2; i++)
		{
			nReturn = m_Child[i]->Insert(result, rectSize);
			if(nReturn == 0)
				return 0;	///< Success
		}
		return 1;			///< Failed - no more room
	}

	// We are a leaf node with space, find out how much space Rect node has
	rectCurr.m_UMax = m_Rect.m_UMax - m_Rect.m_UMin;
	rectCurr.m_VMax = m_Rect.m_VMax - m_Rect.m_VMin;

	// See if we can fit the rectangle:
	if(rectSize->m_UMax > rectCurr.m_UMax)
		return 1; // No, too wide
	if(rectSize->m_VMax > rectCurr.m_VMax)
		return 1; // No, too high

	if(rectSize->m_UMax == rectCurr.m_UMax && rectSize->m_VMax == rectCurr.m_VMax)
	{
		// Fits exactly
		*result = m_Rect;	///< Return the rectangle of rectCurr node
		m_Full = 1;			///< Mark rectCurr node as full now
		return 0;
	}

	// Else Subdivide it
	for(i = 0; i < 2; i++)
	{
		m_Child[i] = new C_BSPPageNode();
		if(m_Child[i] == 0)
			return 1;
		m_Child[i]->m_Rect = m_Rect;
	}

	spareWidth = rectCurr.m_UMax - rectSize->m_UMax;	///< Spare width on the m_UMax
	spareHeight = rectCurr.m_VMax - rectSize->m_VMax;	///< Spare height on the m_VMin

	if(spareWidth > spareHeight)
	{
		// Split like this |
		m_Child[0]->m_Rect.m_UMax = m_Child[1]->m_Rect.m_UMin = m_Rect.m_UMin + rectSize->m_UMax;
	}
	else
	{
		// Split like this -
		m_Child[0]->m_Rect.m_VMax = m_Child[1]->m_Rect.m_VMin = m_Rect.m_VMin + rectSize->m_VMax;
	}

	nReturn = m_Child[0]->Insert(result, rectSize);		///< Insert into first Child
	return nReturn;
}


}; // namespace FontLib
