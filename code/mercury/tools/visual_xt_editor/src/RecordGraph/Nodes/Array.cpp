#include "stdafx.h"

#include "Array.h"
#include "ArrayItem.h"
#include "Embedded/Hide.h"
#include "Embedded/Add.h"

// for redraw
#include "../Graph.h"

namespace RecordGraph
{

C_ArrayNode::C_ArrayNode(LfArray* pArray) :
	C_GenericNode(E_NodeType_Array),
	m_pLfArray(pArray),
	m_Layout(E_ArrayLayout_Vertical),
	m_Attachment(E_ArrayAttachment_Fixed),
	m_ItemConnections(E_ArrayItemConnections_Disabled),
	m_Binding(false)
{
	m_Colour = wxColour(160, 160, 160);
	m_Size = wxSize(64, 32);
	m_bDragRedraw = false;

	C_HideIcon* pHideIcon = new C_HideIcon();
	pHideIcon->SetParent(this);
	m_GenericChildren.PushBack(pHideIcon);

	C_AddIcon* pAddIcon = new C_AddIcon();
	pAddIcon->SetParent(this);
	m_GenericChildren.PushBack(pAddIcon);

	m_DomMonitor.SetCallback(DomCallback, (C_ArrayNode*)this);
}

C_ArrayNode::~C_ArrayNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

void C_ArrayNode::OnLayout(S32 index)
{
	S32 size = m_Children.Size() * 8;
	if(size < 32)
		size = 32;

	if(m_Layout == E_ArrayLayout_Horizontal)
		m_Size = wxSize(size, 32);
	else
		m_Size = wxSize(32, size);

	if(m_Attachment == E_ArrayAttachment_Floating)
	{
		m_bDraggable = true;
		m_bDrawConnection = true;
	}
	else
	{
		wxSize parentSize = GetParent()->GetSize();
		SetPosition(wxPoint(parentSize.GetWidth() - m_Size.GetWidth(), parentSize.GetHeight()));
		m_bDraggable = false;
		m_bDrawConnection = false;
	}

	for(S32 i = 0; i < m_GenericChildren.Size(); ++i)
	{
		I_Node* pNode = m_GenericChildren[i];
		switch(pNode->GetType())
		{
		case E_NodeType_HideIcon:
			pNode->SetPosition(wxPoint(4, 4));
			break;
		case E_NodeType_AddIcon:
			pNode->SetPosition(wxPoint(m_Size.GetWidth() - pNode->GetSize().GetWidth() - 4, 4));
			break;
		}
	}

	C_GenericNode::OnLayout(index);
}

bool C_ArrayNode::OnPopulate()
{
	return C_GenericNode::OnPopulate();
}

void C_ArrayNode::OnBind(Xt::DomBlock* pParentDom)
{
	SetDom(AcquireArray(pParentDom, m_pLfArray->name));
}

void C_ArrayNode::OnBind(Xt::DomArray* pParentDom, S32 iIndex)
{
	SetDom(AcquireArray(pParentDom, iIndex));
}

void C_ArrayNode::SetDom(Xt::DomArray* pArray)
{
	Xt::Monitor(&m_DomMonitor, pArray);

	RefreshControls();
	PopulateBody();
}

void C_ArrayNode::PopulateBody()
{
	Xt::DomArray* pArray = Xt::AsArray(m_DomMonitor);

	ClearChildren();

	if(pArray)
	{
		for(size_t i = 0; i < Xt::Count(pArray); i++)
			AddChild(new C_ArrayItemNode(this, (S32)i));

		Populate();
	}

	m_Binding = true;

	for(S32 i = 0; i < m_Children.Size(); ++i)
		GetChild(i)->Bind(pArray, 0);

	m_Binding = false;
}

void C_ArrayNode::ToggleHideChildren()
{
	// for an array node we still want our children (ArrayItemNodes) to be visible, but their children should be hidden
	m_bHideChildren = !m_bHideChildren;

	for(S32 i = 0; i < m_Children.Size(); ++i )
	{
		GetChild(i)->ToggleHideChildren();
	}
}

void C_ArrayNode::Add()
{
	Xt::DomArray* pArray = AsArray(m_DomMonitor);

	if(pArray)
	{
		Xt::Append(pArray, new Xt::DomBlock);
		SetDefaults(m_pLfArray, pArray, Xt::Count(pArray) - 1);

		PopulateBody();
		GetGraph()->LayoutNodes();
	}
}

void C_ArrayNode::Remove(S32 index)
{
	Xt::DomArray* pArray = AsArray(m_DomMonitor);

	if(pArray)
	{
		Xt::Remove(pArray, index);

		PopulateBody();
		GetGraph()->LayoutNodes();
	}
}

void C_ArrayNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_ArrayNode* pNode)
{
	if (*pMonitor == pEvent->from && !pNode->m_Binding)
	{
		pNode->PopulateBody();
		pNode->GetGraph()->LayoutNodes();
		pNode->GetGraph()->Refresh();
	}
}

// store in parents dom block
void C_ArrayNode::RefreshControls()
{
	Xt::DomArray* pArray = Xt::AsArray(m_DomMonitor);

	if(Xt::DomBlock* pBlock = Xt::AsBlock(pArray->parent))
	{
		wxPoint point = GetPosition();

		if(Xt::DomField* pField = Xt::AcquireField(pBlock, "__x" + m_pLfArray->name))
			point.x = atoi((const char*)pField->value);
		if(Xt::DomField* pField = Xt::AcquireField(pBlock, "__y" + m_pLfArray->name))
			point.y = atoi((const char*)pField->value);

		if(point.x != 0 && point.y != 0)
			SetPosition(point);
	}
}

bool C_ArrayNode::EndDrag(I_Node* pTarget, wxPoint point)
{
	SetPositionAbsolute(point);

	wxPoint relative = GetPosition();

	Xt::DomArray* pArray = Xt::AsArray(m_DomMonitor);

	if(Xt::DomBlock* pBlock = Xt::AsBlock(pArray->parent))
	{
		Xt::DomField* pX = Xt::AcquireField(pBlock, "__x" + m_pLfArray->name);
		Xt::DomField* pY = Xt::AcquireField(pBlock, "__y" + m_pLfArray->name);

		XtString xValue;
		XtsFormat(xValue, 128, "%i", relative.x);
		Xt::Set(pX, xValue);

		XtString yValue;
		XtsFormat(yValue, 128, "%i", relative.y);
		Xt::Set(pY, yValue);
	}
	
	return false;
}

} // RecordGraph