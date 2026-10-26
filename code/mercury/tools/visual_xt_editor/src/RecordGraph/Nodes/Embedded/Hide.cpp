#include "stdafx.h"

#include "Hide.h"

#include "../Generic.h"
#include "../Type.h"

using namespace RecordGraph;

// event node
C_HideIcon::C_HideIcon() : I_Node(E_NodeType_HideIcon)
{
	m_Size = wxSize(8, 8);
	m_bDraggable = false;
}

bool C_HideIcon::MouseClick()
{
	if(m_pParent)
	{
		C_GenericNode* pParent = (C_GenericNode*)m_pParent;
		pParent->ToggleHideChildren();
		return true;
	}
	else
		return false;
}

bool C_HideIcon::SetSelected(const bool bSelected)
{
	return m_pParent->SetSelected(bSelected);
}

void C_HideIcon::Draw(wxDC& dc) const
{
	wxPen pen = *wxBLACK_PEN;
	pen.SetWidth(2);
	dc.SetPen(pen);

	C_GenericNode* pParent = (C_GenericNode*)m_pParent;
		
	if(pParent->GetNumChildren() > 0)
	{
		if(pParent->GetHideChildren())
		{
			wxPoint mid(m_Position.x + m_Size.x, m_Position.y + m_Size.y / 2);
			wxPoint end(m_Position.x, m_Position.x + m_Size.y);
			dc.DrawLine(m_Position, mid);
			dc.DrawLine(mid, end);
		}
		else
		{
			wxPoint mid(m_Position.x + m_Size.x / 2, m_Position.y + m_Size.y);
			wxPoint end(m_Position.x + m_Size.x, m_Position.x);
			dc.DrawLine(m_Position, mid);
			dc.DrawLine(mid, end);
		}
	}
}

