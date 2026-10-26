
#include "stdafx.h"
#include "wx/image.h"

#include "NodeInterface.h"
#include "NodeGraph.h"
#include "Visitor.h"

// default node

I_Node::I_Node() :
	m_pGraph(0),
	m_pParent(0),
	m_iType(0),
	m_bVisible(true),
	m_bDrawConnection(true),
	m_bDrawChildren(true),
	m_bDraggable(true),
	m_bDragSrc(false),
	m_bDragDest(false),
	m_bDragRedraw(false),
	m_bSelected(false)
{
	Clear();
}

I_Node::I_Node(S32 type) :
	m_pGraph(0),
	m_pParent(0),
	m_iType(type),
	m_bVisible(true),
	m_bDrawConnection(true),
	m_bDrawChildren(true),
	m_bDraggable(true),
	m_bDragSrc(false),
	m_bDragDest(false),
	m_bDragRedraw(false),
	m_bSelected(false)
{
	Clear();
}

I_Node::~I_Node()
{
	if(m_pGraph)
		m_pGraph->Clean(this);
}

void I_Node::Clear()
{
    m_Position.x = 0;
    m_Position.y = 0;
    
	//m_show = true;
}


wxPoint I_Node::GetPositionAbsolute() const
{
	wxPoint point = m_Position;
	
	if(m_pParent)
		point += m_pParent->GetPositionAbsolute();

	return point;
}

wxPoint I_Node::GetPosition() const
{
	wxPoint point = m_Position;
	
	return point;
}

void I_Node::SetPositionAbsolute(const wxPoint& position)
{
	wxPoint dest = position;

	if(m_pParent)
		dest -= m_pParent->GetPositionAbsolute();

	SetPosition(dest);
}

I_Node* I_Node::HitTest(wxPoint pt, I_Node* pExclude)
{
	if(pExclude != this && GetRect().Contains(pt))
		return this;
	else
		return NULL;
}

void I_Node::GenerateBitmap()
{
	m_Bitmap = wxBitmap(m_Size.x, m_Size.y);

	wxMemoryDC dc;
	dc.SelectObject(m_Bitmap);

	dc.SetBackground(* wxRED_BRUSH);
	dc.Clear();
	dc.SetBackgroundMode(wxTRANSPARENT);
	
	dc.SetDeviceOrigin(-m_Position.x, -m_Position.y);

	bool bOldSelected = m_bSelected;

	// set drag source to true to generate correct bitmap
	m_bDragSrc = true;
	m_bSelected = true;
	m_bDrawChildren = false;
	Draw(dc);
	m_bDrawChildren = true;
	m_bSelected = bOldSelected;
	m_bDragSrc = false;

	wxPoint pos(0, 0);

	dc.SelectObject(wxNullBitmap);

	// Make the bitmap masked
	wxImage image = m_Bitmap.ConvertToImage();
	image.SetMaskColour(255, 0, 0);
	m_Bitmap = wxBitmap(image);

	//return bitmap;
}

// function returns true if it should redraw
bool I_Node::SetDragSrc(const bool bDragSrc)
{
	m_bDragSrc = bDragSrc;

	return true;
}

// function returns true if it should redraw
bool I_Node::SetDragDest(const bool bDragDest)
{
	m_bDragDest = bDragDest;

	return true;
}

// function returns true if it should redraw
bool I_Node::SetSelected(const bool bSelected)
{
	if(m_pParent)
		m_pParent->BringToFront(this);
	m_bSelected = bSelected;

	return true;
}

void I_Node::Close()
{
	// pass up hierarchy 
	if(m_pParent)
		m_pParent->Close();
}

void I_Node::Accept(I_NodeVisitor *pVisitor)
{
	pVisitor->Visit(this);
}
/*
// parent node

I_ParentNode::~I_ParentNode()
{
	ClearChildren();

	ClearEmbeddedChildren();
	
}

I_Node* I_ParentNode::HitTest(wxPoint pt, I_Node* pExclude)
{
	if(pExclude == this)
		return NULL;

	for(S32 i = 0; i < m_EmbeddedChildren.Size(); ++i)
	{
		I_Node* node = m_EmbeddedChildren[i]->HitTest(pt - m_Position, pExclude);
		
		if(node && node != pExclude)
			return node;
	}

	// go through children backwards since last child gets drawn last
	for(S32 i = m_Children.Size() - 1; i >= 0; --i)
	{
		I_Node* node = m_Children[i]->HitTest(pt - m_Position, pExclude);
		
		if(node && node != pExclude)
			return node;
	}
	
	if(GetRect().Contains(pt))
		return this;
	else
		return NULL;
}

void I_ParentNode::BringToFront(I_Node* pChild)
{
	S32 remove = -1;

	for(S32 i = 0; i < m_Children.Size(); ++i )
	{
		if(pChild == m_Children[i])
		{
			remove = i;
			break;
		}
	}

	if(remove != -1)
	{
		m_Children.EraseByIndex(remove);
		m_Children.PushBack(pChild);
	}
}

void I_ParentNode::AddChild(I_Node* pNode)
{
	pNode->m_pParent = this;
	pNode->m_pGraph = m_pGraph;
	m_Children.PushBack(pNode);
}

void I_ParentNode::ClearChildren()
{
	for(int i = 0; i < m_Children.Size(); ++i)
		delete m_Children[i];

	m_Children.Clear();
}

void I_ParentNode::AddEmbeddedChild(I_Node* pNode)
{
	pNode->m_pParent = this;
	pNode->m_pGraph = m_pGraph;
	m_EmbeddedChildren.PushBack(pNode);
}

void I_ParentNode::ClearEmbeddedChildren()
{
	for(int i = 0; i < m_EmbeddedChildren.Size(); ++i)
		delete m_EmbeddedChildren[i];

	m_EmbeddedChildren.Clear();
}

void I_ParentNode::ToggleHideChildren()
{
	m_bHideChildren = !m_bHideChildren;

	for(S32 i = 0; i < m_Children.Size(); ++i )
	{
		m_Children[i]->ToggleVisible();
	}
}
*/