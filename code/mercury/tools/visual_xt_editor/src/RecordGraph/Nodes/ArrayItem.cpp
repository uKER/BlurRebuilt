#include "stdafx.h"

#include "ArrayItem.h"
#include "Array.h"

// for redraw
#include "../Graph.h"

#include "../../Common/GenerateId.h"

namespace RecordGraph
{

C_ArrayItemNode::C_ArrayItemNode(C_ArrayNode* pParentArray, S32 iIndex) :
	C_GenericNode(E_NodeType_ArrayItem),
	m_pParentArray(pParentArray),
	m_iIndex(iIndex)
{
	m_Colour = wxColour(192, 192, 192);
	m_Size = wxSize(8, 8);

	m_bDraggable = false;
	m_bDrawConnection = false;
}

C_ArrayItemNode::~C_ArrayItemNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

bool C_ArrayItemNode::OnPopulate()
{
	if (m_pParentArray->m_pLfArray && !BuildNode(this, m_pParentArray->m_pLfArray->field))
		return false;

	return C_GenericNode::OnPopulate();
}

void C_ArrayItemNode::OnBind(Xt::DomBlock* pParentDom)
{
	SetDom(AcquireArray(pParentDom, m_pParentArray->m_pLfArray->name));
}

void C_ArrayItemNode::OnBind(Xt::DomArray* pParentDom, int iIndex)
{
	SetDom(pParentDom);
}

bool C_ArrayItemNode::OnBuild()
{
	
	return true;
}

void C_ArrayItemNode::OnLayout(S32 index)
{
	wxSize parentSize = m_pParentArray->GetSize();

	if(m_pParentArray->GetLayout() == E_ArrayLayout_Horizontal)
	{
		m_Alignment = E_NodeAlignment_Vertical;
		m_Position.x = index * m_Size.GetWidth();
		m_Position.y = parentSize.GetHeight() - m_Size.GetHeight();
	}
	else
	{
		m_Alignment = E_NodeAlignment_Horizontal;
		m_Position.x = parentSize.GetWidth();
		m_Position.y = index * m_Size.GetHeight();
	}

	C_GenericNode::OnLayout(index);
}

void C_ArrayItemNode::SetDom(Xt::DomArray* pArray)
{
	Xt::Monitor(&m_DomMonitor, pArray);

	C_GenericNode::OnBind(pArray, m_iIndex);

	if (pArray)
		GenerateIdForBlock(Xt::AsBlock(At(pArray, m_iIndex)), m_pParentArray->m_pLfArray);
}

void C_ArrayItemNode::Close()
{
	m_pParentArray->Remove(m_iIndex);
	
}

bool C_ArrayItemNode::SetSelected(bool bSelected)
{
	bool select = false;
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		select = m_Children[i]->SetSelected(bSelected) || select;
	}

	return I_Node::SetSelected(bSelected) || select;
}

void C_ArrayItemNode::PrepareDC(wxDC& dc) const
{
	if(m_bSelected || m_pParent->GetSelected())
	{
		wxPen pen;
		pen.SetColour(255, 255, 0);
		pen.SetWidth(2);

		dc.SetPen(pen);
	}
	else
		dc.SetPen(*wxBLACK_PEN);

	dc.SetBrush(wxBrush(m_Colour));
}

} // RecordGraph